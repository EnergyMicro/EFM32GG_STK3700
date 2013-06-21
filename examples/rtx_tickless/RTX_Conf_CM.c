/*----------------------------------------------------------------------------
 *      RL-ARM - RTX
 *----------------------------------------------------------------------------
 *      Name:    RTX_Conf_CM.C
 *      Purpose: Configuration of CMSIS RTX Kernel for Cortex-M
 *      Rev.:    V4.60
 *----------------------------------------------------------------------------
 *
 * Copyright (c) 1999-2009 KEIL, 2009-2012 ARM Germany GmbH
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  - Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - Neither the name of ARM  nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/

#include "cmsis_os.h"

#include "em_cmu.h"
#include "em_emu.h"
#include "em_rtc.h"

/*----------------------------------------------------------------------------
 *      RTX User configuration part BEGIN
 *---------------------------------------------------------------------------*/

/* _os_resume and _os_suspend functions must be called as SVC services, so */
/* there is need to make some specific definitions */

extern uint32_t  rt_suspend(void);
extern void      rt_resume(uint32_t sleep_time);

#if defined (__CC_ARM)          /* ARM Compiler */

uint32_t _os_suspend(uint32_t p)                      __svc_indirect(0);
void     _os_resume(uint32_t p, uint32_t sleep_time) __svc_indirect(0);

#elif defined (__ICCARM__)      /* IAR Compiler */

_Pragma("swi_number=0") __swi uint32_t __os_suspend();
static inline uint32_t _os_suspend(uint32_t p)
{
  __asm(
    "mov r12,%0\n" :: "r" (p) : "r12"
    );
  return(__os_suspend());
};

_Pragma("swi_number=0") __swi void __os_resume(uint32_t sleep_time);
static inline void _os_resume(uint32_t p, uint32_t sleep_time)
{
  __asm(
    "mov r12,%0\n" :: "r" (p) : "r12"
    );
  __os_resume(sleep_time);
};

#elif defined (__GNUC__)        /* GNU Compiler */

#if (defined (__CORTEX_M0))
#define SVC_Call(f)                                                            \
  __asm volatile                                                                 \
  (                                                                            \
    "ldr r7,="#f"\n\t"                                                         \
    "mov r12,r7\n\t"                                                           \
    "svc 0"                                                                    \
    :               "=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)         \
    :                "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)         \
    : "r7", "r12", "lr", "cc"                                                  \
  );
#else
#define SVC_Call(f)                                                            \
  __asm volatile                                                                 \
  (                                                                            \
    "ldr r12,="#f"\n\t"                                                        \
    "svc 0"                                                                    \
    :               "=r" (__r0), "=r" (__r1), "=r" (__r2), "=r" (__r3)         \
    :                "r" (__r0),  "r" (__r1),  "r" (__r2),  "r" (__r3)         \
    : "r12", "lr", "cc"                                                        \
  );
#endif

__attribute__((always_inline)) static inline uint32_t _os_suspend(uint32_t pFunc)
{
  (void) pFunc;
  register int __r0 __asm("r0");
  register int __r1 __asm("r1");
  register int __r2 __asm("r2");
  register int __r3 __asm("r3");

  SVC_Call(rt_suspend);
  return (uint32_t) __r0;
}

__attribute__((always_inline)) static inline void _os_resume(uint32_t pFunc, uint32_t sleep_time)
{
  (void) pFunc;
  register int __r0 __asm("r0") = sleep_time;
  register int __r1 __asm("r1");
  register int __r2 __asm("r2");
  register int __r3 __asm("r3");

  SVC_Call(rt_resume);
}
#else
#error "Unknown compiler! Don't know how to create SVC function."
#endif

#define os_suspend()             _os_suspend((uint32_t) rt_suspend)
#define os_resume(sleep_time)    _os_resume((uint32_t) rt_resume, sleep_time)

//-------- <<< Use Configuration Wizard in Context Menu >>> -----------------
//
// <h>Thread Configuration
// =======================
//
//   <o>Number of concurrent running threads <0-250>
//   <i> Defines max. number of threads that will run at the same time.
//   <i> Default: 6
#ifndef OS_TASKCNT
 #define OS_TASKCNT     6
#endif

//   <o>Default Thread stack size [bytes] <64-4096:8><#/4>
//   <i> Defines default stack size for threads with osThreadDef stacksz = 0
//   <i> Default: 200
#ifndef OS_STKSIZE
 #define OS_STKSIZE     75
#endif

//   <o>Main Thread stack size [bytes] <64-4096:8><#/4>
//   <i> Defines stack size for main thread.
//   <i> Default: 200
#ifndef OS_MAINSTKSIZE
 #define OS_MAINSTKSIZE 75
#endif

//   <o>Number of threads with user-provided stack size <0-250>
//   <i> Defines the number of threads with user-provided stack size.
//   <i> Default: 0
#ifndef OS_PRIVCNT
 #define OS_PRIVCNT     0
#endif

//   <o>Total stack size [bytes] for threads with user-provided stack size <0-4096:8><#/4>
//   <i> Defines the combined stack size for threads with user-provided stack size.
//   <i> Default: 0
#ifndef OS_PRIVSTKSIZE
 #define OS_PRIVSTKSIZE 0
#endif

// <q>Check for stack overflow
// <i> Includes the stack checking code for stack overflow.
// <i> Note that additional code reduces the Kernel performance.
#ifndef OS_STKCHECK
 #define OS_STKCHECK    1
#endif

// <o>Processor mode for thread execution
//   <0=> Unprivileged mode
//   <1=> Privileged mode
// <i> Default: Privileged mode
#ifndef OS_RUNPRIV
 #define OS_RUNPRIV     1
#endif

// </h>

// <h>RTX Kernel Timer Tick Configuration
// ======================================
// <q> Use Cortex-M SysTick timer as RTX Kernel Timer
// <i> Use the Cortex-M SysTick timer as a time-base for RTX.
#ifndef OS_SYSTICK
 #define OS_SYSTICK     1
#endif
//
//   <o>Timer clock value [Hz] <1-1000000000>
//   <i> Defines the timer clock value.
//   <i> Default: 12000000  (12MHz)
#ifndef OS_CLOCK
 #define OS_CLOCK    48000000
#endif

//   <o>Timer tick value [us] <1-1000000>
//   <i> Defines the timer tick value.
//   <i> Default: 1000  (1ms)
#ifndef OS_TICK
 #define OS_TICK    (32 * 1000000 / 32768)
#endif

// </h>

// <h>System Configuration
// =======================
//
// <e>Round-Robin Thread switching
// ===============================
//
// <i> Enables Round-Robin Thread switching.
#ifndef OS_ROBIN
 #define OS_ROBIN       1
#endif

//   <o>Round-Robin Timeout [ticks] <1-1000>
//   <i> Defines how long a thread will execute before a thread switch.
//   <i> Default: 5
#ifndef OS_ROBINTOUT
 #define OS_ROBINTOUT   5
#endif

// </e>

// <e>User Timers
// ==============
//   <i> Enables user Timers
#ifndef OS_TIMERS
 #define OS_TIMERS      1
#endif

/*   <o>Number of user timers <0-250> */
/*   <i> Define max. number of user timers that will run at the same time. */
/*   <i> Default: 0  (User timers disabled) */
#ifndef OS_TIMERCNT
 #define OS_TIMERCNT    5
#endif

/*   <o>Timer Thread Priority */
/*                        <1=> Low */
/*     <2=> Below Normal  <3=> Normal  <4=> Above Normal */
/*                        <5=> High */
/*                        <6=> Realtime */
/*   <i> Defines priority for Timer Thread */
/*   <i> Default: High */
#ifndef OS_TIMERPRIO
 #define OS_TIMERPRIO   5
#endif

//   <o>Timer Thread stack size [bytes] <64-4096:8><#/4>
//   <i> Defines stack size for Timer thread.
//   <i> Default: 200
#ifndef OS_TIMERSTKSZ
 #define OS_TIMERSTKSZ  75
#endif

//   <o>Timer Callback Queue size <1-32>
//   <i> Number of concurrent active timer callback functions.
//   <i> Default: 4
#ifndef OS_TIMERCBQSZ
 #define OS_TIMERCBQS   4
#endif

// </e>

//   <o>ISR FIFO Queue size<4=>   4 entries  <8=>   8 entries
//                         <12=> 12 entries  <16=> 16 entries
//                         <24=> 24 entries  <32=> 32 entries
//                         <48=> 48 entries  <64=> 64 entries
//                         <96=> 96 entries
//   <i> ISR functions store requests to this buffer,
//   <i> when they are called from the interrupt handler.
//   <i> Default: 16 entries
#ifndef OS_FIFOSZ
 #define OS_FIFOSZ      16
#endif

// </h>

//------------- <<< end of configuration section >>> -----------------------

// Standard library system mutexes
// ===============================
//  Define max. number system mutexes that are used to protect
//  the arm standard runtime library. For microlib they are not used.
#ifndef OS_MUTEXCNT
 #define OS_MUTEXCNT    8
#endif

/*----------------------------------------------------------------------------
 *      RTX User configuration part END
 *---------------------------------------------------------------------------*/

#define OS_TRV    ((uint32_t)(((double)OS_CLOCK*(double)OS_TICK)/1E6)-1)

/*----------------------------------------------------------------------------
 *      Global Functions
 *---------------------------------------------------------------------------*/

/* This is RTC interrupt handler */
void RTC_IRQHandler(void)
{
  RTC_IntClear(RTC_IFC_COMP0);
}

/*--------------------------- os_idle_demon ---------------------------------*/

void os_idle_demon(void)
{
  RTC_Init_TypeDef init;
  unsigned int sleep;

  /* The idle demon is a system thread, running when no other thread is      */
  /* ready to run.                                                           */

  /* Enable system clock for RTC */

  /* LFXO setup */
  /* For cut D, use 70% boost */
  CMU->CTRL = (CMU->CTRL & ~_CMU_CTRL_LFXOBOOST_MASK) | CMU_CTRL_LFXOBOOST_70PCENT;
  /* For cut C, use 100% boost */
  /*CMU->CTRL    = (CMU->CTRL & ~_CMU_CTRL_LFXOBOOST_MASK) | CMU_CTRL_LFXOBOOST_100PCENT;*/

  EMU->AUXCTRL = (EMU->AUXCTRL & ~_EMU_AUXCTRL_REDLFXOBOOST_MASK) | EMU_AUXCTRL_REDLFXOBOOST;

  /* Ensure LE modules are accessible */
  CMU_ClockEnable(cmuClock_CORELE, true);

  /* Enable osc as LFACLK in CMU (will also enable oscillator if not enabled) */
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

  /* Use a 32 division prescaler to reduce power consumption. */
  CMU_ClockDivSet(cmuClock_RTC, cmuClkDiv_32);

  /* Enable clock to RTC module */
  CMU_ClockEnable(cmuClock_RTC, true);

  init.enable   = false;
  init.debugRun = false;
  init.comp0Top = false; /* Count to max value before wrapping */

  RTC_Init(&init);

  /* Disable interrupt generation from RTC0 */
  RTC_IntDisable(_RTC_IF_MASK);

  /* Enable interrupts */
  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);

  for (;;)
  {
    /* os_suspend stops scheduler and returns time to next event in OS_TICK units */
    sleep = os_suspend();
    if (sleep)
    {
      RTC_CompareSet(0, sleep - 1);
      RTC_IntClear(RTC_IFC_COMP0);
      RTC_IntEnable(RTC_IF_COMP0);
      RTC_CounterReset();
      /* Enter EM2 low power mode - could be replaced with EM1 if required */
      EMU_EnterEM2(true);
      /* get information how long we were in sleep */
      sleep = RTC_CounterGet();
      RTC_Enable(false);
    };
    /* resume scheduler providing information how long MCU was sleeping */
    os_resume(sleep);
  }
}

#if (OS_SYSTICK == 0)   // Functions for alternative timer as RTX kernel timer

/*--------------------------- os_tick_init ----------------------------------*/

// Initialize alternative hardware timer as RTX kernel timer
// Return: IRQ number of the alternative hardware timer
int os_tick_init (void) {

  return (-1);  /* Return IRQ number of timer (0..239) */
}

/*--------------------------- os_tick_irqack --------------------------------*/

// Acknowledge alternative hardware timer interrupt
void os_tick_irqack (void) {
  /* ... */
}

#endif   // (OS_SYSTICK == 0)

/*--------------------------- os_error --------------------------------------*/

void os_error(uint32_t err_code)
{
  (void) err_code;               /* Unused parameter. */
  /* This function is called when a runtime error is detected. Parameter */
  /* 'err_code' holds the runtime error code (defined in RTL.H).         */

  /* HERE: include optional code to be executed on runtime error. */
  for (;;);
}


/*----------------------------------------------------------------------------
 *      RTX Configuration Functions
 *---------------------------------------------------------------------------*/

#include "RTX_CM_lib.h"

/*----------------------------------------------------------------------------
 * end of file
 *---------------------------------------------------------------------------*/
