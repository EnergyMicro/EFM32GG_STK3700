/**************************************************************************//**
 * @file main.c
 * @brief USB host stack device enumeration example project.
 * @author Energy Micro AS
 * @version 3.20.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2010 Energy Micro AS, http://www.energymicro.com</b>
 ******************************************************************************
 *
 * This source code is the property of Energy Micro AS. The source and compiled
 * code may only be used on Energy Micro "EFM32" microcontrollers.
 *
 * This copyright notice may not be removed from the source code nor changed.
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
#include <stdio.h>
#include "em_device.h"
#include "em_cmu.h"
#include "em_usb.h"
#include "segmentlcd.h"

/**************************************************************************//**
 *
 * This example shows how the USB host stack can be used to "probe" the device
 * properties of any device which is attached to the host port.
 *
 * The device attached will not be configured.
 *
 *****************************************************************************/

/*** Variables ***/

STATIC_UBUF( tmpBuf, 1024 );
static USBH_Device_TypeDef device;

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/
int main(void)
{
  char lcdbuffer[8];
  int connectionResult;
  USBH_Init_TypeDef is = USBH_INIT_DEFAULT;

  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  CMU_ClockEnable(cmuClock_GPIO, true);

  SegmentLCD_Init(false);
  SegmentLCD_Write("USBHOST");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);

  USBH_Init(&is);               /* Initialize USB HOST stack */

  for (;;)
  {
    /* Wait for device connection */

    /* Wait for maximum 10 seconds. */
    connectionResult = USBH_WaitForDeviceConnectionB(tmpBuf, 10);

    if ( connectionResult == USB_STATUS_OK )
    {
      SegmentLCD_Write("Device");
      USBTIMER_DelayMs(500);
      SegmentLCD_Write("Added");
      USBTIMER_DelayMs(500);

      if (USBH_QueryDeviceB(tmpBuf, sizeof(tmpBuf), USBH_GetPortSpeed())
          == USB_STATUS_OK)
      {
        USBH_InitDeviceData(&device, tmpBuf, NULL, 0, USBH_GetPortSpeed());

        SegmentLCD_UnsignedHex(device.devDesc.idVendor);
        sprintf(lcdbuffer, "%.4xh", device.devDesc.idProduct);
        SegmentLCD_Write(lcdbuffer);

      }
      else
      {
      }

      while ( USBH_DeviceConnected() ){}
      SegmentLCD_NumberOff();
      SegmentLCD_Write("Device");
      USBTIMER_DelayMs(500);
      SegmentLCD_Write("Removed");
      USBTIMER_DelayMs(500);
      SegmentLCD_Write("USBHOST");
    }

    else if ( connectionResult == USB_STATUS_DEVICE_MALFUNCTION )
    {
    }

    else if ( connectionResult == USB_STATUS_TIMEOUT )
    {
      SegmentLCD_Write("TIMEOUT");
    }

    USBH_Stop();
  }
}
