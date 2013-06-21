/***************************************************************************//**
 * @file  msddmedia.h
 * @brief Media interface for Mass Storage class Device (MSD).
 * @author Energy Micro AS
 * @version 3.20.0
 *******************************************************************************
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
#ifndef __MSDDMEDIA_H
#define __MSDDMEDIA_H

/* NOTE: Only use MSD_SRAM_MEDIA or MSD_FLASH_MEDIA on STK3700         */
#define MSD_SRAM_MEDIA          0   /* 96K "disk" in internal SRAM     */
#define MSD_FLASH_MEDIA         3   /* 512K "disk" in internal FLASH   */

/* NOTE: Don't use the following three options on STK3700              */
#define MSD_PSRAM_MEDIA         1   /* 4M "disk" in external PSRAM     */
#define MSD_SDCARD_MEDIA        2   /* External micro SD-Card "disk"   */
#define MSD_NORFLASH_MEDIA      4   /* 16M "disk" in external NORFLASH */

#if !defined( MSD_MEDIA )
#define MSD_MEDIA  MSD_FLASH_MEDIA  /* Select media type */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*** MSD Media Function prototypes ***/

bool     MSDDMEDIA_CheckAccess( MSDD_CmdStatus_TypeDef *pCmd, uint32_t lba, uint32_t sectors );
void     MSDDMEDIA_Flush( void );
uint32_t MSDDMEDIA_GetSectorCount( void );
bool     MSDDMEDIA_Init( void );
void     MSDDMEDIA_Read(  MSDD_CmdStatus_TypeDef *pCmd, uint8_t *data, uint32_t sectors );
void     MSDDMEDIA_Write( MSDD_CmdStatus_TypeDef *pCmd, uint8_t *data, uint32_t sectors );

#ifdef __cplusplus
}
#endif

#endif /* __MSDDMEDIA_H */
