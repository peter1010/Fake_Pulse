#ifndef _MY_THREADED_MAINLOOP_H_
#define _MY_THREADED_MAINLOOP_H_

/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "pulseaudio.h"

extern pa_threaded_mainloop * my_threaded_mainloop_new(void);

extern int my_threaded_mainloop_start(pa_threaded_mainloop * m);

extern void my_threaded_mainloop_stop(pa_threaded_mainloop *m);

extern void my_threaded_mainloop_free(pa_threaded_mainloop* m);
    
extern pa_mainloop_api * my_threaded_mainloop_get_api(pa_threaded_mainloop * m);

extern int my_threaded_mainloop_in_thread(pa_threaded_mainloop * m);

extern void my_threaded_mainloop_lock(pa_threaded_mainloop * m);

extern void my_threaded_mainloop_unlock(pa_threaded_mainloop * m);
#endif

