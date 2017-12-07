#ifndef _MY_THREADED_MAINLOOP_H_
#define _MY_THREADED_MAINLOOP_H_

/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "config.h"

#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP

#include <pthread.h>

#include "pulseaudio.h"


class CThreadedMainloop
{
public:
    CThreadedMainloop();
    ~CThreadedMainloop();

    int start();
    void stop();

    pa_mainloop_api * get_api();

    int in_thread() const;

    void lock();
    void unlock();

    void signal(int wait_for_accept);
    void wait();
private:

    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool running;
    pa_mainloop_api api;

    static void * static_run(void * arg) { return reinterpret_cast<CThreadedMainloop *>(arg)->run(); };

    void * run();
};

#endif
#endif

