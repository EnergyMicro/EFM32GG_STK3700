/**************************************************************************//**
 * @file
 * @brief RTX tick-less mode demo with LCD off for EFM32GG_STK3700 using RTX
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
#include "cmsis_os.h"
#include "bsp_trace.h"
#include "em_chip.h"
#include "em_emu.h"

typedef char lcdText_t[8];

/* Define memory pool */
osPoolDef(mpool, 16, lcdText_t);
osPoolId  mpool;

/* Define message queue */
osMessageQDef(msgBox, 16, lcdText_t);
osMessageQId msgBox;

/**************************************************************************//**
 * @brief
 *   Thread 1: Print LCD thread - modified to not use LCD
 *****************************************************************************/
void PrintLcdThread(void const *argument)
{
  lcdText_t *rptr;
  osEvent   evt;
  (void) argument;                 /* Unused parameter. */

  while (1)
  {
    /* Wait for message */
    evt = osMessageGet(msgBox, osWaitForever);
    if (evt.status == osEventMessage)
    {
      rptr = evt.value.p;
      /* Free memory allocated for message */
      osPoolFree(mpool, rptr);
    }
  }
}

/* Thread definition */
osThreadDef(PrintLcdThread, osPriorityNormal, 1, 0);


/**************************************************************************//**
 * @brief
 *   Main function is a CMSIS RTOS thread in itself
 *
 * @note
 *   This example uses threads, memory pool and message queue to demonstrate the
 *   usage of these CMSIS RTOS features. In this simple example, the same
 *   functionality could more easily be achieved by doing everything in the main
 *   loop.
 *****************************************************************************/
int main(void)
{
  int count = 0;

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

#if 0
  /* Reduce power consumption by disabling part of RAM */
  /* this requires changing linker script to avoid placing */
  /* data in RAM above 32kB. Blocks 1,2,3 are turn off. */
  EMU_MemPwrDown(_EMU_MEMCTRL_POWERDOWN_BLK123);
  BURTC->POWERDOWN   |= BURTC_POWERDOWN_RAM;     /* turn BURTC RAM off */
  LESENSE->POWERDOWN |= LESENSE_POWERDOWN_RAM;   /* turn off LESENSE RAM */
#endif

  /* Initialize CMSIS RTOS structures */
  /* create memory pool */
  mpool = osPoolCreate(osPool(mpool));
  /* create msg queue */
  msgBox = osMessageCreate(osMessageQ(msgBox), NULL);
  /* create thread 1 */
  osThreadCreate(osThread(PrintLcdThread), NULL);

  /* Infinite loop */
  while (1)
  {
    count = (count + 1) & 0xF;

    /* Send message to PrintLcdThread */
    /* Allocate memory for the message */
    lcdText_t *mptr = osPoolAlloc(mpool);
    /* Set the message content */
    (*mptr)[0] = count >= 10 ? '1' : '0';
    (*mptr)[1] = count % 10 + '0';
    (*mptr)[2] = '\0';
    /* Send message */
    osMessagePut(msgBox, (uint32_t) mptr, osWaitForever);

    /* Wait now for half a second */
    osDelay(500);
  }
}
