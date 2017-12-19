
#include "blob.hpp"
#include "logging.h"

void CBlob::callback(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self) 
{
    CBlob * Self = reinterpret_cast<CBlob *>(self);

    if(Self) {
        DEBUG_MSG("CBlob::callback called");
        Self->doCallback(mainloop);
        if(mainloop && evt) {
            mainloop->defer_free(evt);
        }
        DEBUG_MSG("CBlob::callback returned");
    } else {
        DEBUG_MSG("CBlob::callback Self is none");
    }
}


void CBlob::free(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self)
{
    (void) mainloop;
    (void) evt;

    CBlob * Self = reinterpret_cast<CBlob *>(self);
    if(Self) {
        delete Self;
    }
}
