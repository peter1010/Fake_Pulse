/**
 *
 * copyright (c) 2017 peter leese
 *
 * licensed under the gpl license. see license file in the project root for full license information.  
 */

#include "logging.h"
#include "pulseaudio.h"
#include "defs.h"
#include "originals.h"
#include <stdlib.h>

#include "threaded_mainloop.hpp"
#include "context.hpp"
#include "blob.hpp"
#include "stream.hpp"
#include "operation.hpp"

#define _UNUSED __attribute__((unused))

void init_symbols(void);

#define GET_ORIGINAL(name) FP_##name orig = zz_##name


#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP
bool UseRealThreadedMainloop = false;
#else
#define UseRealThreadedMainloop true
#endif
bool UseRealMainloopApi = true;
bool UseRealPulse = false;
bool UseRealUtilities = true;


bool TraceThreadedMainLoop = false;
bool TraceMainloopApi = false;
bool TraceContext = true;
bool TraceStream = true;
bool TraceUtilities = true;
bool TraceOperation = false;

/*----------------------------------------------------------------------------*/
const char * pa_get_library_version(void)
{
    const char * retVal;
    if(UseRealUtilities) {
        GET_ORIGINAL(get_library_version);
        retVal = orig();
    } else {
        retVal = "11.1.0";
    }
    if(TraceUtilities) {
        DEBUG_MSG("%s() returned %s", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
int pa_channel_map_can_balance(const pa_channel_map * p)
{
    int retVal;
    if(UseRealUtilities) {
        GET_ORIGINAL(channel_map_can_balance);
        retVal = orig(p);
    } else {
        retVal = 0;  /* Means no balance */
        // FIXME
    }
    if(TraceUtilities) {
        DEBUG_MSG("%s returned %i", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
pa_channel_map * pa_channel_map_init(pa_channel_map * p)
{
    pa_channel_map * retVal;
    if(TraceUtilities) {
        DEBUG_MSG("%s called", __func__);
    }
    if(UseRealUtilities) {
        GET_ORIGINAL(channel_map_init);
        retVal = orig(p);
    } else {
        /* Lets always have simple stereo */
        retVal = p;
        p->channels = 2;
        p->map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        p->map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
        // FIXME
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

    if(UseRealPulse) {
        GET_ORIGINAL(context_connect);
        retVal = orig(c, server, flags, api);
    } else {
        retVal = CContext::from_pa(c)->connect(server, flags, api);
    }
    if(TraceContext) {
        DEBUG_MSG("%s returned %i", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_context_disconnect(pa_context * c)
{
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_disconnect);
        orig(c);
    } else {
        CContext::from_pa(c)->disconnect();
    }
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_context_notify_cb_t context_drain_cb_func;
    
static void context_drain_cb(pa_context * c, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_drain_cb_func(c, userdata);
}

pa_operation * pa_context_drain(pa_context * c, pa_context_notify_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        if(cb) {
            context_drain_cb_func = cb;
            cb = context_drain_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_drain);
        retVal = orig(c, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->drain(cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_server_info_cb_t context_get_server_info_cb_func;

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
        if(cb) {
            context_get_server_info_cb_func = cb;
            cb = context_get_server_info_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_get_server_info);
        retVal = orig(c, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->get_server_info(cb, userdata);
    }
    return retVal;

}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_sink_info_cb_t context_get_sink_info_by_name_cb_func;

static void context_get_sink_info_by_name_cb(pa_context *c, const pa_sink_info * info, int eol, void * userdata)
{
    DEBUG_MSG("%s(%i) called ", __func__, eol);
    log_sink_info(info);
    context_get_sink_info_by_name_cb_func(c, info, eol, userdata);
}

pa_operation * pa_context_get_sink_info_by_name(pa_context * c,
        const char * name, pa_sink_info_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s(%s) called ", __func__, name);
        if(cb) {
            context_get_sink_info_by_name_cb_func = cb;
            cb = context_get_sink_info_by_name_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_get_sink_info_by_name);
        retVal = orig(c, name, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->get_sink_info_by_name(name, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_sink_info_cb_t context_get_sink_info_list_cb_func;

static void context_get_sink_info_list_cb(pa_context *c, const pa_sink_info * info, int eol, void * userdata)
{
    DEBUG_MSG("%s(%i) called ", __func__, eol);
    log_sink_info(info);
    context_get_sink_info_list_cb_func(c, info, eol, userdata);
}


pa_operation * pa_context_get_sink_info_list(pa_context * c,
        pa_sink_info_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s() called ", __func__);
        if(cb) {
            context_get_sink_info_list_cb_func = cb;
            cb = context_get_sink_info_list_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_get_sink_info_list);
        retVal = orig(c, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->get_sink_info_list(cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

static pa_sink_input_info_cb_t context_get_sink_input_info_cb_func;

static void context_get_sink_input_info_cb(pa_context *c, const pa_sink_input_info * info, int eol, void * userdata)
{
    DEBUG_MSG("%s(%i) called ", __func__, eol);
    context_get_sink_input_info_cb_func(c, info, eol, userdata);
}


pa_operation * pa_context_get_sink_input_info(pa_context * c,
        uint32_t idx, pa_sink_input_info_cb_t cb, void *userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s(%u) called ", __func__, idx);
        if(cb) {
            context_get_sink_input_info_cb_func = cb;
            cb = context_get_sink_input_info_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_get_sink_input_info);
        retVal = orig(c, idx, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->get_sink_input_info(idx, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

static pa_source_info_cb_t context_get_source_info_list_cb_func;

static void context_get_source_info_list_cb(pa_context *c, const pa_source_info * info, int eol, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_get_source_info_list_cb_func(c, info, eol, userdata);
}

pa_operation * pa_context_get_source_info_list(pa_context *c,
        pa_source_info_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        if(cb) {
            context_get_source_info_list_cb_func = cb;
            cb = context_get_source_info_list_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_get_source_info_list);
        retVal = orig(c, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->get_source_info_list(cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_context_state_t pa_context_get_state(pa_context * c)
{
    pa_context_state_t retVal;
    if(UseRealPulse) {
        GET_ORIGINAL(context_get_state);
        retVal = orig(c);
    } else {
        retVal = CContext::from_pa(c)->get_state();
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
    if(UseRealPulse) {
        GET_ORIGINAL(context_new);
        c = orig(mainloop, name);
    } else {
        c = (new CContext(mainloop, name))->to_pa();
    }
    return c;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_time_event_cb_t time_event_cb_func;

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
        if(cb) {
            time_event_cb_func = cb;
            cb = time_event_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_rttime_new);
        retVal = orig(c, usec, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->rttime_new(usec, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_context_success_cb_t context_sink_input_volume_cb_func;

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
        if(cb) {
            context_sink_input_volume_cb_func = cb;
            cb = context_sink_input_volume_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_set_sink_input_volume);
        retVal = orig(c, idx, volume, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->set_sink_input_volume(idx, volume, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_context_notify_cb_t context_state_cb_func;

static void context_state_cb(pa_context *c, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_state_cb_func(c, userdata);
}

void pa_context_set_state_callback(pa_context * c, pa_context_notify_cb_t cb, void * userdata)
{
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        if(cb) {
            context_state_cb_func = cb;
            cb = context_state_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_set_state_callback);
        orig(c, cb, userdata);
    } else {
        CContext::from_pa(c)->set_state_callback(cb, userdata);
    }
}

#if 0
/*----------------------------------------------------------------------------*/

pa_context * pa_context_ref(pa_context * c)
{
    pa_context * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_ref);
        retVal = orig(c);
    } else {
        retVal = CContext::from_pa(c)->ref();
    }
    return retVal;
}
#endif

/*----------------------------------------------------------------------------*/

void pa_context_unref(pa_context * c)
{
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_unref);
        orig(c);
    } else {
        CContext::from_pa(c)->decRef();
    }
}

/*----------------------------------------------------------------------------*/

pa_cvolume * pa_cvolume_set(pa_cvolume * a, unsigned channels, pa_volume_t v)
{
    pa_cvolume * retVal;
    if(TraceUtilities) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealUtilities) {
        GET_ORIGINAL(cvolume_set);
        retVal = orig(a, channels, v);
    } else {
        retVal = NULL;
        // FIXME
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_cvolume * pa_cvolume_set_balance(pa_cvolume * v, const pa_channel_map * map, float new_balance)
{
    pa_cvolume * retVal;
    if(TraceUtilities) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealUtilities) {
        GET_ORIGINAL(cvolume_set_balance);
        retVal = orig(v, map, new_balance);
    } else {
        retVal = NULL;
        // FIXME
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

size_t pa_frame_size(const pa_sample_spec * spec)
{
    size_t retVal;
    if(UseRealUtilities) {
        GET_ORIGINAL(frame_size);
        retVal = orig(spec);
    } else {
        retVal = 0;
        // FIXME
    }
    if(TraceUtilities) {
        DEBUG_MSG("%s returned %lu ", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_operation_state_t pa_operation_get_state(pa_operation *o)
{
    pa_operation_state_t retVal;
    if(UseRealPulse) {
        GET_ORIGINAL(operation_get_state);
        retVal = orig(o);
    } else {
        retVal = COperation::from_pa(o)->get_state();
    }
    if(TraceOperation) {
        DEBUG_MSG("%s returned %s", __func__, operation_state2str(retVal));
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

void pa_operation_unref(pa_operation *o)
{
    if(TraceOperation) {
        DEBUG_MSG("%s(%p) called ", __func__, o);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(operation_unref);
        orig(o);
    } else {
        COperation::from_pa(o)->decRef();
    }
    if(TraceOperation) {
        DEBUG_MSG("%s returned ", __func__);
    }
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
    if(UseRealUtilities) {
        GET_ORIGINAL(rtclock_now);
        retVal = orig();
    } else {
        retVal = 0;
    }
    if(TraceUtilities) {
        DEBUG_MSG("%s returned %lu", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_begin_write(pa_stream * p, void ** data, size_t * nbytes)
{
    int retVal;
    size_t requestedBytes = *nbytes;
    if(UseRealPulse) {
        GET_ORIGINAL(stream_begin_write);
        retVal = orig(p, data, nbytes);
    } else {
        retVal = CStream::from_pa(p)->begin_write(data, nbytes);
    }
    if(TraceStream) {
        DEBUG_MSG("%s(%lu) returned %lu bytes ", __func__, requestedBytes, *nbytes);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_cancel_write(pa_stream * p)
{
    int retVal;
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_cancel_write);
        retVal = orig(p);
    } else {
        retVal = CStream::from_pa(p)->cancel_write();
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
int pa_stream_connect_playback(pa_stream * s,
        const char * dev, const pa_buffer_attr * attr, pa_stream_flags_t flags,
        const pa_cvolume *volume, pa_stream * sync_stream)
{
    int retVal;
    if(TraceStream) {
        DEBUG_MSG("%s(%s) called ", __func__, dev);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_connect_playback);
        retVal = orig(s, dev, attr, flags, volume, sync_stream);
    } else {
        retVal = CStream::from_pa(s)->connect_playback(dev, attr, flags, volume, sync_stream);
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
    if(TraceStream) {
        DEBUG_MSG("%s(%i) called ", __func__, b);
        if(cb) {
            stream_cork_cb_func = cb;
            cb = stream_cork_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_cork);
        retVal = orig(s, b, cb, userdata);
    } else {
        retVal = CStream::from_pa(s)->cork(b, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_disconnect(pa_stream * s)
{
    int retVal;
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_disconnect);
        retVal = orig(s);
    } else {
        retVal = CStream::from_pa(s)->disconnect();
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

const pa_channel_map * pa_stream_get_channel_map(pa_stream * s)
{
    const pa_channel_map * retVal;
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_get_channel_map);
        retVal = orig(s);
    } else {
        retVal = CStream::from_pa(s)->get_channel_map();
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

uint32_t pa_stream_get_index(pa_stream * s)
{
    uint32_t retVal;
    if(UseRealPulse) {
        GET_ORIGINAL(stream_get_index);
        retVal = orig(s);
    } else {
        retVal = CStream::from_pa(s)->get_index();
    }
    if(TraceStream) {
        DEBUG_MSG("%s returned %u", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_get_latency(pa_stream * s, pa_usec_t * r_usec, int * negative)
{
    int retVal;
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_get_latency);
        retVal = orig(s, r_usec, negative);
    } else {
        retVal = CStream::from_pa(s)->get_latency(r_usec, negative);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

const pa_sample_spec * pa_stream_get_sample_spec(pa_stream * s)
{
    const pa_sample_spec * retVal;
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_get_sample_spec);
        retVal = orig(s);
    } else {
        retVal = CStream::from_pa(s)->get_sample_spec();
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_stream_state_t pa_stream_get_state(pa_stream * p)
{
    pa_stream_state_t retVal;
    if(UseRealPulse) {
        GET_ORIGINAL(stream_get_state);
        retVal = orig(p);
    } else {
        retVal = CStream::from_pa(p)->get_state();
    }
    if(TraceStream) {
        DEBUG_MSG("%s returned %s", __func__, stream_state2str(retVal));
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_get_time(pa_stream * s, pa_usec_t * r_usec)
{
    int retVal;
    if(UseRealPulse) {
        GET_ORIGINAL(stream_get_time);
        retVal = orig(s, r_usec);
    } else {
        retVal = CStream::from_pa(s)->get_time(r_usec);
    }
    if(TraceStream) {
        DEBUG_MSG("%s returned %lu us", __func__, *r_usec);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_stream * pa_stream_new(pa_context * c, const char * name, const pa_sample_spec * ss, const pa_channel_map * map)
{
    pa_stream * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s(%s) called ", __func__, name);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_new);
        retVal = orig(c, name, ss, map);
    } else {
        retVal = (new CStream(CContext::from_pa(c), name, ss, map))->to_pa();
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

int pa_stream_peek(pa_stream * p, const void ** data, size_t * nbytes)
{
    int retVal;
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_peek);
        retVal = orig(p, data, nbytes);
    } else {
        retVal = CStream::from_pa(p)->peek(data, nbytes);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_stream_notify_cb_t stream_state_cb_func;

static void stream_state_cb(pa_stream * s, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    stream_state_cb_func(s, userdata);
}

void pa_stream_set_state_callback(pa_stream * s, pa_stream_notify_cb_t cb, void * userdata)
{
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
        if(cb) {
            stream_state_cb_func = cb;
            cb = stream_state_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_set_state_callback);
        orig(s, cb, userdata);
    } else {
        CStream::from_pa(s)->set_state_callback(cb, userdata);
    }
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_stream_request_cb_t stream_write_cb_func;

static void stream_write_cb(pa_stream * p, size_t nbytes, void * userdata)
{
    DEBUG_MSG("%s(%lu) called ", __func__, nbytes);
    stream_write_cb_func(p, nbytes, userdata);
}


void pa_stream_set_write_callback(pa_stream * p, pa_stream_request_cb_t cb, void * userdata)
{
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
        if(cb) {
            stream_write_cb_func = cb;
            cb = stream_write_cb ;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_set_write_callback);
        orig(p, cb, userdata);
    } else {
        CStream::from_pa(p)->set_write_callback(cb, userdata);
    }
}

/*----------------------------------------------------------------------------*/

void pa_stream_unref(pa_stream * s)
{
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_unref);
        orig(s);
    } else {
        CStream::from_pa(s)->decRef();
    }
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_stream_success_cb_t stream_update_timing_cb_func;

static void stream_update_timing_cb(pa_stream * p, int t, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    stream_update_timing_cb_func(p, t, userdata);
}

pa_operation * pa_stream_update_timing_info(pa_stream * p, pa_stream_success_cb_t cb, void * userdata)
{
    pa_operation * retVal;
    if(TraceStream) {
        DEBUG_MSG("%s called ", __func__);
        if(cb) {
            stream_update_timing_cb_func = cb;
            cb = stream_update_timing_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_update_timing_info);
        retVal = orig(p, cb, userdata);
    } else {
        retVal = CStream::from_pa(p)->update_timing_info(cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_free_cb_t write_free_cb_func;

static void write_free_cb(void * data)
{
    DEBUG_MSG("%s called ", __func__);
    write_free_cb_func(data);
}

int pa_stream_write(pa_stream * p,
        const void * data, size_t nbytes, pa_free_cb_t free_cb, off_t offset, pa_seek_mode_t seek)
{
    int retVal;
    if(TraceStream) {
        DEBUG_MSG("%s(%lu) called ", __func__, nbytes);
        if(free_cb) {
            write_free_cb_func = free_cb;
            free_cb = write_free_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(stream_write);
        retVal = orig(p, data, nbytes, free_cb, offset, seek);
    } else {
        retVal = CStream::from_pa(p)->write(data, nbytes, free_cb, offset, seek);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_volume_t pa_sw_volume_from_linear(double v)
{
    pa_volume_t retVal;
    if(TraceUtilities) {
        DEBUG_MSG("%s called ", __func__);
    }
    if(UseRealUtilities) {
        GET_ORIGINAL(sw_volume_from_linear);
        retVal = orig(v);
    } else {
        retVal = 0;
        // FIXME
    }
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

static struct pa_mainloop_api * pa_mainloop_api_real;

static pa_io_event* io_new_shim(pa_mainloop_api*a, int fd, pa_io_event_flags_t events, pa_io_event_cb_t cb, void *userdata)
{
    DEBUG_MSG("%s called ", __func__);
    (void) a;
    return pa_mainloop_api_real->io_new(pa_mainloop_api_real, fd, events, cb, userdata);
}
    
static void io_enable_shim(pa_io_event* e, pa_io_event_flags_t events)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->io_enable(e, events);
}

static void io_free_shim(pa_io_event* e)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->io_free(e);
}

static void io_set_destroy_shim(pa_io_event *e, pa_io_event_destroy_cb_t cb)
{    
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->io_set_destroy(e, cb);
}

static pa_time_event* time_new_shim(pa_mainloop_api*a, const struct timeval *tv, pa_time_event_cb_t cb, void *userdata)
{
    DEBUG_MSG("%s called ", __func__);
    (void) a;
    return pa_mainloop_api_real->time_new(pa_mainloop_api_real, tv, cb, userdata);
}
    
static void time_restart_shim(pa_time_event* e, const struct timeval *tv)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->time_restart(e, tv);
}

static void time_free_shim(pa_time_event* e)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->time_free(e);
}

static void time_set_destroy_shim(pa_time_event *e, pa_time_event_destroy_cb_t cb)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->time_set_destroy(e, cb);
}

static pa_defer_event * defer_new_shim(pa_mainloop_api * a, pa_defer_event_cb_t cb, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    (void) a;
    return pa_mainloop_api_real->defer_new(pa_mainloop_api_real, cb, userdata);
}

static void defer_enable_shim(pa_defer_event* e, int b)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->defer_enable(e, b);
}
    
static void defer_free_shim(pa_defer_event* e)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->defer_free(e);
}

static void defer_set_destroy_shim(pa_defer_event *e, pa_defer_event_destroy_cb_t cb)
{
    DEBUG_MSG("%s called ", __func__);
    pa_mainloop_api_real->defer_set_destroy(e, cb);
}

static void quit_shim(pa_mainloop_api*a, int retval)
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
size_t pa_usec_to_bytes(pa_usec_t t, const pa_sample_spec *spec)
{
    size_t retVal;
    if(UseRealUtilities) {
        GET_ORIGINAL(usec_to_bytes);
        retVal = orig(t, spec);
    } else {
        retVal = 0;
        // FIXME
    }
    if(TraceUtilities) {
        DEBUG_MSG("%s(%lu) called, returned %lu", __func__, t, retVal);
    }
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

/*----------------------------------------------------------------------------*/
size_t pa_stream_writable_size(pa_stream * p)
{
    size_t retVal;
    if(UseRealPulse) {
        GET_ORIGINAL(stream_writable_size);
        retVal = orig(p);
    } else {
        retVal = CStream::from_pa(p)->writable_size();
    }
    if(TraceStream) {
        DEBUG_MSG("%s returned %lu", __func__, retVal);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
int pa_stream_drop(pa_stream * p _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return 0;
}

/*----------------------------------------------------------------------------*/
const pa_buffer_attr * pa_stream_get_buffer_attr(pa_stream * s _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/
const char * pa_stream_get_device_name(pa_stream * s _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

static pa_context_subscribe_cb_t subscribe_event_cb_func;

static void context_subscribe_event_cb(pa_context *c, pa_subscription_event_type_t event, uint32_t x, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    subscribe_event_cb_func(c, event, x, userdata);
}

void pa_context_set_subscribe_callback(pa_context * c, pa_context_subscribe_cb_t cb, void * userdata)
{
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        if(cb) {
            subscribe_event_cb_func = cb;
            cb = context_subscribe_event_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_set_subscribe_callback);
        orig(c, cb ? context_subscribe_event_cb : cb, userdata);
    } else {
        CContext::from_pa(c)->set_subscribe_callback(cb, userdata);
    }
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static pa_context_success_cb_t context_subscribe_cb_func;

static void context_subscribe_cb(pa_context * c, int v, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_subscribe_cb_func(c, v, userdata);
}


pa_operation * pa_context_subscribe(pa_context * c,
        pa_subscription_mask_t m, pa_context_success_cb_t cb, void *userdata)
{
    pa_operation * retVal;
    if(TraceContext) {
        DEBUG_MSG("%s called ", __func__);
        DEBUG_MSG("%s", subscription_mask2str(m));
        if(cb) {
            context_subscribe_cb_func = cb;
            cb = context_subscribe_cb;
        }
    }
    if(UseRealPulse) {
        GET_ORIGINAL(context_subscribe);
        retVal = orig(c, m, cb, userdata);
    } else {
        retVal = CContext::from_pa(c)->subscribe(m, cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */

void (*callback_func)(pa_mainloop_api*, void *);

static void api_once_callback_cb(pa_mainloop_api *m, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    callback_func(m, userdata);
}

void pa_mainloop_api_once(pa_mainloop_api * m,
        void (*callback)(pa_mainloop_api* m, void * userdata), void * userdata)
{
    if(TraceMainloopApi) {
        DEBUG_MSG("%s called ", __func__);
        if(callback) {
            callback_func = callback;
            callback = api_once_callback_cb;
        }
    }

    if(UseRealMainloopApi) {
        GET_ORIGINAL(mainloop_api_once);
        orig(m, callback, userdata);
    } else {
        CBlob * blob = new CMainloopApiOnce(callback, userdata);
        pa_defer_event * evt = m->defer_new(m, blob->callback, blob);
        m->defer_set_destroy(evt, blob->free);
    }
}

