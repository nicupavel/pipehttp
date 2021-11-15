/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 *
 */

#include "config.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

ph_config_t config = {.port = DEFAULT_SERVER_PORT,
                      .addr = DEFAULT_SERVER_ADDR,
                      .timeout = -1,
                      .output_stdin = 1,
                      .rate = 0,
                      .max_lines = DEFAULT_MAX_LINES,
                      .body_prefix = NULL,
                      .body_suffix = NULL,
                      .line_delimiter = NULL};

void config_parse_opts(int argc, char **argv, ph_config_t *config) {
    int opt, rc;

    if (!config) return;

    while ((opt = getopt(argc, argv, "l:p:a:b:s:d:t:r:ohV")) != -1) {
        switch (opt) {
            case 'l':
                rc = sscanf(optarg, "%u", &config->max_lines);
                if (rc < 1) {
                    config->rate = DEFAULT_MAX_LINES;
                }
                break;
            case 'p':
                rc = sscanf(optarg, "%hu", &config->port);
                if (rc < 1) {
                    config->port = DEFAULT_SERVER_PORT;
                }
                break;
            case 't':
                rc = sscanf(optarg, "%d", &config->timeout);
                if (rc < 1) {
                    config->timeout = -1;
                }
                break;
            case 'r':
                rc = sscanf(optarg, "%u", &config->rate);
                if (rc < 1) {
                    config->rate = 0;
                }
                break;
            case 'a':
                config->addr = optarg;
                break;
            case 'b':
                config->body_prefix = optarg;
                break;
            case 's':
                config->body_suffix = optarg;
                break;
            case 'd':
                config->line_delimiter = optarg;
                break;
            case 'o':
                config->output_stdin = 0;
                break;
            case 'V':
            case 'h':
            default:
                config_help();
                exit(EXIT_SUCCESS);
                break;
        }
    }
}

int config_set_key(ph_config_t *config, const char *key, const void *val) {
    if (key == NULL || val == NULL) return -1;

    if (strcmp(key, "rate") == 0) {        
        config->rate = (*(unsigned int *)val);
    } else if (strcmp(key, "max_lines") == 0) {
        config->max_lines = (*(unsigned int *)val);
    } else if (strcmp(key, "output_stdin") == 0) {
        config->output_stdin = (*(unsigned short int *)val);
    } else if (strcmp(key, "timeout") == 0) {
        config->timeout = (*(int *)val);
    } else {
        return -1;
    }

    return 0;
}

void config_print(ph_config_t *config) {
    if (!config) return;

    fprintf(stderr,
            "Configuration: \n"
            "\tport: %d\n"
            "\taddr: %s\n"
            "\ttimeout: %d\n"
            "\toutput stdin: %d\n"
            "\trate: %d seconds\n"
            "\tmax_lines: %d\n"
            "\tbody_prefix: %s\n"
            "\tbody_suffix: %s\n"
            "\tline_delimiter: %s\n",
            config->port, config->addr, config->timeout, config->output_stdin,
            config->rate, config->max_lines, config->body_prefix,
            config->body_suffix, config->line_delimiter);
}

void config_help(void) {
    fprintf(stdout, "\nph - %s\n\n", PH_VERSION);

    fprintf(
        stdout,
        "Usage: "
        "ph [options...]\n"
        "The following options can also be supplied to the command:\n\n"
        ""
        "  -a <addr>       - The address to bind. Default any\n"
        "  -p <port>       - The port to bind. Default %d\n"
        "  -l <number>     - Max number of lines to hold. Default %d\n"
        "  -t <seconds>    - Inactivity timeout in seconds. Default infinite.\n"
        "  -b <string>     - String to append at the begining of response. "
        "Default none.\n"
        "  -s <string>     - String to append at end of response. Default "
        "none.\n"
        "  -d <string>     - Line delimiter string to append between lines "
        "(except last line). Default none.\n"
        "  -r <seconds>    - Rate limiting incoming lines. Lines comming "
        "faster "
        "will be ignored. Default no limit.\n"
        "  -o              - Don't output stdin to stdout\n"
        "  -h              - This help.\n"
        "  -V              - Display version information and exit.\n"
        "\n\n",
        DEFAULT_SERVER_PORT, DEFAULT_MAX_LINES);
}