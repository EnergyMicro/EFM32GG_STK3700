/***************************************************************************//**
 * @file
 * @brief Non-Volatile Memory Driver configuration.
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
#include "nvm_config.h"
#include "nvm.h"
#include <stddef.h>

/***************************************************************************//**
 * @addtogroup NVM
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup test
 * @{
 ******************************************************************************/

/*******************************************************************************
 *******************************   CONFIG   ************************************
 ******************************************************************************/

/** Configure extra pages to allocate for data security and wear leveling.
 * Minimum 1, but the more you add the better lifetime your system will have. */
#define NVM_PAGES_SCRATCH    3

/* Objects. */
uint16_t  nvm_counter = 0;

/* Page definition.
 * Combine objects with their id, and put them in a page. */
static NVM_Page_t const nvmPageNormal =
{
/*{Pointer to object,          Size of object,         Object ID}, */
  { (uint8_t *) &nvm_counter,  sizeof(nvm_counter),   COUNTER_ID },
  NVM_PAGE_TERMINATION /* Null termination of table. Do not remove! */
};

/* Page definition.
 * Combine objects with their id, and put them in a page.
 * This page contains only one object, since it is going to be
 * used as a wear page. */
static NVM_Page_t const nvmPageWear =
{
/*{Pointer to object,   Size of object,   Object ID}, */
  { (uint8_t *) &nvm_counter,  sizeof(nvm_counter),   COUNTER_ID },
  NVM_PAGE_TERMINATION /* Null termination of table. Do not remove! */
};

/* Register pages.
 * Connect pages to page IDs, and define the type of page. */
static NVM_Page_Table_t const nvmPagesConfig =
{
/*{Page ID,      Page pointer,   Page type}, */
  { PAGE_NORMAL_ID, &nvmPageNormal, nvmPageTypeNormal },
  { PAGE_WEAR_ID,   &nvmPageWear,   nvmPageTypeWear }
};

/* end of configuration, below defines should not be changed */
#define NUMBER_OF_USER_PAGES  (sizeof(nvmPagesConfig)/sizeof(NVM_Page_Descriptor_t))
#define NUMBER_OF_PAGES (NVM_PAGES_SCRATCH+NUMBER_OF_USER_PAGES)

/** Configure where in memory to start storing data. This area should be
 *  reserved using the linker and needs to be aligned with the physical page
 *  grouping of the device.
 *
 *  For the internal flash in the Gecko and Tiny Gecko MCUs, the flash pages are
 *  512 bytes long. This means that the start location must be a multiple of
 *  512 bytes, and that an area equal to 512 bytes * the number of pages and
 *  scratch page must be reserved here.
 *
 *  This can be allocated in the IAR linker file using:
 *
 *    place at address mem:0x1f000 { readonly section NVM_PAGES };
 *
 *  and specifying the pages in the C code using:
 *
 *    const uint8_t nvmData[NVM_PAGE_SIZE*NUMBER_OF_PAGES] @ "NVM_PAGES";
 *
 */

/* Let compiler automatically find space for nvmData in "text" segment and
    properly align it to page */
#ifdef __ICCARM__
#pragma data_alignment = NVM_PAGE_SIZE
static const uint8_t nvmData[NVM_PAGE_SIZE*NUMBER_OF_PAGES] @ ".text";
#else
static const uint8_t nvmData[NVM_PAGE_SIZE*NUMBER_OF_PAGES] __attribute__ ((__aligned__(NVM_PAGE_SIZE))) = { 0xFF };
#endif

static NVM_Config_t const nvmConfig = 
{
  &nvmPagesConfig,
  NUMBER_OF_PAGES,
  NUMBER_OF_USER_PAGES,
  nvmData
};

NVM_Config_t const *NVM_ConfigGet(void)
{
  return( &nvmConfig );
}

/** @} (end addtogroup test */
/** @} (end addtogroup NVM) */
