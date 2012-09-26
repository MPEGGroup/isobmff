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
	$Id: SampleToChunkAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static u32 getEntryCount( struct MP4SampleToChunkAtom *self )
{
	u32 entryCount = 0;
	MP4GetListEntryCount( self->entryList, &entryCount );
	return entryCount;
}

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	u32 entryCount;
	MP4SampleToChunkAtomPtr self;
	err = MP4NoErr;
	self = (MP4SampleToChunkAtomPtr) s;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	entryCount = getEntryCount( self );
	if ( entryCount )
	{
		u32 i;
		for ( i = 0; i < entryCount; i++ )
		{
			char *p;
			err = MP4GetListEntry( self->entryList, i, &p ); if (err) goto bail;
			if ( p )
				free( p );
		}
	}
	MP4DeleteLinkedList( self->entryList );
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

typedef struct
{
	u32 firstChunk;
	u32 samplesPerChunk;
	u32	sampleDescriptionIndex;
} stscEntry, *stscEntryPtr;


static MP4Err setEntry( struct MP4SampleToChunkAtom *self, u32 firstChunkNumber, u32 sampleCount, u32 sampleDescriptionIndex )
{
	stscEntryPtr p;
	stscEntryPtr last;
	MP4Err err;
	u32 entryCount;
	u8 addnew;
	
	err = MP4NoErr;
	
	entryCount = getEntryCount( self );
	addnew = 1;
	
	if (entryCount > 0)
	{
		err = MP4GetListEntry( self->entryList, entryCount-1, (char**) &last ); if (err) goto bail;
		if ((last->samplesPerChunk == sampleCount) && (last->sampleDescriptionIndex == sampleDescriptionIndex))
			addnew = 0;
	}
	
	if (addnew) {
		p = (stscEntryPtr) calloc( 1, sizeof(stscEntry) );
		TESTMALLOC( p );
		p->firstChunk = firstChunkNumber;
		p->samplesPerChunk = sampleCount;
		p->sampleDescriptionIndex = sampleDescriptionIndex;
		err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err mdatMoved( struct MP4SampleToChunkAtom *self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset,
						 struct MP4SampleDescriptionAtom *stsd, 
	                     struct MP4DataReferenceAtom *dref, struct MP4ChunkOffsetAtom *stco )
{
	MP4Err err;
	u32 i;
	u32 entryCount;
	u32 savedDescIndex;
	u32 savedRefIndex;
	u32 savedSelfContained;
	u32 chunkCount;

	err = MP4NoErr;
	savedDescIndex = 0;
	savedRefIndex = 0;
	savedSelfContained = 0;
	entryCount = getEntryCount( self );
	/* if ( stco->offsets == NULL )
		BAILWITHERROR( MP4InvalidMediaErr ); */
	/* if ( entryCount != stco->entryCount )
		BAILWITHERROR( MP4InvalidMediaErr );
	*/
	chunkCount = stco->entryCount;
	for ( i = 0; i < entryCount; i++ )
	{
		stscEntryPtr p;
		u32 chunks;
		
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err)  goto bail;
		if ( p->sampleDescriptionIndex != savedDescIndex )
		{
			GenericSampleEntryAtomPtr sampleEntry;
			MP4DataEntryAtomPtr dataEntry;
			/* get sample desc */
			savedDescIndex = p->sampleDescriptionIndex;
			err = stsd->getEntry( stsd, p->sampleDescriptionIndex, &sampleEntry ); if (err) goto bail;
			/* get data entry */
			savedRefIndex = sampleEntry->dataReferenceIndex;
			err = dref->getEntry( dref, savedRefIndex, &dataEntry ); if (err) goto bail;
			/* mdat? */
			savedSelfContained = (dataEntry->flags & 1);
		}
		if ( savedSelfContained ) 
		{
			/* find the chunk numbers and add the offset */
			if (i == (entryCount - 1))
				chunks = chunkCount + 1 - p->firstChunk;
			else {
				stscEntryPtr q;
				err = MP4GetListEntry( self->entryList, i+1, (char**) &q ); if (err)  goto bail;
				chunks = q->firstChunk - p->firstChunk;
			}
			/* this is wrong;  the stco pointer may be a 64-bit offset co64
			for (j=p->firstChunk-1; j<(p->firstChunk+chunks-1); j++) {
				if ((stco->offsets[j] >= mdatBase) && (stco->offsets[j] < mdatEnd))
					stco->offsets[j] += mdatOffset;
			}
			*/
			err = stco->mdatMoved( stco, p->firstChunk, p->firstChunk+chunks-1, mdatBase, mdatEnd, mdatOffset );
			if (err) goto bail;
		}
		/* if (( savedSelfContained ) && (stco->offsets[i] >= mdatBase) && (stco->offsets[i] < mdatEnd))
			stco->offsets[i] += mdatOffset; */
	}	
bail:
	TEST_RETURN( err );
	return err;
}


static MP4Err lookupSample( MP4AtomPtr s, u32 sampleNumber,
                            u32 *outChunkNumber, u32 *outSampleDescriptionIndex,
                            u32 *outFirstSampleNumberInChunk )
{
	MP4Err err;
	u32 i;
	u32 entryCount;
	stscEntryPtr p;
	u32 firstChunkInRun;
	u32 firstSampleThisChunkRun;

	MP4SampleToChunkAtomPtr self = (MP4SampleToChunkAtomPtr) s;
	if ( (self == NULL) || (sampleNumber == 0) || (outChunkNumber == NULL) )
		BAILWITHERROR( MP4BadParamErr )

	err = MP4NoErr;
	err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	if ( self->foundEntryFirstSampleNumber && (self->foundEntryFirstSampleNumber < sampleNumber) )
	{
		firstSampleThisChunkRun = self->foundEntryFirstSampleNumber;
		i = self->foundEntryNumber;
	}
	else
	{
		firstSampleThisChunkRun = 1;
		i = 0;
	}
	
	*outChunkNumber = 0;
	*outSampleDescriptionIndex = 0;
	for ( ; i < entryCount; i++ )
	{
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err)  goto bail;
		firstChunkInRun = p->firstChunk;
		if ( i == entryCount - 1 )
		{
			/* the last run -- calculate based on spc */
			u32 sampleOffsetInThisChunk;
			sampleOffsetInThisChunk = (sampleNumber - firstSampleThisChunkRun);
			*outChunkNumber = firstChunkInRun + (sampleOffsetInThisChunk / p->samplesPerChunk);
			*outSampleDescriptionIndex = p->sampleDescriptionIndex;
			*outFirstSampleNumberInChunk = ((*outChunkNumber - firstChunkInRun) * p->samplesPerChunk) + firstSampleThisChunkRun;
			self->foundEntryNumber = i;
		}
		else
		{
			u32 nextChunk;
			u32 samplesThisChunkRun;
			stscEntryPtr nextEntryP;
			err = MP4GetListEntry( self->entryList, i + 1, (char**) &nextEntryP ); if (err) goto bail;
			nextChunk = nextEntryP->firstChunk;
			samplesThisChunkRun = (nextChunk - firstChunkInRun) * p->samplesPerChunk;
			if ( sampleNumber <= (firstSampleThisChunkRun + samplesThisChunkRun) )
			{
				u32 sampleOffsetInThisChunk;
				sampleOffsetInThisChunk = (sampleNumber - firstSampleThisChunkRun);
				*outChunkNumber = firstChunkInRun + (sampleOffsetInThisChunk / p->samplesPerChunk);
				*outSampleDescriptionIndex = p->sampleDescriptionIndex;
				*outFirstSampleNumberInChunk = ((*outChunkNumber - firstChunkInRun) * p->samplesPerChunk) + firstSampleThisChunkRun;
				self->foundEntryNumber = i;
				break;
			}
			else
			{
				firstSampleThisChunkRun += samplesThisChunkRun;
			}
		}
	}
	self->foundEntryFirstSampleNumber = firstSampleThisChunkRun;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;	
	u32 entryCount;
	MP4SampleToChunkAtomPtr self = (MP4SampleToChunkAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;	
    entryCount = getEntryCount( self );
	PUT32_V( entryCount );
	for ( i = 0; i < entryCount; i++ )
	{
		stscEntryPtr p;
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err) goto bail;
		PUT32_V( p->firstChunk );
		PUT32_V( p->samplesPerChunk );
		PUT32_V( p->sampleDescriptionIndex );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4SampleToChunkAtomPtr self = (MP4SampleToChunkAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += 4 + (12 * getEntryCount(self));
bail:
	TEST_RETURN( err );

	return err;
}


static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 i;
	u32 entryCount;
	MP4SampleToChunkAtomPtr self = (MP4SampleToChunkAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	
	GET32_V( entryCount );
	
	for ( i = 0; i < entryCount; i++ )
	{
		u32 firstChunk;
		u32 samplesPerChunk;
		u32 sampleDescriptionIndex;

		GET32_V( firstChunk );
		GET32_V( samplesPerChunk );
		GET32_V( sampleDescriptionIndex );
		err = setEntry( self, firstChunk, samplesPerChunk, sampleDescriptionIndex ); if (err) goto bail;
	}
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateSampleToChunkAtom( MP4SampleToChunkAtomPtr *outAtom )
{
	MP4Err err;
	MP4SampleToChunkAtomPtr self;
	
	self = (MP4SampleToChunkAtomPtr) calloc( 1, sizeof(MP4SampleToChunkAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	err = MP4MakeLinkedList( &self->entryList ); if (err) goto bail;
	self->type = MP4SampleToChunkAtomType;
	self->name                = "sample to chunk";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->lookupSample        = lookupSample;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->setEntry = setEntry;
	self->getEntryCount = getEntryCount;
	self->mdatMoved = mdatMoved;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
