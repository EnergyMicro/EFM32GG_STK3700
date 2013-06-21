/**************************************************************************//**
 * @file
 * @brief Binary Support Package demo for EFM32TG_STK3300
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
#include <stdio.h>
/* CMSIS package */
#include "em_device.h"
/* emlib */
#include "em_cmu.h"
#include "em_chip.h"
/* STK Board Support Package */
#include "bsp.h"
#include "bsp_trace.h"
/* STK Drivers */
#include "vddcheck.h"
#include "segmentlcd.h"

/* Prototypes for local functions */
void SysTick_Handler(void);
void Delay(uint32_t dlyTicks);

volatile uint32_t msTicks; /* counts 1ms timeTicks */

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
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
  int   value, delayCount = 0, hfrcoband = 0;
  float current, voltage;
  bool  vboost;
  char  buffer[8];

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize board support package */
  BSP_Init(BSP_INIT_BCC);

  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(SystemCoreClockGet() / 1000)) while (1) ;

  /* Initialize voltage comparator, to check supply voltage */
  VDDCHECK_Init();

  /* Check if voltage is below 3V, if so use voltage boost */
  if (VDDCHECK_LowVoltage(2.9))
  {
    vboost = true;
  }
  else
  {
    vboost = false;
  }

  /* Disable Voltage Comparator */
  VDDCHECK_Disable();

  /* Initialize segment LCD */
  SegmentLCD_Init(vboost);

  /* Infinite loop */
  while (1)
  {
    /* Read and display current */
    current = BSP_CurrentGet();
    value   = (int)(1000 * current);

    /* Check that we fall within displayable value */
    if ((value > 0) && (value < 10000))
    {
      SegmentLCD_Number(value);
    }
    else
    {
      SegmentLCD_Number(-1);
    }

    /* Alternate between voltage and clock frequency */
    if (((delayCount / 10) & 1) == 0)
    {
      voltage = BSP_VoltageGet();
      value   = (int)(voltage * 100);
      SegmentLCD_Symbol(LCD_SYMBOL_DP6, 1);
      sprintf(buffer, "Volt%3d", value);
      SegmentLCD_Write(buffer);
    }
    else
    {
      SegmentLCD_Symbol(LCD_SYMBOL_DP6, 0);
      sprintf(buffer, "%3u MHz", (int)(SystemCoreClockGet() / 1000000));
      SegmentLCD_Write(buffer);
    }
    /* After 5 seconds, use another HFRCO band */
    if (delayCount % 50 == 0)
    {
      switch (hfrcoband)
      {
      case 0:
        CMU_HFRCOBandSet(cmuHFRCOBand_11MHz);
        break;
      case 1:
        CMU_HFRCOBandSet(cmuHFRCOBand_14MHz);
        break;
      case 2:
        CMU_HFRCOBandSet(cmuHFRCOBand_21MHz);
        break;
      default:
        CMU_HFRCOBandSet(cmuHFRCOBand_28MHz);
        /* Restart iteartion */
        hfrcoband = -1;
        break;
      }
      hfrcoband++;
      /* Recalculate delay tick count and baudrate generation */
      if (SysTick_Config(SystemCoreClockGet() / 1000)) while (1) ;
      BSP_Init(BSP_INIT_BCC);
    }

    Delay(100);
    delayCount++;
  }
}
