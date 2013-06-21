/**************************************************************************//**
 * @file main.c
 * @brief Vendor unique USB device example.
 * @author Energy Micro AS
 * @version 3.20.0
 ******************************************************************************
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
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_usb.h"
#include "bsp.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

/**************************************************************************//**
 *
 * This example shows how a vendor unique device can be implemented.
 * A vendor unique device is a device which does not belong to any
 * USB class.
 *
 * Use the file EFM32_Vendor_Unique_Device.inf to install libusb device driver
 * on the host PC. This file reside in example subdirectory:
 * ./host/libusb/efm32-vendor-unique-device-1.2.5.0
 *
 *****************************************************************************/

/*** Typedef's and defines. ***/

#define LED0            0
#define LED1            1

#define VND_GET_LEDS    0x10
#define VND_SET_LED     0x11

/*** Function prototypes. ***/

static int  SetupCmd(const USB_Setup_TypeDef *setup);

/*** Include device descriptor definitions. ***/

#include "descriptors.h"

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  SegmentLCD_Init(false);
  SegmentLCD_Write("usb vud");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  /* Initialize LED driver */
  BSP_LedsInit();
  BSP_LedClear(LED0);
  BSP_LedClear(LED1);

  USBD_Init(&initstruct);

  /*
   * When using a debugger it is pratical to uncomment the following three
   * lines to force host to re-enumerate the device.
   */
  /* USBD_Disconnect(); */
  /* USBTIMER_DelayMs( 1000 ); */
  /* USBD_Connect(); */

  for (;;)
  {
  }
}

/**************************************************************************//**
 * @brief
 *   Handle USB setup commands.
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted.
 *         USB_STATUS_REQ_UNHANDLED when command is unknown, the USB device
 *         stack will handle the request.
 *****************************************************************************/
static int SetupCmd(const USB_Setup_TypeDef *setup)
{
  int             retVal;
  static uint32_t buffer;
  uint8_t         *pBuffer = (uint8_t*) &buffer;

  retVal = USB_STATUS_REQ_UNHANDLED;

  if (setup->Type == USB_SETUP_TYPE_VENDOR)
  {
    switch (setup->bRequest)
    {
    case VND_GET_LEDS:
      /********************/
      *pBuffer = (uint8_t)BSP_LedsGet();
      retVal   = USBD_Write(0, pBuffer, setup->wLength, NULL);
      break;

    case VND_SET_LED:
      /********************/
      if (setup->wValue)
      {
        if ( setup->wIndex == LED0 )
          BSP_LedSet(LED0);
        else if ( setup->wIndex == LED1 )
          BSP_LedSet(LED1);
      }
      else
      {
        if ( setup->wIndex == LED0 )
          BSP_LedClear(LED0);
        else if ( setup->wIndex == LED1 )
          BSP_LedClear(LED1);
      }
      retVal = USB_STATUS_OK;
      break;
    }
  }

  return retVal;
}
