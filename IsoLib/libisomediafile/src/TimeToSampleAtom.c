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
	$Id: TimeToSampleAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

typedef struct
{
	u32 sampleCount;
	u32 sampleDuration;		/* also known as sampleDelta */
} sttsEntry, *sttsEntryPtr;

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4TimeToSampleAtomPtr self;
	err = MP4NoErr;
	self = (MP4TimeToSampleAtomPtr) s;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	if ( self->entryList )
	{
		u32 entryCount;
		u32 i;
		err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
		for ( i = 0; i < entryCount; i++ )
		{
			sttsEntryPtr pe;
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

static MP4Err getTotalDuration( struct MP4TimeToSampleAtom *self, u32 *outDuration )
{
	MP4Err err;
	u32 totalDuration;
	u32 entryCount;
	u32 i;
	err = MP4NoErr;
	
	err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	totalDuration = 0;
	for ( i = 0; i < entryCount; i++ )
	{
		sttsEntryPtr pe;
		err = MP4GetListEntry( self->entryList, i, (char**) &pe ); if (err) goto bail;
		totalDuration += (pe->sampleCount * pe->sampleDuration);	
	}
	*outDuration = totalDuration;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSample( struct MP4TimeToSampleAtom *self, u32 duration )
{
	MP4Err err;
	sttsEntryPtr current;
	
	err = MP4NoErr;
	current = (sttsEntryPtr) self->currentEntry;
	if ( (current == NULL) || (current->sampleDuration != duration) )
	{
		current = (sttsEntryPtr) calloc( 1, sizeof(sttsEntry) );
		TESTMALLOC( current );
		current->sampleCount = 1;
		current->sampleDuration = duration;
		err = MP4AddListEntry( current, self->entryList ); if (err) goto bail;
		self->currentEntry = current;
	}
	else
	{
		current->sampleCount++;
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSamples( struct MP4TimeToSampleAtom *self, u32 sampleCount, MP4Handle durationsH )
{
	MP4Err err;
	u32 hasDurations;
	u32 *durations;
	u32 i;
	
	err = MP4NoErr;
	durations = (u32 *) *durationsH;
	err = MP4GetHandleSize( durationsH, &hasDurations ); if (err) goto bail;
	hasDurations = hasDurations > sizeof(u32) ? 1 : 0;
	for ( i = 0; i < sampleCount; i++ )
	{
		u32 duration;
		duration = hasDurations ? durations[i] : *durations;
		err = addSample( self, duration ); if (err) goto bail;
	}
	err = MP4NoErr;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getTimeForSampleNumber( MP4AtomPtr s, u32 sampleNumber, u64 *outSampleCTS, s32 *outSampleDuration )
{
	MP4Err err;
	u32 i;
	sttsEntryPtr pe;
	u64 entryTime;
	u32 entrySampleNumber;
	u64 sampleCTS;
	s32 sampleDuration;
	u32 entryCount;
	
	MP4TimeToSampleAtomPtr self = (MP4TimeToSampleAtomPtr) s;
	if ( self == NULL ) BAILWITHERROR( MP4BadParamErr )
	err = MP4NoErr;
	err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;

	if ( self->foundEntry && (self->foundEntrySampleNumber < sampleNumber) )
	{
		entryTime = self->foundEntryTime;
		entrySampleNumber = self->foundEntrySampleNumber;
		i = self->foundEntryNumber;
	}
	else
	{
		entryTime = 0;
		entrySampleNumber = 1;
		i = 0;
	}
	sampleCTS = -1;
	sampleDuration = 0;
	
	for ( ; i < entryCount; i++ )
	{
		err = MP4GetListEntry( self->entryList, i, (char**) &pe ); if (err) goto bail;
#if 0
		if ( (entrySampleNumber + pe->sampleCount) >= sampleNumber )
#else
		if ( (entrySampleNumber + pe->sampleCount) > sampleNumber )
#endif
		{
			/* this is the desired entry */
			u64 sampleOffset;
			sampleOffset = sampleNumber - entrySampleNumber;
			sampleCTS = entryTime + ((sampleNumber - entrySampleNumber) * pe->sampleDuration);
			sampleDuration = pe->sampleDuration;
			break;
		}
		else
		{
			/* go to next entry */
			entrySampleNumber += pe->sampleCount;
			entryTime += (pe->sampleCount * pe->sampleDuration);
		}
	}
	if ( outSampleCTS )
		*outSampleCTS = sampleCTS;
	if ( outSampleDuration )
		*outSampleDuration = sampleDuration;
	self->foundEntry = pe;
	self->foundEntryTime = entryTime;
	self->foundEntrySampleNumber = entrySampleNumber;
	self->foundEntryNumber = i;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err findSamples( MP4AtomPtr s, u64 desiredTime, s64 *outPriorSample, s64 *outExactSample, s64 *outNextSample,
						  u32 *outSampleNumber, s32 *outSampleDuration )
{
	MP4Err err;
	u32 entryCount;
	u32 i;

	sttsEntryPtr pe;
	u64 entryTime;
	u32 sampleNumber;
	int foundSample;

	MP4TimeToSampleAtomPtr self = (MP4TimeToSampleAtomPtr) s;
	if ( self == NULL ) BAILWITHERROR( MP4BadParamErr )
	err = MP4NoErr;
	err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	entryTime = 0;
	*outPriorSample  = -1;
	*outExactSample  = -1;
	*outNextSample   = -1;
	*outSampleNumber = 0;
	sampleNumber = 1;
	foundSample = 0;
	
	for ( i = 0; i < entryCount; i++ )
	{
		s64 entryDuration;
		u64 finalTime;
		err = MP4GetListEntry( self->entryList, i, (char**) &pe ); if (err) goto bail;
		entryDuration = pe->sampleDuration * pe->sampleCount;
		finalTime = entryTime + entryDuration;

		if ( entryTime == desiredTime )
		{
			*outExactSample  = entryTime;
			*outSampleNumber = sampleNumber;
			*outSampleDuration = pe->sampleDuration;
			foundSample = 1;
			if ( entryTime + pe->sampleDuration <= finalTime )
			{
				*outNextSample  = entryTime + pe->sampleDuration;
				break;
			}
		}
		else if ( entryTime < desiredTime )
		{
			*outPriorSample = entryTime;
		}
		else if ( entryTime > desiredTime )
		{
			*outNextSample = entryTime;
			break;
		}
		if ( finalTime < desiredTime )
		{
			*outPriorSample = finalTime;
		}
		else if ( finalTime == desiredTime )
		{
			*outPriorSample = finalTime - pe->sampleDuration;
			*outExactSample = finalTime;
			*outSampleNumber = sampleNumber + pe->sampleCount;
			*outSampleDuration = pe->sampleDuration;
			foundSample = 1;
		}
		else if ( finalTime > desiredTime )
		{
			u32 bestSample;
			u64 nextTime;
			s64 timeDifference;
			
			/* next line was: timeDifference = finalTime - desiredTime; corrected by Per Frojdh */
			timeDifference = desiredTime - entryTime;
			
			bestSample = (u32) (timeDifference / pe->sampleDuration);
			if ( foundSample == 0 )
			{
				*outSampleNumber = sampleNumber + bestSample;
				*outSampleDuration = pe->sampleDuration;
				*outPriorSample = entryTime + (bestSample * pe->sampleDuration);
				foundSample = 1;
			}
			nextTime = entryTime + (bestSample * pe->sampleDuration) + pe->sampleDuration;
			if ( nextTime <= finalTime )
			{
				*outNextSample = nextTime;
				break;
			}
		}
		entryTime    += entryDuration;
		sampleNumber += pe->sampleCount;
	}
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;
	sttsEntryPtr pe;
	u32 entryCount;
	MP4TimeToSampleAtomPtr self = (MP4TimeToSampleAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	PUT32_V( entryCount );

	for ( i = 0; i < entryCount; i++ )
	{
		err = MP4GetListEntry( self->entryList, i, (char**) &pe ); if (err) goto bail;
		PUT32_V( pe->sampleCount );
		PUT32_V( pe->sampleDuration );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4TimeToSampleAtomPtr self = (MP4TimeToSampleAtomPtr) s;
	u32 entryCount;
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
	u32 i;
	u32 entryCount;
	MP4TimeToSampleAtomPtr self = (MP4TimeToSampleAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	
	GET32_V( entryCount );
	for ( i = 0; i < entryCount; i++ )
	{
		u32 count;
		s32 duration;
		sttsEntryPtr pe;
		pe = (sttsEntryPtr) calloc( 1, sizeof(sttsEntry) );
		TESTMALLOC( pe );

		GET32_V( count );
		GET32_V( duration );
		pe->sampleCount    = count;
		pe->sampleDuration = duration;
		err = MP4AddListEntry( pe, self->entryList ); if (err) goto bail;
		self->currentEntry = pe;
	}
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateTimeToSampleAtom( MP4TimeToSampleAtomPtr *outAtom )
{
	MP4Err err;
	MP4TimeToSampleAtomPtr self;
	
	self = (MP4TimeToSampleAtomPtr) calloc( 1, sizeof(MP4TimeToSampleAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	
	err = MP4MakeLinkedList( &self->entryList ); if (err) goto bail;
	self->type = MP4TimeToSampleAtomType;
	self->name                = "time to sample";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->findSamples         = findSamples;
	self->getTimeForSampleNumber = getTimeForSampleNumber;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->getTotalDuration = getTotalDuration;
	self->addSamples = addSamples;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
