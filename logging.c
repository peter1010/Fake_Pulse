/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "logging.h"


static FILE * log_out = NULL;

/**
 * Make sure the log_out is valid
 */
static FILE * get_log_handle()
{
    if(!log_out)
    {
        char buf[100];
        const char * home = getenv("HOME");
        snprintf(buf, 100, "%s/tmp/log.txt", home);
        log_out = fopen(buf, "a");
    }
    return log_out;
}

/**
 * Logging function
 *
 * @param[in] level The level
 * @param[in] fmt The format string in the style of printf
 * @param[in] args Variable args
 */
void log_msg(const char * fmt, ...)
{
    FILE * hnd = get_log_handle();

    va_list ap;
    va_start(ap, fmt);

    if(hnd) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);

        fprintf(hnd, "%u.%03li ", (unsigned)ts.tv_sec, ts.tv_nsec/1000000);
        vfprintf(hnd, fmt, ap);
        fprintf(hnd, "\n");
        fflush(hnd);
    }
    va_end(ap);
}

/**
 * Logging function
 *
 * @param[in] level The level
 * @param[in] fmt The format string in the style of printf
 * @param[in] args Variable args
 */
void log_errno(const char * msg)
{
    FILE * hnd = get_log_handle();

    if(hnd) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);

        fprintf(hnd, "%u.%03li %s: %s\n", (unsigned)ts.tv_sec, ts.tv_nsec/1000000, strerror(errno), msg);
        fflush(hnd);
    }
}

