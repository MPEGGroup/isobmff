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
derivative works. Copyright (c) 1999.
*/
/*
	$Id: MP4InputStream.h,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#ifndef INCLUDED_MP4_INPUTSTREAM_H
#define INCLUDED_MP4_INPUTSTREAM_H

#include "MP4Movies.h"
#include "FileMappingObject.h"

#define COMMON_INPUTSTREAM_FIELDS \
	u64 available; \
	u32 debugging; \
	u32 indent; \
	void   (*msg)( struct MP4InputStreamRecord *self, char *msg ); \
	struct FileMappingObjectRecord* (*getFileMappingObject)( struct MP4InputStreamRecord *self ); \
	void   (*destroy)( struct MP4InputStreamRecord *self ); \
	MP4Err (*read8)( struct MP4InputStreamRecord *self,  u32 *outVal, char *msg ); \
	MP4Err (*read16)( struct MP4InputStreamRecord *self, u32 *outVal, char *msg ); \
	MP4Err (*read32)( struct MP4InputStreamRecord *self, u32 *outVal, char *msg ); \
	MP4Err (*readData)( struct MP4InputStreamRecord *self, u64 bytes, char *outData, char *msg ); \
	u64    (*getStreamOffset)( struct MP4InputStreamRecord *self ); \
	MP4Err (*skipData)( struct MP4InputStreamRecord *self, u64 bytes, char *msg );
	
typedef struct MP4InputStreamRecord
{
	COMMON_INPUTSTREAM_FIELDS
} MP4InputStream, *MP4InputStreamPtr;

typedef struct MP4MemoryInputStreamRecord
{
	COMMON_INPUTSTREAM_FIELDS
	char *base;
	char *ptr;
	struct FileMappingObjectRecord* mapping;
} MP4MemoryInputStream, *MP4MemoryInputStreamPtr;

typedef struct MP4FileMappingInputStreamRecord
{
	COMMON_INPUTSTREAM_FIELDS
	char *base;
	u64 current_offset;
	struct FileMappingObjectRecord* mapping;
} MP4FileMappingInputStream, *MP4FileMappingInputStreamPtr;

MP4Err MP4CreateMemoryInputStream( char *base, u32 size, MP4InputStreamPtr *outStream );
MP4Err MP4CreateFileMappingInputStream( struct FileMappingObjectRecord* mapping, MP4InputStreamPtr *outStream );

#endif
