#ifndef _STREAM_HPP_
#define _STREAM_HPP_

/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "config.h"

#include <alsa/asoundlib.h>

#include "pulseaudio.h"


class CContext;

class CStream
{
public:
    CStream(CContext *, const char *, const pa_sample_spec *, const pa_channel_map *);
    static void unref(CStream *);

    int begin_write(void ** data, size_t * nbytes);
    int write(const void *, size_t, pa_free_cb_t, off_t, pa_seek_mode_t);
    int cancel_write();

    int connect_playback(const char * dev, const pa_buffer_attr * attr, pa_stream_flags_t flags,
        const pa_cvolume *volume, pa_stream * sync_stream);
    int disconnect();

    pa_operation * cork(int b, pa_stream_success_cb_t cb, void * userdata);

    const pa_channel_map * get_channel_map();
    uint32_t get_index();
    int get_latency(pa_usec_t * r_usec, int * negative);
    size_t writable_size();
    const pa_sample_spec * get_sample_spec();
    pa_stream_state_t get_state();
    int get_time(pa_usec_t * r_usec);
    void set_state_callback(pa_stream_notify_cb_t cb, void * userdata);

    pa_operation * update_timing_info(pa_stream_success_cb_t cb, void * userdata);
    void set_write_callback( pa_stream_request_cb_t cb, void * userdata);
    int peek(const void ** data, size_t * nbytes);

private:
    ~CStream();

    int test_and_set_access(snd_pcm_hw_params_t * params);
    int test_and_set_formats(snd_pcm_hw_params_t * params);
    int test_and_set_channels(snd_pcm_hw_params_t * params);
    int test_and_set_rates(snd_pcm_hw_params_t * params);
        

    unsigned int mRefCount;
    CContext * mContext;
    snd_pcm_t * mAlsaHnd;
};

#endif
