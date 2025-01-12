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
/**   Device Video Class                                                  */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define UX_SOURCE_CODE


/* Include necessary system files.  */

#include "ux_api.h"
#include "ux_device_class_video.h"
#include "ux_device_stack.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _ux_device_class_video_read_payload_get             PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function obtain payload access pointer from the Video class.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    stream                                Address of video stream       */
/*                                            instance                    */
/*    payload_data                          Pointer to buffer to save     */
/*                                          pointer to payload data       */
/*    payload_length                        Pointer to buffer to save     */
/*                                          Pointer to payload length     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  04-25-2022     Chaoqiong Xiao           Initial Version 6.1.11        */
/*  xx-xx-xxxx     Yajun Xia                Modified comment(s),          */
/*                                            resulting in version 6.x    */
/*                                                                        */
/**************************************************************************/
UINT _ux_device_class_video_read_payload_get(UX_DEVICE_CLASS_VIDEO_STREAM *stream,
                                           UCHAR **payload_data, ULONG *payload_length)
{

UX_SLAVE_ENDPOINT           *endpoint;
UX_SLAVE_DEVICE             *device;


    /* Get the pointer to the device.  */
    device =  &_ux_system_slave -> ux_system_slave_device;

    /* As long as the device is in the CONFIGURED state.  */
    if (device -> ux_slave_device_state != UX_DEVICE_CONFIGURED)
    {

        /* Cannot proceed with command, the interface is down.  */
        return(UX_CONFIGURATION_HANDLE_UNKNOWN);
    }

    /* Check if endpoint is available.  */
    endpoint = stream -> ux_device_class_video_stream_endpoint;
    if (endpoint == UX_NULL)
        return(UX_ERROR);

    /* Check if endpoint direction is OK.  */
    if ((endpoint -> ux_slave_endpoint_descriptor.bEndpointAddress & UX_ENDPOINT_DIRECTION) != UX_ENDPOINT_OUT)
        return(UX_ERROR);

    /* Underflow!!  */
    if (stream -> ux_device_class_video_stream_access_pos -> ux_device_class_video_payload_length == 0)
    {
        return(UX_BUFFER_OVERFLOW);
    }

    /* Obtain payload structure pointer.  */
    *payload_data = stream -> ux_device_class_video_stream_access_pos -> ux_device_class_video_payload_data;
    *payload_length = stream -> ux_device_class_video_stream_access_pos -> ux_device_class_video_payload_length;

    return(UX_SUCCESS);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _uxe_device_class_video_read_payload_get            PORTABLE C      */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yajun Xia, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks errors in video read payload get function      */
/*    call.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    stream                                Address of video stream       */
/*                                            instance                    */
/*    payload_data                          Pointer to buffer to save     */
/*                                          pointer to payload data       */
/*    payload_length                        Pointer to buffer to save     */
/*                                          Pointer to payload length     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_device_class_video_read_payload_get                             */
/*                                          Video read payload get        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  xx-xx-xxxx     Yajun Xia                Initial Version 6.x           */
/*                                                                        */
/**************************************************************************/
UINT _uxe_device_class_video_read_payload_get(UX_DEVICE_CLASS_VIDEO_STREAM *stream,
                                           UCHAR **payload_data, ULONG *payload_length)
{

    /* Sanity check. */
    if ((stream == UX_NULL) || (payload_data == UX_NULL) || (payload_length == UX_NULL))
        return(UX_INVALID_PARAMETER);

    /* Call the actual video read payload get function.  */
    return(_ux_device_class_video_read_payload_get(stream, payload_data, payload_length));
}