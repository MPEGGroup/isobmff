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
	$Id: DataInformationAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4DataInformationAtomPtr self;
	u32 i;
	err = MP4NoErr;
	self = (MP4DataInformationAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	DESTROY_ATOM_LIST
	
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err getOffset( struct MP4DataInformationAtom *self, u32 dataReferenceIndex, u64 *outOffset )
{
	MP4DataReferenceAtomPtr dref;
	MP4Err err;
	
	err = MP4NoErr;
	if ( self->dataReference == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	if ( dataReferenceIndex == 0 )
		BAILWITHERROR( MP4BadParamErr );
	dref = (MP4DataReferenceAtomPtr) self->dataReference;
	err = dref->getOffset( dref, dataReferenceIndex, outOffset ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSampleReference( struct MP4DataInformationAtom *self, u32 sampleCount, u32 dataReferenceIndex, u64 dataOffset, MP4Handle sizesH )
{
	MP4DataReferenceAtomPtr dref;
	MP4Err err;
	
	err = MP4NoErr;
	if ( self->dataReference == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	if ( dataReferenceIndex == 0 )
		BAILWITHERROR( MP4BadParamErr );
	dref = (MP4DataReferenceAtomPtr) self->dataReference;
	err = dref->addSampleReference( dref, sampleCount, dataReferenceIndex, dataOffset, sizesH ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;

}

static MP4Err addSamples( struct MP4DataInformationAtom *self, u32 sampleCount, u32 dataReferenceIndex, MP4Handle sampleH )
{
	MP4DataReferenceAtomPtr dref;
	MP4Err err;
	
	err = MP4NoErr;
	if ( self->dataReference == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	if ( dataReferenceIndex == 0 )
		BAILWITHERROR( MP4BadParamErr );
	dref = (MP4DataReferenceAtomPtr) self->dataReference;
	err = dref->addSamples( dref, sampleCount, dataReferenceIndex, sampleH ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;
}


static MP4Err addAtom( MP4DataInformationAtomPtr self, MP4AtomPtr atom )
{
	MP4Err err;
	err = MP4NoErr;
	err = MP4AddListEntry( atom, self->atomList ); if (err) goto bail;
	switch( atom->type )
	{
		case MP4DataReferenceAtomType:
			if ( self->dataReference )
				BAILWITHERROR( MP4BadDataErr )
			self->dataReference = atom;
			break;			
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	MP4DataInformationAtomPtr self = (MP4DataInformationAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    SERIALIZE_ATOM_LIST( atomList );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4DataInformationAtomPtr self = (MP4DataInformationAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	ADD_ATOM_LIST_SIZE( atomList );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	PARSE_ATOM_LIST(MP4DataInformationAtom)
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateDataInformationAtom( MP4DataInformationAtomPtr *outAtom )
{
	MP4Err err;
	MP4DataInformationAtomPtr self;
	
	self = (MP4DataInformationAtomPtr) calloc( 1, sizeof(MP4DataInformationAtom) );
	TESTMALLOC( self );

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4DataInformationAtomType;
	self->name                = "data information";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	err = MP4MakeLinkedList( &self->atomList ); if (err) goto bail;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->getOffset             = getOffset;
	self->addSamples            = addSamples;
	self->addAtom = addAtom;
	self->addSampleReference = addSampleReference;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
