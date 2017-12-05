/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include "pulseaudio.h"

/**
 * Some Logging code
 */

#define DEBUG_MSG(...) log_msg( __VA_ARGS__)
#define DEBUG_ERRNO(x) log_errno( x)

extern void log_msg(const char * fmt, ...) __attribute__((format (printf, 1, 2)));

extern void log_errno(const char * msg);

extern void log_server_info(const pa_server_info * info);

extern void log_sink_info(const pa_sink_info * info);

extern const char * context_state2str(pa_context_state_t state);

const char * operation_state2str(pa_operation_state_t state);
#endif
