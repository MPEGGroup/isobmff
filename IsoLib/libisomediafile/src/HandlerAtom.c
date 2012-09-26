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
	$Id: HandlerAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4HandlerAtomPtr self = (MP4HandlerAtomPtr) s;
    err = MP4NoErr;
	if ( s == NULL )
		BAILWITHERROR( MP4BadParamErr )
	if ( self->nameUTF8 )
	{
		free( self->nameUTF8 );
		self->nameUTF8 = NULL;
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
	MP4HandlerAtomPtr self = (MP4HandlerAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    if (self->is_qt) {
	   	self->qt_componentType = MP4_FOUR_CHAR_CODE( 'm', 'h', 'l', 'r' );
		self->qt_componentManufacturer = MP4_FOUR_CHAR_CODE( 'a', 'p', 'p', 'l' );
    }

	PUT32( qt_componentType );
	PUT32( handlerType );
	PUT32( qt_componentManufacturer );
	PUT32( qt_componentFlags );
	PUT32( qt_componentFlagsMask );
	if (self->is_qt) {
	   	PUT8( nameLength );
	   	PUTBYTES( self->nameUTF8, self->nameLength );
	}
	else
	{
		PUTBYTES( self->nameUTF8, self->nameLength );
		PUT8_V( 0 );
	}

	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4HandlerAtomPtr self = (MP4HandlerAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += 20 + self->nameLength + 1;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err setName( struct MP4Atom* s, char* name, u32 is_qt )
{
	MP4Err err;
	MP4HandlerAtomPtr self = (MP4HandlerAtomPtr) s;
	char* oldname;
	err = MP4NoErr;
	
	if ( name == NULL )
		BAILWITHERROR( MP4BadParamErr )
	
	oldname = self->nameUTF8;

    self->nameLength = strlen( name );
    self->nameUTF8 = (char*) calloc( 1, self->nameLength + 1 );
    TESTMALLOC( self->nameUTF8 )
    memcpy( self->nameUTF8, name, self->nameLength + 1);
   
   	if ( oldname ) free( oldname );
	
	self->is_qt = is_qt;
   
   bail:
	TEST_RETURN( err );

	return err;
}

static void pascaltocstr( char * s )
{
	u8 size;
	u8 i;
	size = (u8) s[0];
	for (i=0; i<size; i++) s[i] = s[i+1];
	s[size] = '\0';
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	long bytesLeft;
	char debugmsg[ 256 ];
	char htype[ 8 ];
	MP4HandlerAtomPtr self = (MP4HandlerAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GET32( qt_componentType );
	GET32( handlerType );
	MP4TypeToString( self->handlerType, htype );
	sprintf( debugmsg, "handler type is '%s'", htype );
	DEBUG_MSG( debugmsg );
	GET32( qt_componentManufacturer );
	GET32( qt_componentFlags );
	GET32( qt_componentFlagsMask );
	
	bytesLeft = self->size - self->bytesRead;
	if ( bytesLeft < 0 )
		BAILWITHERROR( MP4BadDataErr )
	self->nameUTF8 = (char*) calloc( 1, bytesLeft );
	TESTMALLOC( self->nameUTF8 );
	GETBYTES_MSG( bytesLeft, nameUTF8, "handler name" );
	if ((self->nameUTF8)[0] == (bytesLeft-1)) {
		/* It's a pascal string */
		pascaltocstr( self->nameUTF8 );
		self->nameLength = bytesLeft - 1;
	}
	else if ((self->nameUTF8)[ bytesLeft-1 ] == 0) {
		self->nameLength = bytesLeft - 1;
	}
	else self->nameLength = bytesLeft;
	
	if ( self->nameLength > 0 )
	{
		sprintf( debugmsg, "handler name is '%s'", self->nameUTF8 );
		DEBUG_MSG( debugmsg );
	}
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateHandlerAtom( MP4HandlerAtomPtr *outAtom )
{
	MP4Err err;
	MP4HandlerAtomPtr self;
	
	self = (MP4HandlerAtomPtr) calloc( 1, sizeof(MP4HandlerAtom) );
	TESTMALLOC( self );

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type					= MP4HandlerAtomType;
	self->name              	= "handler reference";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             	= destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->setName	            = setName;

	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
