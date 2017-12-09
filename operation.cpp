/**
 *
 * copyright (c) 2017 peter leese
 *
 * licensed under the gpl license. see license file in the project root for full license information.  
 */


#include "operation.hpp"

pa_operation_state_t COperation::get_state() const 
{
    return PA_OPERATION_DONE; 
};
