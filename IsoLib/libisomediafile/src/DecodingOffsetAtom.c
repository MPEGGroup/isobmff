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
	$Id: DecodingOffsetAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

typedef struct
{
	u32 sampleCount;
	u32 decodingOffset;
} cttsEntry, *cttsEntryPtr;


static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4CompositionOffsetAtomPtr self;
	err = MP4NoErr;
	self = (MP4CompositionOffsetAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	if ( self->entryList )
	{
		u32 entryCount;
		u32 i;
		err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
		for ( i = 0; i < entryCount; i++ )
		{
			cttsEntryPtr pe;
			err = MP4GetListEntry( self->entryList, i, (char**) &pe ); if (err) goto bail;
			free( pe );	
		}
		err = MP4DeleteLinkedList( self->entryList ); if (err) goto bail;
		self->entryList = NULL;
	}
	
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err getOffsetForSampleNumber( MP4AtomPtr s, u32 sampleNumber, s32 *outOffset )
{
	MP4Err       err;
	u32          i;
	u32 entryCount;
	u32          currentSampleNumberOffset;

	MP4CompositionOffsetAtomPtr self = (MP4CompositionOffsetAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	
	err = MP4BadParamErr;
	currentSampleNumberOffset = 0;

	err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;	
	for ( i = 0; i < entryCount; i++ )
	{
		cttsEntryPtr p;
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err) goto bail;
		if ( currentSampleNumberOffset + p->sampleCount >= sampleNumber )
		{
			*outOffset = p->decodingOffset;
			err = MP4NoErr;
			break;
		}
		else
			currentSampleNumberOffset += p->sampleCount;
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSample( struct MP4CompositionOffsetAtom *self, u32 offset )
{
	MP4Err err;
	cttsEntryPtr current;
	
	err = MP4NoErr;
	current = (cttsEntryPtr) self->currentEntry;
	if ( (current == NULL) || (current->decodingOffset != offset) )
	{
		current = (cttsEntryPtr) calloc( 1, sizeof(cttsEntry) );
		TESTMALLOC( current );
		current->sampleCount = 1;
		current->decodingOffset = offset;
		err = MP4AddListEntry( current, self->entryList ); if (err) goto bail;
		self->currentEntry = current;
	}
	else
	{
		current->sampleCount++;
	}
	self->finalSampleNumber++;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSamples(struct MP4CompositionOffsetAtom *self, u32 sampleNumber, u32 sampleCount, MP4Handle offsetsH )
{
	MP4Err err;
	u32 i;
	u32 entryCount;
	u32 offsetCount;
	u32 onlyOffset;


	err = MP4NoErr;
    err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	if ( self->finalSampleNumber )
	{
		s32 skipSamples;
		if ( self->finalSampleNumber >= sampleNumber )
			BAILWITHERROR( MP4BadParamErr );
		skipSamples = sampleNumber - self->finalSampleNumber - 1;

		if ( skipSamples > 0 )
		{
			for ( i = 0; i < (u32) skipSamples; i++ )
			{
				err = addSample( self, 0 ); if (err) goto bail;
			}
		}
	}
	offsetCount = 0;
	onlyOffset  = 0;
	if ( offsetsH )
	{
		err = MP4GetHandleSize( offsetsH, &offsetCount ); if (err) goto bail;
		offsetCount /= sizeof(u32);
		if ( offsetCount == 1 )
			onlyOffset = *(u32*) *offsetsH;
		else if ( offsetCount != sampleCount )
			BAILWITHERROR( MP4BadParamErr );
	}
	if ( sampleCount != offsetCount )
	{
		for ( i = 0; i < sampleCount; i++ )
		{
			err = addSample( self, onlyOffset ); if (err) goto bail;
		}
	}
	else
	{
		u32 *offset;
		offset = (u32*) *offsetsH;
		for ( i = 0; i < sampleCount; i++ )
		{
			err = addSample( self, *offset++ ); if (err) goto bail;
		}
	}
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;

	u32 entryCount;
	MP4CompositionOffsetAtomPtr self = (MP4CompositionOffsetAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	PUT32_V( entryCount );

	for ( i = 0; i < entryCount; i++ )
	{
		cttsEntryPtr p;
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err) goto bail;
		PUT32_V( p->sampleCount );
		PUT32_V( p->decodingOffset );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	u32 entryCount;
	MP4CompositionOffsetAtomPtr self = (MP4CompositionOffsetAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	self->size += 4 + (8 * entryCount);
bail:
	TEST_RETURN( err );

	return err;
}


static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 entries;
	u32 entryCount;
	MP4CompositionOffsetAtomPtr self = (MP4CompositionOffsetAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	
	GET32_V( entryCount );
	for ( entries = 0; entries < entryCount; entries++ )
	{
		u32 count;
		s32 decodingOffset;
		cttsEntryPtr p;

		p = (cttsEntryPtr) calloc( 1, sizeof(cttsEntry) );
		TESTMALLOC( p );
		GET32_V( count );
		GET32_V( decodingOffset );
		p->sampleCount    = count;
		p->decodingOffset = decodingOffset;
		err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
	}
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateCompositionOffsetAtom( MP4CompositionOffsetAtomPtr *outAtom )
{
	MP4Err err;
	MP4CompositionOffsetAtomPtr self;
	
	self = (MP4CompositionOffsetAtomPtr) calloc( 1, sizeof(MP4CompositionOffsetAtom) );
	TESTMALLOC( self );

	err = MP4CreateFullAtom( (MP4AtomPtr) self ); if ( err ) goto bail;
	err = MP4MakeLinkedList( &self->entryList ); if (err) goto bail;
	self->type = MP4CompositionOffsetAtomType;
	self->name                = "decoding offset";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->getOffsetForSampleNumber = getOffsetForSampleNumber;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->addSamples = addSamples;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
