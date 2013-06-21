/**************************************************************************//**
 * @file
 * @brief LCD controller demo for EFM32GG_STK3700 development kit
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
#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "bsp.h"
#include "segmentlcd.h"
#include "bsp_trace.h"

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/* Locatl prototypes */
void Delay(uint32_t dlyTicks);

/**************************************************************************//**
 * @brief SysTick_Handler
 *   Interrupt Service Routine for system tick counter
 * @note
 *   No wrap around protection
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}


/**************************************************************************//**
 * @brief Delays number of msTick Systicks (typically 1 ms)
 * @param dlyTicks Number of ticks to delay
 *****************************************************************************/
void Delay(uint32_t dlyTicks)
{
  uint32_t curTicks;

  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) ;
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  int i;

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Enable two leds to show we're alive */
  BSP_LedsInit();
  BSP_LedSet(0);
  BSP_LedSet(1);

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000)) while (1) ;

  /* Enable LCD without voltage boost */
  SegmentLCD_Init(false);

  /* Infinite loop with test pattern. */
  while (1)
  {
    /* Enable all segments */
    SegmentLCD_AllOn();
    Delay(500);

    /* Disable all segments */
    SegmentLCD_AllOff();

    /* Write a number */
    for (i = 0; i < 10; i++)
    {
      SegmentLCD_Number(i * 1111);
      Delay(200);
    }
    /* Write some text */
    SegmentLCD_Write("Energy@");
    Delay(500);
    SegmentLCD_Write("Micro");
    Delay(500);
    SegmentLCD_Write("Giant");
    Delay(500);
    SegmentLCD_Write("Gecko");
    Delay(1000);

    SegmentLCD_AllOff();

    /* Test segments */
    SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_ANT, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_MINUS, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL3, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL5, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_COL10, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DEGC, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DEGF, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP2, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP3, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP4, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP5, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP6, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_DP10, 1);

    SegmentLCD_Battery(0);
    SegmentLCD_Battery(1);
    SegmentLCD_Battery(2);
    SegmentLCD_Battery(3);
    SegmentLCD_Battery(4);

    SegmentLCD_ARing(0, 1);
    SegmentLCD_ARing(1, 1);
    SegmentLCD_ARing(2, 1);
    SegmentLCD_ARing(3, 1);
    SegmentLCD_ARing(4, 1);
    SegmentLCD_ARing(5, 1);
    SegmentLCD_ARing(6, 1);
    SegmentLCD_ARing(7, 1);

    Delay(1000);
  }
}
