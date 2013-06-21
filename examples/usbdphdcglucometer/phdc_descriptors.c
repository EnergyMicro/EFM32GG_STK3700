/***************************************************************************//**
 * @file phdc_descriptors.c
 * @brief USB descriptor implementation for PHDC Continua Medical Device.
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
#include "phdc_descriptors.h"

EFM32_ALIGN(4)
static const USB_DeviceDescriptor_TypeDef deviceDesc __attribute__ ((aligned(4)))=
{
  .bLength            = USB_DEVICE_DESCSIZE,
  .bDescriptorType    = USB_DEVICE_DESCRIPTOR,
  .bcdUSB             = 0x0200,       /* USB specification numbner */
  .bDeviceClass       = 0x00,         /* DeviceClass specified in the interface descriptor */
  .bDeviceSubClass    = 0,            /* Subclass is Specified in the Interface descriptor */
  .bDeviceProtocol    = 0,            /* Protocol is Specified in the Interface descriptor */
  .bMaxPacketSize0    = USB_EP0_SIZE, /* EP0 FIFO:64Byte (64-byte buffer) */
  .idVendor           = 0x2544,       /* Assume this is Energy Micro's Vendor ID */
  .idProduct          = 0x0006,       /* This is the product ID */
  .bcdDevice          = 0x0001,       /* Device release number in binary-coded decimal */
  .iManufacturer      = 1,            /* Index of string descriptor describing manufacturer */
  .iProduct           = 2,            /* Index of string descriptor describing product */
  .iSerialNumber      = 3,            /* Index of string descriptor describing serial number */
  .bNumConfigurations = 1             /* One configuration */
};

EFM32_ALIGN(4)
const uint8_t configDesc[] __attribute__ ((aligned(4)))=
{
  /*** Configuration descriptor ***/
  USB_CONFIG_DESCSIZE,    /* bLength                                   */
  USB_CONFIG_DESCRIPTOR,  /* bDescriptorType                           */

  0x32,                   /* wTotalLength (LSB) 0x32 bytes             */
  0,                      /* total length (MSB)                        */

  1,                      /* bNumInterfaces                            */
  1,                      /* bConfigurationValue                       */
  0,                      /* iConfiguration                            */
#if defined(BUSPOWERED)
  CONFIG_DESC_BM_RESERVED_D7,    /* bmAttrib: Bus powered              */
#else
  CONFIG_DESC_BM_RESERVED_D7 |   /* bmAttrib: Self powered             */
  CONFIG_DESC_BM_SELFPOWERED,
#endif

  CONFIG_DESC_MAXPOWER_mA( 100 ),/* bMaxPower: 100 mA                  */

  /*** Interface descriptor ***/
  USB_INTERFACE_DESCSIZE,  /* bLength               */
  USB_INTERFACE_DESCRIPTOR,/* bDescriptorType       */
  0,                       /* bInterfaceNumber      */
  0,                       /* bAlternateSetting     */
  NUM_EP_USED,             /* bNumEndpoints         */
  0x0F,                    /* bInterfaceClass       */
  0,                       /* bInterfaceSubClass    */
  0,                       /* bInterfaceProtocol    */
  0,                       /* iInterface            */

  /* PHDC Class Function Descriptor */
  PHDC_CLASS_bLength,
  PHDC_CLASS_bDescriptorType,
  PHDC_CLASS_bPHDCDataCode,
  PHDC_CLASS_bmCapability,

  /* PHDC Function Extension Descriptor */
  PHDC_FUNC_bLength,
  PHDC_FUNC_bDescriptorType,
  PHDC_FUNC_bReserved,
  PHDC_FUNC_bNumberDevSpecs,
  PHDC_FUNC_bGlucometer_L,
  PHDC_FUNC_bGlucometer_H,

  /* EP1 Descriptor */
  C_EP1_DSC_bLength,
  C_EP1_DSC_bDescriptorType,
  C_EP1_DSC_bEndpointAddress,
  C_EP1_DSC_bmAttributes,
  C_EP1_DSC_wMaxPacketSize_L,
  C_EP1_DSC_wMaxPacketSize_H,
  C_EP1_DSC_bInterval,

  /* EP1 QoS Descriptor */
  PHDC_QOS_bLength,
  PHDC_QOS_bDescriptorType,
  PHDC_QOS_bQoSEncodingVersion,
  PHDC_QOS_bmLatencyReliability,

  /* EP2 Descriptor */
  C_EP2_DSC_bLength,
  C_EP2_DSC_bDescriptorType,
  C_EP2_DSC_bEndpointAddress,
  C_EP2_DSC_bmAttributes,
  C_EP2_DSC_wMaxPacketSize_L,
  C_EP2_DSC_wMaxPacketSize_H,
  C_EP2_DSC_bInterval,

  /* EP2 QoS Descriptor */
  PHDC_QOS_bLength,
  PHDC_QOS_bDescriptorType,
  PHDC_QOS_bQoSEncodingVersion,
  PHDC_QOS_bmLatencyReliability
};

STATIC_CONST_STRING_DESC_LANGID( langID, 0x04, 0x09         );
STATIC_CONST_STRING_DESC( iManufacturer, L"Energy Micro AS" );
STATIC_CONST_STRING_DESC( iProduct     , L"EFM32 USB Glucometer Device" );
STATIC_CONST_STRING_DESC( iSerialNumber, L"000000000001"    );

static const void * const strings[] =
{
  &langID,
  &iManufacturer,
  &iProduct,
  &iSerialNumber
};

/* Endpoint buffer sizes */
/* 1 = single buffer, 2 = double buffering, 3 = triple buffering ... */
static const uint8_t bufferingMultiplier[ NUM_EP_USED + 1 ] = { 1, 1, 1 };

static const USBD_Callbacks_TypeDef callbacks =
{
  .usbReset        = NULL,
  .usbStateChange  = UsbStateChange,
  .setupCmd        = SetupCmd,
  .isSelfPowered   = NULL,
  .sofInt          = NULL
};

const USBD_Init_TypeDef initstruct =
{
  .deviceDescriptor    = &deviceDesc,
  .configDescriptor    = configDesc,
  .stringDescriptors   = strings,
  .numberOfStrings     = sizeof(strings)/sizeof(void*),
  .callbacks           = &callbacks,
  .bufferingMultiplier = bufferingMultiplier,
  .reserved            = 0
};

/**************************************************************************//**
 * @brief
 *   Handle USB PHDC class setup commands. This has nothing to do with 20601.
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted.
 *         USB_STATUS_REQ_UNHANDLED when command is unknown, the USB device
 *         stack will handle the request.
 *****************************************************************************/
extern const uint8_t configDesc[];
int SetupCmd(const USB_Setup_TypeDef *setup)
{
  int             retVal;
  EFM32_ALIGN(4)
  static short int EPFlags __attribute__ ((aligned(4))) = 0;

  /* Make one buffer big enough to hold the largest of the descriptors which is
   * 7 bytes. Increasing to 8 */
  STATIC_UBUF( phdcDesc, 8 );

  retVal = USB_STATUS_REQ_UNHANDLED;

  /* Handle PHDC class requests to an interface recipient on interface 0
     Be prepared to handle error conditions; incorrect interface recipient
     and/or incorrect interface. The PHDC standard requires the error
     condition to be handled with a Request Error (but I dont know what
     that is; USBCV20 tests suggest a stall) */
  if (setup->Type == USB_SETUP_TYPE_CLASS)
  {
    if(setup->Recipient == USB_SETUP_RECIPIENT_INTERFACE)
    {
      switch (setup->bRequest)
      {
        /* Clear feature: PHDC device does not handle metadata. Respond with stall.
           Do not need to be concerned about other error conditions */
        case CLEAR_FEATURE:
        case SET_FEATURE:
        /* Why is this direction defined as 'OUT' in the spec? (Bit 7 is 0) */
        if(setup->Direction == USB_SETUP_DIR_OUT)
        {
          /* Requests always come on EP0 so stall EP0. I believe this is addressed as
           * '0' and the USB_EPTYPE_CTRL Energy Micro definition is for EP0 address.
           * However all we need do is set the following error condition and the
           * library implements the correct stall behavior (cool!) */
          retVal = USB_STATUS_REQ_ERR;
        }
        break;

        case GET_STATUS:
        /* Host wants to read information from device to see if a transaction
         * is ongoing on any of the endpoints. Either the bulk-in or bulk-out
         * endpoint could be in the process of transferring an APDU. At this
         * time it is not clear how to obtain the status of the endpoints and
         * the only thing I can think of is to set a flag when the receive and
         * send functions are called. Right now the USBD_IsEPBusy function I
         * have added to the USB APIs returns true if the state is D_EP_TRANSMITTING
         * First check for errors. On error return error code. Do NOT try and
         * call the USB library routine to STALL; it will not work! */
        if( ( setup->Direction == USB_SETUP_DIR_IN ) &&
            ( setup->wValue    == 0                ) &&
            ( setup->wIndex    == 0                ) &&
            ( setup->wLength   == 2                )    )
        {
          EPFlags = 0;
          if(USBD_EpIsBusy(BULK_IN_EP_ADDR))
          {
            /* Endpoint 1 is bit 1. */
            EPFlags = (EPFlags | 2);
          }
          /* EPFlags are set for IN ep's only !. */
          retVal = USBD_Write(0, &EPFlags, 2, NULL);
        }
        else
        {
          retVal = USB_STATUS_REQ_ERR;
        }
        break;
      }
    }
    else
    {
      /* Interface error */
        retVal = USB_STATUS_REQ_ERR;
    }
  }
  else if ((setup->Type == USB_SETUP_TYPE_STANDARD) &&
      (setup->Direction == USB_SETUP_DIR_IN) /*&&
      (setup->Recipient == USB_SETUP_RECIPIENT_INTERFACE)*/)
      /* EM: Assumes that recipient is DEVICE (not 100% sure) */
      /* EM: This way of retrieving descriptors is probably never used anyway */
  {
    /* A PHDC device must extend the standard GET_DESCRIPTOR command   */
    /* with support for PHDC descriptors.                              */
    switch (setup->bRequest)
    {
      case GET_DESCRIPTOR:
      /* These are the PHDC-specific descriptors which are
       * USB_PHDC_CLASSFUNCTION_DESCRIPTOR
       * PHDC_11073PHD_FUNCTION_DESCRIPTOR
       * USB_PHDC_QOS_DESCRIPTOR
       * The endpoint descriptors cannot be accessed by this request. */
      switch((setup->wValue >> 8))
      {
        case USB_PHDC_CLASSFUNCTION_DESCRIPTOR:
        memcpy( phdcDesc,
            &configDesc[ USB_CONFIG_DESCSIZE + USB_INTERFACE_DESCSIZE ],
            PHDC_CLASS_bLength );
        retVal = USBD_Write(0, phdcDesc,
                   EFM32_MIN(sizeof(PHDC_CLASS_bLength), setup->wLength),
                   NULL);
        break;

        case PHDC_11073PHD_FUNCTION_DESCRIPTOR:
        memcpy( phdcDesc,
            &configDesc[ USB_CONFIG_DESCSIZE + USB_INTERFACE_DESCSIZE +
                         PHDC_CLASS_bLength ],
            PHDC_FUNC_bLength );
        retVal = USBD_Write(0, phdcDesc,
                   EFM32_MIN(sizeof(PHDC_FUNC_bLength), setup->wLength),
                   NULL);
        break;

        case USB_PHDC_QOS_DESCRIPTOR:
        memcpy( phdcDesc,
            &configDesc[ USB_CONFIG_DESCSIZE + USB_INTERFACE_DESCSIZE +
                         PHDC_CLASS_bLength + PHDC_FUNC_bLength],
            PHDC_QOS_bLength );
        retVal = USBD_Write(0, phdcDesc,
                   EFM32_MIN(sizeof(PHDC_QOS_bLength), setup->wLength),
                   NULL);
        break;
      }
      break;
    }
  }
  return retVal;
}
