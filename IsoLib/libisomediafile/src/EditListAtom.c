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
	$Id: EditListAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

#define USE_OLD_FILES_WORKAROUND 1

typedef struct
{
	u64 segmentDuration;
	s64 mediaTime;
	u32	mediaRate;
	u32 emptyEdit;
} edtsEntry, *edtsEntryPtr;

static u32 getEntryCount( struct MP4EditListAtom *self )
{
	u32 count = 0;
	MP4GetListEntryCount( self->entryList, &count );
	return count;
}

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4EditListAtomPtr self;
	u32 entryCount;
	u32 i;

	err = MP4NoErr;
	self = (MP4EditListAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	entryCount = getEntryCount( self );
	for ( i = 0; i < entryCount; i++ )
	{
		char *p;
		err = MP4GetListEntry( self->entryList, i, &p ); if (err) goto bail;
		if ( p )
			free( p );
	}
	MP4DeleteLinkedList( self->entryList );
	self->entryList = NULL;
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err getTrackOffset( struct MP4EditListAtom *self, u32 *outTrackStartTime )
{
	MP4Err err;
	u32 entryCount;
	edtsEntryPtr p;
	
	err = MP4NoErr;
	p = NULL;
	
	err = MP4GetListEntryCount( self->entryList, &entryCount );
	if ( entryCount == 0 )
	{
       *outTrackStartTime = 0;
	}
	else
	{
		edtsEntryPtr firstSegment;
		
		err = MP4GetListEntry( self->entryList, 0, (char**) &firstSegment ); if (err) goto bail;
		if ( firstSegment->emptyEdit )
		{
			/* existing track offset  */
			*outTrackStartTime = (u32)firstSegment->segmentDuration;
		}
		else
		{
           *outTrackStartTime = 0;
		}
	}
bail:
	return err;
}

static MP4Err setTrackOffset( struct MP4EditListAtom *self, u32 trackStartTime, u64 trackDuration )
{
	MP4Err err;
	u32 entryCount;
	edtsEntryPtr p;
	
	err = MP4NoErr;
	p = NULL;
	
	err = MP4GetListEntryCount( self->entryList, &entryCount );
	if ( entryCount == 0 )
	{
		p = (edtsEntryPtr) calloc( 1, sizeof(edtsEntry) );
		TESTMALLOC( p );
		p->segmentDuration = trackStartTime;
		p->mediaTime = -1;
		p->mediaRate = 1;
		p->emptyEdit = 1;
		err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
		
		p = (edtsEntryPtr) calloc( 1, sizeof(edtsEntry) );
		TESTMALLOC( p );
		p->segmentDuration = trackDuration;
		p->mediaTime = 0;
		p->mediaRate = 1;
		p->emptyEdit = 0;
		err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
	}
	else
	{
		edtsEntryPtr firstSegment;
		
		err = MP4GetListEntry( self->entryList, 0, (char**) &firstSegment ); if (err) goto bail;
		if ( firstSegment->emptyEdit )
		{
			/* existing track offset -- resize */
			firstSegment->segmentDuration = trackStartTime;
		}
		else
		{
			/* prepend an empty edit */
			p = (edtsEntryPtr) calloc( 1, sizeof(edtsEntry) );
			TESTMALLOC( p );
			p->segmentDuration = trackStartTime;
			p->mediaTime = -1;
			p->mediaRate = 1;
			p->emptyEdit = 1;
			err = MP4PrependListEntry( self->entryList, p ); if (err) goto bail;
		}
	}
bail:
	return err;
}

static MP4Err insertSegment( struct MP4EditListAtom *self, s32 trackStartTime, s32 mediaStartTime, u64 segmentDuration, u32 mediaRate )
{
	MP4Err err;
	u32 entryCount;
	edtsEntryPtr p;
	
	err = MP4NoErr;
	p = NULL;
	
	err = MP4GetListEntryCount( self->entryList, &entryCount );
	if ( entryCount == 0 )
	{
		if ( trackStartTime > 0 )
		{
			/* err = setTrackOffset( self, trackStartTime, segmentDuration ); if (err) goto bail; */
			/* No, that makes two segments and we only want one */
			p = (edtsEntryPtr) calloc( 1, sizeof(edtsEntry) );
			TESTMALLOC( p );
			p->segmentDuration = trackStartTime;
			p->mediaTime = -1;
			p->mediaRate = 1;
			p->emptyEdit = 1;
			err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
		}
		/* this used to be commented out, but without explanation.  we need it now.  dws */
		p = (edtsEntryPtr) calloc( 1, sizeof(edtsEntry) );
		TESTMALLOC( p );
		p->segmentDuration = segmentDuration;
		p->mediaTime = mediaStartTime;
		p->mediaRate = mediaRate;
		err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
		/* to here */
	}
	else
	{
		if ( trackStartTime == -1 )
		{
			/* append this segment */
			p = (edtsEntryPtr) calloc( 1, sizeof(edtsEntry) );
			TESTMALLOC( p );
			p->segmentDuration = segmentDuration;
			p->mediaTime = mediaStartTime;
			p->mediaRate = mediaRate;
			err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
		}
		else
		{
			/* insert this segment */
			/* !!! complete this -- insert an edit into an existing list... !!! */
			BAILWITHERROR( MP4NotImplementedErr );
		}
	}
bail:
	return err;
}

static MP4Err getEffectiveDuration( struct MP4EditListAtom *self, u32 *outDuration )
{
	u32 duration;
	u32 entryCount;
	u32 i;
	MP4Err err;
	
	/* This function returns 0 if there are no edits at all, which means you MUST get the media duration */

	err = MP4NoErr;
	entryCount = getEntryCount( self );
	duration = 0;
	for ( i = 0; i < entryCount; i++ )
	{
		edtsEntryPtr p;
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err) goto bail;
		duration += (u32) p->segmentDuration;
	}
	*outDuration = duration;
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err isEmptyEdit( struct MP4EditListAtom *self, u32 segmentNumber, u32 *outIsEmpty )
{
	MP4Err err;
	u32 entryCount;
	edtsEntryPtr p;
	
	err = MP4NoErr;
	err = MP4GetListEntryCount( self->entryList, &entryCount ); if (err) goto bail;
	
	if (outIsEmpty == NULL) BAILWITHERROR( MP4BadParamErr );
	
	if ((segmentNumber == 1) && (entryCount == 0))
		*outIsEmpty = 0;
	else {
		if ( (segmentNumber == 0) || (segmentNumber > entryCount) )
			BAILWITHERROR( MP4BadParamErr );
		err = MP4GetListEntry( self->entryList, segmentNumber - 1, (char**) &p ); if (err) goto bail;
		*outIsEmpty = p->emptyEdit;
	}
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err getIndSegmentTime( MP4AtomPtr s,
								 u32 segmentIndex,          /* one based */
								 u64 *outSegmentMovieTime,
								 s64 *outSegmentMediaTime,
								 u64 *outSegmentDuration    /* in movie's time scale */
							   )
{
	MP4Err err;
	u32 i;
	u32 entryCount;
	u64 currentMovieTime;
	MP4EditListAtomPtr self = (MP4EditListAtomPtr) s;
	
	/* This function cannot be used if there are no segments at all */
	
	err = MP4NoErr;
	entryCount = getEntryCount( self );
	if ( (self == NULL) || (segmentIndex == 0) || (segmentIndex > entryCount) )
		BAILWITHERROR( MP4BadParamErr )

	currentMovieTime = 0;	
	for ( i = 0; i < entryCount; i++ )
	{
		edtsEntryPtr p;
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err) goto bail;
		if ( i == (segmentIndex - 1) )
		{
			if ( outSegmentMovieTime )
				*outSegmentMovieTime = currentMovieTime;
			if ( outSegmentMediaTime )
			{
				if ( p->emptyEdit )
					*outSegmentMediaTime = -1;
				else
					*outSegmentMediaTime = p->mediaTime;
			}
			if ( outSegmentDuration )
				*outSegmentDuration = p->segmentDuration;
		}
		else
		{
			currentMovieTime += p->segmentDuration;
		}
	}
bail:
	TEST_RETURN( err );

	return err;
}


static MP4Err getTimeAndRate( MP4AtomPtr s, u64 movieTime, u32 movieTimeScale,
							  u32 mediaTimeScale, s64 *outMediaTime, u32 *outMediaRate,
							  u64 *outPriorSegmentEndTime, u64 *outNextSegmentBeginTime )
{
	MP4Err err;

	u32 i;
	u32 entryCount;
	u64 currentMovieTime;
	u64 priorSegmentEndTime;
	u64 nextSegmentBeginTime;
	MP4EditListAtomPtr self = (MP4EditListAtomPtr) s;
	
	/* This function cannot be used if there are no segments at all */
	
	err = MP4NoErr;
	if ( (self == NULL) || (movieTimeScale == 0) || (mediaTimeScale == 0) )
		BAILWITHERROR( MP4BadParamErr )
	currentMovieTime = 0;
	*outMediaTime = -1;
	*outMediaRate = 1 << 16;
	priorSegmentEndTime  = 0;
	nextSegmentBeginTime = 0;
	
	/*
		We rely on there being no consecutive empty edits, and
		this is not tested !!!
	*/
	entryCount = getEntryCount( self );
	for ( i = 0; i < entryCount; i++ )
	{
		edtsEntryPtr p;
		u64 secondsOffset;
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err) goto bail;
		if ( (currentMovieTime + p->segmentDuration) >= movieTime )
		{
			/* found the correct segment */
			if ( p->emptyEdit )
			{
				/* an empty edit */
				*outMediaTime = -1;
				*outMediaRate = p->mediaRate;
				if ( i < entryCount )
					nextSegmentBeginTime = (p+1)->mediaTime;
				break;
			}
			else
			{
				/* normal edit */
				secondsOffset = (movieTime - currentMovieTime) / movieTimeScale;
				*outMediaTime = p->mediaTime + (secondsOffset * mediaTimeScale);
				*outMediaRate = p->mediaRate;
			}
		}
		else
		{
			/* advance to next segment */
			secondsOffset = p->segmentDuration / movieTimeScale;
			priorSegmentEndTime = secondsOffset * mediaTimeScale;
			currentMovieTime += p->segmentDuration;
		}
	}
	if ( outPriorSegmentEndTime )
		*outPriorSegmentEndTime = priorSegmentEndTime;
	if ( outNextSegmentBeginTime )
		*outNextSegmentBeginTime = nextSegmentBeginTime;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;
	u32 entryCount;
	MP4EditListAtomPtr self = (MP4EditListAtomPtr) s;
	err = MP4NoErr;
	entryCount = getEntryCount( self );
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	PUT32_V( entryCount );

	for ( i = 0; i < entryCount; i++ )
	{
		edtsEntryPtr p;
		u32 rate;
		err = MP4GetListEntry( self->entryList, i, (char**) &p ); if (err) goto bail;
		if ( self->version == 1 )
		{
			PUT64_V( p->segmentDuration );
			PUT64_V( p->mediaTime );
		}
		else
		{
			s32 mediaTime = (s32) p->mediaTime;
			PUT32_V( p->segmentDuration );
			PUT32_V( mediaTime );
		}
		rate = p->mediaRate << 16;
		PUT32_V( rate );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	u32 durtimebytes;
	u32 entryCount;
	MP4EditListAtomPtr self = (MP4EditListAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += 4;
	durtimebytes = self->version == 1 ? 16 : 8;
	entryCount = getEntryCount( self );
	self->size += (entryCount * (durtimebytes + 4));
bail:
	TEST_RETURN( err );

	return err;
}


static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 entries;
	u32 entryCount;
	MP4EditListAtomPtr self = (MP4EditListAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	
	GET32_V( entryCount );
	for ( entries = 0; entries < entryCount; entries++ )
	{
		u64 segmentDuration;
		s64 mediaTime;
		u32	mediaRate;
		edtsEntryPtr p;
		p = (edtsEntryPtr) calloc( 1, sizeof(edtsEntry) );
		TESTMALLOC( p );
		if ( self->version == 1 )
		{
			GET64_V( segmentDuration );
			GET64_V( mediaTime );
			if ( mediaTime < 0 )
				p->emptyEdit = 1;
		}
		else
		{
			u32 val;
			s32 sval;
			GET32_V_MSG( val, "segment-duration" );
			segmentDuration = val;
			GET32_V_MSG( sval, "media-time" );
			mediaTime = sval;		
			if ( sval < 0 )
				p->emptyEdit = 1;
		}
		GET32_V( mediaRate );
		/*
		    Earlier versions of this code
		    forgot that mediaRate was fixed32!
		*/
#ifdef USE_OLD_FILES_WORKAROUND
        if ( mediaRate != 1 )
#endif
        {
            mediaRate >>= 16;
        }
		p->segmentDuration  = segmentDuration;
		p->mediaTime        = mediaTime;
		p->mediaRate        = mediaRate;
		err = MP4AddListEntry( p, self->entryList ); if (err) goto bail;
	}
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateEditListAtom( MP4EditListAtomPtr *outAtom )
{
	MP4Err err;
	MP4EditListAtomPtr self;
	
	self = (MP4EditListAtomPtr) calloc( 1, sizeof(MP4EditListAtom) );
	TESTMALLOC( self );

	err = MP4CreateFullAtom( (MP4AtomPtr) self ); if (err) goto bail;
	err = MP4MakeLinkedList( &self->entryList ); if (err) goto bail;
	self->type = MP4EditListAtomType;
	self->name                = "edit list";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->insertSegment = insertSegment;
	self->getTimeAndRate      = getTimeAndRate;
	self->getIndSegmentTime = getIndSegmentTime;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->getEffectiveDuration = getEffectiveDuration;
	self->getEntryCount = getEntryCount;
	self->setTrackOffset = setTrackOffset;
    self->getTrackOffset = getTrackOffset;
	self->isEmptyEdit = isEmptyEdit;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
