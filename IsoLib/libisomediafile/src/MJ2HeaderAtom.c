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
	$Id: MJ2HeaderAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MJ2Atoms.h"
#include <stdlib.h>

static void destroy( MP4AtomPtr s )
{
	ISOErr err = ISONoErr;
	u32 i;
	MJ2HeaderAtomPtr self = (MJ2HeaderAtomPtr) s;
	
	if ( self == NULL )
		BAILWITHERROR( ISOBadParamErr )
	DESTROY_ATOM_LIST
	
	if ( self->super )
		self->super->destroy( s );
		
bail:
	TEST_RETURN( err );

	return;
}

static ISOErr serialize( struct MP4Atom* s, char* buffer )
{
	ISOErr err;
	MJ2HeaderAtomPtr self = (MJ2HeaderAtomPtr) s;
	
	err = ISONoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten; 
	SERIALIZE_ATOM_LIST( atomList );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static ISOErr calculateSize( struct MP4Atom* s )
{
	ISOErr err;
	MJ2HeaderAtomPtr self = (MJ2HeaderAtomPtr) s;
	err = ISONoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	ADD_ATOM_LIST_SIZE( atomList );
bail:
	TEST_RETURN( err );

	return err;
}

static ISOErr addAtom( MJ2HeaderAtomPtr self, ISOAtomPtr atom )
{
	ISOErr err;
	err = ISONoErr;
	err = MP4AddListEntry( atom, self->atomList ); if (err) goto bail;
	switch( atom->type )
	{
		case MJ2ImageHeaderAtomType:
			if ( self->imageHeaderAtom )
				BAILWITHERROR( ISOBadDataErr );
			self->imageHeaderAtom = atom;
			break;
			
		case MJ2BitsPerComponentAtomType:
			if ( self->bitsPerComponentAtom )
				BAILWITHERROR( ISOBadDataErr );
			self->bitsPerComponentAtom = atom;
			break;
			
		case MJ2ColorSpecificationAtomType:
			if ( self->colorSpecificationAtom )
				BAILWITHERROR( ISOBadDataErr );
			self->colorSpecificationAtom = atom;
			break;
#if 0
		/* the following optional atoms are not yet implemented	 */	
		case MJ2PaletteAtomType:
			if ( self->paletteAtom )
				BAILWITHERROR( ISOBadDataErr );
			self->paletteAtom = atom;
			break;
			
		case MJ2ChannelDefinitionAtomType:
			if ( self->channelDefinitionAtom )
				BAILWITHERROR( ISOBadDataErr );
			self->channelDefinitionAtom = atom;
			break;
			
		case MJ2ResolutionAtomType:
			if ( self->resolutionAtom )
				BAILWITHERROR( ISOBadDataErr );
			self->resolutionAtom = atom;
			break;
#endif
	}
bail:
	TEST_RETURN( err );

	return err;
}

static ISOErr createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
   PARSE_ATOM_LIST(MJ2HeaderAtom)
bail:
   TEST_RETURN( err );

   return err;
}

ISOErr MJ2CreateHeaderAtom( MJ2HeaderAtomPtr *outAtom )
{
	ISOErr err;
	MJ2HeaderAtomPtr self;
	
	self = (MJ2HeaderAtomPtr) calloc( 1, sizeof(MJ2HeaderAtom) );
	TESTMALLOC( self );

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	
	self->type					= MJ2JP2HeaderAtomType;
	self->name					= "JPEG 2000 header atom";
	self->destroy				= destroy;
	self->createFromInputStream = (cisfunc) createFromInputStream;
	err = MP4MakeLinkedList( &self->atomList ); if (err) goto bail;
	self->calculateSize			= calculateSize;
	self->serialize				= serialize;	
	self->addAtom				= addAtom;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}

