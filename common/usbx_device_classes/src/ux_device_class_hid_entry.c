/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** USBX Component                                                        */ 
/**                                                                       */
/**   Device HID Class                                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_device_class_hid.h"
#include "ux_device_stack.h"
 

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_device_class_hid_entry                          PORTABLE C      */ 
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function is the entry point of the hid class. It               */ 
/*    will be called by the device stack enumeration module when the      */ 
/*    host has sent a SET_CONFIGURATION command and the hid interface     */
/*    needs to be mounted.                                                */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    command                               Pointer to class command      */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_device_class_hid_initialize       Initialize hid class          */
/*    _ux_device_class_hid_uninitialize     Uninitialize hid class        */
/*    _ux_device_class_hid_activate         Activate hid class            */ 
/*    _ux_device_class_hid_deactivate       Deactivate hid class          */ 
/*    _ux_device_class_hid_control_request  Request control               */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    HID Class                                                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  xx-xx-xxxx     Chaoqiong Xiao           Modified comment(s),          */
/*                                            added error checks support, */
/*                                            resulting in version 6.x    */
/*                                                                        */
/**************************************************************************/
UINT  _ux_device_class_hid_entry(UX_SLAVE_CLASS_COMMAND *command)
{

UINT        status;


    /* The command request will tell us we need to do here, either a enumeration
       query, an activation or a deactivation.  */
    switch (command -> ux_slave_class_command_request)
    {

    case UX_SLAVE_CLASS_COMMAND_INITIALIZE:

        /* Call the init function of the HID class.  */
#if defined(UX_DEVICE_CLASS_HID_ENABLE_ERROR_CHECKING)
        status =  _uxe_device_class_hid_initialize(command);
#else
        status =  _ux_device_class_hid_initialize(command);
#endif

        /* Return the completion status.  */
        return(status);

    case UX_SLAVE_CLASS_COMMAND_UNINITIALIZE:

        /* Call the init function of the HID class.  */
        status =  _ux_device_class_hid_uninitialize(command);
        
        /* Return the completion status.  */
        return(status);

    case UX_SLAVE_CLASS_COMMAND_QUERY:

        /* Check the CLASS definition in the interface descriptor. */
        if (command -> ux_slave_class_command_class == UX_DEVICE_CLASS_HID_CLASS)
            return(UX_SUCCESS);
        else
            return(UX_NO_CLASS_MATCH);

    case UX_SLAVE_CLASS_COMMAND_ACTIVATE:

        /* The activate command is used when the host has sent a SET_CONFIGURATION command
           and this interface has to be mounted. Both Bulk endpoints have to be mounted
           and the hid thread needs to be activated.  */
        status =  _ux_device_class_hid_activate(command);

        /* Return the completion status.  */
        return(status);

    case UX_SLAVE_CLASS_COMMAND_DEACTIVATE:

        /* The deactivate command is used when the device has been extracted.
           The device endpoints have to be dismounted and the hid thread canceled.  */
        status =  _ux_device_class_hid_deactivate(command);
        
        /* Return the completion status.  */
        return(status);

    case UX_SLAVE_CLASS_COMMAND_REQUEST:

        /* The request command is used when the host sends a command on the control endpoint.  */
        status = _ux_device_class_hid_control_request(command);

        /* Return the completion status.  */
        return(status);

    default: 

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_FUNCTION_NOT_SUPPORTED, 0, 0, 0, UX_TRACE_ERRORS, 0, 0)

        /* Return an error.  */
        return(UX_FUNCTION_NOT_SUPPORTED);
    }   
}

