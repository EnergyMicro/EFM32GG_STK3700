/**************************************************************************//**
 * @file main.c
 * @brief Mass Storage Device example.
 * @author Energy Micro AS
 * @version 3.20.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2011 Energy Micro AS, http://www.energymicro.com</b>
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
#include "em_usb.h"
#include "em_assert.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "retargetserial.h"
#include "msdd.h"
#include "msddmedia.h"
#include "usbconfig.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

/**************************************************************************//**
 *
 * This example shows how a Mass Storage Device (MSD) can be implemented.
 *
 * Different kinds of media can be used for data storage. Modify the
 * MSD_MEDIA #define macro in msdmedia.h to select between the different ones.
 *
 *****************************************************************************/

/**************************************************************************//**
 * @brief main - the entrypoint after reset.
 *****************************************************************************/
int main( void )
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  CMU_ClockSelectSet( cmuClock_HF, cmuSelect_HFXO );
  CMU_OscillatorEnable(cmuOsc_LFXO, true, false);
  CMU_ClockEnable(cmuClock_GPIO, true);

  SegmentLCD_Init(false);

  if ( !MSDDMEDIA_Init() )
  {
    EFM_ASSERT( false );
    for( ;; ){}
  }

  SegmentLCD_Write("usbdmsd");
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, true);
  MSDD_Init(gpioPortE, 2);

  for (;;)
  {
    if ( MSDD_Handler() )
    {
      /* There is no pending activity in the MSDD handler.  */
      /* Enter sleep mode to conserve energy.               */

      if ( USBD_SafeToEnterEM2() )
        EMU_EnterEM2(true);
      else
        EMU_EnterEM1();
    }
  }
}
