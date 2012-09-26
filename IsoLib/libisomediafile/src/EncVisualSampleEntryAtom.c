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
	$Id: EncVisualSampleEntryAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4EncVisualSampleEntryAtomPtr self;
	err = MP4NoErr;
	self = (MP4EncVisualSampleEntryAtomPtr) s;
	if ( self == NULL ) BAILWITHERROR( MP4BadParamErr )
	if ( self->SecurityInfo )
	{
		self->SecurityInfo->destroy( self->SecurityInfo );
		self->SecurityInfo = NULL;
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
	MP4EncVisualSampleEntryAtomPtr self = (MP4EncVisualSampleEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	PUTBYTES( self->reserved1, 6 );
	PUT16( dataReferenceIndex );
	PUTBYTES( self->reserved2, 16 );
	PUT16( width );
	PUT16( height );
	/* PUT32( reserved3 ); */
	PUT32( reserved4 );
	PUT32( reserved5 );
	PUT32( reserved6 );
	PUT16( reserved7 );
	PUT8( nameLength );
	PUTBYTES(self->name31, 31 );
	PUT16( reserved8 );
	PUT16( reserved9 );
	SERIALIZE_ATOM_LIST( ExtensionAtomList );
	SERIALIZE_ATOM( SecurityInfo );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4EncVisualSampleEntryAtomPtr self = (MP4EncVisualSampleEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += (6 + 16 + 31 + (4*2)+(1*1)+(4*4));
	ADD_ATOM_SIZE( SecurityInfo );
	ADD_ATOM_LIST_SIZE( ExtensionAtomList );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addAtom( MP4EncVisualSampleEntryAtomPtr self, MP4AtomPtr atom )
{
   MP4Err err;
   err = MP4NoErr;
   if ( atom == NULL )
      BAILWITHERROR( MP4BadParamErr );
   if (atom->type == MP4SecurityInfoAtomType)
   		self->SecurityInfo = atom;
   else { err = MP4AddListEntry( atom, self->ExtensionAtomList ); if (err) goto bail; }
  bail:
   TEST_RETURN( err );

   return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4EncVisualSampleEntryAtomPtr self = (MP4EncVisualSampleEntryAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GETBYTES( 6, reserved1 );
	GET16( dataReferenceIndex );
	GETBYTES( 16, reserved2 );
	GET16( width );
	GET16( height );
	/* GET32( reserved3 ); */
	GET32( reserved4 );
	GET32( reserved5 );
	GET32( reserved6 );
	GET16( reserved7 );
	GET8( nameLength );
	GETBYTES( 31, name31 );
	GET16( reserved8 );
	GET16( reserved9 );
	while ( self->bytesRead < self->size )
	{ 
		MP4AtomPtr atom; 
		err = MP4ParseAtom( (MP4InputStreamPtr) inputStream, &atom ); 
			if (err) goto bail; 
		self->bytesRead += atom->size; 
		if ( ((atom->type)== MP4FreeSpaceAtomType) || ((atom->type)== MP4SkipAtomType)) 
			atom->destroy( atom );
		else {
			err = addAtom( self, atom );
				if (err) goto bail;
		}
	}
	
	if ( self->bytesRead != self->size ) 
		BAILWITHERROR( MP4BadDataErr )

bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateEncVisualSampleEntryAtom( MP4EncVisualSampleEntryAtomPtr *outAtom )
{
	MP4Err err;
	MP4EncVisualSampleEntryAtomPtr self;
	
	self = (MP4EncVisualSampleEntryAtomPtr) calloc( 1, sizeof(MP4EncVisualSampleEntryAtom) );
	TESTMALLOC( self )

	err = MP4CreateEncBaseAtom( (MP4EncBaseSampleEntryAtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4EncVisualSampleEntryAtomType;
	self->name                = "protected visual sample entry";
	self->createFromInputStream	 = (cisfunc) createFromInputStream;
	self->destroy               = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->enc_type				= MP4EncVisualSampleEntryAtomType;
	
	self->width = 0x140;
	self->height = 0xf0;
	self->reserved4 = 0x00480000; self->reserved5 = 0x00480000;
	self->reserved7 = 1;
	self->reserved8 = 0x18;
	self->reserved9 = -1;
	
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
