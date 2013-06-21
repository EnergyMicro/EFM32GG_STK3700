/***************************************************************************//**
 * @file descriptors.h
 * @brief USB descriptors for PHDC Continua Medical Device.
 * @author Energy Micro AS and Lamprey Networks, Inc.
 *         This file was modified from the LED example code demo
 * @version 3.20.0
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2012 Energy Micro AS, http://www.energymicro.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 * 4. The source and compiled code may only be used on Energy Micro "EFM32"
 *    microcontrollers and "EFR4" radios.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Energy Micro AS has no
 * obligation to support this Software. Energy Micro AS is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Energy Micro AS will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 *****************************************************************************/
#ifndef __PHDC_DESCRIPTORS_H
#define __PHDC_DESCRIPTORS_H

#include "em_usb.h"
/*** Function prototypes. ***/

int  SetupCmd(const USB_Setup_TypeDef *setup);
void UsbStateChange( USBD_State_TypeDef oldState, USBD_State_TypeDef newState );

/****************************************************************
 * PHDC Class Function Descriptor                               *
 ****************************************************************/
#define USB_PHDC_CLASSFUNCTION_DESCRIPTOR 0x20
#define PHDC_CLASS_bLength              0x04
#define PHDC_CLASS_bDescriptorType      USB_PHDC_CLASSFUNCTION_DESCRIPTOR
#define PHDC_CLASS_bPHDCDataCode        0x02
#define PHDC_CLASS_bmCapability         0x00

/****************************************************************
 * PHDC Function Extension                                      *
 ****************************************************************/
#define PHDC_11073PHD_FUNCTION_DESCRIPTOR    0x30
#define PHDC_FUNC_bLength               0x06  /* Depends upon number of device specializations */
#define PHDC_FUNC_bDescriptorType       PHDC_11073PHD_FUNCTION_DESCRIPTOR
#define PHDC_FUNC_bReserved             0x00
#define PHDC_FUNC_bNumberDevSpecs       0x01
#define PHDC_FUNC_bGlucometer_L         0x11
#define PHDC_FUNC_bGlucometer_H         0x10

/****************************************************************
 * PHDC QoS (Endpoints 1 & 2)                                   *
 ****************************************************************/
#define USB_PHDC_QOS_DESCRIPTOR         0x21
#define PHDC_QOS_bLength                0x04
#define PHDC_QOS_bDescriptorType        USB_PHDC_QOS_DESCRIPTOR
#define PHDC_QOS_bQoSEncodingVersion    0x01
#define PHDC_QOS_bmLatencyReliability   0x08

/****************************************************************
 * Endpoint1 Descriptor (Endpoint that sends data to host)      *
 ****************************************************************/
#define BULK_IN_EP_ADDR                 0x81             /* Address for bulk in */

#define C_EP1_DSC_bLength               USB_ENDPOINT_DESCSIZE
#define C_EP1_DSC_bDescriptorType       USB_ENDPOINT_DESCRIPTOR
#define C_EP1_DSC_bEndpointAddress      BULK_IN_EP_ADDR  /*Bulk IN*/
#define C_EP1_DSC_bmAttributes          0x02             /*Bulk EndPoint*/
#define C_EP1_DSC_wMaxPacketSize_L      USB_EP0_SIZE     /*EP FIFO : 64byte*/
#define C_EP1_DSC_wMaxPacketSize_H      0x00
#define C_EP1_DSC_bInterval             0x00             /*no use*/

/****************************************************************
 * Endpoint2 Descriptor (Endpoint that reads data from host)    *
 ****************************************************************/
#define BULK_OUT_EP_ADDR                0x02             /* Address for bulk out */

#define C_EP2_DSC_bLength               USB_ENDPOINT_DESCSIZE
#define C_EP2_DSC_bDescriptorType       USB_ENDPOINT_DESCRIPTOR
#define C_EP2_DSC_bEndpointAddress      BULK_OUT_EP_ADDR /*Bulk OUT*/
#define C_EP2_DSC_bmAttributes          0x02             /*Bulk EndPoint*/
#define C_EP2_DSC_wMaxPacketSize_L      USB_EP0_SIZE     /*EP FIFO : 64byte*/
#define C_EP2_DSC_wMaxPacketSize_H      0x00
#define C_EP2_DSC_bInterval             0x00             /*no use*/

#endif
