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
/**   Generic Serial Host module class                                    */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/* Include necessary system files.  */

#define UX_SOURCE_CODE

#include "ux_api.h"
#include "ux_host_class_gser.h"
#include "ux_host_stack.h"

/**************************************************************************/ 
/*                                                                        */ 
/*  FUNCTION                                               RELEASE        */ 
/*                                                                        */ 
/*    _ux_host_class_gser_write                           PORTABLE C      */ 
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */ 
/*    This function writes to the gser interface. The call is blocking    */ 
/*    and only returns when there is either an error or when the transfer */ 
/*    is complete.                                                        */ 
/*                                                                        */ 
/*  INPUT                                                                 */ 
/*                                                                        */ 
/*    gser                                  Pointer to gser class         */ 
/*    data_pointer                          Pointer to data to write      */ 
/*    requested_length                      Length of data to write       */ 
/*    actual_length                         Actual length of data written */ 
/*                                                                        */ 
/*  OUTPUT                                                                */ 
/*                                                                        */ 
/*    Completion Status                                                   */ 
/*                                                                        */ 
/*  CALLS                                                                 */ 
/*                                                                        */ 
/*    _ux_host_stack_transfer_request       Process transfer request      */ 
/*    _ux_host_stack_transfer_request_abort Abort transfer request        */ 
/*    _ux_host_semaphore_get                Get protection semaphore      */ 
/*    _ux_host_semaphore_put                Release protection semaphore  */ 
/*                                                                        */ 
/*  CALLED BY                                                             */ 
/*                                                                        */ 
/*    Application                                                         */ 
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */ 
/*                                                                        */ 
/*  05-19-2020     Chaoqiong Xiao           Initial Version 6.0           */
/*  09-30-2020     Chaoqiong Xiao           Modified comment(s),          */
/*                                            prefixed UX to MS_TO_TICK,  */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Chaoqiong Xiao           Modified comment(s),          */
/*                                            refined macros names,       */
/*                                            resulting in version 6.1.10 */
/*  04-25-2022     Chaoqiong Xiao           Modified comment(s),          */
/*                                            fixed standalone compile,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT  _ux_host_class_gser_write(UX_HOST_CLASS_GSER *gser, 
                                    ULONG interface_index,
                                    UCHAR * data_pointer, 
                                    ULONG requested_length, 
                                    ULONG *actual_length)
{

UX_TRANSFER     *transfer_request;
UINT            status;
ULONG           transfer_request_length;
    
    /* If trace is enabled, insert this event into the trace buffer.  */
    UX_TRACE_IN_LINE_INSERT(UX_TRACE_HOST_CLASS_GSER_WRITE, gser, data_pointer, requested_length, 0, UX_TRACE_HOST_CLASS_EVENTS, 0, 0)
  
    /* Ensure the instance is valid.  */
    if (gser -> ux_host_class_gser_state !=  UX_HOST_CLASS_INSTANCE_LIVE)
    {       

        /* If trace is enabled, insert this event into the trace buffer.  */
        UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_HOST_CLASS_INSTANCE_UNKNOWN, gser, 0, 0, UX_TRACE_ERRORS, 0, 0)

    }

    /* Protect thread reentry to this instance.  */
    status =  _ux_host_semaphore_get(&gser -> ux_host_class_gser_interface_array[interface_index].ux_host_class_gser_semaphore, UX_WAIT_FOREVER);
    if (status != UX_SUCCESS)
        return(status);

    /* Start by resetting the actual length of the transfer.  */
    *actual_length =  0;

    /* Get the pointer to the bulk out endpoint transfer request.  */
    transfer_request =  &gser -> ux_host_class_gser_interface_array[interface_index].ux_host_class_gser_bulk_out_endpoint -> ux_endpoint_transfer_request;
    
    /* Save the interface number in the Transfer Request. */
    transfer_request -> ux_transfer_request_user_specific = (VOID *) (ALIGN_TYPE) interface_index;
    
    /* Perform a transfer on the bulk out endpoint until either the transfer is
       completed or when there is an error.  */
    do
    {

        /* Program the maximum authorized length for this transfer_request.  */
        if (requested_length > transfer_request -> ux_transfer_request_maximum_length)
            transfer_request_length =  transfer_request -> ux_transfer_request_maximum_length;
        else
            transfer_request_length =  requested_length;
                    
        /* Initialize the transfer_request.  */
        transfer_request -> ux_transfer_request_data_pointer =  data_pointer;
        transfer_request -> ux_transfer_request_requested_length =  transfer_request_length;
        
        /* Perform the transfer.  */
        status =  _ux_host_stack_transfer_request(transfer_request);

        /* If the transfer is successful, we need to wait for the transfer request to be completed.  */
        if (status == UX_SUCCESS)
        {
            
            /* Wait for the completion of the transfer request.  */
            status =  _ux_host_semaphore_get(&transfer_request -> ux_transfer_request_semaphore, UX_MS_TO_TICK(UX_HOST_CLASS_GSER_CLASS_TRANSFER_TIMEOUT));

            /* If the semaphore did not succeed we probably have a time out.  */
            if (status != UX_SUCCESS)
            {

                /* All transfers pending need to abort. There may have been a partial transfer.  */
                _ux_host_stack_transfer_request_abort(transfer_request);
                
                /* Update the length of the actual data transferred. We do this after the 
                   abort of the transfer_request in case some data actually went out.  */
                *actual_length +=  transfer_request -> ux_transfer_request_actual_length;
            
                /* Unprotect thread reentry to this instance.  */
                _ux_host_semaphore_put(&gser -> ux_host_class_gser_interface_array[interface_index].ux_host_class_gser_semaphore);

                /* Set the completion code.  */
                transfer_request -> ux_transfer_request_completion_code =  UX_TRANSFER_TIMEOUT;
        
                /* If trace is enabled, insert this event into the trace buffer.  */
                UX_TRACE_IN_LINE_INSERT(UX_TRACE_ERROR, UX_TRANSFER_TIMEOUT, transfer_request, 0, 0, UX_TRACE_ERRORS, 0, 0)
        
                /* There was an error, return to the caller.  */
                return(UX_TRANSFER_TIMEOUT);
            }            
        }
        else
        {

            /* Unprotect thread reentry to this instance.  */
            _ux_host_semaphore_put(&gser -> ux_host_class_gser_interface_array[interface_index].ux_host_class_gser_semaphore);

            /* There was a non transfer error, no partial transfer to be checked */
            return(status);
        }

        /* Update the length of the transfer. Normally all the data has to be sent.  */
        *actual_length +=  transfer_request -> ux_transfer_request_actual_length;
        
        /* Check for completion of transfer. If the transfer is partial, return to caller.
           The transfer is marked as successful but the caller will need to check the length
           actually sent and determine if a partial transfer is OK. */
        if (transfer_request_length !=  transfer_request -> ux_transfer_request_actual_length)
        {

            /* Unprotect thread reentry to this instance.  */
            _ux_host_semaphore_put(&gser -> ux_host_class_gser_interface_array[interface_index].ux_host_class_gser_semaphore);

            /* Return success.  */
            return(UX_SUCCESS);
        }

        /* Update the data pointer for next transfer.  */        
        data_pointer +=  transfer_request_length;
        
        /* Update what is left to send out.  */
        requested_length -=  transfer_request_length;          

    } while (requested_length != 0);
        

    /* Unprotect thread reentry to this instance.  */
    _ux_host_semaphore_put(&gser -> ux_host_class_gser_interface_array[interface_index].ux_host_class_gser_semaphore);

    /* We get here when all the transfers went through without errors.  */
    return(UX_SUCCESS); 
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _uxe_host_class_gser_write                           PORTABLE C     */
/*                                                           6.x          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Chaoqiong Xiao, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks errors in GSER write function call.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    gser                                  Pointer to GSER class         */ 
/*    data_pointer                          Pointer to buffer             */
/*    requested_length                      Requested data write          */
/*    actual_length                         Actual data write             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _ux_host_class_gser_write              GSER write                   */
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
UINT  _uxe_host_class_gser_write(UX_HOST_CLASS_GSER *gser, 
                                    ULONG interface_index,
                                    UCHAR *data_pointer, 
                                    ULONG requested_length, 
                                    ULONG *actual_length)
{

    /* Sanity checks.  */
    if ((gser == UX_NULL) ||
        ((data_pointer == UX_NULL) && (requested_length != 0)) ||
        (actual_length == UX_NULL))
    {
        return(UX_INVALID_PARAMETER);
    }

    /* Invoke GSER write function.  */
    return(_ux_host_class_gser_write(gser, interface_index, data_pointer, requested_length, actual_length));
}
