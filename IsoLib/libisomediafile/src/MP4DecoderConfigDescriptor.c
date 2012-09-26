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
	$Id:
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include <stdlib.h>

static MP4Err calculateSize( struct MP4DescriptorRecord* s )
{
	MP4Err err;
	MP4DecoderConfigDescriptorPtr self = (MP4DecoderConfigDescriptorPtr) s;
	err = MP4NoErr;
	self->size = DESCRIPTOR_TAG_LEN_SIZE; /* tag + length word */
	self->size += 13;
	ADD_DESCRIPTOR_SIZE( decoderSpecificInfo );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
	MP4Err err;
	u8 val;
	MP4DecoderConfigDescriptorPtr self = (MP4DecoderConfigDescriptorPtr) s;
	err = MP4NoErr;
	err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
	buffer += DESCRIPTOR_TAG_LEN_SIZE;
	
	PUT8( objectTypeIndication );
	val = self->streamType << 2;
	if ( self->upstream )
		val |= (1<<1);
	val |= 1;
	PUT8_V( val );
	PUT24( bufferSizeDB );
	PUT32( maxBitrate );
	PUT32( avgBitrate );
	SERIALIZE_DESCRIPTOR( decoderSpecificInfo );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( struct MP4DescriptorRecord* s, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32  val;
	MP4DescriptorPtr desc;
	MP4DecoderConfigDescriptorPtr self = (MP4DecoderConfigDescriptorPtr) s;
	err = MP4NoErr;

	GET8( objectTypeIndication );
	GET8_V_MSG( val, NULL );
	self->upstream = val & (1 << 1);
	self->streamType = val >> 2;
	DEBUG_SPRINTF( "upstream = %d",   self->upstream );
	DEBUG_SPRINTF( "streamType = %d", self->streamType );
	DEBUG_SPRINTF( "reserved = %d",   (val & 1) );
	
	GET16_V_MSG( val, NULL );
	
	self->bufferSizeDB = val << 8;
	GET8_V_MSG( val, NULL );
	self->bufferSizeDB |= val;
	DEBUG_SPRINTF( "bufferSizeDB = %d", self->bufferSizeDB );
	
	GET32( maxBitrate );
	GET32( avgBitrate );
	if ( self->bytesRead < self->size )
	{
		err = MP4ParseDescriptor( inputStream, &desc ); if (err) goto bail;
		self->bytesRead += desc->size;
		if ( desc->tag != MP4DecSpecificInfoDescriptorTag )
		{
			DEBUG_MSG( "note: got bogus decoderspecific info" );
		}
		else
		{
			self->decoderSpecificInfo = desc;
			self->bytesRead += desc->size;
		}
	}
	else
		DEBUG_MSG( "(no decoder specific info)" );
bail:
	TEST_RETURN( err );

	return err;
}
	
static void destroy( struct MP4DescriptorRecord* s )
{
	MP4DecoderConfigDescriptorPtr self = (MP4DecoderConfigDescriptorPtr) s;
	if ( self->decoderSpecificInfo )
	{
		self->decoderSpecificInfo->destroy( self->decoderSpecificInfo );
		self->decoderSpecificInfo = NULL;
	}

	free( s );
}

MP4Err MP4CreateDecoderConfigDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
	SETUP_BASE_DESCRIPTOR( MP4DecoderConfigDescriptor )
	*outDesc = (MP4DescriptorPtr) self;
bail:
	TEST_RETURN( err );

	return err;
}
