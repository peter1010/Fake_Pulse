
#include "blob.hpp"

void CBlob::callback(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self) 
{
    CBlob * Self = reinterpret_cast<CBlob *>(self);

    Self->doCallback(mainloop);
    mainloop->defer_free(evt);
}


void CBlob::free(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self)
{
    (void) mainloop;
    (void) evt;

    CBlob * Self = reinterpret_cast<CBlob *>(self);
    delete Self;
}
