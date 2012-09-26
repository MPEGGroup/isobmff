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
	$Id: MP4DefaultCommand.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static MP4Err calculateSize( struct MP4DescriptorRecord* s )
{
	MP4Err err;


	MP4DefaultDescriptorPtr self = (MP4DefaultDescriptorPtr) s;
	err = MP4NoErr;
	if ( s == NULL )
		BAILWITHERROR( MP4BadParamErr );
	self->size = 5; /* tag + length word */
	self->size += self->dataLength;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
	MP4Err err;
	MP4DefaultDescriptorPtr self = (MP4DefaultDescriptorPtr) s;
	err = MP4NoErr;
	if ( s == NULL )
		BAILWITHERROR( MP4BadParamErr );
	err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
	buffer += DESCRIPTOR_TAG_LEN_SIZE;
	
	PUTBYTES( self->data, self->dataLength );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( struct MP4DescriptorRecord* s, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4DefaultDescriptorPtr self = (MP4DefaultDescriptorPtr) s;
	err = MP4NoErr;
	if ( s == NULL )
		BAILWITHERROR( MP4BadParamErr );
	self->dataLength = self->size - self->bytesRead;
	self->data = (char*) calloc( 1, self->dataLength );
	TESTMALLOC( self->data );
	GETBYTES( self->dataLength, data );
bail:
	TEST_RETURN( err );

	return err;
}
	
static void destroy( struct MP4DescriptorRecord* s )
{
	MP4DefaultDescriptorPtr self = (MP4DefaultDescriptorPtr) s;
	if ( self->data )
	{
		free( self->data );
		self->data = NULL;
	}
	if (self->name) free( self->name );
	free( s );
}

MP4Err MP4CreateDefaultCommand( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
	char msgBuf[ 80 ];
	SETUP_BASE_DESCRIPTOR( MP4DefaultDescriptor )
	sprintf( msgBuf, "(some command with tag 0x%02x and size %d)", tag, size );
	self->name = (char*) calloc( 1, strlen(msgBuf) + 1 );
	TESTMALLOC( self->name )
	strcpy( self->name, msgBuf );
	*outDesc = (MP4DescriptorPtr) self;
bail:
	TEST_RETURN( err );

	return err;
}
