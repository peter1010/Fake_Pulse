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
#include "threaded_mainloop.hpp"

#define _UNUSED __attribute__((unused))

pa_context_notify_cb_t context_state_cb_func = NULL;
void * context_state_cb_data = NULL;
 
pa_context_subscribe_cb_t subscribe_cb = NULL;
void * subscribe_data = 0;

pa_operation * op;


int s_context =  2;
pa_context * context = (pa_context *) &s_context;

void init_symbols(void);

#define GET_ORIGINAL(name) FP_##name orig = zz_##name
#define ListenMode 1

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
// FIXME
int pa_context_connect(pa_context * c, const char * server,
        pa_context_flags_t flags, const pa_spawn_api * api)
{
    int retVal;
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

    if(ListenMode) {
        GET_ORIGINAL(context_connect);
        retVal = orig(c, server, flags, api);
    } else {
        if(api) {
            if(api->prefork) {
                DEBUG_MSG("Calling prefork");
                api->prefork();
            }
            if(api->atfork) {
                DEBUG_MSG("Calling atfork");
                api->atfork();
            }
            if(api->postfork) {
                DEBUG_MSG("Calling postfork");
                api->postfork();
            }
        }
        if(context_state_cb_func) {
            DEBUG_MSG("Calling set context state callback");
            context_state_cb_func(c, context_state_cb_data);
        }
        retVal = 0;
    }
    DEBUG_MSG("%s returned %i", __func__, retVal);
    return retVal;
}

/*----------------------------------------------------------------------------*/
// FIXME
void pa_context_disconnect(pa_context * c _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
}

/*----------------------------------------------------------------------------*/
// FIXME
pa_operation * pa_context_drain(pa_context * c _UNUSED, pa_context_notify_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static pa_server_info_cb_t context_get_server_info_cb_func = NULL;

static void context_get_server_info_cb(pa_context *c, const pa_server_info * info, void *userdata)
{
    DEBUG_MSG("%s called ", __func__);
    log_server_info(info);
    context_get_server_info_cb_func(c, info, userdata);
}

pa_operation * pa_context_get_server_info(pa_context *c _UNUSED, pa_server_info_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    pa_operation * retVal;
    DEBUG_MSG("%s called ", __func__);
    context_get_server_info_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(context_get_server_info);
        retVal = orig(c, cb ? context_get_server_info_cb : cb, userdata);
    } else {
        static pa_server_info info;

        info.user_name = "bob";
        info.host_name = "cobblers";              /**< Host name the daemon is running on */
        info.server_version = "11.1.0";         /**< Version string of the daemon */
        info.server_name = "pulseaudio";

        info.sample_spec.format = PA_SAMPLE_S16LE;
        info.sample_spec.rate = 44100;
        info.sample_spec.channels = 2;

        info.default_sink_name = "sink";
        info.default_source_name = "source";
        info.cookie = 3;                    /**< A random cookie for identifying this instance of PulseAudio. */
        info.channel_map.channels =2;
        info.channel_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        info.channel_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
        if(cb) {
            cb(c, &info, userdata);
        }
        retVal = op;
    }
    return retVal;

}

/*----------------------------------------------------------------------------*/

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
    DEBUG_MSG("%s(%s) called ", __func__, name);
    context_get_sink_info_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(context_get_sink_info_by_name);
        retVal = orig(c, name, cb ? context_get_sink_info_cb : cb, userdata);
    } else {
 
        static pa_sink_info info;
        static pa_format_info format = {PA_ENCODING_PCM, NULL};
        static pa_format_info * formats[1] = {&format};

        info.name = name;
        info.index = 1;
        info.description = "output";
        info.sample_spec.format = PA_SAMPLE_S16LE;
        info.sample_spec.rate = 44100;
        info.sample_spec.channels = 2;
        info.channel_map.channels =2;
        info.channel_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
        info.channel_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
        info.owner_module = 1;
        info.volume.channels = 2;
        info.volume.values[0] = 100;
        info.volume.values[1] = 100;
        info.mute = 0;
        info.monitor_source = 0;
        info.monitor_source_name = "";
        info.latency = 1000;
        info.driver = "alsa";
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


        if(cb) {
            cb(c, &info, 0, userdata);
            cb(c, NULL, 1, userdata);
        }
        retVal = op;
    }
    return retVal;
}


pa_operation * pa_context_get_sink_info_list(pa_context * c _UNUSED,
        pa_sink_info_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

pa_operation * pa_context_get_sink_input_info(pa_context * c _UNUSED,
        uint32_t idx _UNUSED, pa_sink_input_info_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("TODO %s(%i) called ", __func__, idx);
    return NULL;
}

pa_operation * pa_context_get_source_info_list(pa_context *ci _UNUSED,
        pa_source_info_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("TODO %s called ", __func__);
    return NULL;
}

pa_context_state_t pa_context_get_state(pa_context * c)
{
    pa_context_state_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(context_get_state);
        retVal = orig(c);
    } else {
        retVal = PA_CONTEXT_READY;
    }
    DEBUG_MSG("%s returned %s", __func__, context_state2str(retVal));
    return retVal;
}

pa_context * pa_context_new(pa_mainloop_api * mainloop, const char * name)
{
    pa_context * c;
    DEBUG_MSG("%s(%s) called ", __func__, name ? name : "None");
    if(ListenMode) {
        GET_ORIGINAL(context_new);
        c = orig(mainloop, name);
    } else {
        c = context;
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
    time_event_cb_func = cb;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(context_rttime_new);
        retVal = orig(c, usec, cb ? time_event_cb : cb, userdata);
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
    DEBUG_MSG("%s called ", __func__);
    context_sink_input_volume_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(context_set_sink_input_volume);
        retVal = orig(c, idx, volume, cb ? context_sink_input_volume_cb : cb, userdata);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
/**
 * Intercept the callback! 
 */
static void context_state_cb(pa_context *c, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_state_cb_func(c, userdata);
}

void pa_context_set_state_callback(pa_context * c, pa_context_notify_cb_t cb, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_state_cb_func = cb;
    context_state_cb_data = userdata;

    if(ListenMode) {
        GET_ORIGINAL(context_set_state_callback);
        orig(c, cb ? context_state_cb : cb, userdata);
    }
}

/*----------------------------------------------------------------------------*/

void pa_context_unref(pa_context * c)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(context_unref);
        orig(c);
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
    return retVal;
}

/*----------------------------------------------------------------------------*/

pa_stream * pa_stream_new(pa_context * c, const char * name, const pa_sample_spec * ss, const pa_channel_map * map)
{
    pa_stream * retVal;
    DEBUG_MSG("%s(%s) called ", __func__, name);
    if(ListenMode) {
        GET_ORIGINAL(stream_new);
        retVal = orig(c, name, ss, map);
    } else {
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
}

/*----------------------------------------------------------------------------*/

void pa_stream_unref(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(stream_unref);
        orig(s);
    }
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
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_free(pa_threaded_mainloop* m)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_free);
        orig(m);
    } else {
        my_threaded_mainloop_free(m);
    }
}

/*----------------------------------------------------------------------------*/
pa_mainloop_api * pa_threaded_mainloop_get_api(pa_threaded_mainloop * m)
{
    pa_mainloop_api * api;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_get_api);
        api = orig(m);
    } else {
        api = my_threaded_mainloop_get_api(m);
    }
    return api;
}

/*----------------------------------------------------------------------------*/
int pa_threaded_mainloop_in_thread(pa_threaded_mainloop * m)
{
    int retVal;
//    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_in_thread);
        retVal = orig(m);
    } else {
        retVal = my_threaded_mainloop_in_thread(m);
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_lock(pa_threaded_mainloop * m)
{
//    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_lock);
        orig(m);
    } else {
        my_threaded_mainloop_lock(m);
    }
}

/*----------------------------------------------------------------------------*/
pa_threaded_mainloop * pa_threaded_mainloop_new(void)
{
    pa_threaded_mainloop * retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_new);
        retVal = orig();
    } else {
        retVal = my_threaded_mainloop_new();
    }
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_signal(pa_threaded_mainloop * m, int wait_for_accept)
{
    DEBUG_MSG("%s(%i) called ", __func__, wait_for_accept);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_signal);
        orig(m, wait_for_accept);
    } else {
        // FIXME
    }
}

/*----------------------------------------------------------------------------*/
int pa_threaded_mainloop_start(pa_threaded_mainloop * m)
{
    int retVal;
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_start);
        retVal = orig(m);
    } else {
        retVal = my_threaded_mainloop_start(m);
    }
    DEBUG_MSG("%s returned %i ", __func__, retVal);
    return retVal;
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_stop(pa_threaded_mainloop *m)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_stop);
        orig(m);
    } else {
        my_threaded_mainloop_stop(m);
    }
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_unlock(pa_threaded_mainloop *m _UNUSED)
{
//    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_unlock);
        orig(m);
    } else {
        my_threaded_mainloop_unlock(m);
    }
}

/*----------------------------------------------------------------------------*/
void pa_threaded_mainloop_wait(pa_threaded_mainloop * m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_wait);
        orig(m);
    }
    // FIXME
}

size_t pa_usec_to_bytes(pa_usec_t t _UNUSED, const pa_sample_spec *spec _UNUSED)
{
    size_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(usec_to_bytes);
        retVal = orig(t, spec);
    }
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

void pa_context_set_subscribe_callback(pa_context * c _UNUSED, pa_context_subscribe_cb_t cb, void * userdata)
{
    subscribe_cb = cb;
    subscribe_data = userdata;
    DEBUG_MSG("%s called ", __func__);
}

pa_operation * pa_context_subscribe(pa_context * c _UNUSED,
        pa_subscription_mask_t m _UNUSED, pa_context_success_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    DEBUG_MSG("%s", subscription_mask2str(m));
//    if(cb) {
//        cb(c, m, userdata);
//    }
    return op;
}

void (*callback_func)(pa_mainloop_api*, void *) = NULL;

static void callback_cb(pa_mainloop_api *m, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    callback_func(m, userdata);
}

void pa_mainloop_api_once(pa_mainloop_api * m,
        void (*callback)(pa_mainloop_api* m, void * userdata), void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    callback_func = callback;

    if(ListenMode) {
        GET_ORIGINAL(mainloop_api_once);
        orig(m, callback ? callback_cb : callback, userdata);
    }
}

