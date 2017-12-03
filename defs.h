/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "pulseaudio.h"

typedef const char * (*FP_get_library_version)(void);
typedef int (*FP_channel_map_can_balance)(const pa_channel_map *);
typedef pa_channel_map * (*FP_channel_map_init)(pa_channel_map *);
typedef int (*FP_context_connect)(pa_context *, const char *, pa_context_flags_t, const pa_spawn_api *);
typedef void (*FP_context_disconnect)(pa_context * c);
typedef pa_operation * (*FP_context_drain)(pa_context *, pa_context_notify_cb_t, void * );
typedef pa_operation * (*FP_context_get_server_info)(pa_context *, pa_server_info_cb_t, void *);
typedef pa_operation * (*FP_context_get_sink_info_by_name)(pa_context *, const char *, pa_sink_info_cb_t, void *);
typedef pa_operation * (*FP_context_get_sink_info_list)(pa_context *,pa_sink_info_cb_t, void *);
typedef pa_operation * (*FP_context_get_sink_input_info)(pa_context *,uint32_t, pa_sink_input_info_cb_t, void *);
typedef pa_operation * (*FP_context_get_source_info_list)(pa_context *,pa_source_info_cb_t, void *);
typedef pa_context_state_t (*FP_context_get_state)(pa_context *);
typedef pa_context * (*FP_context_new)(pa_mainloop_api *, const char *);
typedef pa_time_event * (*FP_context_rttime_new)(pa_context *,pa_usec_t, pa_time_event_cb_t, void *);
typedef pa_operation * (*FP_context_set_sink_input_volume)(pa_context *,uint32_t, const pa_cvolume *, pa_context_success_cb_t, void *);
typedef void (*FP_context_set_state_callback)(pa_context *, pa_context_notify_cb_t, void *);
typedef void (*FP_context_unref)(pa_context *);
typedef pa_cvolume * (*FP_cvolume_set)(pa_cvolume *, unsigned, pa_volume_t);
typedef pa_cvolume * (*FP_cvolume_set_balance)(pa_cvolume *, const pa_channel_map *, float);
typedef size_t (*FP_frame_size)(const pa_sample_spec *);
typedef pa_operation_state_t (*FP_operation_get_state)(pa_operation *);
typedef void (*FP_operation_unref)(pa_operation *);
typedef const char * (*FP_proplist_gets)(pa_proplist *, const char *);
typedef pa_usec_t (*FP_rtclock_now)(void);
typedef int (*FP_stream_cancel_write)(pa_stream *);
typedef int (*FP_stream_connect_playback)(pa_stream *, const char *, const pa_buffer_attr *, pa_stream_flags_t, const pa_cvolume *, pa_stream *);
typedef pa_operation * (*FP_stream_cork)(pa_stream *, int, pa_stream_success_cb_t, void *);
typedef int (*FP_stream_disconnect)(pa_stream *);
typedef const pa_channel_map * (*FP_stream_get_channel_map)(pa_stream *);
typedef uint32_t (*FP_stream_get_index)(pa_stream *);
typedef int (*FP_stream_get_latency)(pa_stream *, pa_usec_t *, int *);
typedef const pa_sample_spec * (*FP_stream_get_sample_spec)(pa_stream *);
typedef pa_stream_state_t (*FP_stream_get_state)(pa_stream *);
typedef int (*FP_stream_get_time)(pa_stream *, pa_usec_t *);
typedef pa_stream * (*FP_stream_new)(pa_context *, const char *, const pa_sample_spec *, const pa_channel_map *);
typedef void (*FP_stream_set_state_callback)(pa_stream *, pa_stream_notify_cb_t, void *);
typedef void (*FP_stream_set_write_callback)(pa_stream *, pa_stream_request_cb_t, void *);
typedef void (*FP_stream_unref)(pa_stream *);
typedef pa_operation * (*FP_stream_update_timing_info)(pa_stream *, pa_stream_success_cb_t, void *);
typedef int (*FP_stream_write)(pa_stream *, const void *, size_t, pa_free_cb_t, int64_t, pa_seek_mode_t);
typedef pa_volume_t (*FP_sw_volume_from_linear)(double);
typedef void (*FP_threaded_mainloop_free)(pa_threaded_mainloop *);
typedef pa_mainloop_api * (*FP_threaded_mainloop_get_api)(pa_threaded_mainloop *);
typedef int (*FP_threaded_mainloop_in_thread)(pa_threaded_mainloop *);
typedef void (*FP_threaded_mainloop_lock)(pa_threaded_mainloop *);
typedef pa_threaded_mainloop * (*FP_threaded_mainloop_new)(void);
typedef void (*FP_threaded_mainloop_signal)(pa_threaded_mainloop *, int);
typedef int (*FP_threaded_mainloop_start)(pa_threaded_mainloop *);
typedef void (*FP_threaded_mainloop_stop)(pa_threaded_mainloop *);
typedef void (*FP_threaded_mainloop_unlock)(pa_threaded_mainloop *);
typedef void (*FP_threaded_mainloop_wait)(pa_threaded_mainloop *);
typedef size_t (*FP_usec_to_bytes)(pa_usec_t, const pa_sample_spec *);
typedef void (*FP_stream_set_read_callback)(pa_stream *, pa_stream_request_cb_t, void *);
typedef int (*FP_stream_connect_record)(pa_stream *, const char *, const pa_buffer_attr *, pa_stream_flags_t);
typedef size_t (*FP_stream_readable_size)(pa_stream *);
typedef size_t (*FP_stream_writable_size)(pa_stream *);
typedef int (*FP_stream_drop)(pa_stream *);
typedef const pa_buffer_attr * (*FP_stream_get_buffer_attr)(pa_stream *);
typedef const char * (*FP_stream_get_device_name)(pa_stream *);
typedef void (*FP_context_set_subscribe_callback)(pa_context *, pa_context_subscribe_cb_t, void *);
typedef pa_operation * (*FP_context_subscribe)(pa_context *, pa_subscription_mask_t, pa_context_success_cb_t, void *);
typedef void (*FP_mainloop_api_once)(pa_mainloop_api *, void (*callback)(pa_mainloop_api*, void *, void *));
 
