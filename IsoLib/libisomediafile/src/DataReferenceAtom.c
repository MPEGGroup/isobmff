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
	$Id: DataReferenceAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4DataReferenceAtomPtr self;
	u32 i;
	err = MP4NoErr;
	self = (MP4DataReferenceAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	DESTROY_ATOM_LIST
	
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static u32 getEntryCount( struct MP4DataReferenceAtom *self )
{
	u32 entryCount = 0;
	MP4GetListEntryCount( self->atomList, &entryCount );
	return entryCount;
}

static MP4Err getEntry( struct MP4DataReferenceAtom *self, u32 dataReferenceIndex, struct MP4DataEntryAtom **outEntry )
{
	MP4DataEntryAtomPtr entry;
	MP4Err err;
	
	err = MP4NoErr;
	if ( (dataReferenceIndex < 1) || (outEntry == NULL) || (dataReferenceIndex > getEntryCount(self)) )
		BAILWITHERROR( MP4BadParamErr )
	err = MP4GetListEntry( self->atomList, dataReferenceIndex - 1, (char**) &entry ); if (err) goto bail;
	*outEntry = entry;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getOffset( struct MP4DataReferenceAtom *self, u32 dataReferenceIndex, u64 *outOffset )
{
	MP4DataEntryAtomPtr entry;
	MP4Err err;
	
	err = MP4NoErr;
	if ( (dataReferenceIndex < 1) || (outOffset == NULL) || (dataReferenceIndex > getEntryCount(self)) )
		BAILWITHERROR( MP4BadParamErr )
	err = MP4GetListEntry( self->atomList, dataReferenceIndex - 1, (char**) &entry ); if (err) goto bail;
	err = entry->getOffset( entry, outOffset ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSampleReference( struct MP4DataReferenceAtom *self, u32 sampleCount, u32 dataReferenceIndex, u64 dataOffset, MP4Handle sizesH )
{
	MP4DataEntryAtomPtr entry;
	MP4Err err;
	
	err = MP4NoErr;
	if ( (dataReferenceIndex < 1) || (sampleCount == 0) || (dataReferenceIndex > getEntryCount(self)) )
		BAILWITHERROR( MP4BadParamErr )
	err = MP4GetListEntry( self->atomList, dataReferenceIndex - 1, (char**) &entry ); if (err) goto bail;
	err = entry->addSampleReference( entry, dataOffset, sizesH ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;	
}

static MP4Err addSamples( struct MP4DataReferenceAtom *self, u32 sampleCount, u32 dataReferenceIndex, MP4Handle sampleH )
{
	MP4DataEntryAtomPtr entry;
	MP4Err err;
	
	err = MP4NoErr;
	if ( (dataReferenceIndex < 1) || (sampleCount == 0) || (dataReferenceIndex > getEntryCount(self)) )
		BAILWITHERROR( MP4BadParamErr )
	err = MP4GetListEntry( self->atomList, dataReferenceIndex - 1, (char**) &entry ); if (err) goto bail;
	err = entry->addSamples( entry, sampleH ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 entryCount;
	MP4DataReferenceAtomPtr self = (MP4DataReferenceAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;	
    entryCount = getEntryCount( self );
	PUT32_V( entryCount );
	{
		SERIALIZE_ATOM_LIST( atomList );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4DataReferenceAtomPtr self = (MP4DataReferenceAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += 4;
	{
		ADD_ATOM_LIST_SIZE( atomList );
	}
bail:
	TEST_RETURN( err );

	return err;
}

IMPLEMENT_NEW_ADDATOM( MP4DataReferenceAtom )

static MP4Err addDataEntry( struct MP4DataReferenceAtom *self, MP4AtomPtr entry )
{
	MP4Err err;
	err = addAtom( self, entry ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 entryCount;
	u32 i;
	MP4DataReferenceAtomPtr self = (MP4DataReferenceAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	
	GET32_V( entryCount );
	for ( i = 0; i < entryCount; i++ )
	{
		MP4AtomPtr atom;
		err = MP4ParseAtom( inputStream, &atom ); if (err) goto bail;
		self->bytesRead += atom->size;
		err = addAtom( self, atom ); if (err) goto bail;
	}
	if ( self->bytesRead != self->size )
		BAILWITHERROR( MP4BadDataErr )

bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateDataReferenceAtom( MP4DataReferenceAtomPtr *outAtom )
{
	MP4Err err;
	MP4DataReferenceAtomPtr self;
	
	self = (MP4DataReferenceAtomPtr) calloc( 1, sizeof(MP4DataReferenceAtom) );
	if ( self == NULL )
		BAILWITHERROR( MP4NoMemoryErr )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	
	err = MP4MakeLinkedList( &self->atomList ); if (err) goto bail;
	self->type = MP4DataReferenceAtomType;
	self->name = "data reference";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->addDataEntry = addDataEntry;
	self->destroy = destroy;
	self->calculateSize = calculateSize;
	self->serialize = serialize;
	self->getOffset = getOffset;
	self->addSamples = addSamples;
	self->getEntryCount = getEntryCount;
	self->getEntry = getEntry;
	self->addSampleReference = addSampleReference;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
