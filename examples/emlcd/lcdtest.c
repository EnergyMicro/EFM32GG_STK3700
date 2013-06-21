/**************************************************************************//**
 * @file
 * @brief Energy Mode enabled LCD Controller test and demo for EFM32TG_STK3300
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
#include <stddef.h>
#include "em_device.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_lcd.h"
#include "em_gpio.h"
#include "segmentlcd.h"
#include "lcdtest.h"
#include "rtcdrv.h"

/* If running below 3V, we should enable vboost for optimal LCD contrast */
/* but it will increase current / drain battery.                         */
/* Adding this define will check current, and enable according to need   */
#define VBOOST_SUPPORT    1

#if VBOOST_SUPPORT
#include "vddcheck.h"
#endif

/** Demo scroll text */
static char *stext = "Energy Micro        ";

#define DEMO_MODE_NONE    0x0
#define DEMO_MODE_EM3     0x1
#define DEMO_MODE_EM4     0x2

static uint16_t emMode = DEMO_MODE_NONE;
static bool     inEM3  = false;

/* This flag enables/disables vboost on the LCD */
bool oldBoost = false;

/* Local prototypes */
void GPIO_IRQInit(void);
void checkVoltage(void);

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB10)
 *****************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
  GPIO_IntClear(1 << 10);

  if (inEM3)
  {
    emMode = DEMO_MODE_NONE;
    SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 0);
    SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 0);
  }
  else
  {
    emMode = DEMO_MODE_EM4;
    SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 1);
  }
}

/**************************************************************************//**
 * @brief GPIO Interrupt handler (PB9)
 *****************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
  GPIO_IntClear(1 << 9);

  SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 1);
  SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 1);

  emMode = DEMO_MODE_EM3;
}

/**************************************************************************//**
 * @brief Initialize GPIO interrupt on PC14
 *****************************************************************************/
void GPIO_IRQInit(void)
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
 * @brief Callback function lighting the "Antenna symbol"
 *****************************************************************************/
void RtcTrigger(void)
{
  /* Just a few No-OPerations to have a place to put a breakpoint */
  SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
}


/**************************************************************************//**
 * @brief Sleeps in EM1 in given time unless some other IRQ sources has been
 *        enabled
 * @param msec Time in milliseconds
 *****************************************************************************/
void EM1Sleep(uint32_t msec)
{
  /* Wake us up after msec (or joystick pressed) */
  NVIC_DisableIRQ(LCD_IRQn);

  /* Tell AEM we're in EM1 */
  RTCDRV_Trigger(msec, RtcTrigger);
  EMU_EnterEM1();
  NVIC_EnableIRQ(LCD_IRQn);
}

/**************************************************************************//**
 * @brief Sleeps in EM2 in given time unless some other IRQ sources has been
 *        enabled
 * @param msec Time in milliseconds
 *****************************************************************************/
void EM2Sleep(uint32_t msec)
{
  /* Wake us up after msec (or joystick pressed) */
  NVIC_DisableIRQ(LCD_IRQn);
  RTCDRV_Trigger(msec, NULL);
  EMU_EnterEM2(true);
  NVIC_EnableIRQ(LCD_IRQn);
}


/**************************************************************************//**
 * @brief Sleeps in EM3 until GPIO interrupt is triggered
 *****************************************************************************/
void EM3Sleep(void)
{
  inEM3 = true;
  SegmentLCD_Disable();
  EMU_EnterEM3(true);
  SegmentLCD_Init(false);
  inEM3 = false;
}


/**************************************************************************//**
 * @brief Sleeps in EM4 until reset
 *****************************************************************************/
void EM4Sleep(void)
{
  EMU_EnterEM4();
  /* we will never wake up again here - reset required */
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
  for (i = 0; i < (len - 7); i++)
  {
    memcpy(buffer, scrolltext + i, 7);
    SegmentLCD_Write(buffer);
    EM2Sleep(200);
  }
}

/**************************************************************************//**
 * @brief LCD Blink Test
 *****************************************************************************/
void BlinkTest(void)
{
  SegmentLCD_EnergyMode(0, 1);
  SegmentLCD_EnergyMode(1, 1);
  SegmentLCD_EnergyMode(2, 1);
  SegmentLCD_EnergyMode(3, 1);
  SegmentLCD_EnergyMode(4, 1);

  /* 2 minutes to midnight */
  SegmentLCD_Number(2358);
  SegmentLCD_Symbol(LCD_SYMBOL_COL10, 1);
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
  SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
  SegmentLCD_Write(" EFM32 ");
  LCD->BACTRL |= LCD_BACTRL_BLINKEN;
  while (LCD->SYNCBUSY) ;
  EM2Sleep(2000);
  SegmentLCD_EnergyMode(4, 0);
  EM2Sleep(62);
  SegmentLCD_EnergyMode(3, 0);
  EM2Sleep(62);
  SegmentLCD_EnergyMode(2, 0);
  EM2Sleep(62);
  SegmentLCD_EnergyMode(1, 0);
  EM2Sleep(62);
  SegmentLCD_EnergyMode(0, 0);
  LCD->BACTRL &= ~LCD_BACTRL_BLINKEN;
  while (LCD->SYNCBUSY) ;
}


/**************************************************************************//**
 * @brief Check input voltage and enable vboost if it drops too low.
 *****************************************************************************/
#if VBOOST_SUPPORT
void checkVoltage(void)
{
  bool vboost;

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

  if (vboost != oldBoost)
  {
    SegmentLCD_Init(vboost);

    /* Use Antenna symbol to signify enabling of vboost */
    SegmentLCD_Symbol(LCD_SYMBOL_ANT, vboost);
    oldBoost = vboost;
  }
}
#endif
/**************************************************************************//**
 * @brief LCD Test Routine, shows various text and patterns
 *****************************************************************************/
void Test(void)
{
  int i, numberOfIterations = 0;

  /* Initialize GPIO */
  GPIO_IRQInit();

  /* Initialize RTC */
  RTCDRV_Setup(cmuSelect_LFRCO, cmuClkDiv_32);

  /* Loop through funny pattern */
  while (1)
  {
    SegmentLCD_AllOff();
#if VBOOST_SUPPORT
    checkVoltage();
#endif

    if (emMode != DEMO_MODE_NONE)
    {
      SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 1);
      SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 1);
    }
    else
    {
      for (i = 100; i > 0; i--)
      {
        SegmentLCD_Number(i);
        EM2Sleep(10);
      }
      SegmentLCD_NumberOff();

      SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
      SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
      SegmentLCD_Write(" Giant ");
      EM2Sleep(500);
      SegmentLCD_Write(" Gecko ");
      EM2Sleep(1000);
      SegmentLCD_AllOn();
      EM2Sleep(1000);

      SegmentLCD_AllOff();
    }
    if (emMode != DEMO_MODE_NONE)
    {
      SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 1);
      SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 1);
    }
    else
    {
      SegmentLCD_Write("OOOOOOO");
      EM2Sleep(62);
      SegmentLCD_Write("XXXXXXX");
      EM2Sleep(62);
      SegmentLCD_Write("+++++++");
      EM2Sleep(62);
      SegmentLCD_Write("@@@@@@@");
      EM2Sleep(62);
      SegmentLCD_Write("ENERGY ");
      EM2Sleep(250);
      SegmentLCD_Write("@@ERGY ");
      EM2Sleep(62);
      SegmentLCD_Write(" @@RGY ");
      EM2Sleep(62);
      SegmentLCD_Write(" M@@GY ");
      EM2Sleep(62);
      SegmentLCD_Write(" MI@@Y ");
      EM2Sleep(62);
      SegmentLCD_Write(" MIC@@ ");
      EM2Sleep(62);
      SegmentLCD_Write(" MICR@@");
      EM2Sleep(62);
      SegmentLCD_Write(" MICRO@");
      EM2Sleep(62);
      SegmentLCD_Write(" MICRO ");
      EM2Sleep(250);
      SegmentLCD_Write("-EFM32-");
      EM2Sleep(250);

      /* Various eye candy */
      SegmentLCD_AllOff();
      if (emMode != DEMO_MODE_NONE)
      {
        SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 1);
        SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 1);
      }
      for (i = 0; i < 8; i++)
      {
        SegmentLCD_Number(numberOfIterations + i);
        SegmentLCD_ARing(i, 1);
        EM2Sleep(20);
      }
      for (i = 0; i < 8; i++)
      {
        SegmentLCD_Number(numberOfIterations + i);
        SegmentLCD_ARing(i, 0);
        EM2Sleep(100);
      }

      for (i = 0; i < 5; i++)
      {
        SegmentLCD_Number(numberOfIterations + i);
        SegmentLCD_Battery(i);
        SegmentLCD_EnergyMode(i, 1);
        EM2Sleep(100);
        SegmentLCD_EnergyMode(i, 0);
        EM2Sleep(100);
      }
      SegmentLCD_Symbol(LCD_SYMBOL_ANT, 1);
      for (i = 0; i < 4; i++)
      {
        SegmentLCD_EnergyMode(i, 1);
        EM2Sleep(100);
      }
      SegmentLCD_Symbol(LCD_SYMBOL_ANT, 0);
      SegmentLCD_Battery(0);
    }
    /* Energy Modes */
    SegmentLCD_NumberOff();
    SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
    SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);
    if ((emMode != DEMO_MODE_EM3) && (emMode != DEMO_MODE_EM4))
    {
      ScrollText("Energy Mode demo, Press PB0 for EM3 or PB1 for EM4       ");
    }
    SegmentLCD_Write("  EM0  ");
    SegmentLCD_Number(0);
    SegmentLCD_EnergyMode(0, 1);
    SegmentLCD_EnergyMode(1, 1);
    SegmentLCD_EnergyMode(2, 1);
    SegmentLCD_EnergyMode(3, 1);
    SegmentLCD_EnergyMode(4, 1);
    RTCDRV_Delay(4000, false);
    SegmentLCD_Write("  EM1  ");
    SegmentLCD_Number(1111);
    SegmentLCD_EnergyMode(0, 0);
    EM1Sleep(4000);
    SegmentLCD_Write("  EM2  ");
    SegmentLCD_Number(2222);
    SegmentLCD_EnergyMode(1, 0);
    EM2Sleep(4000);

    /* Check if somebody has pressed one of the buttons */
    if (emMode == DEMO_MODE_EM3)
    {
      ScrollText("Going down to EM3, press PB0 to wake up    ");
      SegmentLCD_Write("  EM3  ");
      SegmentLCD_Number(3333);
      RTCDRV_Delay(1000, false);

      /* Wake up on GPIO interrupt */
      EM3Sleep();
      SegmentLCD_Number(0000);
      SegmentLCD_Write("--EM0--");
      RTCDRV_Delay(500, false);
      SegmentLCD_Symbol(LCD_SYMBOL_PAD0, 0);
      SegmentLCD_Symbol(LCD_SYMBOL_PAD1, 0);
      emMode = DEMO_MODE_NONE;
    }
    /* Check if somebody's joystick down */
    if (emMode == DEMO_MODE_EM4)
    {
      ScrollText("Going down to EM4, press reset to restart    ");
      SegmentLCD_Write("  EM4  ");
      SegmentLCD_Number(4444);
      RTCDRV_Delay(1000, false);

      /* Wake up on reset */
      EM4Sleep();
    }
    SegmentLCD_EnergyMode(0, 0);
    SegmentLCD_EnergyMode(1, 0);
    SegmentLCD_EnergyMode(2, 0);
    SegmentLCD_EnergyMode(3, 0);
    SegmentLCD_EnergyMode(4, 0);

    /* Scrolltext */
    ScrollText(stext);

    /* Blink and animation featurs */
    BlinkTest();

    numberOfIterations++;
  }
}

