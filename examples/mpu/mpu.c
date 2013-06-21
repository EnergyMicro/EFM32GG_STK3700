/**************************************************************************//**
 * @file
 * @brief MPU example for EFM32_G8xx_STK.
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
#include "em_device.h"
#include "em_chip.h"
#include "em_mpu.h"
#include "em_rtc.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "segmentlcd.h"
#include "rtcdrv.h"
#include "bsp_trace.h"

/***************************************************************************//**
 *
 *  This example project demonstrate use of the memory protection unit (MPU).
 *  The cpu will normally run in Priviledged state, but when a pushbutton
 *  is pressed on the STK it will switch into User state and intentionally
 *  generate access violation exceptions.
 *
 *  The violation forced by hitting pushbutton PB0 is made by accessing
 *  internal SRAM. Hitting PB1 forces exception by accessing the LCD
 *  peripheral.
 *
 *  To be able to execute in User state at all, User state access must be
 *  granted to flash and SRAM memory. We use two MPU regions for this.
 *  Additionally one MPU section is used to deny User state access to a small
 *  part of SRAM. This section overlaps the section used to grant general
 *  general SRAM access, but it will take precedence because its section number
 *  is higher.
 *  A fourth MPU section is used to deny User state access to LCD registers.
 *
 *  Entry into User state is performed by writing to the CONTROL cpu core
 *  register. Before exiting the memory protection fault handler the cpu
 *  is switched back to Priviledged state. (It is impossible to switch from
 *  User state to Priviledged state in Thread mode).
 *
 ******************************************************************************/


/** Pushbutton macro's */
#define PB0() ( GPIO->P[ 1 ].DIN & ( 1 << 9 ) )
#define PB1() ( GPIO->P[ 1 ].DIN & ( 1 << 10 ) )
#define PB0_PUSHED() ( !PB0() )
#define PB1_PUSHED() ( !PB1() )


/**************************************************************************//**
 * @brief  Delay function, does not depend on interrupts.
 *****************************************************************************/
static void Delay( uint32_t msec )
{
/* RTC frequency is LFXO divided by 32 (prescaler) */
#define RTC_FREQ (32768 / 32)

  RTC_IntDisable( RTC_IF_COMP0 );
  RTC_IntClear( RTC_IF_COMP0 );
  RTC_CompareSet( 0, (RTC_FREQ * msec ) / 1000 ); /* Calculate trigger value */

  RTC_Enable( true );
  while ( !( RTC_IntGet() & RTC_IF_COMP0 ) );     /* Wait for trigger */
  RTC_Enable( false );
}


/**************************************************************************//**
 * @brief LCD scrolls a text over the display, sort of "polled printf".
 *****************************************************************************/
static void ScrollText(char *scrolltext)
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
    Delay( 300 );
  }
}


/**************************************************************************//**
 * @brief  Fire energy mode segments on the LCD.
 *****************************************************************************/
static void BullsEye( int mode )
{
  if ( mode )
  {
    SegmentLCD_EnergyMode( 0, 1 );
    RTCDRV_Delay( 100 , false);
    SegmentLCD_EnergyMode( 1, 1 );
    RTCDRV_Delay( 100 , false);
    SegmentLCD_EnergyMode( 2, 1 );
    RTCDRV_Delay( 100 , false);
    SegmentLCD_EnergyMode( 3, 1 );
    RTCDRV_Delay( 100 , false);
    SegmentLCD_EnergyMode( 4, 1 );
    RTCDRV_Delay( 100 , false);
  }
  else
  {
    SegmentLCD_EnergyMode( 0, 0 );
    SegmentLCD_EnergyMode( 1, 0 );
    SegmentLCD_EnergyMode( 2, 0 );
    SegmentLCD_EnergyMode( 3, 0 );
    SegmentLCD_EnergyMode( 4, 0 );
  }
}


/**************************************************************************//**
 * @brief  Init GPIO to read pushbutton inputs.
 *****************************************************************************/
static void GpioInit( void )
{
  /* Enable GPIO clock */
  CMU_ClockEnable(cmuClock_GPIO, true);

  /* Configure PB10 as input */
  GPIO_PinModeSet(gpioPortB, 10, gpioModeInput, 0);

  /* Configure PB9 as input */
  GPIO_PinModeSet(gpioPortB, 9, gpioModeInput, 0);
}


/**************************************************************************//**
 * @brief  Convert a hex nibble to ASCII.
 *****************************************************************************/
static char Hex2Char( uint32_t value )
{
  value &= 0xF;

  if ( value < 10 )
    return '0' + value;

  return 'A' + value - 10;
}


/**************************************************************************//**
 * @brief  Hard fault exception handler.
 *****************************************************************************/
void HardFault_Handler( void )    /* We should never end up here !           */
{
  for(;;)
  {
    SegmentLCD_Write( " HARD  ");
    Delay( 800 );
    SegmentLCD_Write( " FAULT ");
    Delay( 800 );
  }
}


/**************************************************************************//**
 * @brief  Memory protection fault first level exception handler.
 *****************************************************************************/
#ifdef __CC_ARM  /* MDK-ARM compiler */

__asm void MemManage_Handler( void )
{
  EXTERN MemManage_HandlerC
  TST   LR, #4
  ITE   EQ
  MRSEQ R0, MSP
  MRSNE R0, PSP
  B MemManage_HandlerC
}
#else

#if defined(__GNUC__)
void MemManage_Handler( void ) __attribute__ ((naked));
#endif
void MemManage_Handler( void )
{
  /*
   * Get the appropriate stack pointer, depending on our mode,
   * and use it as a parameter to a C handler.
   */
  __asm("TST   LR, #4");
  __asm("ITE   EQ");
  __asm("MRSEQ R0, MSP");
  __asm("MRSNE R0, PSP");
  __asm("B MemManage_HandlerC");
}
#endif

/**************************************************************************//**
 * @brief  Memory protection fault second level exception handler.
 *****************************************************************************/
void MemManage_HandlerC( uint32_t *stack )
{
  static char text[ 80 ], s[ 5 ];
  uint32_t pc;

  pc = stack[6];                  /* Get stacked return address              */

  strcpy( text, "    MPU FAULT AT PC 0x" );
  s[ 0 ] = Hex2Char( pc >> 12 );
  s[ 1 ] = Hex2Char( pc >> 8 );
  s[ 2 ] = Hex2Char( pc >> 4  );
  s[ 3 ] = Hex2Char( pc );
  s[ 4 ] = '\0';
  strcat( text, s );
  strcat( text, "        " );
  ScrollText( text );

  SCB->CFSR |= 0xFF;              /* Clear all status bits in the            */
                                  /* MMFSR part of CFSR                      */
  __set_CONTROL( 0 );             /* Enter Priviledged state before exit     */
}


/**************************************************************************//**
 * @brief  Main function.
 *****************************************************************************/
int main( void )
{
  int i;
  MPU_RegionInit_TypeDef flashInit       = MPU_INIT_FLASH_DEFAULT;
  MPU_RegionInit_TypeDef sramInit        = MPU_INIT_SRAM_DEFAULT;
  MPU_RegionInit_TypeDef peripheralInit  = MPU_INIT_PERIPHERAL_DEFAULT;

  /* Chip alignment */
  CHIP_Init();

  /* If first word of user data page is non-zero, enable eA Profiler trace */
  BSP_TraceProfilerSetup();

  /* Enable LCD without voltage boost */
  SegmentLCD_Init( false );
  SegmentLCD_AllOff();
  SegmentLCD_Symbol(LCD_SYMBOL_GECKO, 1);
  SegmentLCD_Symbol(LCD_SYMBOL_EFM32, 1);

  GpioInit();

  RTCDRV_Setup( cmuSelect_LFXO, cmuClkDiv_32 );

  ScrollText( "        MPU DEMO  PRESS Pb0 OR Pb1 "
              "TO GENERATE MPU EXCEPTIONS         " );

  MPU_Disable();

  /* Flash memory */
  MPU_ConfigureRegion( &flashInit );

  /* SRAM */
  MPU_ConfigureRegion( &sramInit );

  /* SRAM, a 4k part with priviledged only access, this regions settings  */
  /* will override those of the previous region                           */
  sramInit.regionNo         = 2;
  sramInit.baseAddress      = RAM_MEM_BASE + 0x2000;
  sramInit.size             = mpuRegionSize4Kb;
  sramInit.accessPermission = mpuRegionApPRw;
  MPU_ConfigureRegion( &sramInit );

  /* LCD, priviledged only access */
  peripheralInit.regionNo         = 3;
  peripheralInit.baseAddress      = LCD_BASE;
  peripheralInit.size             = mpuRegionSize128b;
  peripheralInit.accessPermission = mpuRegionApPRw;
  MPU_ConfigureRegion( &peripheralInit );

  MPU_Enable( MPU_CTRL_PRIVDEFENA ); /* Full access to default memory map */
                                     /* in priviledged state              */

  i = 0;
  while ( 1 )
  {
    SegmentLCD_Number( i );          /* Count on numeric diplay */
    i = ( i + 1 ) % 101;
    RTCDRV_Delay( 150 , false);

    if ( PB0_PUSHED() )
    {
      BullsEye( 1 );

      /* Generate an access violation in internal SRAM          */
      __set_CONTROL( 1 );   /* Enter User (unpriviledged) state */
      *(volatile uint32_t *)(RAM_MEM_BASE + 0x2000) = 1;

      BullsEye( 0 );
    }

    if ( PB1_PUSHED() )
    {
      BullsEye( 1 );

      /* Generate an access violation in LCD peripheral         */
      __set_CONTROL( 1 );   /* Enter User (unpriviledged) state */

      BullsEye( 0 );
    }
  }
}
