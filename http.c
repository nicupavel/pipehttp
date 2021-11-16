/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 *
 */
#include "http.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "debug.h"

static long int *http_get_number(char *str) {
    long int *lines = NULL;

    if (!(lines = malloc(sizeof(long int)))) {
        return NULL;
    }

    *lines = strtol(str, NULL, 10);
    if ((errno == ERANGE && (*lines == LONG_MAX || *lines == LONG_MIN)) ||
        (errno != 0 && *lines == 0)) {
        return NULL;
    }
    if (*lines < 0) *lines *= -1;
    debug_print("Lines parse: %ld\n", *lines);

    return lines;
}

int http_parse_request(char *req, void **result) {
    char http_path[256];
    int http_ver;

    int res = sscanf(req, "GET %255s HTTP/1.%1d\n", http_path, &http_ver);
    if (res != 2) {
        debug_print("%s", "Error parsing http get\n");
        return PH_HTTP_ERROR;
    }

    if (strncmp(http_path, "/", sizeof(http_path)) == 0) {
        return PH_HTTP_LINES;
    } else if (strncmp(http_path, "/clear", sizeof(http_path)) == 0) {
        return PH_HTTP_CLEAR;
    } else if (strstr(http_path, "/config")) {
        *result = strdup(http_path);
        return PH_HTTP_CONFIG;
    } else {
        if ((*result = (void **)http_get_number(http_path + 1)))
            return PH_HTTP_LINES;
    }

    return PH_HTTP_ERROR;
}

int http_parse_request_config(const char *path, ph_config_t *config) {
    // Format of GET /config: /config?lines=100&rate=60
    char *known_keys[] = {"rate", "max_lines", "output_stdin", "timeout", NULL};

    char *s = strchr(path, '?');
    if (!s) {
        return 1;
    }
    s++;  // skip '?'

    char **known = known_keys;
    for (; *known; known++) {
        int v = 0;
        char *m = strstr(s, *known);
        if (m) {
            char fmt[32];
            snprintf(fmt, sizeof(fmt), "%s=%%d", *known);
            int c = sscanf(m, fmt, &v);
            if (c == 1) {
                debug_print("Key: %s Value: %d\n", *known, v);
                config_set_key(config, *known, &v);
            }
        }
    }
    //config_print(config);
    return 0;
}

char *http_response_error(void) { return strdup(HTTP_ERROR_RESPONSE); }

char *http_response_ok(void) {
    char *response = (char *)malloc(256);

    snprintf(response, 256, "%s\r\n%s\r\n%s\r\n%s\r\n\r\nOK", "HTTP/1.1 200 OK",
             "Accept-Ranges: bytes", "Content-Length: 2", "Connection: close");

    return response;
}

char *http_response_lines(const char *body) {
    char header[256];

    if (!body) return NULL;

    unsigned long int header_len = 0;
    unsigned long int response_size = 0;
    unsigned long int body_len = strlen(body);

    snprintf(header, 256, "%s\r\n%s\r\n%s%ld\r\n%s", "HTTP/1.1 200 OK",
             "Accept-Ranges: bytes", "Content-Length: ", body_len,
             "Connection: close");

    header_len = strlen(header);

    debug_print("    header len: %lu body len: %lu \n", header_len, body_len);
    debug_print("'%s'\n", body);

    response_size = header_len + 4 + body_len + 1;
    char *response = (char *)malloc(response_size);

    if (!response) {
        fprintf(stderr, "Cannot alloc memory for response\n");
        return NULL;
    }

    memset(response, 0, response_size);
    memcpy(response, header, header_len);
    memcpy(response + header_len, "\r\n\r\n", 4);
    memcpy(response + header_len + 4, body, body_len);
    debug_print("  %ld:%d response length\n", response_size, strlen(response));

    return response;
}
