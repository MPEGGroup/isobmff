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
	$Id: MP4ESDescriptor.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include <string.h>
#include <stdlib.h>


/* JLF 11/00 : added AddDescriptor for needed. */
static MP4Err addDescriptor( struct MP4DescriptorRecord* s, MP4DescriptorPtr desc )
{
	MP4Err err;
	MP4ES_DescriptorPtr self = (MP4ES_DescriptorPtr) s;
	err = MP4NoErr;

	/* try to handle OCI */
	if ( (desc->tag >= MP4ISO_OCI_EXT_RANGE_BEGIN)
		&& (desc->tag <= MP4ISO_OCI_EXT_RANGE_END) ) {
		return MP4AddListEntry( desc, self->IPIDataSet );
	}
		
	switch( desc->tag ) {
			case MP4DecoderConfigDescriptorTag:
				if ( self->decoderConfig )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->decoderConfig = desc;
				break;

			case MP4SLConfigDescriptorTag:
				if ( self->slConfig )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->slConfig = desc;
				break;

			case MP4IPI_DescriptorPointerTag:
				if ( self->ipiPtr )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->ipiPtr = desc;
				break;

			case MPM4QoS_DescriptorTag:
				if ( self->qos )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->qos = desc;
				break;

			case MP4LanguageDescriptorTag:
				err = MP4AddListEntry( desc, self->langDesc ); if (err) goto bail;
				break;

			case MP4IPMP_DescriptorPointerTag:
				err = MP4AddListEntry( desc, self->IPMPDescriptorPointers ); if (err) goto bail;
				break;

			case MP4ContentIdentDescriptorTag:
			case MP4SupplContentIdentDescriptorTag:
				err = MP4AddListEntry( desc, self->IPIDataSet ); if (err) goto bail;
				break;

			default:
				err = MP4AddListEntry( desc, self->extensionDescriptors ); if (err) goto bail;
				break;

	}
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err calculateSize( struct MP4DescriptorRecord* s )
{
	MP4Err err;
	u32 count;
	u32 i;
	MP4DescriptorPtr desc;
	MP4ES_DescriptorPtr self = (MP4ES_DescriptorPtr) s;
	err = MP4NoErr;
	self->size = DESCRIPTOR_TAG_LEN_SIZE; /* tag + length word */
	self->size += 3;
	if ( self->dependsOnES )
		self->size += 2;
	if ( self->OCRESID )
		self->size += 2;
	if ( self->URLStringLength )
		self->size += self->URLStringLength + 1;
	ADD_DESCRIPTOR_SIZE( decoderConfig );
	ADD_DESCRIPTOR_SIZE( slConfig );
	ADD_DESCRIPTOR_SIZE( ipiPtr );
	ADD_DESCRIPTOR_SIZE( qos );
	
	ADD_DESCRIPTOR_LIST_SIZE( IPIDataSet );
	ADD_DESCRIPTOR_LIST_SIZE( langDesc );
	ADD_DESCRIPTOR_LIST_SIZE( IPMPDescriptorPointers );
	ADD_DESCRIPTOR_LIST_SIZE( extensionDescriptors );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
	MP4Err err;
	u8 val;
	MP4ES_DescriptorPtr self = (MP4ES_DescriptorPtr) s;
	err = MP4NoErr;
	err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
	buffer += DESCRIPTOR_TAG_LEN_SIZE;
	
	PUT16( ESID );
	val = self->streamPriority;
	if ( self->OCRESID )
		val |= (1<<5);
	if ( self->URLStringLength )
		val |= (1<<6);
	if ( self->dependsOnES )
		val |= (1<<7);
	PUT8_V( val );

	if ( self->dependsOnES )
	{
		PUT16( dependsOnES );
	}
	if ( self->URLStringLength )
	{
		PUT8( URLStringLength );
		PUTBYTES( self->URLString, self->URLStringLength );
	}
	if ( self->OCRESID )
	{
		PUT16( OCRESID );		
	}
	SERIALIZE_DESCRIPTOR( decoderConfig );
	SERIALIZE_DESCRIPTOR( slConfig );
	SERIALIZE_DESCRIPTOR( ipiPtr );
	SERIALIZE_DESCRIPTOR_LIST( IPIDataSet );
	SERIALIZE_DESCRIPTOR_LIST( IPMPDescriptorPointers );
	SERIALIZE_DESCRIPTOR_LIST( langDesc );
	SERIALIZE_DESCRIPTOR( qos );
	SERIALIZE_DESCRIPTOR_LIST( extensionDescriptors );

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( struct MP4DescriptorRecord* s, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32  val;
	u32 urlflag;
	u32 streamDependenceFlag;
	u32 ocrflag;
	MP4ES_DescriptorPtr self = (MP4ES_DescriptorPtr) s;
	err = MP4NoErr;

	GET16( ESID );
	GET8_V_MSG( val, NULL );
	streamDependenceFlag = val & (1 << 7) ? 1 : 0;
	urlflag = val & (1 << 6) ? 1 : 0;
	ocrflag = val & (1 << 5) ? 1 : 0;
	/* ocrflag = 0;  uncomment this line to ignore OCRFlag in systems V1 files */
	self->streamPriority = val & 0x1f;

	DEBUG_SPRINTF( "streamDependenceFlag = %d",   streamDependenceFlag );
	DEBUG_SPRINTF( "urlflag = %d", urlflag );
	DEBUG_SPRINTF( "streamPriority = %d", self->streamPriority );
	DEBUG_SPRINTF( "OCRStreamFlag = %d", ocrflag );

	/* jlf 12/00: get the streamDepend */
	if ( streamDependenceFlag )
	{
		GET16( dependsOnES );
	}

	if ( urlflag )
	{
		GET8( URLStringLength );
		self->URLString = (char*) calloc( 1, self->URLStringLength );
		TESTMALLOC( self->URLString )
		GETBYTES( self->URLStringLength, URLString );
	}

	if ( ocrflag )
	{
		GET16( OCRESID );
	}
	
	while ( self->bytesRead < self->size )
	{
		MP4DescriptorPtr desc;
		err = MP4ParseDescriptor( inputStream, &desc ); if (err) goto bail;
		switch( desc->tag )
		{
			case MP4DecoderConfigDescriptorTag:
				if ( self->decoderConfig )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->decoderConfig = desc;
				break;

			case MP4SLConfigDescriptorTag:
				if ( self->slConfig )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->slConfig = desc;
				break;

			case MP4IPI_DescriptorPointerTag:
				if ( self->ipiPtr )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->ipiPtr = desc;
				break;

			case MPM4QoS_DescriptorTag:
				if ( self->qos )
					BAILWITHERROR( MP4InvalidMediaErr )
				self->qos = desc;
				break;

			case MP4LanguageDescriptorTag:
				err = MP4AddListEntry( desc, self->langDesc ); if (err) goto bail;
				break;

			case MP4ContentIdentDescriptorTag:
			case MP4SupplContentIdentDescriptorTag:
				err = MP4AddListEntry( desc, self->IPIDataSet ); if (err) goto bail;
				break;

			case MP4IPMP_DescriptorPointerTag:
				err = MP4AddListEntry( desc, self->IPMPDescriptorPointers ); if (err) goto bail;
				break;

			default:
				err = MP4AddListEntry( desc, self->extensionDescriptors ); if (err) goto bail;
				break;
		}
		self->bytesRead += desc->size;
	}
	
bail:
	TEST_RETURN( err );

	return err;
}


static void destroy( struct MP4DescriptorRecord* s )
{
	MP4ES_DescriptorPtr self = (MP4ES_DescriptorPtr) s;
	if ( self->URLString ) {
		free( self->URLString );
		self->URLString = NULL;
	}
	
	DESTROY_DESCRIPTOR( decoderConfig );
	DESTROY_DESCRIPTOR( slConfig );
	DESTROY_DESCRIPTOR( ipiPtr );
	DESTROY_DESCRIPTOR( qos );
	DESTROY_DESCRIPTOR_LIST( IPIDataSet )
	DESTROY_DESCRIPTOR_LIST( langDesc )
	DESTROY_DESCRIPTOR_LIST( IPMPDescriptorPointers )
	DESTROY_DESCRIPTOR_LIST( extensionDescriptors )

	free( s );
bail:
	return;
}

MP4Err MP4CreateES_Descriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
	SETUP_BASE_DESCRIPTOR( MP4ES_Descriptor )
	self->addDescriptor = addDescriptor;
	err = MP4MakeLinkedList( &self->IPIDataSet );             if (err) goto bail;
	err = MP4MakeLinkedList( &self->langDesc );               if (err) goto bail;
	err = MP4MakeLinkedList( &self->IPMPDescriptorPointers ); if (err) goto bail;
	err = MP4MakeLinkedList( &self->extensionDescriptors );   if (err) goto bail;
	*outDesc = (MP4DescriptorPtr) self;
bail:
	TEST_RETURN( err );

	return err;
}
