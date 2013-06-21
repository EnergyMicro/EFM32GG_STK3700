/***************************************************************************//**
 * @file
 * @brief Non-Volatile Memory configuration.
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
#ifndef __NVMCONFIG_H
#define __NVMCONFIG_H

#include <stdint.h>
#include <stdbool.h>

#include "nvm.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * @addtogroup NVM
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup test
 * @{
 ******************************************************************************/

/*******************************************************************************
 ****************************   CONFIGURATION   ********************************
 ******************************************************************************/




/*******************************************************************************
 ******************************   TYPEDEFS   ***********************************
 ******************************************************************************/

/* Object IDs.
 * Enum used to store IDs in a readable format. */
typedef enum
{
  COUNTER_ID
} NVM_Object_Ids;

/* Page IDs.
 * Enum used to store IDs in a readable format. */
typedef enum
{
  PAGE_NORMAL_ID,
  PAGE_WEAR_ID
} NVM_Page_Ids;

/*******************************************************************************
 **************************   GLOBAL VARIABLES   *******************************
 ******************************************************************************/

extern uint16_t  nvm_counter;

NVM_Config_t const *NVM_ConfigGet(void);

/** @} (end defgroup test) */
/** @} (end addtogroup NVM) */

#ifdef __cplusplus
}
#endif

#endif /* __NVMCONFIG_H */
