/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "logging.h"
#include "pulseaudio.h"

#define _UNUSED __attribute__((unused))

const char * pa_get_library_version()
{
    DEBUG_MSG("%s called", __func__);
    return "11.1.0";
}

int pa_channel_map_can_balance(const pa_channel_map * p _UNUSED)
{
    DEBUG_MSG("%s called", __func__);
    return 0;
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
    DEBUG_MSG("%s called ", __func__);
    if(server) {
        DEBUG_MSG("server to connect to  is %s", server);
    }
    DEBUG_MSG("%s called2 ", __func__);
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
    DEBUG_MSG("%s done", __func__);
    return 0;
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

pa_operation * pa_context_get_server_info(pa_context *c _UNUSED, pa_server_info_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
#if 0
    static struct fake_server_info info;

    info.user_name = "bob";
    info.host_name = "cobblers";              /**< Host name the daemon is running on */
    info.server_version = "11.1.0";         /**< Version string of the daemon */
    info.server_name = "pulseaudio";
    fake_sample_spec sample_spec;         /**< Default sample specification */
    const char *default_sink_name;      /**< Name of default sink. */
    const char *default_source_name;    /**< Name of default source. */
    uint32_t cookie;                    /**< A random cookie for identifying this instance of PulseAudio. */
    fake_channel_map channel_map;         /**< Default channel map. \since 0.9.15 */
#endif

    return NULL;

}

/** Get information about a sink by its name */
pa_operation * pa_context_get_sink_info_by_name(pa_context * c _UNUSED,
        const char * name _UNUSED, pa_sink_info_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
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
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

pa_context * pa_context_new(pa_mainloop_api * mainloop _UNUSED, const char * name _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
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
    return NULL;
}

void pa_context_set_state_callback(pa_context * c _UNUSED, pa_context_notify_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

void pa_context_unref(pa_context * c _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
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
    DEBUG_MSG("%s called ", __func__);
    return 0;
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
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

int pa_threaded_mainloop_in_thread(pa_threaded_mainloop * m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

void pa_threaded_mainloop_lock(pa_threaded_mainloop * m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

pa_threaded_mainloop * pa_threaded_mainloop_new(void)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

void pa_threaded_mainloop_signal(pa_threaded_mainloop *m _UNUSED, int wait_for_accept _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

int pa_threaded_mainloop_start(pa_threaded_mainloop *m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return 0;
}

void pa_threaded_mainloop_stop(pa_threaded_mainloop *m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

void pa_threaded_mainloop_unlock(pa_threaded_mainloop *m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

void pa_threaded_mainloop_wait(pa_threaded_mainloop * m _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
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

void pa_context_set_subscribe_callback(pa_context * c _UNUSED, pa_context_subscribe_cb_t cb _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

pa_operation * pa_context_subscribe(pa_context * c _UNUSED,
        pa_subscription_mask_t m _UNUSED, pa_context_success_cb_t cb _UNUSED, void *userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
    return NULL;
}

void pa_mainloop_api_once(pa_mainloop_api * m _UNUSED,
        void (*callback)(pa_mainloop_api* m, void * userdata) _UNUSED, void * userdata _UNUSED)
{
    DEBUG_MSG("%s called ", __func__);
}

