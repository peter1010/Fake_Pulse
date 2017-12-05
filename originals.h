/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "defs.h"

#define DECLARE_FP(name) extern FP_##name zz_##name;

DECLARE_FP(get_library_version);
DECLARE_FP(channel_map_can_balance);
DECLARE_FP(channel_map_init);
DECLARE_FP(context_connect);
DECLARE_FP(context_disconnect);
DECLARE_FP(context_drain);
DECLARE_FP(context_get_server_info);
DECLARE_FP(context_get_sink_info_by_name);
DECLARE_FP(context_get_sink_info_list);
DECLARE_FP(context_get_sink_input_info);
DECLARE_FP(context_get_source_info_list);
DECLARE_FP(context_get_state);
DECLARE_FP(context_new);
DECLARE_FP(context_rttime_new);
DECLARE_FP(context_set_sink_input_volume);
DECLARE_FP(context_set_state_callback);
DECLARE_FP(context_unref);
DECLARE_FP(cvolume_set);
DECLARE_FP(cvolume_set_balance);
DECLARE_FP(frame_size);
DECLARE_FP(operation_get_state);
DECLARE_FP(operation_unref);
DECLARE_FP(proplist_gets);
DECLARE_FP(rtclock_now);
DECLARE_FP(stream_begin_write);
DECLARE_FP(stream_cancel_write);
DECLARE_FP(stream_connect_playback);
DECLARE_FP(stream_cork);
DECLARE_FP(stream_disconnect);
DECLARE_FP(stream_get_channel_map);
DECLARE_FP(stream_get_index);
DECLARE_FP(stream_get_latency);
DECLARE_FP(stream_get_sample_spec);
DECLARE_FP(stream_get_state);
DECLARE_FP(stream_get_time);
DECLARE_FP(stream_new);
DECLARE_FP(stream_peek);
DECLARE_FP(stream_set_state_callback);
DECLARE_FP(stream_set_write_callback);
DECLARE_FP(stream_unref);
DECLARE_FP(stream_update_timing_info);
DECLARE_FP(stream_write);
DECLARE_FP(sw_volume_from_linear);
DECLARE_FP(threaded_mainloop_free);
DECLARE_FP(threaded_mainloop_get_api);
DECLARE_FP(threaded_mainloop_in_thread);
DECLARE_FP(threaded_mainloop_lock);
DECLARE_FP(threaded_mainloop_new);
DECLARE_FP(threaded_mainloop_signal);
DECLARE_FP(threaded_mainloop_start);
DECLARE_FP(threaded_mainloop_stop);
DECLARE_FP(threaded_mainloop_unlock);
DECLARE_FP(threaded_mainloop_wait);
DECLARE_FP(usec_to_bytes);
DECLARE_FP(stream_set_read_callback);
DECLARE_FP(stream_connect_record);
DECLARE_FP(stream_readable_size);
DECLARE_FP(stream_writable_size);
DECLARE_FP(stream_drop);
DECLARE_FP(stream_get_buffer_attr);
DECLARE_FP(stream_get_device_name);
DECLARE_FP(context_set_subscribe_callback);
DECLARE_FP(context_subscribe);
DECLARE_FP(mainloop_api_once);

