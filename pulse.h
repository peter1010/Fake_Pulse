#ifndef __PULSE_H__
#define __PULSE_H__

/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

// Concepts were taken from pulseaudio header files and adapted for my use.

#include <stdint.h>

enum fake_channel_position
{
    INVALID = -1,
    MONO = 0,
    LEFT,
    RIGHT
};

struct fake_channel_map
{
    uint8_t number_of_channels;
    enum fake_channel_position map[1];
};

#endif
