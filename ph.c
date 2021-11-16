/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 *
 */
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "config.h"
#include "debug.h"
#include "dlist.h"
#include "http.h"
#include "messages.h"
#include "server.h"

int main(int argc, char *argv[]) {
    int len, rc;
    int listen_sd = -1, new_sd = -1;
    int shutdown = 0, clear_unused_fds = 0;
    int close_connection;
    char buffer[READ_BUF_LEN];
    struct pollfd fds[DEFAULT_SERVER_MAX_CLIENTS];
    int nfds = 2, current_size = 0, i, j;

    extern ph_config_t config;
    config_parse_opts(argc, argv, &config);
    config_print(&config);

    if (messages_init(config.max_lines) < 0) {
        exit(EXIT_FAILURE);
    }

    if ((listen_sd = server_setup_socket(&config)) < 0) {
        server_print_error(listen_sd);
        exit(EXIT_FAILURE);
    }

    int flags = fcntl(fileno(stdin), F_GETFD, 0);
    if (fcntl(fileno(stdin), F_SETFL, flags | O_NONBLOCK) < 0) {
        fprintf(stderr, "Error calling fcntl in %s: %s\n", __FUNCTION__,
                strerror(errno));
        return EXIT_FAILURE;
    }

    memset(fds, 0, sizeof(fds));

    fds[0].fd = listen_sd;
    fds[0].events = POLLIN;

    fds[1].fd = fileno(stdin);
    fds[1].events = POLLIN;

    if (config.timeout > 0) {
        config.timeout *= 1000;
    }

    do {
        rc = poll(fds, nfds, config.timeout);

        if (rc < 0) {
            perror("poll() error");
            break;
        }

        if (rc == 0) {
            fprintf(stderr, "  poll() timed out.\n");
            break;
        }

        current_size = nfds;
        for (i = 0; i < current_size; i++) {
            if (fds[i].revents == 0) continue;

            if (fds[i].revents != POLLIN) {
                debug_print("fd=%d; events: %s%s%s\n", fds[i].fd,
                            (fds[i].revents & POLLIN) ? "POLLIN " : "",
                            (fds[i].revents & POLLHUP) ? "POLLHUP " : "",
                            (fds[i].revents & POLLERR) ? "POLLERR " : "");
                continue;
            }
            if (fds[i].fd == listen_sd) {
                new_sd = accept(listen_sd, NULL, NULL);

                if (new_sd < 0) {
                    if (errno != EWOULDBLOCK) {
                        perror("accept() error");
                        shutdown = 1;
                    }
                    break;
                }

                flags = fcntl(new_sd, F_GETFD, 0);
                if (fcntl(new_sd, F_SETFD, flags | O_NONBLOCK)) {
                    fprintf(stderr,
                            "Error setting O_NONBLOCK on accepted socket!\n");
                    break;
                }
                debug_print("  Incoming connection - %d\n", new_sd);
                fds[nfds].fd = new_sd;
                fds[nfds].events = POLLIN;
                nfds++;
            } else if (fds[i].fd == fileno(stdin)) {
                do {
                    rc = read(fds[i].fd, buffer, sizeof(buffer) - 1);

                    if (rc < 0) {
                        if (errno != EWOULDBLOCK) {
                            perror("recv() error");
                            close_connection = 1;
                        }
                        break;
                    }

                    if (rc == 0) {
                        debug_print("%s\n", "Connection closed\n");
                        close_connection = 1;
                        break;
                    }

                    len = rc;
                    if (message_add_chunk(buffer, len) < 0) {
                        break;
                    }

                } while (1);

                // Only save a complete line if not received faster than rate
                if (message_check_save(config.rate, config.output_stdin) < 0) {
                    break;
                }
            } else {
                close_connection = 0;
                do {
                    rc = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (rc < 0) {
                        if (errno != EWOULDBLOCK) {
                            perror("recv() error");
                            close_connection = 1;
                        }
                        break;
                    }
                    if (rc == 0) {
                        debug_print("%s", "Connection closed\n");
                        close_connection = 1;
                        break;
                    }
                    len = rc;
                    buffer[len] = '\0';
                    debug_print("%s\n", buffer);

                    char *response = NULL;
                    void *result = NULL;
                    int type = http_parse_request(buffer, &result);

                    if (type == PH_HTTP_ERROR) {
                        response = http_response_error();
                    } else if (type == PH_HTTP_CLEAR) {
                        messages_clear();
                        response = http_response_ok();
                    } else if (type == PH_HTTP_CONFIG) {
                        if (http_parse_request_config(result, &config) < 0) {
                            response = http_response_error();
                        } else {
                            response = http_response_ok();
                            // Call list resize even if no config max_lines change
                            messages_resize(config.max_lines);
                        }
                    } else if (type == PH_HTTP_LINES) {
                        long int lines = 0;
                        if (result != NULL) {
                            lines = *((long int *)result);
                            debug_print("Lines: %ld\n", lines);
                        }
                        if (lines > config.max_lines || lines == 0)
                            lines = config.max_lines;

                        char *http_body = messages_get_formated(
                            lines, config.body_prefix, config.body_suffix,
                            config.line_delimiter);

                        if (http_body) {
                            response = http_response_lines(http_body);
                            free(http_body);
                        } else {
                            response = http_response_error();
                        }
                    } else {
                        response = http_response_error();
                    }

                    if (!response) {
                        break;
                    }

                    rc = send(fds[i].fd, response, strlen(response), 0);
                    free(response);
                    if (rc < 0) {
                        perror("send() error");
                        close_connection = 1;
                        break;
                    }
                } while (1);

                if (close_connection) {
                    close(fds[i].fd);
                    fds[i].fd = -1;
                    clear_unused_fds = 1;
                }
            } /* End of connection is readable             */
        }     /* End of loop pollable descriptors          */

        if (clear_unused_fds) {
            clear_unused_fds = 0;
            for (i = 0; i < nfds; i++) {
                if (fds[i].fd == -1) {
                    for (j = i; j < nfds; j++) {
                        fds[j].fd = fds[j + 1].fd;
                    }
                    i--;
                    nfds--;
                }
            }
        }

    } while (!shutdown);

    for (i = 0; i < nfds; i++) {
        if (fds[i].fd >= 0) close(fds[i].fd);
    }
    messages_clear();
    return 0;
}
