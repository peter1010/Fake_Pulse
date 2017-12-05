/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#include "my_threaded_mainloop.h"

struct ThreadedMainLoop
{
    pthread_t thread;
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


pa_threaded_mainloop * threaded_mainloop_new(void)
{
    struct ThreadedMainLoop * loop = malloc(sizeof(struct ThreadedMainLoop));
    return (pa_threaded_mainloop *)loop;
}

void threaded_mainloop_free(pa_threaded_mainloop* m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    free(loop);
}


int threaded_mainloop_start(pa_threaded_mainloop * m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    loop->running = true;
    return pthread_create(&loop->thread, NULL, thread_entry, (void *)loop);
}

void threaded_mainloop_stop(pa_threaded_mainloop *m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    loop->running = false;
    pthread_join(loop->thread, NULL);
}

pa_mainloop_api * my_threaded_mainloop_get_api(pa_threaded_mainloop * m)
{
    struct ThreadedMainLoop * loop = (struct ThreadedMainLoop *)m;
    return &loop->api;
}
