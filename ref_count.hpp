#ifndef _REF_COUNT_HPP_
#define _REF_COUNT_HPP_

/**
 *
 * copyright (c) 2017 peter leese
 *
 * licensed under the gpl license. see license file in the project root for full license information.  
 */

#include "config.h"

class CRefCount
{
public:
    CRefCount() : mRefCount(0) {};

    void incRef() {mRefCount++; };
    void decRef() {if(mRefCount <= 0) delete this; else mRefCount--;};

private:
    unsigned int mRefCount;

    // Make private the default copy assignment not appropriate?
    CRefCount & operator=(const CRefCount & other) {(void)other; mRefCount=0; return *this;};
protected:
    virtual ~CRefCount() = 0;
};

// Pure virtual destructors still get called on deletion of deprived classes!
inline CRefCount::~CRefCount() {};

#endif
