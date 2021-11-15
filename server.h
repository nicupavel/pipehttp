/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */
#ifndef __PH_SERVER_H
#define __PH_SERVER_H

#include "config.h"

#define PH_SERVER_BACKLOG 32

#define PH_SERVER_ERROR_SOCKET      -50
#define PH_SERVER_ERROR_FCNTL       -51
#define PH_SERVER_ERROR_SETSOCKOPT  -52
#define PH_SERVER_ERROR_BIND        -53
#define PH_SERVER_ERROR_LISTEN      -54


int server_setup_socket(ph_config_t *config);
void server_print_error(int err);

#endif