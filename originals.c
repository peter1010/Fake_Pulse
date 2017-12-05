/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "defs.h"

#define DEFINE_FP(name) FP_##name zz_##name = NULL

DEFINE_FP(get_library_version);
DEFINE_FP(channel_map_can_balance);
DEFINE_FP(channel_map_init);
DEFINE_FP(context_connect);
DEFINE_FP(context_disconnect);
DEFINE_FP(context_drain);
DEFINE_FP(context_get_server_info);
DEFINE_FP(context_get_sink_info_by_name);
DEFINE_FP(context_get_sink_info_list);
DEFINE_FP(context_get_sink_input_info);
DEFINE_FP(context_get_source_info_list);
DEFINE_FP(context_get_state);
DEFINE_FP(context_new);
DEFINE_FP(context_rttime_new);
DEFINE_FP(context_set_sink_input_volume);
DEFINE_FP(context_set_state_callback);
DEFINE_FP(context_unref);
DEFINE_FP(cvolume_set);
DEFINE_FP(cvolume_set_balance);
DEFINE_FP(frame_size);
DEFINE_FP(operation_get_state);
DEFINE_FP(operation_unref);
DEFINE_FP(proplist_gets);
DEFINE_FP(rtclock_now);
DEFINE_FP(stream_begin_write);
DEFINE_FP(stream_cancel_write);
DEFINE_FP(stream_connect_playback);
DEFINE_FP(stream_cork);
DEFINE_FP(stream_disconnect);
DEFINE_FP(stream_get_channel_map);
DEFINE_FP(stream_get_index);
DEFINE_FP(stream_get_latency);
DEFINE_FP(stream_get_sample_spec);
DEFINE_FP(stream_get_state);
DEFINE_FP(stream_get_time);
DEFINE_FP(stream_new);
DEFINE_FP(stream_peek);
DEFINE_FP(stream_set_state_callback);
DEFINE_FP(stream_set_write_callback);
DEFINE_FP(stream_unref);
DEFINE_FP(stream_update_timing_info);
DEFINE_FP(stream_write);
DEFINE_FP(sw_volume_from_linear);
DEFINE_FP(threaded_mainloop_free);
DEFINE_FP(threaded_mainloop_get_api);
DEFINE_FP(threaded_mainloop_in_thread);
DEFINE_FP(threaded_mainloop_lock);
DEFINE_FP(threaded_mainloop_new);
DEFINE_FP(threaded_mainloop_signal);
DEFINE_FP(threaded_mainloop_start);
DEFINE_FP(threaded_mainloop_stop);
DEFINE_FP(threaded_mainloop_unlock);
DEFINE_FP(threaded_mainloop_wait);
DEFINE_FP(usec_to_bytes);
DEFINE_FP(stream_set_read_callback);
DEFINE_FP(stream_connect_record);
DEFINE_FP(stream_readable_size);
DEFINE_FP(stream_writable_size);
DEFINE_FP(stream_drop);
DEFINE_FP(stream_get_buffer_attr);
DEFINE_FP(stream_get_device_name);
DEFINE_FP(context_set_subscribe_callback);
DEFINE_FP(context_subscribe);
DEFINE_FP(mainloop_api_once);

