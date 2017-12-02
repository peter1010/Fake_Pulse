/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

#include "logging.h"

#define _UNUSED __attribute__((unused))

extern void * __libc_dlsym(void * handle, const char * symbol);
extern void * _dl_sym(void * handle, const char * symbol, void * callee);

void * libHandle = NULL;

#define MAGIC_HND (void *)(0x1865FE45)
/**
 * 
 *
 */
void * dlopen(const char * filename, int flags)
{
    void * handle;

    if(filename && (strncmp("libpulse.so", filename, 11) == 0)) {
        DEBUG_MSG("dlopen called for %s", filename);
        handle = MAGIC_HND;
    } else {
        handle = dlmopen(LM_ID_BASE, filename, flags);
    }
    return handle;
}

int pa_dummy()
{
    DEBUG_MSG("pa_dummy called\n");
    return 0;
}

void * dlsym(void * handle, const char * symbol)
{
    int pa = 0;
    void * address = NULL;
    static void * (*the_real_one)(void * handle, const char *symbol) = NULL;

    if(handle == MAGIC_HND) {
        DEBUG_MSG("dlsym called for %s", symbol);
        handle = (void *)RTLD_DEFAULT;
        pa = 1;
    }

    if(the_real_one == NULL) {
//        the_real_one = __libc_dlsym(RTLD_NEXT, "dlsym");
        the_real_one = _dl_sym(RTLD_NEXT, "dlsym", dlsym);
    }
    address = the_real_one(handle, symbol);
    if(address == NULL) {
        DEBUG_MSG("No symbole %s found", symbol);
        if(pa) {
            return pa_dummy;
        }
    }
    return address;
}

int dlclose(void * handle)
{
    static int (*the_real_one)(void * handle) = NULL;

    if(handle == MAGIC_HND) {
        DEBUG_MSG("dlclose called");
        return 0;
    } 
    if(the_real_one == NULL) {
        the_real_one = dlsym(RTLD_NEXT, "dlclose");
    }
    return the_real_one(handle);
}

#if 0
char * getenv(const char * name)
{
    fprintf(stderr, "stderr: getenv called for %s\n", name);
    fflush(stderr);
    return NULL;
}
#endif
