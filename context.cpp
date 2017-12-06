/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include <unistd.h>

#include "context.hpp"
#include "logging.h"

static pa_operation * op;


CContext::CContext(pa_mainloop_api * api, const char * name)
{
    (void) name;
    (void) api;
    refCount = 1;
}

void CContext::unref(CContext * self)
{
    self->refCount--;
    if(self->refCount <= 0) {
        delete self;
    }
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
        DEBUG_MSG("Calling set context state callback");
        state_cb_func(reinterpret_cast<pa_context *>(this), state_cb_data);
    }
    return 0;
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
        cb(reinterpret_cast<pa_context *>(this), &info, userdata);
    }
    return op;
}

pa_operation * CContext::get_sink_info_by_name(const char * name, pa_sink_info_cb_t cb, void * userdata)
{
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
        cb(reinterpret_cast<pa_context *>(this), &info, 0, userdata);
        cb(reinterpret_cast<pa_context *>(this), NULL, 1, userdata);
    }
    return op;
}



