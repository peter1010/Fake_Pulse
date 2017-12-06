/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "threaded_mainloop.hpp"

struct ThreadedMainLoop
{
    pthread_t thread;
    pthread_mutex_t mutex;
    bool running;
    pa_mainloop_api api;
};

static void * thread_entry(void * arg)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)arg;
    while(loop->running) {
        sleep(1);
    }
    return NULL;
}


pa_threaded_mainloop * my_threaded_mainloop_new(void)
{
    pthread_mutexattr_t attr;
    struct ThreadedMainLoop * loop = new struct ThreadedMainLoop;
    loop->running = false;
    do {
        if(pthread_mutexattr_init(&attr) != 0)
            break;;
        if(pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
            break;
        if(pthread_mutex_init(&loop->mutex, &attr) != 0)
            break;

        if(pthread_mutexattr_destroy(&attr) != 0)
            break;
    }
    while(0);
    return (pa_threaded_mainloop *)loop;
}

void my_threaded_mainloop_free(pa_threaded_mainloop* m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    pthread_mutex_destroy(&loop->mutex);
    free(loop);
}


int my_threaded_mainloop_start(pa_threaded_mainloop * m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    loop->running = true;
    return pthread_create(&loop->thread, NULL, thread_entry, (void *)loop);
}

void my_threaded_mainloop_stop(pa_threaded_mainloop *m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    loop->running = false;
    pthread_join(loop->thread, NULL);
}

int my_threaded_mainloop_in_thread(pa_threaded_mainloop * m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    return (pthread_self() == loop->thread) ? 1 : 0;
}

void my_threaded_mainloop_lock(pa_threaded_mainloop * m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    pthread_mutex_lock(&loop->mutex);
}

void my_threaded_mainloop_unlock(pa_threaded_mainloop * m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    pthread_mutex_unlock(&loop->mutex);
}


pa_mainloop_api * my_threaded_mainloop_get_api(pa_threaded_mainloop * m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    return &loop->api;
}
