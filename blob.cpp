
#include "blob.hpp"
#include "logging.h"

void CBlob::callback(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self) 
{
    CBlob * Self = reinterpret_cast<CBlob *>(self);

    DEBUG_MSG("%s called", __func__);
    Self->doCallback(mainloop);
    mainloop->defer_free(evt);
    DEBUG_MSG("%s returned", __func__);
}


void CBlob::free(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self)
{
    (void) mainloop;
    (void) evt;

    CBlob * Self = reinterpret_cast<CBlob *>(self);
    delete Self;
}
