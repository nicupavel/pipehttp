/*
 *    Author: Nicu Pavel <npavel@linuxconsulting.ro>
 *    Copyright (c) 2021 Green Electronics LLC
 *    The MIT License (MIT)
 * 
 */
#ifndef __PH_DEBUG_H
#define __PH_DEBUG_H

#ifdef DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

#define debug_print(...)                  \
    do                                    \
    {                                     \
        if (DEBUG)                        \
            fprintf(stderr, __VA_ARGS__); \
    } while (0)

#define TRACE(x)            \
    do                      \
    {                       \
        if (DEBUG)          \
            debug_printf x; \
    } while (0)
#endif
