/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "logging.h"
#include "pulseaudio.h"
#include "defs.h"
#include "originals.h"
#include <stdlib.h>

#include "threaded_mainloop.hpp"
#include "context.hpp"
#include "blob.hpp"

#define _UNUSED __attribute__((unused))

void init_symbols(void);

#define GET_ORIGINAL(name) FP_##name orig = zz_##name
#define ListenMode 1

#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
bool UseRealThreadedMainloop = false;
#else
#define UseRealThreadedMainloop true
#endif

bool UseRealMainloopApi = true;
bool UseRealContext = true;

bool TraceThreadedMainLoop = true;
bool TraceMainloopApi = false;
bool TraceContext = true;


/*----------------------------------------------------------------------------*/
const char * pa_get_library_version(void)
{
    const char * retVal;
    if(ListenMode) {
        GET_ORIGINAL(get_library_version);
        retVal = orig();
    } else {
        retVal = "11.1.0";
    }
    DEBUG_MSG("%s() returned %s", __func__, retVal);
//    exit(0);
    return retVal;
}

/*----------------------------------------------------------------------------*/
int pa_channel_map_can_balance(const pa_channel_map * p)
{
    int retVal;
    if(ListenMode) {
        GET_ORIGINAL(channel_map_can_balance);
        retVal = orig(p);
    } else {
        retVal = 0;  /* Means no balance */
    }
    DEBUG_MSG("%s returned %i", __func__, retVal);
    return retVal;
}

/*----------------------------------------------------------------------------*/
pa_channel_map * pa_channel_map_init(pa_channel_map * p)
{
    pa_channel_map * retVal;
    DEBUG_MSG("%s called", __func__);
    if(ListenMode) {
        GET_ORIGINAL(channel_map_init);
        retVal = orig(p);
    } else {
        /* Lets always have simple stereo */
        retVal = p;
        p->channels = 2;
        p->map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        p->map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
int pa_context_connect(pa_context * c, const char * server,
        pa_context_flags_t flags, const pa_spawn_api * api)
{
    int retVal;
    if(TraceContext) {
        DEBUG_MSG("%s(%s) called ", __func__, server);
        if(api) {
            if(api->prefork) {
                DEBUG_MSG("Has prefork callback");
            }
            if(api->atfork) {
                DEBUG_MSG("Has atfork callback");
            }
            if(api->postfork) {
                DEBUG_MSG("Has postfork");
            }
        }
    }

    if(UseRealContext) {
        GET_ORIGINAL(context_connect);
        retVal = orig(c, server, flags, api);
    } else {
        retVal = reinterpret_cast<CContext *>(c)->connect(server, flags, api);
    }
    DEBUG_MSG("%s returned %i", __func__, retVal);
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_context_disconnect(pa_context * c)
{
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_disconnect);
        orig(c);
    } else {
        reinterpret_cast<CContext *>(c)->disconnect();
    }
}

/*----------------------------------------------------------------------------*/
pa_operation * pa_context_drain(pa_context * c _UNUSED, pa_context_notify_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_server_info_cb_t context_get_server_info_cb_func = NULL;

static void context_get_server_info_cb(pa_context *c, const pa_server_info * info, void *userdata)
{
    DEBUG_MSG("%s called ", __func__);
    log_server_info(info);
    context_get_server_info_cb_func(c, info, userdata);
}

pa_operation * pa_context_get_server_info(pa_context *c, pa_server_info_cb_t cb, void *userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        context_get_server_info_cb_func = cb;
        cb = cb ? context_get_server_info_cb : cb;
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_get_server_info);
        retVal = orig(c, cb, userdata);
    } else {
        retVal = reinterpret_cast<CContext *>(c)->get_server_info(cb, userdata);
    }
    return retVal;

}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_sink_info_cb_t context_get_sink_info_cb_func = NULL;

static void context_get_sink_info_cb(pa_context *c, const pa_sink_info * info, int eol, void * userdata)
{
    DEBUG_MSG("%s(%i) called ", __func__, eol);
    log_sink_info(info);
    context_get_sink_info_cb_func(c, info, eol, userdata);
}

pa_operation * pa_context_get_sink_info_by_name(pa_context * c,
        const char * name, pa_sink_info_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s(%s) called ", __func__, name);
        context_get_sink_info_cb_func = cb;
        cb = cb ? context_get_sink_info_cb : cb;
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_get_sink_info_by_name);
        retVal = orig(c, name, cb, userdata);
    } else {
        retVal = reinterpret_cast<CContext *>(c)->get_sink_info_by_name(name, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_operation * pa_context_get_sink_info_list(pa_context * c _UNUSED,
        pa_sink_info_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/

pa_operation * pa_context_get_sink_input_info(pa_context * c _UNUSED,
        uint32_t idx _UNUSED, pa_sink_input_info_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("TODO %s(%i) called ", __func__, idx);
    return NULL;
}

/*----------------------------------------------------------------------------*/

pa_operation * pa_context_get_source_info_list(pa_context *ci _UNUSED,
        pa_source_info_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/

pa_context_state_t pa_context_get_state(pa_context * c)
{
    pa_context_state_t retVal;
    if(UseRealContext) {
        GET_ORIGINAL(context_get_state);
        retVal = orig(c);
    } else {
        retVal = reinterpret_cast<CContext *>(c)->get_state();
    }
    if(TraceContext) {
        DEBUG_MSG("%s returned %s", __func__, context_state2str(retVal));
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_context * pa_context_new(pa_mainloop_api * mainloop, const char * name)
{
    pa_context * c;
    if(TraceContext) {
        DEBUG_MSG("%s(%s) called ", __func__, name ? name : "None");
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_new);
        c = orig(mainloop, name);
    } else {
        c = reinterpret_cast<pa_context *>(new CContext(mainloop, name));
    }
    return c;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_time_event_cb_t time_event_cb_func = NULL;

static void time_event_cb(pa_mainloop_api * api, pa_time_event *evt, const struct timeval * t, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    time_event_cb_func(api, evt, t, userdata);
}

pa_time_event * pa_context_rttime_new(pa_context * c,
        pa_usec_t usec, pa_time_event_cb_t cb, void *userdata)
{
    pa_time_event * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        time_event_cb_func = cb;
        cb = cb ? time_event_cb : cb;
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_rttime_new);
        retVal = orig(c, usec, cb, userdata);
    } else {
        retVal = reinterpret_cast<CContext *>(c)->rttime_new(usec, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_context_success_cb_t context_sink_input_volume_cb_func = NULL;

static void context_sink_input_volume_cb(pa_context * c, int v, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_sink_input_volume_cb_func(c, v, userdata);
}

pa_operation * pa_context_set_sink_input_volume(pa_context * c,
        uint32_t idx, const pa_cvolume * volume, pa_context_success_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        context_sink_input_volume_cb_func = cb;
        cb = cb ? context_sink_input_volume_cb : cb;
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_set_sink_input_volume);
        retVal = orig(c, idx, volume, cb, userdata);
    } else {
        retVal = reinterpret_cast<CContext *>(c)->set_sink_input_volume(idx, volume, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_context_notify_cb_t context_state_cb_func = NULL;

static void context_state_cb(pa_context *c, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_state_cb_func(c, userdata);
}

void pa_context_set_state_callback(pa_context * c, pa_context_notify_cb_t cb, void * userdata)
{
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        context_state_cb_func = cb;
        cb = cb ? context_state_cb : cb;
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_set_state_callback);
        orig(c, cb, userdata);
    } else {
        reinterpret_cast<CContext *>(c)->set_state_callback(cb, userdata);
    }
}

/*----------------------------------------------------------------------------*/

void pa_context_unref(pa_context * c)
{
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealContext) {
        GET_ORIGINAL(context_unref);
        orig(c);
    } else {
        CContext::unref(reinterpret_cast<CContext *>(c));
    }
}

/*----------------------------------------------------------------------------*/

pa_cvolume * pa_cvolume_set(pa_cvolume * a, unsigned channels, pa_volume_t v)
{
    pa_cvolume * retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(cvolume_set);
        retVal = orig(a, channels, v);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_cvolume * pa_cvolume_set_balance(pa_cvolume * v _UNUSED, const pa_channel_map * map _UNUSED, float new_balance _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/

size_t pa_frame_size(const pa_sample_spec * spec)
{
    size_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(frame_size);
        retVal = orig(spec);
    }
    // FIXME
//    DEBUG_MSG("%s returned %lu ", __func__, retVal);
    return retVal;
}


pa_operation_state_t pa_operation_get_state(pa_operation *o)
{
    pa_operation_state_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(operation_get_state);
        retVal = orig(o);
    } else {
        // FIXME
        retVal = PA_OPERATION_RUNNING;
    }
    DEBUG_MSG("%s returned %s", __func__, operation_state2str(retVal));
    return retVal;
}

/*----------------------------------------------------------------------------*/

void pa_operation_unref(pa_operation *o)
{
//    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(operation_unref);
        orig(o);
    }
    // FIXME
}

/*----------------------------------------------------------------------------*/

const char * pa_proplist_gets(pa_proplist * p _UNUSED, const char * key _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/

pa_usec_t pa_rtclock_now(void)
{
    pa_usec_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(rtclock_now);
        retVal = orig();
    } else {
        retVal = 0;
    }
//    DEBUG_MSG("%s returned %lu", __func__, retVal);
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_begin_write(pa_stream *p _UNUSED, void **data _UNUSED, size_t *nbytes _UNUSED)
{
    int retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_begin_write);
        retVal = orig(p, data, nbytes);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_cancel_write(pa_stream * p _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return 0;
}

/*----------------------------------------------------------------------------*/
int pa_stream_connect_playback(pa_stream * s,
        const char * dev, const pa_buffer_attr * attr, pa_stream_flags_t flags,
        const pa_cvolume *volume, pa_stream * sync_stream)
{
    int retVal;
    DEBUG_MSG("%s(%s) called ", __func__, dev);
    if(ListenMode) {
        GET_ORIGINAL(stream_connect_playback);
        retVal = orig(s, dev, attr, flags, volume, sync_stream);
    } else {
        // FIXME
        retVal = 0;
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_stream_success_cb_t stream_cork_cb_func = NULL;

static void stream_cork_cb(pa_stream * p, int t, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    stream_cork_cb_func(p, t, userdata);
}


pa_operation * pa_stream_cork(pa_stream * s, int b, pa_stream_success_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    DEBUG_MSG("%s called ", __func__);
    stream_cork_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(stream_cork);
        retVal = orig(s, b, cb ? stream_cork_cb : cb, userdata);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_disconnect(pa_stream * s)
{
    int retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_disconnect);
        retVal = orig(s);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/

const pa_channel_map * pa_stream_get_channel_map(pa_stream * s _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/

uint32_t pa_stream_get_index(pa_stream * s)
{
    uint32_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(stream_get_index);
        retVal = orig(s);
    }
    // FIXME
    DEBUG_MSG("%s returned %u", __func__, retVal);
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_get_latency(pa_stream * s _UNUSED, pa_usec_t * r_usec _UNUSED, int * negative _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return 0;
}

/*----------------------------------------------------------------------------*/

const pa_sample_spec * pa_stream_get_sample_spec(pa_stream * s)
{
    const pa_sample_spec * retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_get_sample_spec);
        retVal = orig(s);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_stream_state_t pa_stream_get_state(pa_stream * p)
{
    pa_stream_state_t retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_get_state);
        retVal = orig(p);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_get_time(pa_stream * s, pa_usec_t * r_usec)
{
    int retVal;
//    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_get_time);
        retVal = orig(s, r_usec);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_stream * pa_stream_new(pa_context * c, const char * name, const pa_sample_spec * ss, const pa_channel_map * map)
{
    pa_stream * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s(%s) called ", __func__, name);
    }
    if(UseRealContext) {
        GET_ORIGINAL(stream_new);
        retVal = orig(c, name, ss, map);
    } else {
        // FIXME
        retVal = NULL;
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_peek(pa_stream * p _UNUSED, const void **data _UNUSED, size_t *nbytes _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return 0;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_stream_notify_cb_t stream_state_cb_func = NULL;

static void stream_state_cb(pa_stream * s, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    stream_state_cb_func(s, userdata);
}

void pa_stream_set_state_callback(pa_stream * s _UNUSED, pa_stream_notify_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    stream_state_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(stream_set_state_callback);
        orig(s, cb ? stream_state_cb : cb, userdata);
    }
    // FIXME
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_stream_request_cb_t stream_write_cb_func = NULL;

static void stream_write_cb(pa_stream * p, size_t nbytes, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    stream_write_cb_func(p, nbytes, userdata);
}


void pa_stream_set_write_callback(pa_stream * p _UNUSED, pa_stream_request_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    stream_write_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(stream_set_write_callback);
        orig(p, cb ? stream_write_cb : cb, userdata);
    }
    // FIXME
}

/*----------------------------------------------------------------------------*/

void pa_stream_unref(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_unref);
        orig(s);
    }
    // FIXME
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_stream_success_cb_t stream_update_timing_cb_func = NULL;

static void stream_update_timing_cb(pa_stream * p, int t, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    stream_update_timing_cb_func(p, t, userdata);
}

pa_operation * pa_stream_update_timing_info(pa_stream * p, pa_stream_success_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    DEBUG_MSG("%s called ", __func__);
    stream_update_timing_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(stream_update_timing_info);
        retVal = orig(p, cb ? stream_update_timing_cb : cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_free_cb_t write_free_cb_func = NULL;

void write_free_cb(void * data)
{
    DEBUG_MSG("%s called ", __func__);
    write_free_cb_func(data);
}

int pa_stream_write(pa_stream * p,
        const void * data, size_t nbytes, pa_free_cb_t free_cb, off_t offset, pa_seek_mode_t seek)
{
    int retVal;
    write_free_cb_func = free_cb;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_write);
        retVal = orig(p, data, nbytes, free_cb ? write_free_cb : free_cb, offset, seek);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_volume_t pa_sw_volume_from_linear(double v)
{
    pa_volume_t retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(sw_volume_from_linear);
        retVal = orig(v);
    }
    // FIXME
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_free(pa_threaded_mainloop* m)
{
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_free);
        orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        delete reinterpret_cast<CThreadedMainloop *>(m);
#endif
    }
}

/*----------------------------------------------------------------------------*/

struct pa_mainloop_api * pa_mainloop_api_real = NULL;

pa_io_event* io_new_shim(pa_mainloop_api*a, int fd, pa_io_event_flags_t events, pa_io_event_cb_t cb, void *userdata)
{
    DEBUG_MSG("%s called ", __func__);
    (void) a;
    return pa_mainloop_api_real->io_new(pa_mainloop_api_real, fd, events, cb, userdata);
}
    
void io_enable_shim(pa_io_event* e, pa_io_event_flags_t events)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->io_enable(e, events);
}

void io_free_shim(pa_io_event* e)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->io_free(e);
}

void io_set_destroy_shim(pa_io_event *e, pa_io_event_destroy_cb_t cb)
{    
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->io_set_destroy(e, cb);
}

pa_time_event* time_new_shim(pa_mainloop_api*a, const struct timeval *tv, pa_time_event_cb_t cb, void *userdata)
{
    DEBUG_MSG("%s called ", __func__);
    (void) a;
    return pa_mainloop_api_real->time_new(pa_mainloop_api_real, tv, cb, userdata);
}
    
void time_restart_shim(pa_time_event* e, const struct timeval *tv)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->time_restart(e, tv);
}

void time_free_shim(pa_time_event* e)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->time_free(e);
}

void time_set_destroy_shim(pa_time_event *e, pa_time_event_destroy_cb_t cb)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->time_set_destroy(e, cb);
}

pa_defer_event * defer_new_shim(pa_mainloop_api * a, pa_defer_event_cb_t cb, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    (void) a;
    return pa_mainloop_api_real->defer_new(pa_mainloop_api_real, cb, userdata);
}

void defer_enable_shim(pa_defer_event* e, int b)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->defer_enable(e, b);
}
    
void defer_free_shim(pa_defer_event* e)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->defer_free(e);
}

void defer_set_destroy_shim(pa_defer_event *e, pa_defer_event_destroy_cb_t cb)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->defer_set_destroy(e, cb);
}

void quit_shim(pa_mainloop_api*a, int retval)
{
    DEBUG_MSG("%s called ", __func__);
    (void) a;
    pa_mainloop_api_real->quit(pa_mainloop_api_real, retval);
}


pa_mainloop_api * pa_threaded_mainloop_get_api(pa_threaded_mainloop * m)
{
    pa_mainloop_api * api;
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_get_api);
        api = orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        api = reinterpret_cast<CThreadedMainloop *>(m)->get_api();
#endif
    }

    if(TraceMainloopApi) {
        pa_mainloop_api_real = api;
        static pa_mainloop_api fake = {
            api->userdata,
            io_new_shim,
            io_enable_shim,
            io_free_shim,
            io_set_destroy_shim,
            time_new_shim,
            time_restart_shim,
            time_free_shim,
            time_set_destroy_shim,
            defer_new_shim,
            defer_enable_shim,
            defer_free_shim,
            defer_set_destroy_shim,
            quit_shim
        };
        api = &fake;
    }
    return api;
}

/*----------------------------------------------------------------------------*/
int pa_threaded_mainloop_in_thread(pa_threaded_mainloop * m)
{
    int retVal;

    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_in_thread);
        retVal = orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        retVal = reinterpret_cast<CThreadedMainloop *>(m)->in_thread();
#endif
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_lock(pa_threaded_mainloop * m)
{
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_lock);
        orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        reinterpret_cast<CThreadedMainloop *>(m)->lock();
#endif
    }
}

/*----------------------------------------------------------------------------*/
pa_threaded_mainloop * pa_threaded_mainloop_new(void)
{
    pa_threaded_mainloop * retVal;
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_new);
        retVal = orig();
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        retVal = reinterpret_cast<pa_threaded_mainloop *>(new CThreadedMainloop());
#endif
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_signal(pa_threaded_mainloop * m, int wait_for_accept)
{
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s(%i) called ", __func__, wait_for_accept);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_signal);
        orig(m, wait_for_accept);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        reinterpret_cast<CThreadedMainloop *>(m)->signal(wait_for_accept);
#endif
    }
}

/*----------------------------------------------------------------------------*/
int pa_threaded_mainloop_start(pa_threaded_mainloop * m)
{
    int retVal;
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_start);
        retVal = orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        retVal = reinterpret_cast<CThreadedMainloop *>(m)->start();
#endif
    }
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s returned %i ", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_stop(pa_threaded_mainloop *m)
{
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_stop);
        orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        reinterpret_cast<CThreadedMainloop *>(m)->stop();
#endif
    }
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_unlock(pa_threaded_mainloop *m)
{
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_unlock);
        orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        reinterpret_cast<CThreadedMainloop *>(m)->unlock();
#endif
    }
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_wait(pa_threaded_mainloop * m)
{
    if(TraceThreadedMainLoop) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealThreadedMainloop) {
        GET_ORIGINAL(threaded_mainloop_wait);
        orig(m);
#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
    } else {
        reinterpret_cast<CThreadedMainloop *>(m)->wait();
#endif
    }
}

/*----------------------------------------------------------------------------*/
size_t pa_usec_to_bytes(pa_usec_t t _UNUSED, const pa_sample_spec *spec _UNUSED)
{
    size_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(usec_to_bytes);
        retVal = orig(t, spec);
    }
    // FIXME
//    DEBUG_MSG("%s(%lu) called, returned %lu", __func__, t, retVal);
    return retVal;
}

void pa_stream_set_read_callback(pa_stream * p _UNUSED, pa_stream_request_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
}

int pa_stream_connect_record(pa_stream * s _UNUSED,
        const char *dev _UNUSED, const pa_buffer_attr *attr _UNUSED, pa_stream_flags_t flags _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return 0;
}

size_t pa_stream_readable_size(pa_stream * p _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return 0;
}

size_t pa_stream_writable_size(pa_stream * p)
{
    size_t retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_writable_size);
        retVal = orig(p);
    }
    // FIXME
    return retVal;
}

int pa_stream_drop(pa_stream * p _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return 0;
}

const pa_buffer_attr * pa_stream_get_buffer_attr(pa_stream * s _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

const char * pa_stream_get_device_name(pa_stream * s _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_context_subscribe_cb_t subscribe_event_cb_func = NULL;

static void context_subscribe_event_cb(pa_context *c, pa_subscription_event_type_t event, uint32_t x, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    subscribe_event_cb_func(c, event, x, userdata);
}

void pa_context_set_subscribe_callback(pa_context * c, pa_context_subscribe_cb_t cb, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    subscribe_event_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(context_set_subscribe_callback);
        orig(c, cb ? context_subscribe_event_cb : cb, userdata);
    } else {
        reinterpret_cast<CContext *>(c)->set_subscribe_callback(cb, userdata);
    }
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_context_success_cb_t context_subscribe_cb_func = NULL;

static void context_subscribe_cb(pa_context * c, int v, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_subscribe_cb_func(c, v, userdata);
}


pa_operation * pa_context_subscribe(pa_context * c,
        pa_subscription_mask_t m, pa_context_success_cb_t cb, void *userdata)
{
    pa_operation * retVal;
    DEBUG_MSG("%s called ", __func__);
    DEBUG_MSG("%s", subscription_mask2str(m));
    context_subscribe_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(context_subscribe);
        retVal = orig(c, m, cb ? context_subscribe_cb : cb, userdata);
    } else {
        retVal = reinterpret_cast<CContext *>(c)->subscribe(m, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

void (*callback_func)(pa_mainloop_api*, void *) = NULL;

static void callback_cb(pa_mainloop_api *m, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    callback_func(m, userdata);
}

void pa_mainloop_api_once(pa_mainloop_api * m,
        void (*callback)(pa_mainloop_api* m, void * userdata), void * userdata)
{
    if(TraceMainloopApi) {
        DEBUG_MSG("%s called ", __func__);
        callback_func = callback;
        callback = callback ? callback_cb : callback;
    }

    if(UseRealMainloopApi) {
        GET_ORIGINAL(mainloop_api_once);
        orig(m, callback, userdata);
    } else {
        CBlob * blob = new CBlob(callback, userdata);
        pa_defer_event * evt = m->defer_new(m, blob->callback, blob);
        m->defer_set_destroy(evt, blob->free);
    }
}

