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
	$Id: SyncSampleAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

#define allocation_size 1024

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4SyncSampleAtomPtr self;
	err = MP4NoErr;
	self = (MP4SyncSampleAtomPtr) s;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	if ( self->sampleNumbers )
	{
		free( self->sampleNumbers );
		self->sampleNumbers = NULL;
	}
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err isSyncSample( MP4AtomPtr s, u32 sampleNumber, u32 *outSync )
{
	MP4Err err;
	u32 i;
	u32 *p;
	MP4SyncSampleAtomPtr self = (MP4SyncSampleAtomPtr) s;
	
	err = MP4NoErr;
	p = self->sampleNumbers;
	if ( (self == NULL) || (outSync == NULL) || (sampleNumber == 0) )
    {
		BAILWITHERROR( MP4BadParamErr );
    }
	*outSync = 0;
	for ( i = 0; i < self->entryCount; i++, p++ )
	{
		u32 syncSample;
		syncSample = *p;
		if ( sampleNumber == syncSample )
		{
			*outSync = 1;
			break;
		}
		else if ( sampleNumber < syncSample )
		{
			break;
		}
	}
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSamples( struct MP4SyncSampleAtom *self, u32 beginningSampleNumber,
                          u32 sampleCount, MP4Handle syncSamplesH )
{
	MP4Err err;
	u32 newSampleCount, newSize;
	u32 *src;
	u32 *dst;
	u32 i;
	u32 handleSize;
	u32 samplesInHandle;
    u32 firstSyncSampleCount = 0;
		
	if ( (self == NULL) || (syncSamplesH == NULL) || (beginningSampleNumber == 0) )
    {
		BAILWITHERROR( MP4BadParamErr );
    }
	err = MP4GetHandleSize( syncSamplesH, &handleSize ); if (err) goto bail;
	samplesInHandle = handleSize / sizeof(u32);


    if ( self->entryCount == 0 )
    {
       /*
         If this is an empty atom and we are now being called it
         can mean:

         a) All samples were sync until this one, and we need to
            make sync sample entries for them all right now, or

         b) All samples were non-sync -- and we either got another
            non-sync (which doesn't change our state) or we finally
            got a sync sample and we can set an entry count containing
            just this sample.
        */
/* GUIDO 25/07/00 Case a) applies independently of samplesInHandle */
/*     if ( (samplesInHandle == 0) && (self->nonSyncFlag == 0) ) */
       if ( self->nonSyncFlag == 0)
       {
          /* first non-sync -- set all prior to sync */
          firstSyncSampleCount = (beginningSampleNumber - 1);
          newSampleCount = samplesInHandle + firstSyncSampleCount;
          self->nonSyncFlag = 1;
       }
       else
       {
          self->nonSyncFlag = (samplesInHandle == 0);
          newSampleCount = samplesInHandle;
       }
    }
    else
    {
       newSampleCount = samplesInHandle + self->entryCount;
    }
	newSize = newSampleCount * sizeof(u32);
	if (newSize > self->allocatedSize) {
		self->allocatedSize += allocation_size;
		if (self->allocatedSize < newSize) self->allocatedSize = newSize;
		
		if (self->sampleNumbers != NULL) 
			self->sampleNumbers = (u32*) realloc( self->sampleNumbers, self->allocatedSize );
			else
			self->sampleNumbers = (u32*) calloc( self->allocatedSize, 1 );
		TESTMALLOC( self->sampleNumbers );
	}
	src = (u32*) *syncSamplesH;
	dst = &self->sampleNumbers[self->entryCount];
    for ( i = 0; i < firstSyncSampleCount; i++ )
    {
       *dst++ = (i + 1);
    }
	for ( i = 0; i < samplesInHandle; i++ )
	{
		u32 sampleOffset;
		sampleOffset = *src++;
        if (( sampleOffset == 0 ) || (sampleOffset > sampleCount))
        {
           BAILWITHERROR( MP4BadParamErr );
        }
		*dst++ = beginningSampleNumber + (sampleOffset - 1);
	}
	self->entryCount = newSampleCount;
bail:
	TEST_RETURN( err );

	return err;	
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;
	MP4SyncSampleAtomPtr self = (MP4SyncSampleAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	PUT32( entryCount );
	for ( i = 0; i < self->entryCount; i++ )
	{
		PUT32_V( self->sampleNumbers[i] );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4SyncSampleAtomPtr self = (MP4SyncSampleAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += 4 + (4 * self->entryCount);
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 entries;
	u32* p;
	MP4SyncSampleAtomPtr self = (MP4SyncSampleAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GET32( entryCount );

	self->sampleNumbers = (u32 *) calloc( self->entryCount + 1, sizeof(u32) );
	self->allocatedSize = (self->entryCount + 1) * sizeof(u32);
	TESTMALLOC( self->sampleNumbers )
	for ( entries = 0, p = self->sampleNumbers; entries < self->entryCount; entries++, p++ )
	{
		u32 sampleNumber;
		GET32_V( sampleNumber );
		*p = sampleNumber;
	}
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateSyncSampleAtom( MP4SyncSampleAtomPtr *outAtom )
{
	MP4Err err;
	MP4SyncSampleAtomPtr self;
	
	self = (MP4SyncSampleAtomPtr) calloc( 1, sizeof(MP4SyncSampleAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );	if ( err ) goto bail;
	self->type = MP4SyncSampleAtomType;
	self->name                = "sync sample";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->isSyncSample        = isSyncSample;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->addSamples = addSamples;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
