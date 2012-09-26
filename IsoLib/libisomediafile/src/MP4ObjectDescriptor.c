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
	$Id: MP4ObjectDescriptor.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include <stdlib.h>
#include <string.h>

static MP4Err addDescriptor( struct MP4DescriptorRecord* s, MP4DescriptorPtr desc )
{
	MP4Err err;
	MP4ObjectDescriptorPtr self = (MP4ObjectDescriptorPtr) s;
	err = MP4NoErr;
	switch( desc->tag )
	{
		case MP4ES_DescriptorTag:
			err = MP4AddListEntry( desc, self->ESDescriptors ); if (err) goto bail;
			break;

		case MP4ES_ID_IncDescriptorTag:
			err = MP4AddListEntry( desc, self->ES_ID_IncDescriptors ); if (err) goto bail;
			break;

		case MP4ES_ID_RefDescriptorTag:
			err = MP4AddListEntry( desc, self->ES_ID_RefDescriptors ); if (err) goto bail;
			break;

		case MP4IPMP_DescriptorPointerTag:
			err = MP4AddListEntry( desc, self->IPMPDescriptorPointers ); if (err) goto bail;
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
	MP4ObjectDescriptorPtr self = (MP4ObjectDescriptorPtr) s;
	err = MP4NoErr;
	self->size = DESCRIPTOR_TAG_LEN_SIZE;
	self->size += 2;
	if ( self->URLStringLength )
		self->size += self->URLStringLength + 1;
	ADD_DESCRIPTOR_LIST_SIZE( ESDescriptors );
	ADD_DESCRIPTOR_LIST_SIZE( ES_ID_IncDescriptors );
	ADD_DESCRIPTOR_LIST_SIZE( ES_ID_RefDescriptors );
	ADD_DESCRIPTOR_LIST_SIZE( OCIDescriptors );
	ADD_DESCRIPTOR_LIST_SIZE( IPMPDescriptorPointers );
	ADD_DESCRIPTOR_LIST_SIZE( extensionDescriptors );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
	MP4Err err;
	u16 val;
	MP4ObjectDescriptorPtr self = (MP4ObjectDescriptorPtr) s;
	err = MP4NoErr;
	err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
	buffer += DESCRIPTOR_TAG_LEN_SIZE;
	
	val = self->objectDescriptorID << 6;
	if ( self->URLStringLength )
		val |= (1<<5);
	val |= 0x1f;
	PUT16_V( val );

	if ( self->URLStringLength )
	{
		PUT8( URLStringLength );
		PUTBYTES( self->URLString, self->URLStringLength );
	}
	SERIALIZE_DESCRIPTOR_LIST( ESDescriptors );
	SERIALIZE_DESCRIPTOR_LIST( ES_ID_IncDescriptors );
	SERIALIZE_DESCRIPTOR_LIST( ES_ID_RefDescriptors );
	SERIALIZE_DESCRIPTOR_LIST( OCIDescriptors );
	SERIALIZE_DESCRIPTOR_LIST( IPMPDescriptorPointers );
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
	MP4ObjectDescriptorPtr self = (MP4ObjectDescriptorPtr) s;
	err = MP4NoErr;
	GET16_V_MSG( val, NULL );
	self->objectDescriptorID = val >> 6;
	urlflag = val & (1 << 5) ? 1 : 0;
	DEBUG_SPRINTF( "objectDescriptorID = %d",   self->objectDescriptorID );
	DEBUG_SPRINTF( "urlflag = %d",   urlflag );

	if ( urlflag )
	{
		GET8( URLStringLength );
		self->URLString = (char*) calloc( 1, self->URLStringLength );
		TESTMALLOC( self->URLString )
		GETBYTES( self->URLStringLength, URLString );
	}
	while ( self->bytesRead < self->size )
	{
		MP4DescriptorPtr desc;
		err = MP4ParseDescriptor( inputStream, &desc ); if (err) goto bail;
		if ( (desc->tag >= MP4ContentClassificationDescriptorTag) &&
			(desc->tag <= MP4ISO_OCI_EXT_RANGE_END) )
		{
			err = MP4AddListEntry( desc, self->OCIDescriptors ); if (err) goto bail;
		}
		else
		{
			switch( desc->tag )
			{
				case MP4ES_DescriptorTag:
					err = MP4AddListEntry( desc, self->ESDescriptors ); if (err) goto bail;
					break;
	
				case MP4ES_ID_IncDescriptorTag:
					err = MP4AddListEntry( desc, self->ES_ID_IncDescriptors ); if (err) goto bail;
					break;

				case MP4ES_ID_RefDescriptorTag:
					err = MP4AddListEntry( desc, self->ES_ID_RefDescriptors ); if (err) goto bail;
					break;
	
				case MP4IPMP_DescriptorPointerTag:
					err = MP4AddListEntry( desc, self->IPMPDescriptorPointers ); if (err) goto bail;
					break;
	
				default:
					err = MP4AddListEntry( desc, self->extensionDescriptors ); if (err) goto bail;
					break;
			}
		}
		self->bytesRead += desc->size;
	}
	
bail:
	TEST_RETURN( err );

	return err;
}
	
static void destroy( struct MP4DescriptorRecord* s )
{
	MP4ObjectDescriptorPtr self = (MP4ObjectDescriptorPtr) s;
	if ( self->URLString )
	{
		free( self->URLString );
		self->URLString = NULL;
	}

	DESTROY_DESCRIPTOR_LIST( ESDescriptors );
	DESTROY_DESCRIPTOR_LIST( ES_ID_IncDescriptors );
	DESTROY_DESCRIPTOR_LIST( ES_ID_RefDescriptors );
	DESTROY_DESCRIPTOR_LIST( OCIDescriptors );
	DESTROY_DESCRIPTOR_LIST( IPMPDescriptorPointers );
	DESTROY_DESCRIPTOR_LIST( extensionDescriptors );

	free( self );
bail:
	return;
}

static MP4Err removeESDS( struct MP4DescriptorRecord* s )
{
   MP4Err err;
   MP4ObjectDescriptorPtr self = (MP4ObjectDescriptorPtr) s;
   err = MP4NoErr;
   DESTROY_DESCRIPTOR_LIST( ESDescriptors );
   err = MP4MakeLinkedList(  &self->ESDescriptors ); if (err) goto bail;
  bail:
   return err;
}

MP4Err MP4CreateObjectDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
	SETUP_BASE_DESCRIPTOR( MP4ObjectDescriptor )
	self->addDescriptor = addDescriptor;
	self->removeESDS    = removeESDS;
	err = MP4MakeLinkedList( &self->ESDescriptors );          if (err) goto bail;

	err = MP4MakeLinkedList( &self->ES_ID_IncDescriptors );   if (err) goto bail;
	err = MP4MakeLinkedList( &self->ES_ID_RefDescriptors );   if (err) goto bail;
	err = MP4MakeLinkedList( &self->OCIDescriptors );         if (err) goto bail;
	err = MP4MakeLinkedList( &self->IPMPDescriptorPointers ); if (err) goto bail;
	err = MP4MakeLinkedList( &self->extensionDescriptors );   if (err) goto bail;
	*outDesc = (MP4DescriptorPtr) self;
bail:
	TEST_RETURN( err );

	return err;
}
