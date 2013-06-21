/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                         uC/OS-III example code
*                                  Application configuration header file
*
*                                   Energy Micro EFM32 (EFM32GG990F1024)
*                                              with the
*                               Energy Micro EFM32GG990F1024-STK Starter Kit
*
* @file   app_cfg.h       
* @brief
* @author Energy Micro AS
* @version 3.20.0
******************************************************************************
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
*********************************************************************************************************
*/
#ifndef  __APP_CFG_H
#define  __APP_CFG_H

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************************************
*                                         TASKS PRIORITIES
*********************************************************************************************************
*/
#define  OS_TASK_TMR_PRIO       (OS_LOWEST_PRIO-2U)

#define  APP_CFG_TASK_START_PRIO                20U /* lowest priority task */
#define  APP_CFG_TASK_ONE_PRIO                   6U
#define  APP_CFG_TASK_TWO_PRIO                   4U /* highest priority task */
#define  APP_CFG_TASK_THREE_PRIO                 5U


/*
*********************************************************************************************************
*                                         TASK STACK SIZES
*                          Size of the task stacks (# of CPU_STK entries)
*********************************************************************************************************
*/
#define  APP_CFG_TASK_START_STK_SIZE           128U
#define  APP_CFG_TASK_ONE_STK_SIZE             128U
#define  APP_CFG_TASK_TWO_STK_SIZE             128U
#define  APP_CFG_TASK_THREE_STK_SIZE           128U

#if 0
void App_TaskIdleHook(void);
void App_TaskCreateHook(OS_TCB *ptcb);
void App_TaskDelHook(OS_TCB *ptcb);
void App_TaskReturnHook(OS_TCB  *ptcb);
void App_TaskStatHook(void);
void App_TaskSwHook(void);
void App_TCBInitHook(OS_TCB *ptcb);
void App_TimeTickHook(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* end of __APP_CFG_MODULE_H */
