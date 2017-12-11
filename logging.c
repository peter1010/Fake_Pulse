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
#include <unistd.h>

#include "logging.h"

static FILE * log_out = NULL;
static struct timespec prev_ts = {0, 0};
static unsigned int msg_missed = 0;
static unsigned msg_per_sec = 0;
 
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

static void logging_done()
{
    if(log_out)
    {
        fflush(log_out);
//        fclose(log_out);
//        log_out = NULL;
    }
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

        if(ts.tv_sec == prev_ts.tv_sec) {
            if(msg_per_sec > 50) {
                msg_missed++;
            }
        } else {
            prev_ts.tv_sec = ts.tv_sec;
            if(msg_missed > 0) {
                fprintf(hnd, "!! %u debug messages missed !!\n", msg_missed);
                msg_missed = 0;
                msg_per_sec = 1;
            } else {
                msg_per_sec = 0;
            }
        }

        if(msg_missed == 0) {
            msg_per_sec++;
            fprintf(hnd, "%i %u.%03li ", getpid(), (unsigned)ts.tv_sec, ts.tv_nsec/1000000);
            vfprintf(hnd, fmt, ap);
            fprintf(hnd, "\n");
            fflush(hnd);
        }
        logging_done();
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
    logging_done();
}

void log_server_info(const pa_server_info * info)
{
    msg_per_sec = 0;
    msg_missed = 0;
    DEBUG_MSG("SERVER INFO");
    if(info) {
        DEBUG_MSG("\tuser_name = %s", info->user_name);
        DEBUG_MSG("\thost_name = %s", info->host_name);
        DEBUG_MSG("\tserver_version = %s", info->server_version);
        DEBUG_MSG("\tserver_name = %s", info->server_name);
        DEBUG_MSG("\tdef sample format = %i", info->sample_spec.format);
        DEBUG_MSG("\tdef sample rate = %i", info->sample_spec.rate);
        DEBUG_MSG("\tdef sample chans = %i", info->sample_spec.channels);
        DEBUG_MSG("\tdef sink name = %s", info->default_sink_name);
        DEBUG_MSG("\tdef source name = %s", info->default_source_name);
        DEBUG_MSG("\tcookie = %i", info->cookie);
        DEBUG_MSG("\tMap channels =%i", info->channel_map.channels);
        for(int i = 0; i < info->channel_map.channels; i++) {
            DEBUG_MSG("\t\tMap[%i]  =%i", i, info->channel_map.map[i]);
        }
    } else {
        DEBUG_MSG("\tNULL");
    }
}

void log_sink_info(const pa_sink_info * info)
{
    msg_per_sec = 0;
    msg_missed = 0;
    DEBUG_MSG("SINK INFO");
    if(info) {
        DEBUG_MSG("\tname = %s", info->name);
        DEBUG_MSG("\tindex = %i", info->index);
        DEBUG_MSG("\tdesc = %s", info->description);
        DEBUG_MSG("\tdef sample format = %i", info->sample_spec.format);
        DEBUG_MSG("\tdef sample rate = %i", info->sample_spec.rate);
        DEBUG_MSG("\tdef sample chans = %i", info->sample_spec.channels);
        DEBUG_MSG("\tMap channels =%i", info->channel_map.channels);
        for(int i = 0; i < info->channel_map.channels; i++) {
            DEBUG_MSG("\t\tMap[%i]  =%i", i, info->channel_map.map[i]);
        }
        DEBUG_MSG("owner =%i", info->owner_module);
        DEBUG_MSG("\tVolume channels =%i", info->volume.channels);
        for(int i = 0; i < info->channel_map.channels; i++) {
            DEBUG_MSG("\t\tMap[%i]  =%i", i, info->volume.values[i]);
        }
        DEBUG_MSG("\tMute =%i", info->mute);
        DEBUG_MSG("\tMonitor source =%i", info->monitor_source);
        DEBUG_MSG("\tMonitor source_name =%s", info->monitor_source_name);
        DEBUG_MSG("\tLatency =%lu", info->latency);
        DEBUG_MSG("\tDriver =%s", info->driver);
#if 0        
        info.flags = PA_SINK_HARDWARE;
        info.proplist = NULL;
        info.configured_latency = 1000;
        info.base_volume = 50;
        info.state = PA_SINK_IDLE;
        info.n_volume_steps = 1;
        info.card =  1;
        info.n_ports = 0; 
        info.ports = NULL;
        info.active_port = NULL;
        info.n_formats = 1;
        info.formats = formats;
#endif
    }
}

const char * context_state2str(pa_context_state_t state)
{
    const char * str = "UNKNOWN";
    switch(state) {
        case PA_CONTEXT_UNCONNECTED: str = "UNCONNECTED"; break;
        case PA_CONTEXT_CONNECTING: str = "CONNECTING"; break;
        case PA_CONTEXT_AUTHORIZING: str = "AUTHORIZING"; break;
        case PA_CONTEXT_SETTING_NAME: str = "SETTING_NAME"; break;
        case PA_CONTEXT_READY: str = "READY"; break;
        case PA_CONTEXT_FAILED: str = "FAILED"; break;
        case PA_CONTEXT_TERMINATED: str = "TERMINATING"; break;
    }
    return str;
}

const char * operation_state2str(pa_operation_state_t state)
{
    const char * str = "UNKNOWN";
    switch(state) {
        case PA_OPERATION_RUNNING : str = "RUNNING"; break;
        case PA_OPERATION_DONE : str = "DONE"; break;
        case PA_OPERATION_CANCELLED: str = "CANCELLED"; break;
    }
    return str;
}

const char * stream_state2str(pa_stream_state_t state)
{
    const char * str = "UNKNOWN";
    switch(state) {
        case PA_STREAM_UNCONNECTED : str = "UNCONNECTED"; break;
        case PA_STREAM_CREATING : str = "CREATING"; break;
        case PA_STREAM_READY : str = "READY"; break;
        case PA_STREAM_FAILED : str = "FAILED"; break;
        case PA_STREAM_TERMINATED : str = "TERMINATED"; break;
    }
    return str;
}
 

const char * subscription_mask2str(pa_subscription_mask_t m)
{
    static char buf[100];
    buf[0] = 0;

    if(m & PA_SUBSCRIPTION_MASK_SINK) {
        strcat(buf, "SINK, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_SOURCE) {
        strcat(buf, "SRC, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_SINK_INPUT) {
        strcat(buf, "SINK_IN, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT) {
        strcat(buf, "SRC_OUT, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_MODULE) {
        strcat(buf, "MODULE, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_CLIENT) {
        strcat(buf, "CLIENT, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_SAMPLE_CACHE) {
        strcat(buf, "CACHE, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_SERVER) {
        strcat(buf, "SERVER, ");
    }
    if(m & PA_SUBSCRIPTION_MASK_CARD) {
        strcat(buf, "CARD, ");
    }
    return buf;
}



