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

#define _UNUSED __attribute__((unused))

pa_context_notify_cb_t context_set_state_cb_func = NULL;
void * context_set_state_cb_data = NULL;

pa_context_subscribe_cb_t subscribe_cb = NULL;
void * subscribe_data = 0;

pa_operation * op;

int s_mainloop = 1;
pa_threaded_mainloop * mainloop = (pa_threaded_mainloop *) &s_mainloop;

pa_mainloop_api mainloop_api = {0};

int s_context =  2;
pa_context * context = (pa_context *) &s_context;

#define GET_ORIGINAL(name) FP_##name orig = zz_##name
#define ListenMode 1

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

int pa_channel_map_can_balance(const pa_channel_map * p _UNUSED)
{
    int retVal;
    if(ListenMode) {
        GET_ORIGINAL(channel_map_can_balance);
        retVal = orig(p);
    } else {
        retVal = 0;
    }
    DEBUG_MSG("%s returned %i", __func__, retVal);
    return retVal;
}

pa_channel_map * pa_channel_map_init(pa_channel_map * p)
{
    DEBUG_MSG("%s called", __func__);
    p->channels = 2;
    p->map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
    p->map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;

    return p;
}

int pa_context_connect(pa_context * c _UNUSED, const char * server,
        pa_context_flags_t flags _UNUSED, const pa_spawn_api * api)
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
        if(context_set_state_cb_func) {
            DEBUG_MSG("Calling set context state callback");
            context_set_state_cb_func(c, context_set_state_cb_data);
        }
        retVal = 0;
    }
    DEBUG_MSG("%s returned %i", __func__, retVal);
    return retVal;
}

void pa_context_disconnect(pa_context * c _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

pa_operation * pa_context_drain(pa_context * c _UNUSED, pa_context_notify_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

static pa_server_info_cb_t context_get_server_info_cb_func = NULL;

void context_get_server_info_cb(pa_context *c, const pa_server_info * info, void *userdata)
{
    DEBUG_MSG("%s called ", __func__);
    DEBUG_MSG("user_name = %s", info->user_name);
    DEBUG_MSG("host_name = %s", info->host_name);
    DEBUG_MSG("server_version = %s", info->server_version);
    DEBUG_MSG("server_name = %s", info->server_name);

    context_get_server_info_cb_func(c, info, userdata);
}

pa_operation * pa_context_get_server_info(pa_context *c _UNUSED, pa_server_info_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    pa_operation * retVal;
    DEBUG_MSG("%s called ", __func__);
    context_get_server_info_cb_func = cb;
    if(ListenMode) {
        GET_ORIGINAL(context_get_server_info);
        if(cb) {
            retVal = orig(c, context_get_server_info_cb, userdata);
        } else {
            retVal = orig(c, NULL, userdata);
        }
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
        info.channel_map.map[0] = 1;         /**< Default channel map. \since 0.9.15 */
        info.channel_map.map[1] = 2;
        if(cb) {
            cb(c, &info, userdata);
        }
        retVal = op;
    }
    return retVal;

}

/** Get information about a sink by its name */
pa_operation * pa_context_get_sink_info_by_name(pa_context * c,
        const char * name, pa_sink_info_cb_t cb, void * userdata)
{
    static pa_sink_info info;
    static pa_format_info format = {PA_ENCODING_PCM, NULL};
    static pa_format_info * formats[1] = {&format};

    info.name = "sink";
    info.index = 1;
    info.description = "output";
    info.sample_spec.format = PA_SAMPLE_S16LE;
    info.sample_spec.rate = 44100;
    info.sample_spec.channels = 2;
    info.channel_map.channels =2;
    info.channel_map.map[0] = 1;
    info.channel_map.map[1] = 2;
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


    DEBUG_MSG("%s(%s) called ", __func__, name);
    if(cb) {
        cb(c, &info, PA_OK, userdata);
    }
    return op;
}


pa_operation * pa_context_get_sink_info_list(pa_context * c _UNUSED,
        pa_sink_info_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

pa_operation * pa_context_get_sink_input_info(pa_context * c _UNUSED,
        uint32_t idx _UNUSED, pa_sink_input_info_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

pa_operation * pa_context_get_source_info_list(pa_context *ci _UNUSED,
        pa_source_info_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}


pa_context_state_t pa_context_get_state(pa_context *c _UNUSED)
{
    pa_context_state_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(context_get_state);
        retVal = orig(c);
    } else {
        retVal = PA_CONTEXT_READY;
    }
    DEBUG_MSG("%s returned %i", __func__, retVal);
    return retVal;
}

pa_context * pa_context_new(pa_mainloop_api * mainloop _UNUSED, const char * name _UNUSED)
{
    pa_context * c;
    DEBUG_MSG("%s(%s) called ", __func__, name);
    if(ListenMode) {
        GET_ORIGINAL(context_new);
        c = orig(mainloop, name);
    } else {
        c = context;
    }
    return c;
}

pa_time_event * pa_context_rttime_new(pa_context * c _UNUSED,
        pa_usec_t usec _UNUSED, pa_time_event_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

pa_operation * pa_context_set_sink_input_volume(pa_context * c _UNUSED,
        uint32_t idx _UNUSED, const pa_cvolume * volume _UNUSED, pa_context_success_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return op;
}

static void context_set_state_cb(pa_context *c, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_set_state_cb_func(c, userdata);
}

void pa_context_set_state_callback(pa_context * c _UNUSED, pa_context_notify_cb_t cb, void * userdata)
{
    DEBUG_MSG("%s called ", __func__);
    context_set_state_cb_func = cb;
    context_set_state_cb_data = userdata;

    if(ListenMode) {
        GET_ORIGINAL(context_set_state_callback);
        if(cb) {
            orig(c, context_set_state_cb, userdata);
        } else {
            orig(c, NULL, userdata);
        }
    }
}

void pa_context_unref(pa_context * c _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(context_unref);
        orig(c);
    }
}

pa_cvolume * pa_cvolume_set(pa_cvolume * a _UNUSED, unsigned channels _UNUSED, pa_volume_t v _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

pa_cvolume * pa_cvolume_set_balance(pa_cvolume * v _UNUSED, const pa_channel_map * map _UNUSED, float new_balance _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

size_t pa_frame_size(const pa_sample_spec * spec _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

pa_operation_state_t pa_operation_get_state(pa_operation *o _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

void pa_operation_unref(pa_operation *o _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

const char * pa_proplist_gets(pa_proplist * p _UNUSED, const char * key _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

pa_usec_t pa_rtclock_now(void)
{
    pa_usec_t retVal;
    if(ListenMode) {
        GET_ORIGINAL(rtclock_now);
        retVal = orig();
    } else {
        retVal = 0;
    }
    DEBUG_MSG("%s returned %lu", __func__, retVal);
    return retVal;
}

int pa_stream_cancel_write(pa_stream * p _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

int pa_stream_connect_playback(pa_stream * s _UNUSED,
        const char * dev _UNUSED, const pa_buffer_attr * attr _UNUSED, pa_stream_flags_t flags _UNUSED,
        const pa_cvolume *volume _UNUSED, pa_stream * sync_stream _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

pa_operation * pa_stream_cork(pa_stream * s _UNUSED, int b _UNUSED, pa_stream_success_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

int pa_stream_disconnect(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

const pa_channel_map * pa_stream_get_channel_map(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

uint32_t pa_stream_get_index(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

int pa_stream_get_latency(pa_stream * s _UNUSED, pa_usec_t * r_usec _UNUSED, int * negative _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

const pa_sample_spec * pa_stream_get_sample_spec(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

pa_stream_state_t pa_stream_get_state(pa_stream * p _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

int pa_stream_get_time(pa_stream * s _UNUSED, pa_usec_t * r_usec _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

pa_stream * pa_stream_new(pa_context * c _UNUSED, const char * name _UNUSED, const pa_sample_spec * ss _UNUSED, const pa_channel_map * map _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

void pa_stream_set_state_callback(pa_stream * s _UNUSED, pa_stream_notify_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

void pa_stream_set_write_callback(pa_stream * p _UNUSED, pa_stream_request_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

void pa_stream_unref(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

pa_operation * pa_stream_update_timing_info(pa_stream * p _UNUSED, pa_stream_success_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

int pa_stream_write(pa_stream * p _UNUSED,
        const void * data _UNUSED, size_t nbytes _UNUSED, pa_free_cb_t free_cb _UNUSED, int64_t offset _UNUSED, pa_seek_mode_t seek _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

pa_volume_t pa_sw_volume_from_linear(double v _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

void pa_threaded_mainloop_free(pa_threaded_mainloop* m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

pa_mainloop_api * pa_threaded_mainloop_get_api(pa_threaded_mainloop * m _UNUSED)
{
    pa_mainloop_api * api;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_get_api);
        api = orig(m);
    } else {
        api = &mainloop_api;
    }
    return api;
}

int pa_threaded_mainloop_in_thread(pa_threaded_mainloop * m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

void pa_threaded_mainloop_lock(pa_threaded_mainloop * m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_lock);
        orig(m);
    }
}

pa_threaded_mainloop * pa_threaded_mainloop_new(void)
{
    pa_threaded_mainloop * retVal;
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_new);
        retVal = orig();
    } else {
        retVal = mainloop;
    }
    return retVal;
}

void pa_threaded_mainloop_signal(pa_threaded_mainloop * m _UNUSED, int wait_for_accept)
{
    DEBUG_MSG("%s(%i) called ", __func__, wait_for_accept);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_signal);
        orig(m, wait_for_accept);
    }
}

int pa_threaded_mainloop_start(pa_threaded_mainloop *m _UNUSED)
{
    int retVal;
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_start);
        retVal = orig(m);
    } else {
        retVal = PA_OK;
    }
    DEBUG_MSG("%s returned %i ", __func__, retVal);
    return retVal;
}

void pa_threaded_mainloop_stop(pa_threaded_mainloop *m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_stop);
        orig(m);
    }
}

void pa_threaded_mainloop_unlock(pa_threaded_mainloop *m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_unlock);
        orig(m);
    }
}

void pa_threaded_mainloop_wait(pa_threaded_mainloop * m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    if(ListenMode) {
        GET_ORIGINAL(threaded_mainloop_wait);
        orig(m);
    }
}

size_t pa_usec_to_bytes(pa_usec_t t _UNUSED, const pa_sample_spec *spec _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

void pa_stream_set_read_callback(pa_stream * p _UNUSED, pa_stream_request_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

int pa_stream_connect_record(pa_stream * s _UNUSED,
        const char *dev _UNUSED, const pa_buffer_attr *attr _UNUSED, pa_stream_flags_t flags _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

size_t pa_stream_readable_size(pa_stream * p _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

size_t pa_stream_writable_size(pa_stream * p _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

int pa_stream_drop(pa_stream * p _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

const pa_buffer_attr * pa_stream_get_buffer_attr(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

const char * pa_stream_get_device_name(pa_stream * s _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
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

    if(m & PA_SUBSCRIPTION_MASK_SINK) {
        DEBUG_MSG("SINK");
    }
    if(m & PA_SUBSCRIPTION_MASK_SOURCE) {
        DEBUG_MSG("SRC");
    }
    if(m & PA_SUBSCRIPTION_MASK_SINK_INPUT) {
        DEBUG_MSG("SINK_IN");
    }
    if(m & PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT) {
        DEBUG_MSG("SRC_OUT");
    }
    if(m & PA_SUBSCRIPTION_MASK_MODULE) {
        DEBUG_MSG("MODULE");
    }
    if(m & PA_SUBSCRIPTION_MASK_CLIENT) {
        DEBUG_MSG("CLIENT");
    }
    if(m & PA_SUBSCRIPTION_MASK_SAMPLE_CACHE) {
        DEBUG_MSG("CACHE");
    }
    if(m & PA_SUBSCRIPTION_MASK_SERVER) {
        DEBUG_MSG("SERVER");
    }
    if(m & PA_SUBSCRIPTION_MASK_CARD) {
        DEBUG_MSG("CARD");
    }

//    if(cb) {
//        cb(c, m, userdata);
//    }
    return op;
}

void pa_mainloop_api_once(pa_mainloop_api * m _UNUSED,
        void (*callback)(pa_mainloop_api* m, void * userdata) _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

