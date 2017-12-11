/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include <unistd.h>

#include "context.hpp"
#include "logging.h"
#include "operation.hpp"
#include "blob.hpp"


pa_sink_info CContext::mInfo;

CContext::CContext(pa_mainloop_api * api, const char * name)
{
    static pa_format_info format = {PA_ENCODING_PCM, NULL};
    static pa_format_info * formats[1] = {&format};

    (void) name;
    mMainloopApi = api;
    state_cb_func = NULL; 
    subscribe_cb_func = NULL; 

    mInfo.name = "sink";
    mInfo.index = 1;
    mInfo.description = "output";
    mInfo.sample_spec.format = PA_SAMPLE_S16LE;
    mInfo.sample_spec.rate = 44100;
    mInfo.sample_spec.channels = 2;
    mInfo.channel_map.channels =2;
    mInfo.channel_map.map[0] = PA_CHANNEL_POSITION_FRONT_LEFT;
    mInfo.channel_map.map[1] = PA_CHANNEL_POSITION_FRONT_RIGHT;
    mInfo.owner_module = 1;
    mInfo.volume.channels = 2;
    mInfo.volume.values[0] = 100;
    mInfo.volume.values[1] = 100;
    mInfo.mute = 0;
    mInfo.monitor_source = 0;
    mInfo.monitor_source_name = "";
    mInfo.latency = 1000;
    mInfo.driver = "alsa";
    mInfo.flags = PA_SINK_HARDWARE;
    mInfo.proplist = NULL;
    mInfo.configured_latency = 1000;
    mInfo.base_volume = 50;
    mInfo.state = PA_SINK_IDLE;
    mInfo.n_volume_steps = 1;
    mInfo.card =  1;
    mInfo.n_ports = 0; 
    mInfo.ports = NULL;
    mInfo.active_port = NULL;
    mInfo.n_formats = 1;
    mInfo.formats = formats;
}

CContext::~CContext()
{
}

int CContext::connect(const char * server, pa_context_flags_t flags, const pa_spawn_api * api)
{
    (void) server;
    (void) flags;

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
    if(state_cb_func) {
        DEBUG_MSG("Queuing \'set context state callback\'");
        incRef();
        mainloop_once(new CContextNotifyCb(state_cb_func, to_pa(), state_cb_data));
    }
    return 0;
}

void disconnect()
{
}

pa_operation * CContext::get_server_info(pa_server_info_cb_t cb, void *userdata)
{
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
        incRef();
        mainloop_once(new CServerInfoCb(cb, to_pa(), &info, userdata));
    }
    return (new COperation())->to_pa();
}

pa_operation * CContext::get_sink_info_by_name(const char * name, pa_sink_info_cb_t cb, void * userdata)
{
    (void)name;

    if(cb) {
        incRef();
        mainloop_once(new CSinkInfoCb(cb, to_pa(), &mInfo, userdata));
        incRef();
        mainloop_once(new CSinkInfoCb(cb, to_pa(), NULL, userdata));
    }
    return (new COperation())->to_pa();
}


pa_operation * CContext::set_sink_input_volume(uint32_t idx, 
            const pa_cvolume * volume, pa_context_success_cb_t cb, void * userdata)
{
    (void) idx;
    (void) volume;

    if(cb) {
        incRef();
        mainloop_once(new CContextSuccessCb(cb, to_pa(), 1, userdata));
    }
    return (new COperation())->to_pa();
}

pa_operation * CContext::subscribe(pa_subscription_mask_t m, pa_context_success_cb_t cb, void *userdata)
{
    (void) m;
    // FIXME
    if(cb) {
        incRef();
        mainloop_once(new CContextSuccessCb(cb, to_pa(), 1, userdata));
    }
    return (new COperation())->to_pa();
}

/**
 * Wrapper for mainloop->time_new
 */
pa_time_event * CContext::rttime_new(pa_usec_t usec, pa_time_event_cb_t cb, void *userdata)
{
    const struct timeval tv = 
    {
        static_cast<time_t>(usec/1000000),
        static_cast<suseconds_t>(usec % 1000000)
    };
    return mMainloopApi->time_new(mMainloopApi, &tv, cb, userdata);
}

void CContext::disconnect()
{
}

pa_operation * CContext::get_sink_info_list(pa_sink_info_cb_t cb, void * userdata)
{
    if(cb) {
        incRef();
        mainloop_once(new CSinkInfoCb(cb, to_pa(), &mInfo, userdata));
        incRef();
        mainloop_once(new CSinkInfoCb(cb, to_pa(), NULL, userdata));
    }
    return (new COperation())->to_pa();
}

pa_operation * CContext::get_source_info_list(pa_source_info_cb_t cb, void * userdata)
{
    (void) cb;
    (void) userdata;
    // FIXME

    return (new COperation())->to_pa();
}

pa_operation * CContext::drain(pa_context_notify_cb_t cb, void * userdata)
{
    if(cb) {
        incRef();
        mainloop_once(new CContextNotifyCb(cb, to_pa(), userdata));
    }
    return (new COperation())->to_pa();
}

pa_operation * CContext::get_sink_input_info(uint32_t idx, pa_sink_input_info_cb_t cb, void *userdata)
{
    (void) idx;
    (void) cb;
    (void) userdata;
    // FIXME
    return (new COperation())->to_pa();
}

void CContext::mainloop_once(CBlob * blob)
{
    pa_defer_event * evt = mMainloopApi->defer_new(mMainloopApi, blob->callback, blob);
    mMainloopApi->defer_set_destroy(evt, blob->free);
}
 
