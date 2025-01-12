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
/**   Audio Class                                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"
#include "ux_host_class_audio.h"
#include "ux_host_stack.h"


/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_host_class_audio_write                          PORTABLE C      */ 
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function writes to the audio streaming interface.              */ 
/*                                                                        */
/*    Note the request packet size should not exceed endpoint max packet  */
/*    size, and the request length should be aligned with packet size.    */
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    audio                                 Pointer to audio class        */ 
/*    audio_transfer_request                Pointer to transfer request   */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_host_class_audio_transfer_request Start audio transfer request  */ 
/*    _ux_host_stack_class_instance_verify  Verify instance is valid      */ 
/*    _ux_host_mutex_on                     Get mutex                     */
/*    _ux_host_mutex_off                    Release mutex                 */
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
/*    Audio Class                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Chaoqiong Xiao           Modified comment(s),          */
/*                                            refined macros names,       */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022     Chaoqiong Xiao           Modified comment(s),          */
/*                                            fixed standalone compile,   */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Chaoqiong Xiao           Modified comment(s),          */
/*                                            refined packet size manage, */
/*                                            protect reentry with mutex, */
/*                                            refined transfer implement, */
/*                                            fixed error return code,    */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT  _ux_host_class_audio_write(UX_HOST_CLASS_AUDIO *audio, UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST *audio_transfer_request)
{

UINT        status;
ULONG       mps;

    /* If trace is enabled, insert this event into the trace buffer.  */
    UX_TRACE_IN_LINE_INSERT(UX_TRACE_HOST_CLASS_AUDIO_WRITE, audio, audio_transfer_request -> ux_host_class_audio_transfer_request_data_pointer, 
                            audio_transfer_request -> ux_host_class_audio_transfer_request_requested_length, 0, UX_TRACE_HOST_CLASS_EVENTS, 0, 0)

    /* Ensure the instance is valid.  */
    if (_ux_host_stack_class_instance_verify(_ux_system_host_class_audio_name, (VOID *) audio) != UX_SUCCESS)
    {        

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_CLASS, UX_HOST_CLASS_INSTANCE_UNKNOWN);

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_HOST_CLASS_INSTANCE_UNKNOWN, audio, 0, 0, UX_TRACE_ERRORS, 0, 0)

        return(UX_HOST_CLASS_INSTANCE_UNKNOWN);
    }

    /* Protect thread reentry to this instance.  */
    _ux_host_mutex_on(&audio -> ux_host_class_audio_mutex);

    /* Ensure we have a selected interface that allows isoch transmission.  */
    if (audio -> ux_host_class_audio_isochronous_endpoint -> ux_endpoint_descriptor.wMaxPacketSize == 0)
    {

        /* Unprotect thread reentry to this instance.  */
        _ux_host_mutex_off(&audio -> ux_host_class_audio_mutex);

        /* Error trap. */
        _ux_system_error_handler(UX_SYSTEM_LEVEL_THREAD, UX_SYSTEM_CONTEXT_CLASS, UX_HOST_CLASS_AUDIO_WRONG_INTERFACE);

        /* Return error status.  */
        return(UX_HOST_CLASS_AUDIO_WRONG_INTERFACE);
    }

    /* Correct packet size to apply (not exceeding endpoint max packet size).  */
    mps = _ux_host_class_audio_max_packet_size_get(audio);
    if ((audio_transfer_request -> ux_host_class_audio_transfer_request_packet_size == 0) ||
        (audio_transfer_request -> ux_host_class_audio_transfer_request_packet_size > mps))
        audio_transfer_request -> ux_host_class_audio_transfer_request_packet_size = mps;

     /* Ask the stack to hook this transfer request to the iso ED.  */
    status =  _ux_host_class_audio_transfer_request(audio, audio_transfer_request);

    /* Unprotect thread reentry to this instance.  */
    _ux_host_mutex_off(&audio -> ux_host_class_audio_mutex);

    /* Return completion status.  */
    return(status);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _uxe_host_class_audio_write                         PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks errors in audio write function call.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    audio                                 Pointer to audio class        */ 
/*    audio_transfer_request                Pointer to transfer request   */ 
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_host_class_audio_write           Hook a audio write request     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Chaoqiong Xiao           Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
UINT  _uxe_host_class_audio_write(UX_HOST_CLASS_AUDIO *audio, UX_HOST_CLASS_AUDIO_TRANSFER_REQUEST *audio_transfer_request)
{

    /* Sanity checks.  */
    if ((UX_NULL == audio) || (audio_transfer_request == UX_NULL))
        return(UX_INVALID_PARAMETER);

    /* Invoke audio write function.  */
    return(_ux_host_class_audio_write(audio, audio_transfer_request));
}
