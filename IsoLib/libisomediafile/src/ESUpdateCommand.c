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
	$Id: ESUpdateCommand.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include <stdlib.h>

static MP4Err addDescriptor( struct MP4DescriptorRecord* s, MP4DescriptorPtr desc )
{
	MP4Err err;
	MP4ESDescriptorUpdatePtr self = (MP4ESDescriptorUpdatePtr) s;
	err = MP4NoErr;
	switch( desc->tag )
	{
		case MP4ES_DescriptorTag:
			err = MP4AddListEntry( desc, self->ESDescriptors ); if (err) goto bail;
			break;

		case MP4ES_ID_RefDescriptorTag:
			err = MP4AddListEntry( desc, self->ES_ID_RefDescriptors ); if (err) goto bail;
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
	MP4ESDescriptorUpdatePtr self = (MP4ESDescriptorUpdatePtr) s;
	err = MP4NoErr;
	self->size = DESCRIPTOR_TAG_LEN_SIZE;
	self->size += 2;
	ADD_DESCRIPTOR_LIST_SIZE( ESDescriptors );
	ADD_DESCRIPTOR_LIST_SIZE( ES_ID_RefDescriptors );
	ADD_DESCRIPTOR_LIST_SIZE( extensionDescriptors );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
	MP4Err err;
	u32 val;
	MP4ESDescriptorUpdatePtr self = (MP4ESDescriptorUpdatePtr) s;
	err = MP4NoErr;
	err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
	buffer += DESCRIPTOR_TAG_LEN_SIZE;
	val = self->objectDescriptorID << 6;
	PUT16_V( val );
	SERIALIZE_DESCRIPTOR_LIST( ESDescriptors );
	SERIALIZE_DESCRIPTOR_LIST( ES_ID_RefDescriptors );
	SERIALIZE_DESCRIPTOR_LIST( extensionDescriptors );

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( struct MP4DescriptorRecord* s, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 val;
	MP4ESDescriptorUpdatePtr self = (MP4ESDescriptorUpdatePtr) s;
	err = MP4NoErr;
	
	GET16_V( val );
	self->objectDescriptorID = val >> 6;
	while ( self->bytesRead < self->size )
	{
		MP4DescriptorPtr desc;
		err = MP4ParseDescriptor( inputStream, &desc ); if (err) goto bail;
		switch( desc->tag )
		{
			case MP4ES_DescriptorTag:
				err = MP4AddListEntry( desc, self->ESDescriptors ); if (err) goto bail;
				break;

			case MP4ES_ID_RefDescriptorTag:
				err = MP4AddListEntry( desc, self->ES_ID_RefDescriptors ); if (err) goto bail;
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
	MP4ESDescriptorUpdatePtr self = (MP4ESDescriptorUpdatePtr) s;
	DESTROY_DESCRIPTOR_LIST( ESDescriptors )
	DESTROY_DESCRIPTOR_LIST( ES_ID_RefDescriptors )
	DESTROY_DESCRIPTOR_LIST( extensionDescriptors )

	free( s );
bail:
	return;
}

static MP4Err removeESDS( struct MP4DescriptorRecord* s )
{
   MP4Err err;
   MP4ESDescriptorUpdatePtr self = (MP4ESDescriptorUpdatePtr) s;
   err = MP4NoErr;
   DESTROY_DESCRIPTOR_LIST( ESDescriptors );
   err = MP4MakeLinkedList(  &self->ESDescriptors ); if (err) goto bail;
  bail:
   return err;
}

MP4Err MP4CreateESDescriptorUpdate( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
	SETUP_BASE_DESCRIPTOR( MP4ESDescriptorUpdate )
	self->addDescriptor = addDescriptor;
	self->removeESDS    = removeESDS;
	err = MP4MakeLinkedList( &self->ESDescriptors );      if (err) goto bail;
	err = MP4MakeLinkedList( &self->ES_ID_RefDescriptors );      if (err) goto bail;
	err = MP4MakeLinkedList( &self->extensionDescriptors );   if (err) goto bail;
	*outDesc = (MP4DescriptorPtr) self;
bail:
	TEST_RETURN( err );

	return err;
}
