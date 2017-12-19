/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

#include "logging.h"
#include "originals.h"


extern "C" void * __libc_dlsym(void *, const char *);
extern "C" void * _dl_sym(void *, const char *, void *);

static void * libPulseHandle = NULL;

#define MAGIC_HND (void *)(0x1865FE45)

/**
 * Intercept the call to dlopen() 
 *
 */
void * dlopen(const char * filename, int flags)
{
    static void * (*the_real_dlopen)(const char * filename, int flags) = NULL;

    if(the_real_dlopen == NULL) {
        the_real_dlopen = reinterpret_cast<void *(*)(const char *, int)>(dlsym(RTLD_NEXT, "dlopen"));
        if(the_real_dlopen == NULL) {
            DEBUG_MSG("Failed to find dlopen()");
            return NULL;
        }
    }

    void * handle = the_real_dlopen(filename, flags);

    if(filename && (strncmp("libpulse.so", filename, 11) == 0)) {
        DEBUG_MSG("dlopen(%s) called", filename);
        libPulseHandle = handle;
        handle = MAGIC_HND;
    }
    return handle;
}

int pa_dummy()
{
    DEBUG_MSG("pa_dummy called\n");
    return 0;
}

/**
 * Intercept the call to dlsym()
 *
 */
void * dlsym(void * handle, const char * symbol)
{
    void * address = NULL;
    static void * (*the_real_dlsym)(void * handle, const char *symbol) = NULL;

    if(the_real_dlsym == NULL) {
        the_real_dlsym = reinterpret_cast<void *(*)(void *, const char *)>(
                    _dl_sym(RTLD_NEXT, "dlsym", reinterpret_cast<void *>(dlsym)));
        if(the_real_dlsym == NULL) {
            DEBUG_MSG("Failed to find dlsym()");
            return NULL;
        }
    }

    if(strcmp(symbol, "dlsym") == 0) {
        // Avoid recursive call
        address = reinterpret_cast<void *>(dlsym);

    } else if(handle == MAGIC_HND) {
        // Symbol from libpulse?

        address = the_real_dlsym(libPulseHandle, symbol);
//        DEBUG_MSG("dlsym(%s) called", symbol);

        if(address) {
            char buf[100];
            strcpy(buf, symbol);
            buf[0] = 'z';
            buf[1] = 'z';
            void ** pSaveAddress = reinterpret_cast<void **>(the_real_dlsym(RTLD_DEFAULT, buf));
            if(pSaveAddress) {
                *pSaveAddress = address;
            } else {
                DEBUG_MSG("WARNING: Save Symbol %s missing", buf);
            }
        }
        address = the_real_dlsym(RTLD_DEFAULT, symbol);
        if(address == NULL) {
            address = reinterpret_cast<void *>(pa_dummy);
            DEBUG_MSG("WARNING: Shim function  %s missing", symbol);
        }
    } else {
        address = the_real_dlsym(handle, symbol);
    }
    return address;

}

/**
 * Intercept the dlclose method
 */
int dlclose(void * handle)
{
    static int (*the_real_dlclose)(void * handle) = NULL;
    
    if(the_real_dlclose == NULL) {
        the_real_dlclose = reinterpret_cast<int(*)(void *)>(dlsym(RTLD_NEXT, "dlclose"));
        if(the_real_dlclose == NULL) {
            DEBUG_MSG("Failed to find dlclose()");
            return 0;
        }
    }

    if(handle == MAGIC_HND) {
        DEBUG_MSG("dlclose called");
        handle = libPulseHandle;
    } 
    return the_real_dlclose(handle);
}

#define LOAD_SYM(name) dlsym(handle, "zz_" #name)

/**
 * Get pulse symbols for the case when pulse has linked dynamically
 * (as opposed to opened with dlopen)
 */
void init_symbols(void)
{
    if(zz_get_library_version == NULL) {
        void * handle = dlopen("libpulse.so", RTLD_LAZY);

	LOAD_SYM(get_library_version);
	LOAD_SYM(channel_map_can_balance);
	LOAD_SYM(channel_map_init);
	LOAD_SYM(context_connect);
	LOAD_SYM(context_disconnect);
	LOAD_SYM(context_drain);
	LOAD_SYM(context_get_server_info);
	LOAD_SYM(context_get_sink_info_by_name);
	LOAD_SYM(context_get_sink_info_list);
	LOAD_SYM(context_get_sink_input_info);
	LOAD_SYM(context_get_source_info_list);
	LOAD_SYM(context_get_state);
	LOAD_SYM(context_new);
	LOAD_SYM(context_rttime_new);
	LOAD_SYM(context_set_sink_input_volume);
	LOAD_SYM(context_set_state_callback);
	LOAD_SYM(context_unref);
	LOAD_SYM(cvolume_set);
	LOAD_SYM(cvolume_set_balance);
	LOAD_SYM(frame_size);
	LOAD_SYM(operation_get_state);
	LOAD_SYM(operation_unref);
	LOAD_SYM(proplist_gets);
	LOAD_SYM(rtclock_now);
	LOAD_SYM(stream_cancel_write);
	LOAD_SYM(stream_connect_playback);
	LOAD_SYM(stream_cork);
	LOAD_SYM(stream_disconnect);
	LOAD_SYM(stream_get_channel_map);
	LOAD_SYM(stream_get_index);
	LOAD_SYM(stream_get_latency);
	LOAD_SYM(stream_get_sample_spec);
	LOAD_SYM(stream_get_state);
	LOAD_SYM(stream_get_time);
	LOAD_SYM(stream_new);
	LOAD_SYM(stream_set_state_callback);
	LOAD_SYM(stream_set_write_callback);
	LOAD_SYM(stream_unref);
	LOAD_SYM(stream_update_timing_info);
	LOAD_SYM(stream_write);
	LOAD_SYM(sw_volume_from_linear);
	LOAD_SYM(threaded_mainloop_free);
	LOAD_SYM(threaded_mainloop_get_api);
	LOAD_SYM(threaded_mainloop_in_thread);
	LOAD_SYM(threaded_mainloop_lock);
	LOAD_SYM(threaded_mainloop_new);
	LOAD_SYM(threaded_mainloop_signal);
	LOAD_SYM(threaded_mainloop_start);
	LOAD_SYM(threaded_mainloop_stop);
	LOAD_SYM(threaded_mainloop_unlock);
	LOAD_SYM(threaded_mainloop_wait);
	LOAD_SYM(usec_to_bytes);
        LOAD_SYM(stream_set_read_callback);
	LOAD_SYM(stream_connect_record);
	LOAD_SYM(stream_readable_size);
        LOAD_SYM(stream_writable_size);
	LOAD_SYM(stream_drop);
	LOAD_SYM(stream_get_buffer_attr);
	LOAD_SYM(stream_get_device_name);
	LOAD_SYM(context_set_subscribe_callback);
	LOAD_SYM(context_subscribe);
	LOAD_SYM(mainloop_api_once);
        dlclose(handle);
    }
}
