/**************************************************************************//**
 * @file
 * @brief Non-Volatile Memory Driver use example.
 * @author Energy Micro AS
 * @version 3.20.0
 *******************************************************************************
 * @section License
 * <b>(C) Copyright 2013 Energy Micro AS, http://www.energymicro.com</b>
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
#include <stdbool.h>

#include "em_device.h"

#include "bsp.h"
#include "bsp_trace.h"

#include "em_chip.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_gpio.h"

#include "segmentlcd.h"

#include "nvm_config.h"
#include "nvm.h"

/******************************************************************************
*  This example shows simple usage of NVM module.
*
*  It stores object (single int in this case) into flash region declared as
*  non-volatile memory area. It uses wear leveling and demonstrates simple use of
*  the module. There are two types of pages: "normal" dedicated for storing
*  multiple objects which doesn't change often and "wear" which can store single
*  object (but this could be structure containing many fields) that can often
*  change. In this example on two pages same object is stored but NVM could handle
*  multiple pages with different objects.
*
*  PB0 - short press recalls data from "normal" page
*  PB0 - long press store data to "normal" page
*  PB1 - counts up, and after releasing stores data to "wear" page
*
*  RESET - resets CPU and if there were valid data in NVM recovers last data value.
*
*  LED1 - signals writing to flash
*  LED0 - signals reading from flash (invisible due to short time)
*
*  In case of fatal error LED0 blinks showing place in code that caused it.
******************************************************************************/

#define LED_ALL_OFF               0
#define LED_READ_ON               1
#define LED_STORE_ON              2
#define LED_FATAL_ON              1

#define KEY_PB0_PIN_NUMBER        9
#define KEY_PB1_PIN_NUMBER        10

#define KEY_PB0_PIN               gpioPortB, 9
#define KEY_PB1_PIN               gpioPortB, 10

#define FATAL_AT_INIT             1
#define FATAL_AT_READ             2
#define FATAL_AT_STORE            3

#define MAX_DISPLAYABLE_NUMBER    9999

/* delays specified in miliseconds */
#define DELAY_BETWEEN_COUNTS      200
#define DELAY_AFTER_PB1           500
#define DELAY_1MS                 1
#define DELAY_LED_BLINK           250
#define DELAY_FATAL_BLINK         1000

/* time needed to enable store function - defined in miliseconds */
#define KEY_STORE_TIME            500

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB0 key)
 *        Sets the hours
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << KEY_PB0_PIN_NUMBER);
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB1 key)
 *        Sets the minutes
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << KEY_PB1_PIN_NUMBER);
}

/**************************************************************************//**
 * @brief Setup GPIO interrupt to set the time
 *****************************************************************************/
void APP_GpioSetup(void)
{
  /* Enable GPIO in CMU */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure pins for keys PB0 and PB1 as input */
  GPIO_PinModeSet(KEY_PB0_PIN, gpioModeInput, 0);
  GPIO_PinModeSet(KEY_PB1_PIN, gpioModeInput, 0);

  /* Set falling edge interrupt for both pins */
  GPIO_IntConfig(KEY_PB0_PIN, false, true, true);
  GPIO_IntConfig(KEY_PB1_PIN, false, true, true);

  /* Enable interrupt in core for even and odd gpio interrupts */
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/**************************************************************************//**
 * @brief Simple busy wait with milisecond resolution
 *****************************************************************************/
void APP_BusyWait(uint32_t msWait)
{
  uint32_t waitTop = msWait * (CMU_ClockFreqGet(cmuClock_CORE) / 1000);

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL        |= 1;
  DWT->CYCCNT       = 0;

  while (DWT->CYCCNT < waitTop) ;
}

/**************************************************************************//**
 * @brief Fatal error handling
 *****************************************************************************/
void APP_Fatal(uint32_t pulses)
{
  uint32_t i;

  while (1)
  {
    for (i = 0; i < pulses; i++)
    {
      BSP_LedsSet(LED_FATAL_ON);
      APP_BusyWait(DELAY_LED_BLINK);
      BSP_LedsSet(LED_ALL_OFF);
      APP_BusyWait(DELAY_LED_BLINK);
    }
    APP_BusyWait(DELAY_FATAL_BLINK);
  }
}

/**************************************************************************//**
 * @brief Store objects to specific page (volume)
 *****************************************************************************/
NVM_Result_t  APP_StoreData(NVM_Page_Ids page)
{
  NVM_Result_t result;

  BSP_LedsSet(LED_STORE_ON);
  result = NVM_Write(page, NVM_WRITE_ALL_CMD);
  BSP_LedsSet(LED_ALL_OFF);

  if (result != nvmResultOk)
    APP_Fatal(FATAL_AT_STORE);

  return result;
}

/**************************************************************************//**
 * @brief Read objects from page (volume)
 *****************************************************************************/
NVM_Result_t  APP_ReadData(NVM_Page_Ids page)
{
  NVM_Result_t result;

  BSP_LedsSet(LED_READ_ON);
  result = NVM_Read(page, COUNTER_ID);
  BSP_LedsSet(LED_ALL_OFF);

  if (result != nvmResultOk)
    APP_Fatal(FATAL_AT_READ);
  return result;
}

/**************************************************************************//**
 * @brief Initialize NVM and restore objects
 *****************************************************************************/
NVM_Result_t  APP_RestoreData(void)
{
  NVM_Result_t result;

  /* initialize NVM module */
  result = NVM_Init(NVM_ConfigGet());

  if (result == nvmResultError)
  { /* Ups, looks like no valid data in flash! */
    /* This could happen on first run after flashing. */
    /* So, we have to erase NVM */
    BSP_LedsSet(LED_STORE_ON);
    result = NVM_Erase(0);

    /* Store initial data/configuration */
    if (result == nvmResultOk)
      result = APP_StoreData(PAGE_NORMAL_ID);
    /* if wear page contains different data/object than normal page */
    /* it could be resonable to write wear page here too. */
    BSP_LedsSet(LED_ALL_OFF);
  }

  /* if init phase went correctly, try to restore data. */
  if (result == nvmResultOk)
  { /* Try to restore data from wear page, if failed read it from normal page */
    BSP_LedsSet(LED_READ_ON);
    result = NVM_Read(PAGE_WEAR_ID, COUNTER_ID);
    if (result == nvmResultNoPage)
      result = APP_ReadData(PAGE_NORMAL_ID);
  }
  BSP_LedsSet(LED_ALL_OFF);

  return result;
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  uint32_t j = 0;

  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Ensure core frequency has been updated */
  SystemCoreClockUpdate();

  /* Init LCD with no voltage boost */
  SegmentLCD_Init(false);

  /* configure GPIO */
  APP_GpioSetup();

  if (APP_RestoreData() != nvmResultOk)
    APP_Fatal(FATAL_AT_INIT);

  while (1)
  {
    SegmentLCD_Number(nvm_counter);
    SegmentLCD_Write("SLEEP");
    /* go to sleep and wait for key pressing */
    EMU_EnterEM2(false);
    if (!GPIO_PinInGet(KEY_PB0_PIN))  /* PB0 */
    {
      j = 0;
      SegmentLCD_Write("...");
      while (!GPIO_PinInGet(KEY_PB0_PIN) && (j++ < KEY_STORE_TIME))
        APP_BusyWait(DELAY_1MS);
      if (!GPIO_PinInGet(KEY_PB0_PIN))
      { /* store */
        SegmentLCD_Write("STORE");
        APP_StoreData(PAGE_NORMAL_ID);
      }
      else
      {
        /* restore */
        SegmentLCD_Write("RECALL");
        APP_ReadData(PAGE_NORMAL_ID);
      }
      APP_BusyWait(DELAY_AFTER_PB1);  /* delay to keep displayed text */
    }
    if (!GPIO_PinInGet(KEY_PB1_PIN))  /* PB1 */
    {
      SegmentLCD_Write("count");
      while (!GPIO_PinInGet(KEY_PB1_PIN))
      {
        APP_BusyWait(DELAY_BETWEEN_COUNTS);
        nvm_counter++;
        if (nvm_counter > MAX_DISPLAYABLE_NUMBER)
          nvm_counter = 0;
        SegmentLCD_Number(nvm_counter);
      }
      APP_StoreData(PAGE_WEAR_ID);
    }
  }
}
