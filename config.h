#ifndef _CONFIG_H_
#define _CONFIG_H_

/**
 * Normally better to use the libpulse.so
 * threaded mainloop as it is fully
 * implemented. where this is not possible
 * define this (you will loss ability to
 * do I/O and Timer events
 */
//#define INCLUDE_SIMPLE_THREADED_MAINLOOP

#endif
