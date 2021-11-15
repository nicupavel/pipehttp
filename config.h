/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */

#ifndef __PH_CONFIG_H
#define __PH_CONFIG_H

#define PH_VERSION "1.0.0"

#define DEFAULT_SERVER_MAX_CLIENTS 200
#define DEFAULT_SERVER_ADDR "0.0.0.0"
#define DEFAULT_SERVER_PORT 8000
#define READ_BUF_LEN 4096
#define MAX_READ_SIZE READ_BUF_LEN * 1024
#define DEFAULT_MAX_LINES 1000

typedef struct ph_config_
{
    unsigned short int port;
    unsigned short int output_stdin;
    int timeout;
    unsigned int max_lines;
    unsigned int rate;
    const char *addr;
    const char *body_prefix;
    const char *body_suffix;
    const char *line_delimiter;
} ph_config_t;

void config_parse_opts(int argc, char **argv, ph_config_t *config);
int config_set_key(ph_config_t *config, const char *key, const void *val);
void config_print(ph_config_t *config);
void config_help(void);

#endif
