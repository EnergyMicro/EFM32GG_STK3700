/**************************************************************************//**
 * @file
 * @brief A very simple demonstration of different power modes.
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
#include <stdlib.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_wdog.h"
#include "rtcdrv.h"
#include "bsp_trace.h"

/** Counts 1ms timeTicks */
volatile uint32_t msTicks;

/**************************************************************************//**
 * @brief SysTick_Handler
 * Interrupt Service Routine for system tick counter
 *****************************************************************************/
void SysTick_Handler(void)
{
  msTicks++;       /* increment counter necessary in Delay()*/
}


/**************************************************************************//**
 * @brief SysTick_Disable
 * Disable systick interrupts
 *****************************************************************************/
void SysTick_Disable(void)
{
  SysTick->CTRL = 0x0000000;
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
  WDOG_Init_TypeDef wInit = WDOG_INIT_DEFAULT;
  int i;

  /* Chip revision alignment and errata fixes */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Watchdog setup - Use defaults, excepts for these :*/
  wInit.em2Run = true;
  wInit.em3Run = true;
  wInit.perSel = wdogPeriod_4k; /* 4k 1kHz periods should give ~4 seconds in EM3 */

  /* Do the demo forever. */

  /* EM0 - 1 sec HFXO  */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }
  Delay(1000);

  /* EM0 - 1 sec HFRCO */
  CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFRCO);
  /* Setup SysTick Timer for 1 msec interrupts  */
  if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
  {
    while (1) ;
  }
  Delay(1000);

  /* Turn off systick */
  SysTick_Disable();

  /* EM1 - 1 sec */
  RTCDRV_Trigger(1000, NULL);
  EMU_EnterEM1();

  /* EM2 - 1 sec */
  RTCDRV_Trigger(1000, NULL);
  EMU_EnterEM2(true);

  /* EM1 - 1 sec */
  RTCDRV_Trigger(1000, NULL);
  EMU_EnterEM1();

  /* EM2 - 1 sec */
  RTCDRV_Trigger(1000, NULL);
  EMU_EnterEM2(true);

  /* Up and down from EM2 each 10 msec */
  for (i=0; i < 10; i++)
  {
    RTCDRV_Trigger(10, NULL);
    EMU_EnterEM2(true);
    RTCDRV_Delay(10, false);
  }

  /* EM2 - 1 sec */
  RTCDRV_Trigger(1000, NULL);
  EMU_EnterEM2(true);

  /* Up and down from EM2 each 2 msec */
  for (i=0; i < 10; i++)
  {
    RTCDRV_Trigger(2, NULL);
    EMU_EnterEM2(true);
  }

  /* EM2 - 1 sec */
  RTCDRV_Trigger(1000, NULL);
  EMU_EnterEM2(true);

  /* Up and down from EM2 each msec */
  for (i=0; i < 10; i++)
  {
    RTCDRV_Trigger(1, NULL);
    EMU_EnterEM2(true);
  }

  /* EM2 - 1 sec */
  RTCDRV_Trigger(1000, NULL);
  EMU_EnterEM2(true);

  /* Start watchdog */
  WDOG_Init(&wInit);

  /* Enter EM3 - Watchdog will reset chip (and confuse debuggers) */
  EMU_EnterEM3(true);

  /* We will never reach this point */
  return 0;
}
