/**************************************************************************//**
 * @file
 * @brief Using the userpage in EFM32GG onboard flash.
 * @details
 *   Read/write data to the userpage in flash on the EFM32GG
 *
 * @par Usage
 * @li PB0 Increases the number.
 * @li PB1 Saves the number to the userpage
 *
 *
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

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_emu.h"
#include "em_msc.h"
#include "em_gpio.h"
#include "segmentlcd.h"
#include "rtcdrv.h"
#include "bsp_trace.h"

#define USERPAGE    0x0FE00000 /**< Address of the user page */

typedef struct
{
  uint32_t number;           /**< Number to display and save to flash */
  uint32_t numWrites;        /**< Number of saves to flash */
} UserData_TypeDef;

volatile UserData_TypeDef userData;                   /**< User data contents */

volatile bool             rtcFlag;                    /**< Flag used by the RTC timing routines */
volatile bool             recentlySaved;              /**< Flag to indicate successful write */

msc_Return_TypeDef        currentError = mscReturnOk; /** < Latest error encountered */

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB9)
 *        Saves the number to the userpage
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 9);

  /* Increase data - Wrap at 10000 */
  userData.number = (userData.number + 1) % 10000;

  /* Update the display */
  SegmentLCD_Number(userData.number);
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB10)
 *        Increases number when PB0 is pressed. Wraps at 10000.
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
    msc_Return_TypeDef ret;

  /* Acknowledge interrupt */
  GPIO_IntClear(1 << 10);

  /* Initialize the MSC for writing */
  MSC_Init();

  /* Erase the page */
  ret = MSC_ErasePage((uint32_t *) USERPAGE);

  /* Check for errors. If there are errors, set the global error variable and
   * deinitialize the MSC */
  if (ret != mscReturnOk)
  {
    currentError = ret;
    MSC_Deinit();
    return;
  }

  /* Increase the number of saves */
  userData.numWrites++;

  /* Write data to the userpage */
  ret = MSC_WriteWord((uint32_t *) USERPAGE, (void *) &userData,
                      sizeof(UserData_TypeDef));

  /* Check for errors. If there are errors, set the global error variable and
   * deinitialize the MSC */
  if (ret != mscReturnOk)
  {
    currentError = ret;
    MSC_Deinit();
    return;
  }
  /* Deinitialize the MSC. This disables writing and locks the MSC */
  MSC_Deinit();

  /* Signal completion of save. The number of saves will be displayed */
  recentlySaved = true;
}

/**************************************************************************//**
 * @brief Setup GPIO interrupt to set the time
 *****************************************************************************/
void gpioSetup(void)
{
  /* Enable GPIO in CMU */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PB9 and PB10 as input */
  GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0);
  GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);

  /* Set falling edge interrupt for both ports */
  GPIO_IntConfig(gpioPortB, 9, false, true, true);
  GPIO_IntConfig(gpioPortB, 10, false, true, true);

  /* Enable interrupt in core for even and odd gpio interrupts */
  NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
  NVIC_EnableIRQ(GPIO_EVEN_IRQn);

  NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
  NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/**************************************************************************//**
 * @brief Callback used for the RTC.
 * @details
 *   Because GPIO interrupts can alswo wake up the CM3 from sleep it is
 *   necessary for the correct timing of the scrolling text to make sure that
 *   the wake-up source is the RTC.
 *****************************************************************************/
void RTC_TimeOutHandler(void)
{
  rtcFlag = false;
}

/**************************************************************************//**
 * @brief Sleeps in EM2 in given time
 * @param msec Time in milliseconds
 *****************************************************************************/
void EM2Sleep(uint32_t msec)
{
  /* Wake us up after msec */
  NVIC_DisableIRQ(LCD_IRQn);
  rtcFlag = true;
  RTCDRV_Trigger(msec, RTC_TimeOutHandler);
  /* The rtcFlag variable is set in the RTC interrupt routine using the callback
   * RTC_TimeOutHandler. This makes sure that the elapsed time is correct. */
  while (rtcFlag)
  {
    EMU_EnterEM2(true);
  }
  NVIC_EnableIRQ(LCD_IRQn);
}

/**************************************************************************//**
 * @brief LCD scrolls a text over the display, sort of "polled printf"
 *****************************************************************************/
void ScrollText(char *scrolltext)
{
  int  i, len;
  char buffer[8];

  buffer[7] = 0x00;
  len       = strlen(scrolltext);
  if (len < 7) return;
  for (i = 0; (!recentlySaved) && (i < (len - 7)); i++)
  {
    memcpy(buffer, scrolltext + i, 7);
    SegmentLCD_Write(buffer);
    EM2Sleep(125);
  }
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
  /* Chip errata */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Initialize LCD controller without boost */
  SegmentLCD_Init(false);

  /* Disable all segments */
  SegmentLCD_AllOff();

  /* Copy contents of the userpage (flash) into the userData struct */
  memcpy((void *) &userData, (void *) USERPAGE, sizeof(UserData_TypeDef));

  /* Special case for uninitialized data */
  if (userData.number > 10000)
    userData.number = 0;
  if (userData.numWrites == 0xFFFFFFFF)
    userData.numWrites = 0;

  /* Display the number */
  SegmentLCD_Number(userData.number);

  /* Setup GPIO interrupts. PB0 to increase number, PB1 to save to flash */
  gpioSetup();

  /* No save has occured yet */
  recentlySaved = false;

  /* Main loop - just scroll informative text describing the current state of
   * the system */
  while (1)
  {
    switch (currentError)
    {
    case mscReturnInvalidAddr:
      ScrollText("     ERROR: INVALID ADDRESS      ");
      break;
    case mscReturnLocked:
      ScrollText("     ERROR: USER PAGE IS LOCKED      ");
      break;
    case mscReturnTimeOut:
      ScrollText("     ERROR: TIMEOUT OCCURED      ");
      break;
    case mscReturnUnaligned:
      ScrollText("     ERROR: UNALIGNED ACCESS     ");
    default:
      if (recentlySaved)
      {
        recentlySaved = false;
        SegmentLCD_Number(userData.numWrites);
        ScrollText("     SAVE NUMBER       ");
      }
      else
      {
        SegmentLCD_Number(userData.number);
        ScrollText("     PRESS PB0 TO INCREASE NUMBER. PB1 TO SAVE TO INTERNAL FLASH        ");
      }
      break;
    }
  }
}
