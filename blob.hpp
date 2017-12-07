
#include "pulseaudio.h"

class CBlob
{
public:
    CBlob(void (*callback)(pa_mainloop_api *, void *), void * userdata);

    static void callback(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self); 

    static void free(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self);

private:
    void (*mCallback)(pa_mainloop_api *, void *); 
    void * mUserdata;

};
