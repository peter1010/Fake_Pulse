/**
 *
 * copyright (c) 2017 peter leese
 *
 * licensed under the gpl license. see license file in the project root for full license information.  
 */


#include <alsa/asoundlib.h>
#include "stream.hpp"
#include "context.hpp"
#include "logging.h"
#include "operation.hpp"
#include "blob.hpp"

static const snd_pcm_access_t accesses[] = {
    SND_PCM_ACCESS_MMAP_INTERLEAVED,
    SND_PCM_ACCESS_MMAP_NONINTERLEAVED,
    SND_PCM_ACCESS_MMAP_COMPLEX,
    SND_PCM_ACCESS_RW_INTERLEAVED,
    SND_PCM_ACCESS_RW_NONINTERLEAVED,
};


static const snd_pcm_format_t formats[] = {
    SND_PCM_FORMAT_S8,
    SND_PCM_FORMAT_U8,
    SND_PCM_FORMAT_S16_LE,
    SND_PCM_FORMAT_S16_BE,
    SND_PCM_FORMAT_U16_LE,
    SND_PCM_FORMAT_U16_BE,
    SND_PCM_FORMAT_S24_LE,
    SND_PCM_FORMAT_S24_BE,
    SND_PCM_FORMAT_U24_LE,
    SND_PCM_FORMAT_U24_BE,
    SND_PCM_FORMAT_S32_LE,
    SND_PCM_FORMAT_S32_BE,
    SND_PCM_FORMAT_U32_LE,
    SND_PCM_FORMAT_U32_BE,
    SND_PCM_FORMAT_FLOAT_LE,
    SND_PCM_FORMAT_FLOAT_BE,
//    SND_PCM_FORMAT_FLOAT64_LE,
//    SND_PCM_FORMAT_FLOAT64_BE,
    SND_PCM_FORMAT_MU_LAW,
    SND_PCM_FORMAT_A_LAW,
    SND_PCM_FORMAT_S24_3LE,
    SND_PCM_FORMAT_S24_3BE,
    SND_PCM_FORMAT_U24_3LE,
    SND_PCM_FORMAT_U24_3BE,
    SND_PCM_FORMAT_S20_3LE,
    SND_PCM_FORMAT_S20_3BE,
    SND_PCM_FORMAT_U20_3LE,
    SND_PCM_FORMAT_U20_3BE,
    SND_PCM_FORMAT_S18_3LE,
    SND_PCM_FORMAT_S18_3BE,
    SND_PCM_FORMAT_U18_3LE,
    SND_PCM_FORMAT_U18_3BE,
}; 

/**
 *
 */
CStream::CStream(CContext * context, const char * name,
        const pa_sample_spec * desired_sample_spec,
        const pa_channel_map * desired_channel_map)
{
    (void) name;
    (void) desired_sample_spec;
    (void) desired_channel_map;

    // Ignore sample spec and channel maps use the default!
    mContext = context;
    mAlsaHnd = NULL;
    mParams = NULL;
    context->incRef();
}

/**
 *
 */
CStream::~CStream()
{
    if(mAlsaHnd || mParams) {
        disconnect();
    }
}

int CStream::begin_write(void **data,  size_t *nbytes)
{
    (void) data;
    (void) nbytes;

    return 0;
}

/**
 * Ignore all passed in parameters
 */
int CStream::connect_playback(const char * dev, const pa_buffer_attr * attr, pa_stream_flags_t flags,
        const pa_cvolume *volume, pa_stream * sync_stream)
{
    (void) dev;
    (void) attr;
    (void) flags;
    (void) volume;
    (void) sync_stream;

    int rc;
    do {
        if((rc = snd_pcm_open(&mAlsaHnd, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0)) < 0)
        {
            DEBUG_MSG("Failed to open PCM: %s", snd_strerror(rc));
            break;
        }

        if((rc = snd_pcm_hw_params_malloc(&mParams)) < 0)
        {
            DEBUG_MSG("Failed to alloc hw params: %s", snd_strerror(rc));
            break;
        }

        if((rc = snd_pcm_hw_params_any(mAlsaHnd, mParams))  < 0)
        {
            DEBUG_MSG("Failed to read config space: %s", snd_strerror(rc));
            break;
        }

        if((rc = snd_pcm_hw_params_set_rate_resample(mAlsaHnd, mParams, 0))  < 0)
        {
            DEBUG_MSG("Failed to disable resampling: %s", snd_strerror(rc));
            break;
        }

        if((rc = test_and_set_access()) < 0)
        {
            break;
        }
    
        if((rc = test_and_set_formats()) < 0)
        {
            break;
        }
    
        if((rc = test_and_set_channels()) < 0)
        {
            break;
        }

        if((rc = test_and_set_rates()) < 0)
        {
            break;
        }
#if 0
    test_rates(handle, params);
    int val = 48000;
    int dir = 0;
    if((rc = snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir)) < 0)
    {
        fprintf(stderr, "Failed to open PCM: %s\n", snd_strerror(rc));
        exit(1);
    }

    printf("Set sample rate to %i\n", val);

    snd_pcm_uframes_t frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);
#endif
        if((rc = snd_pcm_hw_params(mAlsaHnd, mParams)) < 0)
        {
            DEBUG_MSG("Failed to set HW parameters: %s", snd_strerror(rc));
            break;
        }
    } while(false);


    if(rc < 0) {
        disconnect();
    }
    return rc < 0 ? 1 : 0;
}

/**
 *
 */
int CStream::test_and_set_access()
{
    int rc;
    unsigned int i;
    for(i = 0; i < sizeof(accesses)/sizeof(snd_pcm_access_t); i++) {
        if((rc = snd_pcm_hw_params_test_access(mAlsaHnd, mParams, accesses[i])) < 0) {
            continue;
        } else {
            DEBUG_MSG("Access type %s is supported", snd_pcm_access_name(accesses[i]));
            break;
        }
    }
    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_access(mAlsaHnd, mParams, accesses[i])) < 0)
        {
            DEBUG_MSG("Failed to set access mode: %s", snd_strerror(rc));
        }
    }
    return rc;
}
 
/**
 *
 */
int CStream::test_and_set_formats()
{
    int rc;
    unsigned int i;
    for(i = 0; i < sizeof(formats)/sizeof(snd_pcm_format_t); i++) {
        if((rc = snd_pcm_hw_params_test_format(mAlsaHnd, mParams, formats[i])) < 0) {
            continue;
        } else {
            DEBUG_MSG("Format type %s is supported", snd_pcm_format_name(formats[i]));
            break;
        }
    }
    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_format(mAlsaHnd, mParams, SND_PCM_FORMAT_S16_LE)) < 0)
        {
            DEBUG_MSG("Failed to set format: %s", snd_strerror(rc));
        }
    }
    return rc;
}

/**
 *
 */
int CStream::test_and_set_channels()
{
    int rc;
    unsigned int min;
    unsigned int max;
    unsigned int i;

    if((rc = snd_pcm_hw_params_get_channels_min(mParams, &min)) < 0) {
	DEBUG_MSG("cannot get minimum channels count: %s", snd_strerror(rc));
	return rc;
    }
    if((rc = snd_pcm_hw_params_get_channels_max(mParams, &max)) < 0) {
	DEBUG_MSG("cannot get maximum channels count: %s", snd_strerror(rc));
	return rc;
    }
    if(max > 2) {
        max = 2;
    }
    for (i = max; i >= min; --i) {
	if (snd_pcm_hw_params_test_channels(mAlsaHnd, mParams, i) == 0)
	    break;
    }
    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_channels(mAlsaHnd, mParams, i)) < 0)
        {
            DEBUG_MSG("Failed to set channels: %s", snd_strerror(rc));
        }
    }
    return rc;
}

/**
 *
 */
int CStream::test_and_set_rates()
{
    int rc;
    unsigned int min;
    unsigned int max;
    if((rc = snd_pcm_hw_params_get_rate_min(mParams, &min, NULL)) < 0) {
	DEBUG_MSG("cannot get minimum rate: %s", snd_strerror(rc));
	return rc;
    }
    if((rc = snd_pcm_hw_params_get_rate_max(mParams, &max, NULL)) < 0) {
	DEBUG_MSG("cannot get maximum rate: %s", snd_strerror(rc));
	return rc;
    }
//    if((rc = snd_pcm_hw_params_set_rate(handle, mParams, max-1, 0)) < 0) {
//	fprintf(stderr, "cannot Set maximum rate: %s\n", snd_strerror(rc));
//	return;
//    }
    printf("Sample rates:");
    if (min == max)
	printf(" %u", min);
    else if (!snd_pcm_hw_params_test_rate(mAlsaHnd, mParams, min + 1, 0))
	printf(" %u-%u", min, max);
    else {
        unsigned int i;
        for(i = min; i < max; i++) {
//		any_rate = 0;
//                for (i = 0; i < ARRAY_SIZE(rates); ++i) {
			if (!snd_pcm_hw_params_test_rate(mAlsaHnd, mParams, i, 0)) {
//				any_rate = 1;
				printf(" %u", i);
			}
        }
//		if (!any_rate)
//			printf(" %u-%u", min, max);
    }
    return rc;
}

int CStream::disconnect()
{
    if(mAlsaHnd) {
        snd_pcm_close(mAlsaHnd);
        mAlsaHnd = NULL;
    }
    if(mParams) {
        snd_pcm_hw_params_free(mParams);
        mParams = NULL;
    }
    return 0;
}

int CStream::cancel_write()
{
    return 0;
}

const pa_channel_map * CStream::get_channel_map()
{
    static pa_channel_map map;
    if(!mParams) {
        return NULL;
    }

    int rc;
    unsigned int channels;
    if((rc = snd_pcm_hw_params_get_channels(mParams, &channels )) < 0)
    {
        DEBUG_MSG("Failed to get channels: %s", snd_strerror(rc));
        return NULL;
    }
    map.channels = channels;

    for(int i = 0; i < map.channels; i++) {
        map.map[i] = static_cast<pa_channel_position_t>(i+1);
    }
    return &map;
}

uint32_t CStream::get_index() const
{
    return 34790;
}

int CStream::get_latency(pa_usec_t * r_usec, int * negative)
{
    (void) r_usec;
    (void) negative;

    return 0;
}

size_t CStream::writable_size()
{
    return 0;
}

const pa_sample_spec * CStream::get_sample_spec()
{
    static pa_sample_spec spec;

    if(!mParams) {
        return NULL;
    }

    int rc;
    unsigned int channels;
    if((rc = snd_pcm_hw_params_get_channels(mParams, &channels )) < 0) {
        DEBUG_MSG("Failed to get channels: %s", snd_strerror(rc));
        return NULL;
    }
    spec.channels = channels;

    unsigned int rate;
    if((rc = snd_pcm_hw_params_get_rate(mParams, &rate, NULL)) < 0) {
        DEBUG_MSG("Failed to get rate: %s", snd_strerror(rc));
        return NULL;
    }
    spec.rate = rate;
    
    snd_pcm_format_t format;
    if((rc = snd_pcm_hw_params_get_format(mParams, &format)) < 0) {
        DEBUG_MSG("Failed to get format: %s", snd_strerror(rc));
        return NULL;
    }
    switch(format) {
    
        case SND_PCM_FORMAT_S8:
        case SND_PCM_FORMAT_U8:
            spec.format = PA_SAMPLE_U8;
            break;

        case SND_PCM_FORMAT_MU_LAW:
            spec.format = PA_SAMPLE_ULAW;
            break;

        case SND_PCM_FORMAT_A_LAW:
            spec.format = PA_SAMPLE_ALAW;
            break;

        case SND_PCM_FORMAT_S16_LE:
        case SND_PCM_FORMAT_U16_LE:
            spec.format = PA_SAMPLE_S16LE;
            break;

        case SND_PCM_FORMAT_S16_BE:
        case SND_PCM_FORMAT_U16_BE:
            spec.format = PA_SAMPLE_S16BE;
            break;

        case SND_PCM_FORMAT_S18_3LE:
        case SND_PCM_FORMAT_U18_3LE:
        case SND_PCM_FORMAT_S20_3LE:
        case SND_PCM_FORMAT_U20_3LE:
        case SND_PCM_FORMAT_S24_3LE:
        case SND_PCM_FORMAT_U24_3LE:
            spec.format = PA_SAMPLE_S24LE;
            break;

        case SND_PCM_FORMAT_S18_3BE:
        case SND_PCM_FORMAT_U18_3BE:
        case SND_PCM_FORMAT_S20_3BE:
        case SND_PCM_FORMAT_U20_3BE:
        case SND_PCM_FORMAT_S24_3BE:
        case SND_PCM_FORMAT_U24_3BE:
            spec.format = PA_SAMPLE_S24BE;
            break;
 
        case SND_PCM_FORMAT_S24_LE:     // 32-bit word
        case SND_PCM_FORMAT_U24_LE:
            spec.format = PA_SAMPLE_S24_32LE;
            break;

        case SND_PCM_FORMAT_S24_BE:     // 32-bit word
        case SND_PCM_FORMAT_U24_BE:
            spec.format = PA_SAMPLE_S24_32BE;
            break;

        case SND_PCM_FORMAT_S32_LE:
        case SND_PCM_FORMAT_U32_LE:
            spec.format = PA_SAMPLE_S32LE;
            break;

        case SND_PCM_FORMAT_S32_BE:
        case SND_PCM_FORMAT_U32_BE:
            spec.format = PA_SAMPLE_S32BE;
            break;

        case SND_PCM_FORMAT_FLOAT_LE:
            spec.format = PA_SAMPLE_FLOAT32LE;
            break;

        case SND_PCM_FORMAT_FLOAT_BE:
            spec.format = PA_SAMPLE_FLOAT32BE;
            break;

        default:
            return NULL;
    }
    return &spec;
}


pa_stream_state_t CStream::get_state()
{
    return (pa_stream_state_t)0;
}

int CStream::get_time(pa_usec_t * r_usec)
{
    (void) r_usec;

    return 0;
}

int CStream::write(const void * data, size_t nbytes, pa_free_cb_t free_cb, off_t offset, pa_seek_mode_t seek)
{
    (void) data;
    (void) nbytes;
    (void) free_cb;
    (void) offset;
    (void) seek;

    return 0;
}

void CStream::set_write_callback( pa_stream_request_cb_t cb, void * userdata)
{
    mWrite_cb = cb;
    mWrite_userdata = userdata;
}

void CStream::set_state_callback(pa_stream_notify_cb_t cb, void * userdata)
{
    mState_cb = cb;
    mState_userdata = userdata;
}

pa_operation * CStream::update_timing_info(pa_stream_success_cb_t cb, void * userdata)
{
    mContext->mainloop_once(new CStreamSuccessCb(cb, to_pa(), 0, userdata));
    return (new COperation())->to_pa();
}

pa_operation * CStream::cork(int b, pa_stream_success_cb_t cb, void * userdata)
{
    (void) b;
    (void) cb;
    (void) userdata;

    return NULL;
}


int CStream::peek(const void ** data, size_t * nbytes)
{
    (void) data;
    (void) nbytes;

    return 0;
}

