#ifndef _STREAM_HPP_
#define _STREAM_HPP_

/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "config.h"

#include "ref_count.hpp"

#include <alsa/asoundlib.h>

#include "pulseaudio.h"


class CContext;

class CStream : public CRefCount
{
public:
    static CStream * from_pa(pa_stream * s) {return reinterpret_cast<CStream *>(s);};
    static const CStream * from_pa(const pa_stream * s) {return reinterpret_cast<const CStream *>(s);};
    pa_stream * to_pa() {return reinterpret_cast<pa_stream *>(this);};

    CStream(CContext *, const char *, const pa_sample_spec *, const pa_channel_map *);

    int begin_write(void ** data, size_t * nbytes);
    int write(const void *, size_t, pa_free_cb_t, off_t, pa_seek_mode_t);
    int cancel_write();

    int connect_playback(const char * dev, const pa_buffer_attr * attr, pa_stream_flags_t flags,
        const pa_cvolume *volume, pa_stream * sync_stream);
    int disconnect();

    pa_operation * cork(int b, pa_stream_success_cb_t cb, void * userdata);

    const pa_channel_map * get_channel_map();
    uint32_t get_index() const;
    int get_latency(pa_usec_t * r_usec, int * negative);
    size_t writable_size() const;
    const pa_sample_spec * get_sample_spec();
    pa_stream_state_t get_state() const { return toPaState(mState); };
    int get_time(pa_usec_t * r_usec);
    void set_state_callback(pa_stream_notify_cb_t cb, void * userdata);

    pa_operation * update_timing_info(pa_stream_success_cb_t cb, void * userdata);
    void set_write_callback( pa_stream_request_cb_t cb, void * userdata);
    int peek(const void ** data, size_t * nbytes);

    int drop() {return 0;};
    const pa_buffer_attr * get_buffer_attr() { return NULL; };
    const char * get_device_name() const { return NULL; };
    size_t readable_size() const {return 0; };
    void set_read_callback(pa_stream_request_cb_t cb, void * userdata) {(void) cb; (void) userdata;};

    int connect_record(const char *dev, const pa_buffer_attr *attr, pa_stream_flags_t flags) 
    {
        (void) dev; (void) attr; (void) flags; return 0; 
    };
private:

    int setup_alsa(bool toTest);
    int test_and_set_access(snd_pcm_hw_params_t * params);
    int test_and_set_format(snd_pcm_hw_params_t * params);
    int test_and_set_channel(snd_pcm_hw_params_t * params);
    int test_and_set_rate(snd_pcm_hw_params_t * params);
    int test_and_set_buffer(snd_pcm_hw_params_t * params);


    enum InternalState {
        UNCONNECTED,
        CHECKED,
        CONNECTED,
        READY,
        RUNNING,
        FAILED
    };
    
    InternalState mState;

    static pa_stream_state_t toPaState(InternalState state);
    void set_state(InternalState newState);

    CContext * mContext;
    snd_pcm_t * mAlsaHnd;

    snd_pcm_uframes_t mBufferSize;
    size_t mFrameSize;

    pa_sample_spec mSpec;
    pa_channel_map mMap;

    pa_stream_request_cb_t mWrite_cb;
    void * mWrite_userdata;
    static void AlsaCallback(snd_async_handler_t * hnd);
    void setup_alsa_callback();
    snd_async_handler_t * mAlsaCbHandler;

    pa_stream_notify_cb_t mState_cb;
    void * mState_userdata;

protected:
    virtual ~CStream();
};

#endif
