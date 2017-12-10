
#include "pulseaudio.h"

class CBlob
{
public:
    CBlob() {};

    static void callback(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self); 

    static void free(pa_mainloop_api * mainloop, pa_defer_event * evt, void * self);

protected:
    virtual void doCallback(pa_mainloop_api * mainloop) = 0;
    virtual ~CBlob() = 0;
};


class CMainloopApiOnce : public CBlob
{
public:
    CMainloopApiOnce(void (*cb)(pa_mainloop_api *, void *), void * ud)
        : mCallback(cb), mUserdata(ud) {};

    virtual void doCallback(pa_mainloop_api * mainloop) { mCallback(mainloop, mUserdata); };

private:
    void (*mCallback)(pa_mainloop_api *, void *); 
    pa_mainloop_api * mMainloop;
    void * mUserdata;

protected:
    virtual ~CMainloopApiOnce() {};
};


class CStreamSuccessCb : public CBlob
{
public:
    CStreamSuccessCb(pa_stream_success_cb_t cb, pa_stream * st, int su, void * ud)
        : mCallback(cb), mStream(st), mSuccess(su), mUserdata(ud) {};

    virtual void doCallback(pa_mainloop_api * mainloop) { (void)mainloop; mCallback(mStream, mSuccess, mUserdata); };

private:
    pa_stream_success_cb_t mCallback;
    pa_stream * mStream;
    int mSuccess;
    void * mUserdata;

protected:
    virtual ~CStreamSuccessCb() {};
};


class CServerInfoCb : public CBlob
{
public:
    CServerInfoCb(pa_server_info_cb_t cb, pa_context * c, pa_server_info * in, void * ud)
        : mCallback(cb), mContext(c), mInfo(in), mUserdata(ud) {};

    virtual void doCallback(pa_mainloop_api * mainloop) { (void)mainloop; mCallback(mContext, mInfo, mUserdata); };
private:
    pa_server_info_cb_t mCallback;
    pa_context * mContext;
    pa_server_info * mInfo;
    void * mUserdata;

protected:
    virtual ~CServerInfoCb() {};
};
 
class CSinkInfoCb : public CBlob
{
public:
     CSinkInfoCb(pa_sink_info_cb_t cb, pa_context * c, pa_sink_info * in, void * ud)
        : mCallback(cb), mContext(c), mInfo(in), mUserdata(ud) {};

     virtual void doCallback(pa_mainloop_api * mainloop) { (void)mainloop; mCallback(mContext, mInfo, mInfo ? 0:1, mUserdata); };
private:
    pa_sink_info_cb_t mCallback;
    pa_context * mContext;
    pa_sink_info * mInfo;
    void * mUserdata;

protected:
    virtual ~CSinkInfoCb() {};
};
 

