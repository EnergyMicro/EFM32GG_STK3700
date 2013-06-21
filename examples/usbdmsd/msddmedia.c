/**************************************************************************//**
 * @file  msddmedia.c
 * @brief Media interface for Mass Storage class Device (MSD).
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
#include "em_usb.h"

#include "msdd.h"
#include "msddmedia.h"

#if ( MSD_MEDIA == MSD_PSRAM_MEDIA )
#include "em_ebi.h"
#endif

#if ( MSD_MEDIA == MSD_SDCARD_MEDIA )
#include "microsd.h"
#include "diskio.h"
#include "bsp.h"
#include "bsp_trace.h"
#endif

#if ( MSD_MEDIA == MSD_FLASH_MEDIA )
#include "em_msc.h"
#include "em_int.h"
#endif

#if ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
#include "norflash.h"
#endif

#if ( MSD_MEDIA == MSD_SRAM_MEDIA )

  /* Figure out if the SRAM is large enough */
  #if ( SRAM_SIZE < (128*1024) )
  #error "SRAM based media can only be used on devices with 128K SRAM size."
  #endif

  #define MEDIA_SIZE (96*1024)
  EFM32_ALIGN(4)
  static uint8_t storage[ MEDIA_SIZE ];

#elif ( MSD_MEDIA == MSD_PSRAM_MEDIA )

  #define MEDIA_SIZE (4*1024*1024)
  static uint8_t *storage;

#elif ( MSD_MEDIA == MSD_SDCARD_MEDIA )

#elif ( MSD_MEDIA == MSD_FLASH_MEDIA )

  #if ( FLASH_SIZE < (128*1024) )
    #error "Internal FLASH based media can only be used on devices with 128K or larger FLASH."
  #else

    /* The first 64K of FLASH is reserved for application code. */
    #define MEDIA_SIZE ( FLASH_SIZE - (64 * 1024) )

    #if ( FLASH_SIZE >= (512*1024) )
      #define FLASH_PAGESIZE 4096
    #else
      #define FLASH_PAGESIZE 2048
    #endif

    struct
    {
      uint8_t *pPageBase;
      bool    pendingWrite;
    } flashStatus;

    static uint8_t  *storage = (uint8_t*)(64*1024);
    static uint32_t flashPageSize = FLASH_PAGESIZE;
    STATIC_UBUF( flashPageBuf, FLASH_PAGESIZE );

  #endif

#elif ( MSD_MEDIA == MSD_NORFLASH_MEDIA )

    struct
    {
      uint8_t *pPageBase;
      bool    pendingWrite;
    } flashStatus;

    static uint8_t  *storage;
    static uint8_t  *flashPageBuf;
    static uint32_t flashPageSize;
#else

  #error "Illegal media definition."

#endif

#define FLUSH_TIMER           0       /* Timer id. */
#define FLUSH_TIMER_TIMEOUT   250     /* Unit is milliseconds. */

static uint32_t numSectors;

#if ( MSD_MEDIA == MSD_FLASH_MEDIA ) || ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
/**************************************************************************//**
 * @brief
 *   Erase and rewrite a flash page.
 *****************************************************************************/
#if ( MSD_MEDIA == MSD_FLASH_MEDIA )
#if !defined(__CROSSWORKS_ARM) && defined(__GNUC__)
__attribute__ ((section(".ram"),noinline)) void FlushFlash( void )
#endif
#if defined(__CROSSWORKS_ARM)
__attribute__ ((section(".fast"),noinline)) void FlushFlash( void )
#endif
#if defined(__CC_ARM)  /* MDK-ARM compiler */
#pragma arm section code="ram_code"
void FlushFlash( void )
#endif
#if defined(__ICCARM__) /* IAR compiler */
/* Suppress warnings originating from use of INT_Disable/Enable()       */
/* "Call to a non __ramfunc function from within a __ramfunc function"  */
/* "Possible rom access from within a __ramfunc function"               */
#pragma diag_suppress=Ta022
#pragma diag_suppress=Ta023
__ramfunc void FlushFlash( void )
#endif
{
  /* We can't serve interrupts while erasing or writing to flash. */
  INT_Disable();

  MSC->LOCK = MSC_UNLOCK_CODE;

  /* Erase flash page */
  MSC_ErasePage( (uint32_t*)flashStatus.pPageBase );

  /* Program flash page */
  MSC_WriteWord( (uint32_t*)flashStatus.pPageBase, flashPageBuf, flashPageSize );

  MSC->LOCK = 0;

  INT_Enable();
}
#if defined( __ICCARM__ )
#pragma diag_default=Ta022
#pragma diag_default=Ta023
#endif
#if defined(__CC_ARM)  /* MDK-ARM compiler */
#pragma arm section code
#endif
#else
static void FlushFlash( void )
{
  /* Erase flash sector */
  NORFLASH_EraseSector( (uint32_t)flashStatus.pPageBase );

  /* Program flash sector */
  NORFLASH_Program( (uint32_t)flashStatus.pPageBase, flashPageBuf, flashPageSize );
}
#endif
#endif

#if ( MSD_MEDIA == MSD_FLASH_MEDIA ) || ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
/**************************************************************************//**
 * @brief
 *   Flush pending media write operations.
 *   This will only affect flash based media.
 *****************************************************************************/
static void FlushTimerTimeout(void)
{
  MSDDMEDIA_Flush();
}
#endif

/**************************************************************************//**
 * @brief
 *   Check if a media access is legal, prepare for later data transmissions.
 *
 * @param[in] pCmd
 *   Points to a MSDD_CmdStatus_TypeDef structure which holds info about the
 *   current transfer.
 *
 * @param[in] lba
 *   Media "Logical Block Address".
 *
 * @param[in] sectors
 *   Number of 512 byte sectors to transfer.
 *
 * @return
 *   True if legal access, false otherwise.
 *****************************************************************************/
bool MSDDMEDIA_CheckAccess( MSDD_CmdStatus_TypeDef *pCmd,
                            uint32_t lba, uint32_t sectors )
{
  if ( ( lba + sectors ) > numSectors )
    return false;

  #if ( MSD_MEDIA == MSD_SRAM_MEDIA ) || ( MSD_MEDIA == MSD_PSRAM_MEDIA )
  pCmd->pData    = &storage[ lba * 512 ];
  pCmd->xferType = XFER_MEMORYMAPPED;
  #endif

  #if ( MSD_MEDIA == MSD_SDCARD_MEDIA )
  pCmd->lba      = lba;
  pCmd->xferType = XFER_INDIRECT;
  pCmd->maxBurst = MEDIA_BUFSIZ;
  #endif

  #if ( MSD_MEDIA == MSD_FLASH_MEDIA ) || ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
  pCmd->lba   = lba;
  pCmd->pData = &storage[ lba * 512 ];
  if ( pCmd->direction && !flashStatus.pendingWrite )
  {
    pCmd->xferType = XFER_MEMORYMAPPED;
  }
  else
  {
    pCmd->xferType = XFER_INDIRECT;
    pCmd->maxBurst = MEDIA_BUFSIZ;
  }
  #endif

  pCmd->xferLen = sectors * 512;

  return true;
}

/**************************************************************************//**
 * @brief
 *   Flush pending media writes.
 *****************************************************************************/
void MSDDMEDIA_Flush( void )
{
  #if ( MSD_MEDIA == MSD_FLASH_MEDIA ) || ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
  if ( flashStatus.pendingWrite )
  {
    flashStatus.pendingWrite = false;
    USBTIMER_Stop( FLUSH_TIMER );
    FlushFlash();
  }
  #endif
}

/**************************************************************************//**
 * @brief
 *   Get number of 512 byte sectors on the media.
 *
 * @return
 *   Number of sectors on media.
 *****************************************************************************/
uint32_t MSDDMEDIA_GetSectorCount( void )
{
  return numSectors;
}

/**************************************************************************//**
 * @brief
 *   Initialize the storage media interface.
 *****************************************************************************/
bool MSDDMEDIA_Init( void )
{
  #if ( MSD_MEDIA != MSD_SDCARD_MEDIA ) && ( MSD_MEDIA != MSD_NORFLASH_MEDIA )
  numSectors = MEDIA_SIZE / 512;
  #endif

  #if ( MSD_MEDIA == MSD_PSRAM_MEDIA )
  storage = (uint8_t*)EBI_BankAddress( EBI_BANK2 );
  storage[0] = 0;   /* To force new "format disk" when host detects disk. */
  #endif

  #if ( MSD_MEDIA == MSD_SDCARD_MEDIA )
  /* Enable SPI access to MicroSD card */
  BSP_PeripheralAccess( BSP_MICROSD, true );
  MICROSD_init();

  if ( disk_initialize( 0 ) != 0 )
    return false;

  /* Get numSectors from media. */
  if ( disk_ioctl( 0, GET_SECTOR_COUNT, &numSectors ) != RES_OK )
    return false;
  #endif

  #if ( MSD_MEDIA == MSD_FLASH_MEDIA )
  flashStatus.pendingWrite = false;
  MSC_Init();                         /* Unlock and calibrate flash timing  */
  MSC_Deinit();                       /* Lock flash                         */
  #endif

  #if ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
  flashStatus.pendingWrite = false;
  NORFLASH_Init();                    /* Initialize NORFLASH interface      */

  storage       = (uint8_t*)NORFLASH_DeviceInfo()->baseAddress;
  flashPageSize = NORFLASH_DeviceInfo()->sectorSize;
  /* Use external PSRAM as page (flash sector) buffer */
  flashPageBuf  = (uint8_t*)EBI_BankAddress( EBI_BANK2 );
  numSectors    = NORFLASH_DeviceInfo()->deviceSize / 512;
  #endif

  return true;
}

/**************************************************************************//**
 * @brief
 *   Read from indirectly accessed media.
 *
 * @param[in] pCmd
 *   Points to a MSDD_CmdStatus_TypeDef structure which holds info about the
 *   current transfer.
 *
 * @param[in] data
 *   Pointer to data buffer.
 *
 * @param[in] sectors
 *   Number of 512 byte sectors to read from media.
 *****************************************************************************/
void MSDDMEDIA_Read( MSDD_CmdStatus_TypeDef *pCmd, uint8_t *data, uint32_t sectors )
{
  #if ( MSD_MEDIA == MSD_SRAM_MEDIA ) || ( MSD_MEDIA == MSD_PSRAM_MEDIA )
  (void)pCmd;
  (void)data;
  (void)sectors;
  #endif

  #if ( MSD_MEDIA == MSD_SDCARD_MEDIA )
  disk_read( 0, data, pCmd->lba, sectors );
  #endif

  #if ( MSD_MEDIA == MSD_FLASH_MEDIA ) || ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
  /* Write pending data to flash before starting the read operation. */
  MSDDMEDIA_Flush();
  memcpy( data, pCmd->pData, sectors * 512 );
  pCmd->pData += sectors * 512;
  #endif
}

/**************************************************************************//**
 * @brief
 *   Write to indirectly accessed media.
 *
 * @param[in] pCmd
 *   Points to a MSDD_CmdStatus_TypeDef structure which holds info about the
 *   current transfer.
 *
 * @param[in] data
 *   Pointer to data buffer.
 *
 * @param[in] sectors
 *   Number of 512 byte sectors to write to media.
 *****************************************************************************/
void MSDDMEDIA_Write( MSDD_CmdStatus_TypeDef *pCmd, uint8_t *data, uint32_t sectors )
{
  #if ( MSD_MEDIA == MSD_SRAM_MEDIA ) || ( MSD_MEDIA == MSD_PSRAM_MEDIA )
  (void)pCmd;
  (void)data;
  (void)sectors;
  #endif

  #if ( MSD_MEDIA == MSD_SDCARD_MEDIA )
  disk_write( 0, data, pCmd->lba, sectors );
  #endif

  #if ( MSD_MEDIA == MSD_FLASH_MEDIA ) || ( MSD_MEDIA == MSD_NORFLASH_MEDIA )
  unsigned int i;
  uint32_t offset;

  i = 0;
  while ( i < sectors )
  {
    if ( !flashStatus.pendingWrite )
    {
      /* Copy an entire flash page to the page buffer */
      flashStatus.pendingWrite = true;
      flashStatus.pPageBase    = (uint8_t*)((uint32_t)pCmd->pData & ~( flashPageSize - 1 ));
      offset                    = pCmd->pData - flashStatus.pPageBase;
      memcpy( flashPageBuf, flashStatus.pPageBase, flashPageSize );

      /* Write the received data in the page buffer */
      memcpy( flashPageBuf + offset, data, 512 );
      data        += 512;
      pCmd->pData += 512;

      USBTIMER_Start( FLUSH_TIMER, FLUSH_TIMER_TIMEOUT, FlushTimerTimeout );
    }
    else
    {
      /* Check if current sector is located in the page buffer. */
      offset = pCmd->pData - flashStatus.pPageBase;
      if ( offset >= flashPageSize )
      {
        /*
         * Current sector not located in page buffer, flush pending data
         * before continuing.
         */
        MSDDMEDIA_Flush();
        i--;
      }
      else
      {
        /* Write the received data in the page buffer */
        memcpy( flashPageBuf + offset, data, 512 );
        data        += 512;
        pCmd->pData += 512;

        USBTIMER_Start( FLUSH_TIMER, FLUSH_TIMER_TIMEOUT, FlushTimerTimeout );
      }
    }
    i++;
  }
  #endif
}
