
#include <alsa/asoundlib.h>
#include "stream.hpp"
#include "context.hpp"
#include "logging.h"

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
    SND_PCM_FORMAT_FLOAT64_LE,
    SND_PCM_FORMAT_FLOAT64_BE,
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


CStream::CStream(CContext * context, const char * name,
        const pa_sample_spec * desired_sample_spec,
        const pa_channel_map * desired_channel_map)
{
    (void) name;
    (void) desired_sample_spec;
    (void) desired_channel_map;

    // Ignore sample spec and channel maps use the default!
    mRefCount = 0;
    mContext = context;
    mAlsaHnd = NULL;
    context->ref();
}

CStream::~CStream()
{
    if(mAlsaHnd) {
        disconnect();
    }
}

/**
 * Decrement the reference count and delete if no more references
 * held
 */
void CStream::unref(CStream * self)
{
    if(self->mRefCount == 0) {
        delete self;
    } else {
        self->mRefCount--;
    }
}


int CStream::begin_write(void **data,  size_t *nbytes)
{
    (void) data;
    (void) nbytes;

    return 0;
}

/**
 * Ignore all
 */
int CStream::connect_playback(const char * dev, const pa_buffer_attr * attr, pa_stream_flags_t flags,
        const pa_cvolume *volume, pa_stream * sync_stream)
{
    (void) dev;
    (void) attr;
    (void) flags;
    (void) volume;
    (void) sync_stream;

    int rc = snd_pcm_open(&mAlsaHnd, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0);
    if( rc < 0)
    {
        DEBUG_MSG("Failed to open PCM: %s", snd_strerror(rc));
        return rc;
    }

    do {
        snd_pcm_hw_params_t * params;
        if((rc = snd_pcm_hw_params_malloc(&params)) < 0)
        {
            DEBUG_MSG("Failed to alloc hw params: %s", snd_strerror(rc));
            break;
        }

        do {
            if((rc = snd_pcm_hw_params_any(mAlsaHnd, params))  < 0)
            {
                DEBUG_MSG("Failed to read config space: %s", snd_strerror(rc));
                break;
            }

            if((rc = snd_pcm_hw_params_set_rate_resample(mAlsaHnd, params, 0))  < 0)
            {
                DEBUG_MSG("Failed to disable resampling: %s", snd_strerror(rc));
                break;
            }

            if((rc = test_and_set_access(params)) < 0)
            {
                break;
            }
    
            if((rc = test_and_set_formats(params)) < 0)
            {
                break;
            }
    
            if((rc = test_and_set_channels(params)) < 0)
            {
                break;
            }

            if((rc = test_and_set_rates(params)) < 0)
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
            if((rc = snd_pcm_hw_params(mAlsaHnd, params)) < 0)
            {
                DEBUG_MSG("Failed to set HW parameters: %s", snd_strerror(rc));
                break;
            }
        } while(false);

    } while(false);

    if(rc < 0) {
        snd_pcm_close(mAlsaHnd);
    }
    return rc < 0 ? 1 : 0;
}

/**
 *
 */
int CStream::test_and_set_access(snd_pcm_hw_params_t * params)
{
    int rc;
    unsigned int i;
    for(i = 0; i < sizeof(accesses)/sizeof(snd_pcm_access_t); i++) {
        if((rc = snd_pcm_hw_params_test_access(mAlsaHnd, params, accesses[i])) < 0) {
            continue;
        } else {
            DEBUG_MSG("Access type %s is supported", snd_pcm_access_name(accesses[i]));
            break;
        }
    }
    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_access(mAlsaHnd, params, accesses[i])) < 0)
        {
            DEBUG_MSG("Failed to set access mode: %s", snd_strerror(rc));
        }
    }
    return rc;
}
 
/**
 *
 */
int CStream::test_and_set_formats(snd_pcm_hw_params_t * params)
{
    int rc;
    unsigned int i;
    for(i = 0; i < sizeof(formats)/sizeof(snd_pcm_format_t); i++) {
        if((rc = snd_pcm_hw_params_test_format(mAlsaHnd, params, formats[i])) < 0) {
            continue;
        } else {
            DEBUG_MSG("Format type %s is supported", snd_pcm_format_name(formats[i]));
            break;
        }
    }
    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_format(mAlsaHnd, params, SND_PCM_FORMAT_S16_LE)) < 0)
        {
            DEBUG_MSG("Failed to set format: %s", snd_strerror(rc));
        }
    }
    return rc;
}

/**
 *
 */
int CStream::test_and_set_channels(snd_pcm_hw_params_t * params)
{
    int rc;
    unsigned int min = 0;
    unsigned int max = 0;
    unsigned int i;

    if((rc = snd_pcm_hw_params_get_channels_min(params, &min)) < 0) {
	DEBUG_MSG("cannot get minimum channels count: %s", snd_strerror(rc));
	return rc;
    }
    if((rc = snd_pcm_hw_params_get_channels_max(params, &max)) < 0) {
	DEBUG_MSG("cannot get maximum channels count: %s", snd_strerror(rc));
	return rc;
    }
    if(max > 2) {
        max = 2;
    }
    for (i = max; i >= min; --i) {
	if (snd_pcm_hw_params_test_channels(mAlsaHnd, params, i) == 0)
	    break;
    }
    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_channels(mAlsaHnd, params, i)) < 0)
        {
            DEBUG_MSG("Failed to set channels: %s", snd_strerror(rc));
        }
    }
    return rc;
}


int CStream::test_and_set_rates(snd_pcm_hw_params_t * params)
{
    int rc;
    unsigned int min = 0;
    unsigned int max = 0;
    if((rc = snd_pcm_hw_params_get_rate_min(params, &min, NULL)) < 0) {
	DEBUG_MSG("cannot get minimum rate: %s", snd_strerror(rc));
	return rc;
    }
    if((rc = snd_pcm_hw_params_get_rate_max(params, &max, NULL)) < 0) {
	DEBUG_MSG("cannot get maximum rate: %s", snd_strerror(rc));
	return rc;
    }
//    if((rc = snd_pcm_hw_params_set_rate(handle, params, max-1, 0)) < 0) {
//	fprintf(stderr, "cannot Set maximum rate: %s\n", snd_strerror(rc));
//	return;
//    }
    printf("Sample rates:");
    if (min == max)
	printf(" %u", min);
    else if (!snd_pcm_hw_params_test_rate(mAlsaHnd, params, min + 1, 0))
	printf(" %u-%u", min, max);
    else {
        unsigned int i;
        for(i = min; i < max; i++) {
//		any_rate = 0;
//                for (i = 0; i < ARRAY_SIZE(rates); ++i) {
			if (!snd_pcm_hw_params_test_rate(mAlsaHnd, params, i, 0)) {
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
    int rc = 0;
    if(mAlsaHnd) {
        rc = snd_pcm_close(mAlsaHnd);
        mAlsaHnd = NULL;
    }
    return rc == 0 ? 1: 0;
}

int CStream::cancel_write()
{
    return 0;
}

const pa_channel_map * CStream::get_channel_map()
{
    return NULL;
}

uint32_t CStream::get_index()
{
    return 0;
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
    return NULL;
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
    (void) cb;
    (void) userdata;
}

void CStream::set_state_callback(pa_stream_notify_cb_t cb, void * userdata)
{
    (void) cb;
    (void) userdata;
}

pa_operation * CStream::update_timing_info(pa_stream_success_cb_t cb, void * userdata)
{
    (void) cb;
    (void) userdata;

    return NULL;
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

