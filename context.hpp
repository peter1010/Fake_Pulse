#ifndef _MY_CONTEXT_H_
#define _MY_CONTEXT_H_

/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "pulseaudio.h"


class CContext
{
public:
    CContext(pa_mainloop_api * api, const char * name);
    ~CContext();

    pa_context_state_t get_state() const { return PA_CONTEXT_READY; };

    void set_state_callback(pa_context_notify_cb_t cb, void * userdata) { state_cb_func = cb; state_cb_data = userdata;};

    int connect(const char * server, pa_context_flags_t flags, const pa_spawn_api * api);

    pa_operation * get_server_info(pa_server_info_cb_t cb, void *userdata);

    pa_operation * get_sink_info_by_name(const char * name, pa_sink_info_cb_t cb, void * userdata);
    
    static void unref(CContext * self);
private:
    
    pa_context_notify_cb_t state_cb_func; 
    void * state_cb_data;

    int refCount;
};

#endif

