/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */
#include "messages.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "debug.h"
#include "dlist.h"

static DList messages;
static char *message = NULL;
static unsigned int message_size = 0, message_alloc_size = 0;

struct timespec ts_last;
struct timespec ts_now;

static int message_new(void) {
    if (!(message = (char *)malloc(READ_BUF_LEN))) {
        fprintf(stderr, "Cannot allocate buffer\n");
        return -1;
    }
    message_size = 0;
    message_alloc_size = READ_BUF_LEN;

    return 0;
}

void message_free(void) {
    free(message);
}

int messages_init(unsigned int lines) {
    dlist_init(&messages, free, lines);
    return message_new();
}

void messages_clear(void) { return dlist_clear(&messages); }
void messages_resize(unsigned int new_size) { return dlist_resize(&messages, new_size); }

int message_add_chunk(char *buffer, int len) {
    char *tmp = NULL;
    if ((message_size + len + 1) > message_alloc_size) {
        fprintf(stderr, "Alloc: %d bytes\n", message_alloc_size + READ_BUF_LEN);
        tmp = realloc(message, message_alloc_size + READ_BUF_LEN);
        if (!tmp) {
            fprintf(stderr, "Cannot realloc message !\n");
            return -1;
        }
        message = tmp;
        message_alloc_size += READ_BUF_LEN;
    }
    memcpy(message + message_size, buffer, len);
    message_size += len;
    return 0;
}

int message_check_save(unsigned int rate, unsigned int output) {
    if (message_size > 0 && message[message_size - 1] == '\n') {
        message[message_size] = '\0';

        // Check if rate limiting is respected
        clock_gettime(CLOCK_MONOTONIC, &ts_now);
        if (ts_last.tv_sec > 0 && ts_now.tv_sec - ts_last.tv_sec < rate) {
            free(message);
            debug_print("%s", "Skip save\n");
        } else {
            clock_gettime(CLOCK_MONOTONIC, &ts_last);
            dlist_insert(&messages, NULL, message);
            if (output) {
                fprintf(stdout, "%s", message);
                fflush(stdout);
            }
        }
        return message_new();
    }
    return 0;
}

char *message_get_current() { return message; }

char *messages_get_formated(const unsigned int lines, const char *prefix,
                            const char *suffix, const char *line_delimiter) {
    unsigned long int total_messages_size = 0;
    unsigned int prefix_len = 0;
    unsigned int suffix_len = 0;
    unsigned int line_delimiter_len = 0;

    DListElmt *e;

    debug_print("Requested lines: %u\n", lines);

    if (prefix) {
        prefix_len = strlen(prefix);
    }

    if (suffix) {
        suffix_len = strlen(suffix);
    }

    if (line_delimiter) {
        line_delimiter_len = strlen(line_delimiter);
    }

    unsigned int l;
    for (l = 0, e = dlist_head(&messages); e != NULL && l < lines;
         e = e->next, l++) {
        if (e->data) {
            total_messages_size += strlen((char *)e->data);
            if (line_delimiter && l < lines - 1 && e->next != NULL) {
                total_messages_size += line_delimiter_len;
            }
        }
    }

    if (prefix) {
        total_messages_size += prefix_len;
    }
    if (suffix) {
        total_messages_size += suffix_len;
    }

    debug_print("Total messages size: %ld\n", total_messages_size);

    char *body = (char *)malloc(total_messages_size + 1);
    if (!body) {
        fprintf(stderr, "Cannot alloc memory for messages\n");
        return NULL;
    }
    memset(body, 0, total_messages_size + 1);

    unsigned long int seek = 0;

    if (prefix) {
        memcpy(body, prefix, prefix_len);
        seek += strlen(prefix);
    }

    for (l = 0, e = dlist_head(&messages); e != NULL && l < lines;
         e = e->next, l++) {
        if (e->data) {
            unsigned int len = strlen(e->data);
            memcpy(body + seek, e->data, len);
            seek += len;
            if (line_delimiter && l < lines - 1 && e->next != NULL) {
                memcpy(body + seek, line_delimiter, line_delimiter_len);
                seek += line_delimiter_len;
            }
        }
    }

    if (suffix) {
        memcpy(body + seek, suffix, suffix_len);
    }

    return body;
}