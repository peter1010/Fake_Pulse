#ifndef _OPERATION_HPP_
#define _OPERATION_HPP_

/**
 *
 * copyright (c) 2017 peter leese
 *
 * licensed under the gpl license. see license file in the project root for full license information.  
 */


#include "config.h"
#include "ref_count.hpp"

#include "pulseaudio.h"

class COperation : public CRefCount
{
public:
    static COperation * from_pa(pa_operation * o) { return reinterpret_cast<COperation *>(o); };
    pa_operation * to_pa() { return reinterpret_cast<pa_operation *>(this); };

    COperation() {};

// PA_OPERATION_RUNNING,
// PA_OPERATION_DONE,
// PA_OPERATION_CANCELLED
 
    pa_operation_state_t get_state() const;
    

private:
    virtual ~COperation() {};
};

#endif
