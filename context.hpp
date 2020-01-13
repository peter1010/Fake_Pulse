#ifndef _MY_CONTEXT_H_
#define _MY_CONTEXT_H_

/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "config.h"

#include "ref_count.hpp"

#include "pulseaudio.h"

class CBlob;

class CContext : public CRefCount
{
public:
    static CContext * from_pa(pa_context * c) { return reinterpret_cast<CContext *>(c); };
    pa_context * to_pa() { return reinterpret_cast<pa_context *>(this); };

    CContext(pa_mainloop_api * api, const char * name);

    pa_context_state_t get_state() const 
        { return PA_CONTEXT_READY; };

    void set_state_callback(pa_context_notify_cb_t cb, void * userdata)
        { mStateCb_func = cb; state_cb_data = userdata;};

    void set_subscribe_callback(pa_context_subscribe_cb_t cb, void * userdata)
        { subscribe_cb_func = cb; subscribe_cb_data = userdata;};

    int connect(const char * server, pa_context_flags_t flags, const pa_spawn_api * api);
    void disconnect();

    pa_operation * get_server_info(pa_server_info_cb_t cb, void *userdata);

    pa_operation * get_sink_info_by_name(const char * name, pa_sink_info_cb_t cb, void * userdata);
    
    pa_operation * get_sink_info_list(pa_sink_info_cb_t cb, void * userdata);

    pa_operation * get_source_info_list(pa_source_info_cb_t cb, void * userdata);

    pa_operation * get_sink_input_info(uint32_t idx, pa_sink_input_info_cb_t cb, void *userdata);
    
    pa_operation * set_sink_input_volume(uint32_t idx, 
            const pa_cvolume * volume, pa_context_success_cb_t cb, void * userdata);

    pa_operation * subscribe(pa_subscription_mask_t m, pa_context_success_cb_t cb, void *userdata);

    pa_time_event * rttime_new(pa_usec_t usec, pa_time_event_cb_t cb, void *userdata);

    pa_operation * drain(pa_context_notify_cb_t cb, void * userdata);

// internal..
//
    void mainloop_once(CBlob * blob);
private:
    static pa_sink_info mInfo;

    pa_mainloop_api * mMainloopApi;

    pa_context_notify_cb_t mStateCb_func; 
    void * state_cb_data;
    
    pa_context_subscribe_cb_t subscribe_cb_func; 
    void * subscribe_cb_data;
    
protected:
    virtual ~CContext(); // Must be called indirectly via unref
};

#endif

