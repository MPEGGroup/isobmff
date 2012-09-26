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
	$Id: ItemInfoEntryAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	ISOItemInfoEntryAtomPtr self = (ISOItemInfoEntryAtomPtr) s;
    err = MP4NoErr;
	if ( s == NULL )
		BAILWITHERROR( MP4BadParamErr )
	if ( self->item_name ) {
		free( self->item_name );
		self->item_name = NULL;
	}
	if ( self->content_type ) {
		free( self->content_type );
		self->content_type = NULL;
	}
	if ( self->content_encoding ) {
		free( self->content_encoding );
		self->content_encoding = NULL;
	}
	
	if ( s->super )
		s->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	ISOItemInfoEntryAtomPtr self = (ISOItemInfoEntryAtomPtr) s;
	u8 x;
	
	err = MP4NoErr;
	x = 0;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	PUT16( item_ID );
	PUT16( protection_index );
	if ( self->item_name )
	{
		u32 len = strlen( self->item_name ) + 1;
		PUTBYTES( self->item_name, len );
	} else { PUT8_V( x ); }
	
	if ( self->content_type )
	{
		u32 len = strlen( self->content_type ) + 1;
		PUTBYTES( self->content_type, len );
	}  else { PUT8_V( x ); }
	
	if ( self->content_encoding )
	{
		u32 len = strlen( self->content_encoding ) + 1;
		PUTBYTES( self->content_encoding, len );
	}  else { PUT8_V( x ); }

	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	ISOItemInfoEntryAtomPtr self = (ISOItemInfoEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += 7;	/* includes the null terminators of the three strings */
	
	if ( self->item_name )
		self->size += strlen( self->item_name );
	if ( self->content_type )
		self->size += strlen( self->content_type );
	if ( self->content_encoding )
		self->size += strlen( self->content_encoding );

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	long bytesToRead;
	char inputstr[4096];
	char* str;
	u32 i;
	char msgString[200];

	ISOItemInfoEntryAtomPtr self = (ISOItemInfoEntryAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	
	GET16( item_ID );
	GET16( protection_index );
	
	bytesToRead = self->size - self->bytesRead;
	if ( bytesToRead < 0 )
		BAILWITHERROR( MP4BadDataErr )
	
	if (bytesToRead > 0) { 
		GETBYTES_V( bytesToRead, inputstr );
	} else inputstr[0] = inputstr[1] = inputstr[2] = 0;
	
	str = inputstr;
	
	if (bytesToRead > 0) {
		i = strlen( str ); bytesToRead -= i+1;
		self->item_name = calloc( i+1, 1 );
		strcpy( self->item_name, str ); str = str + i + 1;
		sprintf( msgString, "itemName = '%s'", self->item_name );
		inputStream->msg( inputStream, msgString );
	}
	else self->item_name = NULL;
	
	if (bytesToRead > 0) {
		i = strlen( str ); bytesToRead -= i+1;
		self->content_type = calloc( i+1, 1 );
		strcpy( self->content_type, str ); str = str + i + 1;
		sprintf( msgString, "contentType = '%s'", self->content_type );
		inputStream->msg( inputStream, msgString );
	}
	else self->content_type = NULL;

	if (bytesToRead > 0) {
		i = strlen( str ); bytesToRead -= i+1;
		self->content_encoding = calloc( i+1, 1 );
		strcpy( self->content_encoding, str );
		sprintf( msgString, "contentEncoding = '%s'", self->content_encoding );
		inputStream->msg( inputStream, msgString );
	}
	else self->content_encoding = NULL;
	
	if (bytesToRead != 0)
		{ BAILWITHERROR( MP4BadDataErr ); }

bail:
	TEST_RETURN( err );

	return err;
}

MP4Err ISOCreateItemInfoEntryAtom( ISOItemInfoEntryAtomPtr *outAtom )
{
	MP4Err err;
	ISOItemInfoEntryAtomPtr self;
	
	self = (ISOItemInfoEntryAtomPtr) calloc( 1, sizeof(ISOItemInfoEntryAtom) );
	if ( self == NULL )	BAILWITHERROR( MP4NoMemoryErr )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = ISOItemInfoEntryAtomType;
	self->name                = "item info entry";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;

	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
