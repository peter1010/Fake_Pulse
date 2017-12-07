
#include "blob.hpp"

CBlob::CBlob(void (*callback)(pa_mainloop_api* m, void * userdata), void * userdata)
{
    mCallback = callback;
    mUserdata = userdata;
}

void CBlob::callback(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self) 
{
    CBlob * Self = reinterpret_cast<CBlob *>(self);

    Self->mCallback(mainloop, Self->mUserdata);
    mainloop->defer_free(evt);
}


void CBlob::free(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self)
{
    (void) mainloop;
    (void) evt;

    CBlob * Self = reinterpret_cast<CBlob *>(self);
    delete Self;
}
