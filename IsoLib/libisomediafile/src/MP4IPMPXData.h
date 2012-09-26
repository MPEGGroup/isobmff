/*
This software module was originally developed by Apple Computer, Inc.
in the course of development of MPEG-4. 
This software module is an implementation of a part of one or 
more MPEG-4 tools as specified by MPEG-4. 
ISO/IEC gives users of MPEG-4 free license to this
software module or modifications thereof for use in hardware 
or software products claiming conformance to MPEG-4.
Those intending to use this software module in hardware or software
products are advised that its use may infringe existing patents.
The original developer of this software module and his/her company,
the subsequent editors and their companies, and ISO/IEC have no
liability for use of this software module or modifications thereof
in an implementation.
Copyright is not released for non MPEG-4 conforming
products. Apple Computer, Inc. retains full right to use the code for its own
purpose, assign or donate the code to a third party and to
inhibit third parties from using the code for non 
MPEG-4 conforming products.
This copyright notice must be included in all copies or
derivative works. Copyright (c) 1999, 2000.
*/
#ifndef INCLUDED_MP4_IPMPX_DATA_H
#define INCLUDED_MP4_IPMPX_DATA_H

#include "MP4LinkedList.h"
#include "MP4InputStream.h"

enum 
{
    MP4IPMP_OpaqueDataTag = 0x01,
    MP4IPMP_AudioWatermarkingInitTag= 0x02,
    MP4IPMP_VideoWatermarkingInitTag= 0x03,
    MP4IPMP_SelectiveDecryptionInitTag= 0x04,
    MP4IPMP_KeyDataTag= 0x05,
    MP4IPMP_SendAudioWatermarkTag= 0x06,
    MP4IPMP_SendVideoWatermarkTag= 0x07,
    MP4IPMP_RightsDataTag= 0x08,
    MP4IPMP_Secure_ContainerTag = 0x09,
    MP4IPMP_AddToolNotificationListenerTag = 0x0A,
    MP4IPMP_RemoveToolNotificationListenerTag = 0x0B,
    MP4IPMP_InitAuthenticationTag = 0x0C,
    MP4IPMP_MutualAuthenticationTag = 0x0D,
    MP4IPMP_UserQueryTag = 0x0E,
    MP4IPMP_UserQueryResponseTag = 0x0F,
    MP4IPMP_ToolParamCapabilitiesQueryTag = 0x10,
    MP4IPMP_ToolParamCapabilitiesResponseTag = 0x11,
    MP4IPMP_GetToolsTag = 0x12,
    MP4IPMP_GetToolsResponseTag = 0x13,
    MP4IPMP_GetToolContextTag = 0x14,
    MP4IPMP_GetToolContextResponseTag = 0x15,
    MP4IPMP_ConnectToolTag = 0x16,
    MP4IPMP_DisconnectToolTag = 0x17,
    MP4IPMP_NotifyToolEventTag = 0x18,
    MP4IPMP_CanProcessTag = 0x19
};

#define IPMPXDATA_TAG_LEN_SIZE 5

typedef struct MP4IPMPByteArrayRecord
{
    u32 size;
    char* data;
} MP4IPMPByteArray, *MP4IPMPByteArrayPtr;

typedef struct MP4IPMPToolIdRecord
{
    u64 upperPart;
    u64 lowerPart;
} MP4IPMPToolId, *MP4IPMPToolIdPtr;

#define MP4_IPMPX_DATA_BASE \
    u32 tag;                \
    u32 size;               \
    u8 version;             \
    char* name;             \
    u32 bytesRead;          \
    u32 bytesWritten;       \
    MP4Err (*createFromInputStream)( struct MP4IPMPXDataRecord* self, struct MP4InputStreamRecord* inputStream ); \
    MP4Err (*serialize)( struct MP4IPMPXDataRecord* self, char* buffer ); \
    MP4Err (*calculateSize)( struct MP4IPMPXDataRecord* self ); \
   void   (*destroy)( struct MP4IPMPXDataRecord* self );

typedef struct MP4IPMPXDataRecord
{
    MP4_IPMPX_DATA_BASE
} MP4IPMPXData, *MP4IPMPXDataPtr;

typedef struct MP4IPMPXDefaultDataRecord
{
    MP4_IPMPX_DATA_BASE
    u32 dataLength;
    char* data;
} MP4IPMPXDefaultData, *MP4IPMPXDefaultDataPtr;

typedef struct MP4IPMPInitializeRecord
{
    u32 size;             
    char *name;          
    u32 bytesRead;       
    u32 bytesWritten;
    u8 controlPointCode;
    u8 sequenceCode;
    MP4LinkedList ipmpxData;
    MP4Err (*createFromInputStream)( struct MP4IPMPInitializeRecord* self, struct MP4InputStreamRecord* inputStream );
    MP4Err (*serialize)( struct MP4IPMPInitializeRecord* self, char* buffer );
    MP4Err (*calculateSize)( struct MP4IPMPInitializeRecord* self );
    void   (*destroy)( struct MP4IPMPInitializeRecord* self );
    MP4Err (*addIPMPXData)( struct MP4IPMPInitializeRecord *self, MP4IPMPXDataPtr data );
} MP4IPMPInitialize, *MP4IPMPInitializePtr;

MP4Err MP4ParseIPMPXData( struct MP4InputStreamRecord* inputStream, MP4IPMPXDataPtr *outData );

MP4Err MP4EncodeBaseIPMPXData( struct MP4IPMPXDataRecord* self, char* buffer );

MP4Err MP4CreateIPMPInitialize( MP4IPMPInitializePtr *outIpmpInit );
MP4Err MP4CreateIPMPXDefaultData( u32 tag, u32 size, u32 bytesRead, MP4IPMPXDataPtr *outData );
#endif
