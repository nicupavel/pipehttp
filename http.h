
/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */
#ifndef __PH_HTTP_H
#define __PH_HTTP_H

#include "config.h"

#define HTTP_ERROR_RESPONSE "HTTP/1.1 404 Not Found.\r\nContent-Length: 9\r\nConnection: Closed\r\n\r\nNOT FOUND"

enum http_result {
    PH_HTTP_ERROR = -1,
    PH_HTTP_LINES = 1,    
    PH_HTTP_CLEAR,
    PH_HTTP_CONFIG,
    PH_HTTP_MAX_HTTP
};

int http_parse_request(char *req, void **result);
int http_parse_request_config(const char *path, ph_config_t *config);
char *http_response_error(void);
char *http_response_ok(void);
char *http_response_lines(const char *body);
#endif
