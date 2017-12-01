/**
 *
 * Copyright (c) 2017 Peter Leese
 *
 * Licensed under the GPL License. See LICENSE file in the project root for full license information.  
 */

#include "logging.h"
#include "pulse.h"

const char * pa_get_library_version()
{
    DEBUG_MSG("%s called", __func__);
    return "11.1.0";
}

int pa_channel_map_can_balance()
{
    DEBUG_MSG("%s called", __func__);
    return 0;
}

struct fake_channel_map * pa_channel_map_init(struct fake_channel_map * map)
{
    DEBUG_MSG("%s called", __func__);
    return map;
}
