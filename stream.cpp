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

static const snd_pcm_access_t g_accesses[] = {
//    SND_PCM_ACCESS_MMAP_INTERLEAVED,
    SND_PCM_ACCESS_RW_INTERLEAVED,
    SND_PCM_ACCESS_RW_NONINTERLEAVED,
//    SND_PCM_ACCESS_MMAP_NONINTERLEAVED,    
//    SND_PCM_ACCESS_MMAP_COMPLEX,
};


static const snd_pcm_format_t g_formats[] = {
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
    SND_PCM_FORMAT_S8,
    SND_PCM_FORMAT_U8,
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
 * Convert from ALSA format to PA formats. There is a many to 1
 * relationship.
 *
 * @param[in] format The ALSA format
 *
 * @return The equivalent PA format
 */
static pa_sample_format_t alsa2pa_format(snd_pcm_format_t format) {

    switch(format) {

        case SND_PCM_FORMAT_S8:
        case SND_PCM_FORMAT_U8:
            return PA_SAMPLE_U8;

        case SND_PCM_FORMAT_MU_LAW:
            return PA_SAMPLE_ULAW;

        case SND_PCM_FORMAT_A_LAW:
            return PA_SAMPLE_ALAW;

        case SND_PCM_FORMAT_S16_LE:
        case SND_PCM_FORMAT_U16_LE:
            return PA_SAMPLE_S16LE;

        case SND_PCM_FORMAT_S16_BE:
        case SND_PCM_FORMAT_U16_BE:
            return PA_SAMPLE_S16BE;

        case SND_PCM_FORMAT_S18_3LE:
        case SND_PCM_FORMAT_U18_3LE:
        case SND_PCM_FORMAT_S20_3LE:
        case SND_PCM_FORMAT_U20_3LE:
        case SND_PCM_FORMAT_S24_3LE:
        case SND_PCM_FORMAT_U24_3LE:
            return PA_SAMPLE_S24LE;

        case SND_PCM_FORMAT_S18_3BE:
        case SND_PCM_FORMAT_U18_3BE:
        case SND_PCM_FORMAT_S20_3BE:
        case SND_PCM_FORMAT_U20_3BE:
        case SND_PCM_FORMAT_S24_3BE:
        case SND_PCM_FORMAT_U24_3BE:
            return PA_SAMPLE_S24BE;

        case SND_PCM_FORMAT_S24_LE:     // 32-bit word
        case SND_PCM_FORMAT_U24_LE:
            return PA_SAMPLE_S24_32LE;

        case SND_PCM_FORMAT_S24_BE:     // 32-bit word
        case SND_PCM_FORMAT_U24_BE:
            return PA_SAMPLE_S24_32BE;

        case SND_PCM_FORMAT_S32_LE:
        case SND_PCM_FORMAT_U32_LE:
            return PA_SAMPLE_S32LE;

        case SND_PCM_FORMAT_S32_BE:
        case SND_PCM_FORMAT_U32_BE:
            return PA_SAMPLE_S32BE;

        case SND_PCM_FORMAT_FLOAT_LE:
            return PA_SAMPLE_FLOAT32LE;

        case SND_PCM_FORMAT_FLOAT_BE:
            return PA_SAMPLE_FLOAT32BE;

        default:
            return PA_SAMPLE_INVALID;
    }
}

/**
 * Convert from PA format to ALSA formats. There is a 1 to many
 * relationship so return a list
 *
 * @param[in] format The PA format
 *
 * @return A list of equivalent ALSA formats with a sentinel value of
 *      SND_PCM_FORMAT_UNKNOWN to mark the end of the list
 */
static const snd_pcm_format_t * pa2alsa_formats(pa_sample_format_t format) {

    switch(format) {

        case PA_SAMPLE_U8:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S8,
                SND_PCM_FORMAT_U8,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_ULAW:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_MU_LAW,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_ALAW:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_A_LAW,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S16LE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S16_LE,
                SND_PCM_FORMAT_U16_LE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S16BE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S16_BE,
                SND_PCM_FORMAT_U16_BE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S24LE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S18_3LE,
                SND_PCM_FORMAT_U18_3LE,
                SND_PCM_FORMAT_S20_3LE,
                SND_PCM_FORMAT_U20_3LE,
                SND_PCM_FORMAT_S24_3LE,
                SND_PCM_FORMAT_U24_3LE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S24BE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S18_3BE,
                SND_PCM_FORMAT_U18_3BE,
                SND_PCM_FORMAT_S20_3BE,
                SND_PCM_FORMAT_U20_3BE,
                SND_PCM_FORMAT_S24_3BE,
                SND_PCM_FORMAT_U24_3BE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S24_32LE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S24_LE,
                SND_PCM_FORMAT_U24_LE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S24_32BE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S24_BE,
                SND_PCM_FORMAT_U24_BE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S32LE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S32_LE,
                SND_PCM_FORMAT_U32_LE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_S32BE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_S32_BE,
                SND_PCM_FORMAT_U32_BE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_FLOAT32LE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_FLOAT_LE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        case PA_SAMPLE_FLOAT32BE:
        {
            static const snd_pcm_format_t list[] = {
                SND_PCM_FORMAT_FLOAT_BE,
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }

        default:
        {
            static const snd_pcm_format_t list[] = {
	        SND_PCM_FORMAT_UNKNOWN
            };
            return list;
        }
    }
}


/**
 * Create a stream class. At his point the ALSA library
 * has yet to be queried or setup.
 *
 * @param[in] context The parent context object
 * @param[in] name The name of the stream (ignored)
 *
 */
CStream::CStream(CContext * context, const char * name,
        const pa_sample_spec * desired_sample_spec,
        const pa_channel_map * desired_channel_map)
{
    (void) name;

    mContext = context;
    mAlsaHnd = NULL;
    mState = UNCONNECTED;
    mWrite_cb = NULL;

    if(desired_sample_spec) {
        mSpec = *desired_sample_spec;
    } else {
        mSpec.channels = 0;
        mSpec.format = PA_SAMPLE_INVALID;
        mSpec.rate = 0;
    }
    if(desired_channel_map) {
        mMap = *desired_channel_map;
    } else {
        mMap.channels = 0;
    }
    context->incRef();
}

/**
 *
 */
CStream::~CStream()
{
    mContext->decRef();
    mContext = NULL;
    disconnect();
}

static uint32_t buffer[512*2048];

/**
 * Called when the application wants PA to provide a
 * buffer (to avoid copy)
 *
 * @param[out] data The allocated buffer
 * @param[out] nBytes The size of the buffer
 *
 * @return zero on success
 */
int CStream::begin_write(void **data,  size_t *nbytes)
{
    size_t req_bytes = *nbytes;

    *data = (void *) buffer;
    *nbytes = req_bytes < 32768 ? req_bytes : 32768;
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

    disconnect();
    const int rc = setup_alsa(false);
    return rc < 0 ? 1 : 0;
}

/**
 *
 */
int CStream::setup_alsa(bool toTest)
{
    snd_pcm_hw_params_t * hw_params = NULL;
    snd_pcm_sw_params_t * sw_params = NULL;

    if(mState == FAILED) {
        return -1; // Already checked and failed
    } else if(mAlsaHnd) {
        return 0;  // Already done.
    } else if(toTest && (mState != UNCONNECTED)) {
        return 0; // Already checked
    }
 
    int rc;
    do {
        if((rc = snd_pcm_open(&mAlsaHnd, "hw:0,0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
            DEBUG_MSG("Failed to open PCM: %s", snd_strerror(rc));
            break;
        }

        if((rc = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
            DEBUG_MSG("Failed to alloc hw params: %s", snd_strerror(rc));
            break;
        }

        if((rc = snd_pcm_hw_params_any(mAlsaHnd, hw_params)) < 0) {
            DEBUG_MSG("Failed to read config space: %s", snd_strerror(rc));
            break;
        }

        if((rc = snd_pcm_hw_params_set_rate_resample(mAlsaHnd, hw_params, 0)) < 0) {
            DEBUG_MSG("Failed to disable resampling: %s", snd_strerror(rc));
            break;
        }

        if(!toTest) {
            if((rc = test_and_set_access(hw_params)) < 0) {
                break;
            }
        }

        if((rc = test_and_set_format(hw_params)) < 0) {
            break;
        }

        if((rc = test_and_set_channel(hw_params)) < 0) {
            break;
        }

        if((rc = test_and_set_rate(hw_params)) < 0) {
            break;
        }

        if(!toTest) {
            if((rc = test_and_set_buffer(hw_params)) < 0) {
                break;
            }
            
            if((rc = snd_pcm_hw_params(mAlsaHnd, hw_params)) < 0) {
                DEBUG_MSG("Failed to set HW parameters: %s", snd_strerror(rc));
                break;
            }

            if((rc = snd_pcm_sw_params_malloc(&sw_params)) < 0) {
                DEBUG_MSG("Failed to alloc hw params: %s", snd_strerror(rc));
                break;
            }

            if((rc = snd_pcm_sw_params_current(mAlsaHnd, sw_params)) < 0) {
                DEBUG_MSG("Failed to get current sw settings: %s", snd_strerror(rc));
                break;
            }
            if((rc = snd_pcm_sw_params_set_period_event(mAlsaHnd, sw_params, 1)) < 0) {
                DEBUG_MSG("Failed to set period events: %s", snd_strerror(rc));
                break;
            }
            if((rc = snd_pcm_sw_params(mAlsaHnd, sw_params)) < 0) {
                DEBUG_MSG("Failed to set SW parameters: %s", snd_strerror(rc));
                break;
            }
            if((rc = snd_pcm_prepare(mAlsaHnd)) < 0) {
                DEBUG_MSG("Failed to prepare: %s", snd_strerror(rc));
                break;
            }
        }
    } while(false);

    DEBUG_MSG("Final ALSA status %i", rc);
    if(hw_params) {
        snd_pcm_hw_params_free(hw_params);
    }
    if(sw_params) {
        snd_pcm_sw_params_free(sw_params);
    }


    if(rc < 0) {
        set_state(FAILED);
        if(mAlsaHnd) {
            snd_pcm_close(mAlsaHnd);
            mAlsaHnd = NULL;
        }
    } else if(toTest) {
        set_state(CHECKED);
        snd_pcm_close(mAlsaHnd);
        mAlsaHnd = NULL;
    } else {
        set_state(READY);
        setup_alsa_callback();
    }
    return rc;
}

/**
 *
 */
int CStream::test_and_set_access(snd_pcm_hw_params_t * params)
{
    int rc;
    unsigned int i;

    for(i = 0; i < sizeof(g_accesses)/sizeof(snd_pcm_access_t); i++) {
        if((rc = snd_pcm_hw_params_test_access(mAlsaHnd, params, g_accesses[i])) < 0) {
            continue;
        } else {
            DEBUG_MSG("Access type %s is supported", snd_pcm_access_name(g_accesses[i]));
            break;
        }
    }
    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_access(mAlsaHnd, params, g_accesses[i])) < 0) {
            DEBUG_MSG("Failed to set access mode: %s", snd_strerror(rc));
        }
    }
    return rc;
}

/**
 *
 */
int CStream::test_and_set_format(snd_pcm_hw_params_t * params)
{
    int rc;
    snd_pcm_format_t format;
    unsigned int i;

    const snd_pcm_format_t * formats = pa2alsa_formats(mSpec.format);
    for(i = 0;;i++) {
        format = formats[i];
        DEBUG_MSG("Testing %s", snd_pcm_format_name(format));
        if(format == SND_PCM_FORMAT_UNKNOWN) {
            break;
        } else if((rc = snd_pcm_hw_params_test_format(mAlsaHnd, params, format)) >= 0) {
            DEBUG_MSG("Format type %s is supported", snd_pcm_format_name(format));
            break;
        }
    }

    if(format == SND_PCM_FORMAT_UNKNOWN) {
        for(i = 0; i < sizeof(g_formats)/sizeof(snd_pcm_format_t); i++) {
            format = g_formats[i];
            DEBUG_MSG("Testing %s", snd_pcm_format_name(format));
            if((rc = snd_pcm_hw_params_test_format(mAlsaHnd, params, format)) < 0) {
                continue;
            } else {
                DEBUG_MSG("Format type %s is supported", snd_pcm_format_name(format));
                break;
            }
        }
    }

    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_format(mAlsaHnd, params, format)) < 0) {
            DEBUG_MSG("Failed to set format: %s", snd_strerror(rc));
        } else {
            mSpec.format = alsa2pa_format(format);
        }
    }
    return rc;
}

/**
 *
 */
int CStream::test_and_set_channel(snd_pcm_hw_params_t * params)
{
    int rc;
    unsigned int chans = mMap.channels;

    if((chans == 0) || ((rc = snd_pcm_hw_params_test_channels(mAlsaHnd, params, chans)) < 0)) {
        unsigned int min;
        unsigned int max;

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
        for (chans = max; chans >= min; --chans) {
	    if ((rc = snd_pcm_hw_params_test_channels(mAlsaHnd, params, chans)) >= 0)
	        break;
        }
    } else {
        rc = 0;
    }

    if(rc >= 0) {
        if((rc = snd_pcm_hw_params_set_channels(mAlsaHnd, params, chans)) < 0) {
            DEBUG_MSG("Failed to set channels: %s", snd_strerror(rc));
        } else {
            mMap.channels = chans;
            mSpec.channels = chans;
            for(unsigned int i = 0; i < chans; i++) {
                mMap.map[i] = static_cast<pa_channel_position_t>(i+1);
            }
        }
    }
    return rc;
}

/**
 *
 */
int CStream::test_and_set_rate(snd_pcm_hw_params_t * params)
{
    int rc;
    unsigned int rate = mSpec.rate;

    DEBUG_MSG("Rate is : %u", rate);

    if((rate == 0) || ((rc = snd_pcm_hw_params_test_rate(mAlsaHnd, params, rate, 0)) < 0)) {

        unsigned int min;
        unsigned int max;
        if((rc = snd_pcm_hw_params_get_rate_min(params, &min, NULL)) < 0) {
	    DEBUG_MSG("cannot get minimum rate: %s", snd_strerror(rc));
	    return rc;
        }
        if((rc = snd_pcm_hw_params_get_rate_max(params, &max, NULL)) < 0) {
	    DEBUG_MSG("cannot get maximum rate: %s", snd_strerror(rc));
	    return rc;
        }
        if (min == max) {
            rate = min;
        } else if ((rc = snd_pcm_hw_params_test_rate(mAlsaHnd, params, min + 1, 0)) >= 0) {
	    printf(" %u-%u", min, max);
        }else {
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
#if 0
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

        unsigned int rate;
        if((rc = snd_pcm_hw_params_get_rate(params, &rate, NULL)) < 0) {
            DEBUG_MSG("Failed to get rate: %s", snd_strerror(rc));
            // return NULL;
        }
    } else {
        rc = 0;
    }

    if(rc >= 0) {
        int dir = 0;
        if((rc = snd_pcm_hw_params_set_rate_near(mAlsaHnd, params, &rate, &dir)) < 0) {
            DEBUG_MSG("Failed to set the rate: %s", snd_strerror(rc));
        } else {
            mSpec.rate = rate;
        }
    }
    return rc;
}

int CStream::test_and_set_buffer(snd_pcm_hw_params_t * params)
{
    unsigned int buffer_time = 100000;
    int dir = -1;
    int rc;
    do {
        if((rc = snd_pcm_hw_params_set_buffer_time_near(mAlsaHnd, params, &buffer_time, &dir)) < 0) {
            DEBUG_MSG("Failed to set Buffer time: %s", snd_strerror(rc));
            break;
        }
        if((rc = snd_pcm_hw_params_get_buffer_size(params, &mBufferSize)) < 0) {
            DEBUG_MSG("Failed to get Buffer size: %s", snd_strerror(rc));
            break;
        }
        unsigned int period_time = buffer_time/2;
        if((rc = snd_pcm_hw_params_set_period_time_near(mAlsaHnd, params, &period_time, &dir)) < 0) {
            DEBUG_MSG("Failed to set Period time: %s", snd_strerror(rc));
            break;
        }

        snd_pcm_format_t fmt;
        snd_pcm_hw_params_get_format(params, &fmt);
        mFrameSize = snd_pcm_format_size(fmt, mSpec.channels);

    } while(0);
    return rc;
}

int CStream::disconnect()
{
    if(mAlsaHnd) {
        snd_pcm_close(mAlsaHnd);
        mAlsaHnd = NULL;
        set_state(UNCONNECTED);
    }
    return 0;
}

int CStream::cancel_write()
{
    return 0;
}

const pa_channel_map * CStream::get_channel_map()
{
    setup_alsa(true);
    return &mMap;
}

uint32_t CStream::get_index() const
{
    return 34790;
}

int CStream::get_latency(pa_usec_t * r_usec, int * negative)
{
    *r_usec = 0;
    *negative = 0;
    return 0;
}

size_t CStream::writable_size()
{
    size_t amount = 0;
    if(mState == READY) {
        amount = mBufferSize;
    } else if(mState == RUNNING) {
        amount = snd_pcm_avail(mAlsaHnd);
    } else {
        amount = 0;
    }
    return amount * mFrameSize;
}

const pa_sample_spec * CStream::get_sample_spec()
{
    setup_alsa(true);
    return &mSpec;
}

pa_stream_state_t CStream::toPaState(InternalState state) 
{
    switch(state) {
        case UNCONNECTED:
        case CHECKED:
            return PA_STREAM_UNCONNECTED;

        case READY:
        case RUNNING:
            return PA_STREAM_READY;

        default:
            return PA_STREAM_FAILED;
    }
}



void CStream::set_state(InternalState newState)
{
    if(newState != mState)
    {
        const InternalState oldState = mState;
        mState = newState;
        if(mState_cb && (toPaState(newState) != toPaState(oldState))) {
            mContext->mainloop_once(new CStreamNotifyCb(mState_cb, to_pa(), mState_userdata));
        }
    }
}

unsigned long soFar = 0;
int CStream::get_time(pa_usec_t * r_usec)
{
    if(mState == RUNNING) {
        const snd_pcm_uframes_t avail = snd_pcm_avail(mAlsaHnd);
        *r_usec = 10000 * (soFar - (mBufferSize - avail))/480;
    } else {
        *r_usec = 0;
    }
    return 0;
}

int CStream::write(const void * data, size_t nbytes, pa_free_cb_t free_cb, off_t offset, pa_seek_mode_t seek)
{
    const snd_pcm_uframes_t frames = nbytes / mFrameSize;
    soFar += frames;

    if((offset != 0) || (seek != PA_SEEK_RELATIVE) || (data == NULL)) {
        return -1;
    }
    if(mState == READY) {
        snd_pcm_start(mAlsaHnd);
        mState = RUNNING;
    }
    if(mAlsaHnd) {
        snd_pcm_writei(mAlsaHnd, data, frames);
    }
    if(free_cb) {
        free_cb(const_cast<void *>(data));
    }
    return 0;
}

void CStream::AlsaCallback(snd_async_handler_t * ahnd)
{
    CStream * self = reinterpret_cast<CStream *>(snd_async_handler_get_callback_private(ahnd));
    if(self->mWrite_cb) {
        const unsigned int avail = self->mFrameSize * snd_pcm_avail_update(self->mAlsaHnd);
        if(avail > 0) {
            DEBUG_MSG("Queuing a set write callback(%u)", avail);
            self->mContext->mainloop_once(new CStreamRequestCb(self->mWrite_cb, self->to_pa(), avail, self->mWrite_userdata));
        }
    }
}

void CStream::setup_alsa_callback()
{
    if(mAlsaHnd && mWrite_cb) {
        snd_async_add_pcm_handler(&mAlsaCbHandler, mAlsaHnd, AlsaCallback, reinterpret_cast<void *>(this));
    }
}

void CStream::set_write_callback( pa_stream_request_cb_t cb, void * userdata)
{
    mWrite_userdata = userdata;
    if(mWrite_cb != cb) {
        mWrite_cb = cb;
        setup_alsa_callback();
    }
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

    mContext->mainloop_once(new CStreamSuccessCb(cb, to_pa(), 0, userdata));
    return (new COperation())->to_pa();
}


int CStream::peek(const void ** data, size_t * nbytes)
{
    (void) data;
    (void) nbytes;

    return 0;
}

