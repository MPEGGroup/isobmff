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
	$Id: TextMetaSampleEntryAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4TextMetaSampleEntryAtomPtr self;
	err = MP4NoErr;
	self = (MP4TextMetaSampleEntryAtomPtr) s;
	if ( self == NULL ) BAILWITHERROR( MP4BadParamErr )
	DESTROY_ATOM_LIST_F( ExtensionAtomList )

	if ( self->content_encoding ) {
		free( self->content_encoding );
		self->content_encoding = NULL;
	}
	if ( self->mime_format ) {
		free( self->mime_format );
		self->mime_format = NULL;
	}

	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	MP4TextMetaSampleEntryAtomPtr self = (MP4TextMetaSampleEntryAtomPtr) s;
	u8 x;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	
	PUTBYTES( self->reserved, 6 );
	PUT16( dataReferenceIndex );

	x = 0;
	
	if ( self->content_encoding )
	{
		u32 len = strlen( self->content_encoding ) + 1;
		PUTBYTES( self->content_encoding, len );
	} else { PUT8_V( x ); }
	
	if ( self->mime_format )
	{
		u32 len = strlen( self->mime_format ) + 1;
		PUTBYTES( self->mime_format, len );
	}  else { PUT8_V( x ); }
	

	SERIALIZE_ATOM_LIST( ExtensionAtomList );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4TextMetaSampleEntryAtomPtr self = (MP4TextMetaSampleEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += 8 + 2;	/* the null terminators of the two strings */

	if ( self->content_encoding )
		self->size += strlen( self->content_encoding );
	if ( self->mime_format )
		self->size += strlen( self->mime_format );

	ADD_ATOM_LIST_SIZE( ExtensionAtomList );
bail:
	TEST_RETURN( err );

	return err;
}
	
static MP4Err readstring( char** b, MP4TextMetaSampleEntryAtomPtr self, MP4InputStreamPtr inputStream, char *fieldname )
{
	u32 byte;
	MP4Err err;
	char temp[1024];
	char msgString[200];
	char* p;
	
	err = MP4NoErr;
	p = &(temp[0]);
	
	for (;;) {
		GET8_V_MSG( byte, NULL );
		*p++ = byte;
		if (byte==0) break;
	}
	
	*b = calloc( strlen(temp)+1, 1 );
	TESTMALLOC( *b );
	strcpy( *b, temp );
	sprintf( msgString, "%s = '%s'", fieldname, *b );
	inputStream->msg( inputStream, msgString );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4TextMetaSampleEntryAtomPtr self = (MP4TextMetaSampleEntryAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GETBYTES( 6, reserved );
	GET16( dataReferenceIndex );

	if (self->size > self->bytesRead) 
		{ err = readstring( &(self->content_encoding), self, inputStream, "content_encoding" ); if (err) goto bail; }
		else self->content_encoding = NULL;
	if (self->size > self->bytesRead) 
		{ err = readstring( &(self->mime_format), self, inputStream, "mime_format" ); if (err) goto bail; }
		else self->mime_format = NULL;
	
	GETATOM_LIST( ExtensionAtomList );

bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateTextMetaSampleEntryAtom( MP4TextMetaSampleEntryAtomPtr *outAtom )
{
	MP4Err err;
	MP4TextMetaSampleEntryAtomPtr self;
	
	self = (MP4TextMetaSampleEntryAtomPtr) calloc( 1, sizeof(MP4TextMetaSampleEntryAtom) );
	TESTMALLOC( self )

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4TextMetaSampleEntryAtomType;
	self->name                = "Text meta sample entry";
	err = MP4MakeLinkedList( &self->ExtensionAtomList ); if (err) goto bail;
	self->createFromInputStream	 = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
		
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
