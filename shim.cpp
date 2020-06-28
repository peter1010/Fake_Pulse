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
static void * (*theRealDlopen)(const char * filename, int flags) = NULL;
static void * (*theRealDlsym)(void * handle, const char *symbol) = NULL;
static int (*theRealDlclose)(void * handle) = NULL;

#define MAGIC_HND (void *)(0x1865FE45)

/**
 * Set the global variable that holds the address of the real version of
 * dlopen(). Return non-zero on success
 */
static int set_real_dlopen()
{
    void * (*func)(const char * filename, int flags)
                = reinterpret_cast<void *(*)(const char *, int)>(dlsym(RTLD_NEXT, "dlopen"));
    if(func == NULL) {
        DEBUG_MSG("Failed to find dlopen()");
        return 0;
    }

    theRealDlopen = func;
    return 1;
}


/**
 * Set the global variable that holds the address of the real version of
 * dlsym(). Return non-zero on success
 */
static int set_real_dlsym()
{
    void * (*func)(void * handle, const char *symbol)
                = reinterpret_cast<void *(*)(void *, const char *)>(
                    _dl_sym(RTLD_NEXT, "dlsym", reinterpret_cast<void *>(dlsym)));
    if(func == NULL) {
        DEBUG_MSG("Failed to find dlsym()");
        return 0;
    }

    theRealDlsym = func;
    return 1;
}


/**
 * Set the global variable that holds the address of the real version of
 * dlclose(). Return non-zero on success
 */
static int set_real_dlclose()
{
    int (*func)(void * handle) = reinterpret_cast<int(*)(void *)>(dlsym(RTLD_NEXT, "dlclose"));
    if(func == NULL) {
        DEBUG_MSG("Failed to find dlclose()");
        return 0;
    }

    theRealDlclose = func;
    return 1;
}


/**
 * Intercept the call to dlopen(), this is called by the application, we are checking
 * for the application trying to open libpulse.so
 *
 * If the caller is opening the libpulse.so, return a fake handle
 */
void * dlopen(const char * filename, int flags)
{
    if(theRealDlopen == NULL) {
        if(!set_real_dlopen()) {
            return NULL;
        }
    }

    void * handle = theRealDlopen(filename, flags);

    if(filename && (strncmp("libpulse.so", filename, 11) == 0)) {
        DEBUG_MSG("dlopen(%s) called", filename);
        libPulseHandle = handle;
        handle = MAGIC_HND;
    }
    return handle;
}


/**
 * Dummy function used when we have yet to implement our version of the PA function to be faked
 */
int pa_dummy()
{
    DEBUG_MSG("pa_dummy called\n");
    return 0;
}

/**
 * Intercept the call to dlsym(), this is called by the application, we are checking
 * when it tries to get a symbole for a PA function.
 *
 * If the PA function is requested return address of our version
 *
 */
void * dlsym(void * handle, const char * symbol)
{
    void * address = NULL;

    if(theRealDlsym == NULL) {
        if(!set_real_dlsym()) {
            return NULL;
        }
    }

    if(strcmp(symbol, "dlsym") == 0) {
        // Avoid recursive call
        address = reinterpret_cast<void *>(dlsym);

    } else if(handle == MAGIC_HND) {
        // Symbol from libpulse? return address of our implementation

        address = theRealDlsym(RTLD_DEFAULT, symbol);
        if(address == NULL) {
            address = reinterpret_cast<void *>(pa_dummy);
            DEBUG_MSG("WARNING: Shim function  %s missing", symbol);
        }
    } else {
        address = theRealDlsym(handle, symbol);
    }
    return address;

}

/**
 * Intercept the dlclose method
 */
int dlclose(void * handle)
{
    if(theRealDlclose == NULL) {
        if(!set_real_dlclose()) {
            return 0;
        }
    }

    if(handle == MAGIC_HND) {
        DEBUG_MSG("dlclose called");
        handle = libPulseHandle;
    }
    return theRealDlclose(handle);
}

/**
 * Get the real PA function address
 */
static void * load_pulse_sym(void * handle, const char * symbol)
{
    void * address =  theRealDlsym(handle, symbol);
    if(address == NULL) {
        DEBUG_MSG("The real version of libpulse does not support %s", symbol);
    }
    return address;
}

#define LOAD_SYM(name) zz_##name = reinterpret_cast<FP_##name>(load_pulse_sym(handle, "pa_" # name))

/**
 * Get real pulse symbols for the case when we need to call pulse
 */
void init_symbols(void)
{
    if(zz_get_library_version == NULL) {
        if(theRealDlopen == NULL) {
            if(!set_real_dlopen()) {
                return;
            }
        }

        if(theRealDlsym == NULL) {
            if(!set_real_dlsym()) {
                return;
            }
        }

        DEBUG_MSG("Loading PA symbols from real library");

        void * handle = theRealDlopen("libpulse.so", RTLD_LAZY);

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

        if(theRealDlclose == NULL) {
            if(!set_real_dlclose()) {
                return;
            }
        }
        theRealDlclose(handle);
    }
}
