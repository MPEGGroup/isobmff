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
/*
	$Id: MP4Descriptors.h,v 1.5 2002/10/01 12:49:19 julien Exp $
*/
#ifndef INCLUDED_MP4_DESCRIPTORS_H
#define INCLUDED_MP4_DESCRIPTORS_H

#include "MP4OSMacros.h"
#include "MP4LinkedList.h"
#include "MP4InputStream.h"
#include "MP4IPMPXData.h"

enum
{
	MP4ForbiddenZeroDescriptorTag      = 0x0,
	MP4ObjectDescriptorTag            = 0x01,
	MP4InitialObjectDescriptorTag     = 0x02,
	MP4ES_DescriptorTag               = 0x03,
	MP4DecoderConfigDescriptorTag     = 0x04,
	MP4DecSpecificInfoDescriptorTag    = 0x05,
	MP4SLConfigDescriptorTag          = 0x06,
	MP4ContentIdentDescriptorTag      = 0x07,
	MP4SupplContentIdentDescriptorTag = 0x08,
	MP4IPI_DescriptorPointerTag       = 0x09,
	MP4IPMP_DescriptorPointerTag       = 0x0A,
	MP4IPMP_DescriptorTag              = 0x0B,
	MPM4QoS_DescriptorTag              = 0x0C,
	MP4RegistrationDescriptorTag       = 0x0D,
	
	MP4ES_ID_IncDescriptorTag           = 0x0E,
	MP4ES_ID_RefDescriptorTag           = 0x0F,
	MP4_IOD_Tag                         = 0x10,
	MP4_OD_Tag                          = 0x11,
    MP4IPI_DescrPointerRefTag           = 0x12,
    MP4ExtendedProfileLevelDescrTag     = 0x13,
    MP4profileLevelIndicationIndexDescrTag = 0x14,
    MP4_IPMP_ToolDescriptorPointerTag   = 0x15,

	MP4ISO_DESC_RANGE_BEGIN      = 0x16,
	MP4ISO_DESC_RANGE_END        = 0x3F,
	

	MP4ContentClassificationDescriptorTag = 0x40,
	MP4KeyWordDescriptorTag               = 0x41,
	MP4RatingDescriptorTag                = 0x42,
	MP4LanguageDescriptorTag              = 0x43,
	MP4ShortTextualDescriptorTag          = 0x44,
	MP4ExpandedTextualDescriptorTag       = 0x45,
	MP4ContentCreatorNameDescriptorTag    = 0x46,
	MP4ContentCreationDateDescriptorTag   = 0x47,
	MP4OCICreatorNameDescriptorTag        = 0x48,
	MP4OCICreationDateDescriptorTag       = 0x49,
	
	MP4ISO_OCI_EXT_RANGE_BEGIN       = 0x4A,
	MP4ISO_OCI_EXT_RANGE_END         = 0x5F,
	
    MP4IPMP_ToolListDescriptorTag    = 0x60,
    MP4IPMP_ToolParamDescriptorTag   = 0x61,
    MP4IPMP_ToolDescriptorTag        = 0x62,
    MP4IPMP_ToolTag                  = 0x63,

	MP4ISO_RESERVED_RANGE_BEGIN      = 0x64,
	MP4ISO_RESERVED_RANGE_END        = 0xBF,
	
	MP4_USER_DESC_RANGE_BEGIN        = 0xC0,
	MP4_USER_DESC_RANGE_END          = 0xFE,
	
	MP4ForbiddenFFTag                = 0xFF
};

enum
{
	MP4ObjectDescriptorUpdateTag = 0x01,
	MP4ObjectDescriptorRemoveTag = 0x02,
	MP4ESDescriptorUpdateTag    = 0x03,
	MP4ESDescriptorRemoveTag    = 0x04,
	MP4IPMP_DescriptorUpdateTag  = 0x05,
	MP4IPMP_DescriptorRemoveTag  = 0x06,
    MP4ESDRemove_Tag             = 0x07,
    MP4ObjectDescrExecuteTag     = 0x08,
    MP4IPMP_ToolDescriptorUpdateTag   = 0x09,
    MP4IPMP_ToolDescriptorRemoveTag   = 0x0A,

	MP4ISO_RESERVED_COMMANDS_BEGIN = 0x0B,
	MP4ISO_RESERVED_COMMANDS_END   = 0xBF,
	
	MP4USER_RESERVED_COMMANDS_BEGIN = 0xC0,
	MP4USER_RESERVED_COMMANDS_END   = 0xFE
	
};

enum
{
	SLConfigPredefinedNull = 1,
	SLConfigPredefinedMP4 = 2
};

#define MP4_BASE_DESCRIPTOR \
	u32 tag;                \
	u32 size;               \
	char *name;             \
	u32 bytesRead;          \
	u32 bytesWritten;       \
	MP4Err (*createFromInputStream)( struct MP4DescriptorRecord* self, struct MP4InputStreamRecord* inputStream ); \
	MP4Err (*serialize)( struct MP4DescriptorRecord* self, char* buffer ); \
	MP4Err (*calculateSize)( struct MP4DescriptorRecord* self ); \
	void   (*destroy)( struct MP4DescriptorRecord* self );


#define DESCRIPTOR_TAG_LEN_SIZE 5

typedef struct MP4DescriptorRecord
{
	MP4_BASE_DESCRIPTOR
} MP4Descriptor, *MP4DescriptorPtr;

typedef struct MP4DefaultDescriptorRecord
{
	MP4_BASE_DESCRIPTOR
	u32 dataLength;
	char *data;
} MP4DefaultDescriptor, *MP4DefaultDescriptorPtr;

typedef struct MP4ES_ID_IncDescriptorRecord
{
	MP4_BASE_DESCRIPTOR
	u32 trackID;
} MP4ES_ID_IncDescriptor, *MP4ES_ID_IncDescriptorPtr;

typedef struct MP4ES_ID_RefDescriptorRecord
{
	MP4_BASE_DESCRIPTOR
	u32 refIndex;
} MP4ES_ID_RefDescriptor, *MP4ES_ID_RefDescriptorPtr;

#define MP4_OBJECT_DESCRIPTOR \
	MP4_BASE_DESCRIPTOR \
	MP4Err (*addDescriptor)( struct MP4DescriptorRecord* self, MP4DescriptorPtr desc ); \
	MP4Err (*removeESDS)( struct MP4DescriptorRecord* self ); \
	u32 objectDescriptorID; \
	u32 URLStringLength; \
	char *URLString; \
	MP4LinkedList ES_ID_IncDescriptors; \
	MP4LinkedList ESDescriptors; \
	MP4LinkedList OCIDescriptors; \
	MP4LinkedList IPMPDescriptorPointers; \
	MP4LinkedList extensionDescriptors;
	
typedef struct MP4InitialObjectDescriptorRecord
{
	MP4_OBJECT_DESCRIPTOR
	u32 inlineProfileFlag;
	u32 OD_profileAndLevel;
	u32 scene_profileAndLevel;
	u32 audio_profileAndLevel;
	u32 visual_profileAndLevel;
	u32 graphics_profileAndLevel;
    MP4DescriptorPtr IPMPToolListDescriptor;
    MP4LinkedList IPMPToolDescriptors;
} MP4InitialObjectDescriptor, *MP4InitialObjectDescriptorPtr;

typedef struct MP4ObjectDescriptorRecord
{
	MP4_OBJECT_DESCRIPTOR
	MP4LinkedList ES_ID_RefDescriptors;
} MP4ObjectDescriptor, *MP4ObjectDescriptorPtr;

typedef struct MP4ES_DescriptorRecord
{
	MP4_BASE_DESCRIPTOR
	MP4Err (*addDescriptor)( struct MP4DescriptorRecord* self, MP4DescriptorPtr desc );
	u32 ESID;
	u32 dependsOnES;
	u32 streamPriority;
	u32 OCRESID;
	u32 URLStringLength;
	char *URLString;
	MP4DescriptorPtr decoderConfig;
	MP4DescriptorPtr slConfig;
	MP4DescriptorPtr ipiPtr;
	MP4DescriptorPtr qos;
	MP4LinkedList IPIDataSet;
	MP4LinkedList langDesc;
	MP4LinkedList IPMPDescriptorPointers;
 	MP4LinkedList extensionDescriptors;
} MP4ES_Descriptor, *MP4ES_DescriptorPtr;

typedef struct MP4DecoderConfigDescriptorRecord
{
	MP4_BASE_DESCRIPTOR
	u32 objectTypeIndication;
	u32 streamType;
	u32 upstream;
	u32 bufferSizeDB;
	u32 maxBitrate;
	u32 avgBitrate;
	MP4DescriptorPtr decoderSpecificInfo;
} MP4DecoderConfigDescriptor, *MP4DecoderConfigDescriptorPtr;

typedef struct MP4SLConfigDescriptorRecord
{
	MP4_BASE_DESCRIPTOR
	u32 predefined;
	u32 useAccessUnitStartFlag;
	u32 useAccessUnitEndFlag;
	u32 useRandomAccessPointFlag;
	u32 useRandomAccessUnitsOnlyFlag;
	u32 usePaddingFlag;
	u32 useTimestampsFlag;
	u32 useIdleFlag;
	u32 durationFlag;

	u32 timestampResolution;
	u32 OCRResolution;
	u32 timestampLength;
	u32 OCRLength;
	u32 AULength;
	u32 instantBitrateLength;
	u32 degradationPriorityLength;
	u32 AUSeqNumLength;
	u32 packetSeqNumLength;
	u32 timeScale;
	u32 AUDuration;
	u32 CUDuration;
	u64	startDTS;
	u64 startCTS;
	MP4LinkedList extensionDescriptors;
} MP4SLConfigDescriptor, *MP4SLConfigDescriptorPtr;

typedef struct MP4ObjectDescriptorUpdateRecord
{
	MP4_BASE_DESCRIPTOR
	MP4Err (*addDescriptor)( struct MP4DescriptorRecord* self, MP4DescriptorPtr desc );
	MP4LinkedList objectDescriptors;
	MP4LinkedList extensionDescriptors;
} MP4ObjectDescriptorUpdate, *MP4ObjectDescriptorUpdatePtr;

typedef struct MP4ESDescriptorUpdateRecord
{
	MP4_OBJECT_DESCRIPTOR
	MP4LinkedList ES_ID_RefDescriptors;
} MP4ESDescriptorUpdate, *MP4ESDescriptorUpdatePtr;

typedef struct MP4IPMPToolRecord
{
    MP4_BASE_DESCRIPTOR
    MP4IPMPToolIdPtr toolId;
    MP4LinkedList alternateToolIds;
    MP4IPMPByteArrayPtr toolParamDesc;
    MP4LinkedList toolUrls;
} MP4IPMPTool, *MP4IPMPToolPtr;

typedef struct MP4IPMPToolListDescriptorRecord
{
    MP4_BASE_DESCRIPTOR
    MP4Err (*addDescriptor) ( struct MP4DescriptorRecord* self, MP4DescriptorPtr desc );
    MP4LinkedList ipmpTools;
} MP4IPMPToolListDescriptor, *MP4IPMPToolListDescriptorPtr;

typedef struct MP4IPMPDescriptorPointerRecord
{
    MP4_BASE_DESCRIPTOR
    u32 ipmpDescriptorId; /* fb_reso u8 */
    u32 ipmpToolDescriptorId; /* fb_reso u16 */
    u16 ipmpEsId; /* fb_reso u16 */
} MP4IPMPDescriptorPointer, *MP4IPMPDescriptorPointerPtr;

typedef struct MP4IPMPToolDescriptorRecord
{
    MP4_BASE_DESCRIPTOR
    u16 ipmpToolDescriptorId; /* fb_reso u16 */
    MP4IPMPToolIdPtr ipmpToolId;
    MP4IPMPByteArrayPtr url;
    MP4IPMPInitializePtr initData;
    MP4LinkedList ipmpxData;
    MP4Err (*addIPMPXData)( struct MP4DescriptorRecord* self, MP4IPMPXDataPtr data );
} MP4IPMPToolDescriptor, *MP4IPMPToolDescriptorPtr;

typedef struct MP4IPMPToolDescriptorUpdateRecord
{
	MP4_BASE_DESCRIPTOR
	MP4Err (*addDescriptor)( struct MP4DescriptorRecord* self, MP4DescriptorPtr desc );
	MP4LinkedList ipmpToolDescriptors;
} MP4IPMPToolDescriptorUpdate, *MP4IPMPToolDescriptorUpdatePtr;


MP4Err MP4ParseDescriptor( struct MP4InputStreamRecord* inputStream, MP4DescriptorPtr *outDesc );
MP4Err MP4ParseCommand( struct MP4InputStreamRecord* inputStream, MP4DescriptorPtr *outDesc );

MP4Err MP4CopyDescriptor( MP4DescriptorPtr inDesc, MP4DescriptorPtr *outDesc );
MP4Err MP4EncodeBaseDescriptor( struct MP4DescriptorRecord* self, char* buffer );

MP4Err MP4CreateDefaultCommand( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateObjectDescriptorUpdate( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateESDescriptorUpdate( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );

MP4Err MP4CreateInitialObjectDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateES_Descriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateDefaultDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateDecoderConfigDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateES_ID_IncDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateES_ID_RefDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateSLConfigDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateObjectDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );

MP4Err MP4CreateIPMPTool( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateIPMPToolListDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateIPMPDescriptorPointer( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateIPMPToolDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
MP4Err MP4CreateIPMPToolDescriptorUpdate( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );

#endif
