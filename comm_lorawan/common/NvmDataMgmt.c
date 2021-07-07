/*!
 * \file      NvmDataMgmt.c
 *
 * \brief     NVM context management implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 *               ___ _____ _   ___ _  _____ ___  ___  ___ ___
 *              / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 *              \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 *              |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 *              embedded.connectivity.solutions===============
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 *
 * \author    Daniel Jaeckle ( STACKFORCE )
 *
 * \author    Johannes Bruder ( STACKFORCE )
 */

#include <stdio.h>
#include <string.h>
#include "utilities.h"
#include "nvmm.h"
#include "LoRaMac.h"
#include "NvmDataMgmt.h"
#include "nvm3.h"
#include "nvm3_hal_flash.h"

/*!
 * Enables/Disables the context storage management storage.
 * Must be enabled for LoRaWAN 1.0.4 or later.
 */
#ifndef CONTEXT_MANAGEMENT_ENABLED
#define CONTEXT_MANAGEMENT_ENABLED         1
#endif

#define MARK_KEY  0xAAAAAAAA

static uint16_t NvmNotifyFlags = 0;

Ecode_t status;

size_t numberOfObjects;

void NvmDataMgmtInit( void ) {

  LoRaMacNvmData_t* nvm;
  uint8_t clean_data[1024];
  memset(clean_data,0,1024);

  status = nvm3_open(nvm3_defaultHandle,nvm3_defaultInit);
  if (status != ECODE_NVM3_OK) {
    // Handle error
  }

  // Get the number of valid keys already in NVM3
  numberOfObjects = nvm3_countObjects(nvm3_defaultHandle);
  // Skip if we have initial keys. If not, generate objects and store
  // persistently in NVM3 before proceeding.
  if (numberOfObjects < 8) {
    // Erase all objects and write initial data to NVM3
      nvm3_eraseAll(nvm3_defaultHandle);
      nvm3_writeData(nvm3_defaultHandle, 1, clean_data, sizeof( uint32_t) );
      nvm3_writeData(nvm3_defaultHandle, 2, clean_data, sizeof( nvm->Crypto ) );
      nvm3_writeData(nvm3_defaultHandle, 3, clean_data, sizeof( nvm->MacGroup1 ) );
      nvm3_writeData(nvm3_defaultHandle, 4, clean_data, sizeof( nvm->MacGroup2 ) );
      nvm3_writeData(nvm3_defaultHandle, 5, clean_data, sizeof( nvm->SecureElement ) );
      nvm3_writeData(nvm3_defaultHandle, 6, clean_data, sizeof( nvm->RegionGroup1 ) );
      nvm3_writeData(nvm3_defaultHandle, 7, clean_data, sizeof( nvm->RegionGroup2 ) );
      nvm3_writeData(nvm3_defaultHandle, 8, clean_data, sizeof( nvm->ClassB ) );
  }

  return;

}

void NvmDataMgmtEvent( uint16_t notifyFlags )
{
    NvmNotifyFlags = notifyFlags;
}

uint16_t NvmDataMgmtStore( void )
{
#if( CONTEXT_MANAGEMENT_ENABLED == 1 )
    uint16_t dataSize = 0;
    MibRequestConfirm_t mibReq;
    mibReq.Type = MIB_NVM_CTXS;
    LoRaMacMibGetRequestConfirm( &mibReq );
    LoRaMacNvmData_t* nvm = mibReq.Param.Contexts;

    // Input checks
    if( NvmNotifyFlags == LORAMAC_NVM_NOTIFY_FLAG_NONE )
    {
        // There was no update.
        return 0;
    }
    if( LoRaMacStop( ) != LORAMAC_STATUS_OK )
    {
        return 0;
    }

    // Check if mark key has been set already
    uint32_t set_key;
    nvm3_readData(nvm3_defaultHandle, 1, ( uint8_t* ) &set_key, sizeof( uint32_t ) );

    if ( set_key == 0 ) {
        set_key = MARK_KEY;
        nvm3_writeData(nvm3_defaultHandle, 1, ( uint8_t* ) &set_key, sizeof( uint32_t ));
    }

    // Crypto
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_CRYPTO ) ==
        LORAMAC_NVM_NOTIFY_FLAG_CRYPTO )
    {
        nvm3_writeData(nvm3_defaultHandle, 2, ( uint8_t* ) &nvm->Crypto, sizeof( nvm->Crypto ));
    }

    // MacGroup1
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP1 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP1 )
    {
        nvm3_writeData(nvm3_defaultHandle, 3, ( uint8_t* ) &nvm->MacGroup1, sizeof( nvm->MacGroup1 ));
    }

    // MacGroup2
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP2 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_MAC_GROUP2 )
    {
        nvm3_writeData(nvm3_defaultHandle, 4, ( uint8_t* ) &nvm->MacGroup2, sizeof( nvm->MacGroup2 ));
    }

    // Secure element
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_SECURE_ELEMENT ) ==
        LORAMAC_NVM_NOTIFY_FLAG_SECURE_ELEMENT )
    {
        nvm3_writeData(nvm3_defaultHandle, 5, ( uint8_t* ) &nvm->SecureElement, sizeof( nvm->SecureElement ));
    }

    // Region group 1
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP1 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP1 )
    {
        nvm3_writeData(nvm3_defaultHandle, 6, ( uint8_t* ) &nvm->RegionGroup1, sizeof( nvm->RegionGroup1 ));
    }

    // Region group 2
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP2 ) ==
        LORAMAC_NVM_NOTIFY_FLAG_REGION_GROUP2 )
    {
        nvm3_writeData(nvm3_defaultHandle, 7, ( uint8_t* ) &nvm->RegionGroup2, sizeof( nvm->RegionGroup2 ));
    }

    // Class b
    if( ( NvmNotifyFlags & LORAMAC_NVM_NOTIFY_FLAG_CLASS_B ) ==
        LORAMAC_NVM_NOTIFY_FLAG_CLASS_B )
    {
        nvm3_writeData(nvm3_defaultHandle, 8, ( uint8_t* ) &nvm->RegionGroup2, sizeof( nvm->RegionGroup2 ));
    }

    // Reset notification flags
    NvmNotifyFlags = LORAMAC_NVM_NOTIFY_FLAG_NONE;

    // Do repacking if needed
    if (nvm3_repackNeeded(nvm3_defaultHandle)) {
      status = nvm3_repack(nvm3_defaultHandle);
      if (status != ECODE_NVM3_OK) {
        // Handle error
      }
    }

    // Resume LoRaMac
    LoRaMacStart( );
    return dataSize;
#else
    return 0;
#endif
}

uint16_t NvmDataMgmtRestore( void )
{
#if( CONTEXT_MANAGEMENT_ENABLED == 1 )

  MibRequestConfirm_t mibReq;
  mibReq.Type = MIB_NVM_CTXS;
  LoRaMacMibGetRequestConfirm( &mibReq );
  LoRaMacNvmData_t* nvm = mibReq.Param.Contexts;

  uint32_t set_key;
  nvm3_readData(nvm3_defaultHandle, 1, ( uint8_t* ) &set_key, sizeof( uint32_t ) );

  if ( set_key == MARK_KEY ) {

    nvm3_readData(nvm3_defaultHandle, 2, ( uint8_t* ) &nvm->Crypto, sizeof( nvm->Crypto ) );
    nvm3_readData(nvm3_defaultHandle, 3, ( uint8_t* ) &nvm->MacGroup1, sizeof( nvm->MacGroup1 ) );
    nvm3_readData(nvm3_defaultHandle, 4, ( uint8_t* ) &nvm->MacGroup2, sizeof( nvm->MacGroup2 ) );
    nvm3_readData(nvm3_defaultHandle, 5, ( uint8_t* ) &nvm->SecureElement, sizeof( nvm->SecureElement ) );
    nvm3_readData(nvm3_defaultHandle, 6, ( uint8_t* ) &nvm->RegionGroup1, sizeof( nvm->RegionGroup1 ) );
    nvm3_readData(nvm3_defaultHandle, 7, ( uint8_t* ) &nvm->RegionGroup2, sizeof( nvm->RegionGroup2 ) );
    nvm3_readData(nvm3_defaultHandle, 8, ( uint8_t* ) &nvm->ClassB, sizeof( nvm->ClassB ) );

    return sizeof( LoRaMacNvmData_t );
  }
  else {
      return 0;
  }

#endif
  return 0;

}
