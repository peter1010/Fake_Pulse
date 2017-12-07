/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */


#include "config.h"

#ifdef INCLUDE_SIMPLE_THREADED_MAINLOOP

#include <stdbool.h>
#include <unistd.h>
#include "threaded_mainloop.hpp"


void * CThreadedMainloop::run()
{
    while(running) {
        pthread_mutex_lock(&mutex);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        
        sleep(1);
    }
    return NULL;
}


CThreadedMainloop::CThreadedMainloop()
{
   running = false;
}

CThreadedMainloop::~CThreadedMainloop()
{
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
}


int CThreadedMainloop::start()
{
    do {
        pthread_mutexattr_t attr;

        if(pthread_mutexattr_init(&attr) != 0)
            break;
        if(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
            break;
        if(pthread_mutex_init(&mutex, &attr) != 0)
            break;

        if(pthread_mutexattr_destroy(&attr) != 0)
            break;

        if(pthread_cond_init(&cond, NULL) != 0)
            break;
    }
    while(0);
    running = true;
    return pthread_create(&thread, NULL, static_run, reinterpret_cast<void *>(this));
}

void CThreadedMainloop::stop()
{
    running = false;
    pthread_join(thread, NULL);
}

int CThreadedMainloop::in_thread() const
{
    return (pthread_self() == thread) ? 1 : 0;
}

void CThreadedMainloop::lock()
{
    pthread_mutex_lock(&mutex);
}

void CThreadedMainloop::unlock()
{
    pthread_mutex_unlock(&mutex);
}


pa_mainloop_api * CThreadedMainloop::get_api()
{
    return &api;
}
    
void CThreadedMainloop::signal(int wait_for_accept)
{
}

void CThreadedMainloop::wait()
{
    pthread_cond_wait(&cond, &mutex);
}

#endif
