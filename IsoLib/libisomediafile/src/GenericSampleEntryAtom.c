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
	$Id: GenericSampleEntryAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4GenericSampleEntryAtomPtr self;
	err = MP4NoErr;
	self = (MP4GenericSampleEntryAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
		
	if ( self->data )
	{
		free( self->data );
		self->data = NULL;
	}
	DESTROY_ATOM_LIST_F( ExtensionAtomList )
	
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	MP4GenericSampleEntryAtomPtr self = (MP4GenericSampleEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	PUTBYTES( self->reserved, 6 );
	PUT16( dataReferenceIndex );
	PUTBYTES( self->data, self->dataSize );
	SERIALIZE_ATOM_LIST( ExtensionAtomList );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4GenericSampleEntryAtomPtr self = (MP4GenericSampleEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += 8 + self->dataSize;
	ADD_ATOM_LIST_SIZE( ExtensionAtomList );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4GenericSampleEntryAtomPtr self = (MP4GenericSampleEntryAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GETBYTES( 6, reserved );
	GET16( dataReferenceIndex );
	self->dataSize = self->size - self->bytesRead;
	self->data = (char*) malloc( self->dataSize );
	TESTMALLOC( self->data );
	GETBYTES( self->dataSize, data );
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateGenericSampleEntryAtom( MP4GenericSampleEntryAtomPtr *outAtom )
{
	MP4Err err;
	MP4GenericSampleEntryAtomPtr self;
	
	self = (MP4GenericSampleEntryAtomPtr) calloc( 1, sizeof(MP4GenericSampleEntryAtom) );
	TESTMALLOC( self );

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type                  = MP4GenericSampleEntryAtomType;
	self->name                  = "generic sample entry";
	err = MP4MakeLinkedList( &self->ExtensionAtomList ); if (err) goto bail;
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy               = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
