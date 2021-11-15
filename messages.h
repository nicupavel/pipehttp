/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */
#ifndef __PH_MESSAGES_H
#define __PH_MESSAGES_H

int messages_init(unsigned int lines);
void messages_clear(void);
void messages_resize(unsigned int new_size);
void message_free(void);
int message_add_chunk(char *buffer, int len);
int message_check_save(unsigned int rate, unsigned int output);
char *messages_get_formated(const unsigned int lines, const char *prefix, const char *suffix, const char *line_delimiter);
char *message_get_current();

#endif