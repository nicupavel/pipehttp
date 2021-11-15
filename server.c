/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "server.h"

int server_setup_socket(ph_config_t *config)
{
    int server_fd, flags;
    struct sockaddr_in addr;
    int reuse = 1;

    if (!config)
    {
        return PH_SERVER_ERROR_SOCKET;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return PH_SERVER_ERROR_SOCKET;
    }

    flags = fcntl(server_fd, F_GETFD, 0);
    if (fcntl(server_fd, F_SETFD, flags | O_NONBLOCK))
    {
        close(server_fd);
        return PH_SERVER_ERROR_FCNTL;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse,
                   sizeof(reuse)) < 0)
    {
        close(server_fd);
        return PH_SERVER_ERROR_SETSOCKOPT;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(config->port);

    if (config->addr) {
        addr.sin_addr.s_addr = inet_addr(config->addr);    
    } else {
        addr.sin_addr.s_addr = INADDR_ANY;
    }
    
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        close(server_fd);
        return PH_SERVER_ERROR_BIND;
    }

    if (listen(server_fd, PH_SERVER_BACKLOG) < 0)
    {
        close(server_fd);
        return PH_SERVER_ERROR_LISTEN;
    }

    return server_fd;
}

void server_print_error(int err)
{
    switch (err)
    {
    case PH_SERVER_ERROR_SOCKET:
        fprintf(stderr, "Socket socket() error!\n");
        break;
    case PH_SERVER_ERROR_SETSOCKOPT:
        fprintf(stderr, "Socket setsockopt() error!\n");
        break;
    case PH_SERVER_ERROR_FCNTL:
        fprintf(stderr, "Socket fcntl() error!\n");
        break;
    case PH_SERVER_ERROR_BIND:
        fprintf(stderr, "Socket bind() error!\n");
        break;
    case PH_SERVER_ERROR_LISTEN:
        fprintf(stderr, "Socket bind() error!\n");
        break;
    default:
        fprintf(stderr, "Uknown error!\n");
        break;
    }
}