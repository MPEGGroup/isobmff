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
	$Id: MP4TrackReader.h,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#ifndef INCLUDED_MP4_TRACK_READER_H
#define INCLUDED_MP4_TRACK_READER_H
#include "MP4Atoms.h"

#define TRACK_READER_ENTRIES \
	MP4Err (*destroy)( struct MP4TrackReaderStruct* self ); \
	MP4Err (*getNextAccessUnit)( struct MP4TrackReaderStruct* self, MP4Handle outAccessUnit, u32 *outSize, u32 *outSampleFlags, s32 *outCTS, s32 *outDTS, u32 *outDuration, u8* outPad ); \
	MP4Err (*getNextPacket)( struct MP4TrackReaderStruct* self, MP4Handle outSample, u32 *outSize ); \
	MP4Err (*setSLConfig)( struct MP4TrackReaderStruct* self, MP4SLConfig slconfig ); \
	MP4Movie movie; \
	MP4Track track; \
	MP4Media media; \
	MP4Handle sampleH; \
	MP4SLConfig slconfig; \
	MP4EditListAtomPtr elst; \
	u32      movieTimeScale; \
	u32      mediaTimeScale; \
	u32      trackSegments; \
	u32      nextSegment; \
	u64      segmentEndTime; \
	u64      segmentMovieTime; \
	s64		 segmentMediaTime; \
	u64      segmentBeginTime; \
	u32      sequenceNumber; \
	u32      segmentSampleCount; \
	u32      sampleDescIndex; \
	u32		 currentSampleNumber; \
	u32      nextSampleNumber;

typedef struct MP4TrackReaderStruct
{
	TRACK_READER_ENTRIES
	u32 isODTrack;
} *MP4TrackReaderPtr;

MP4Err MP4CreateODTrackReader( MP4Movie theMovie, MP4Track theTrack, MP4TrackReaderPtr *outReader );
MP4Err MP4CreateOrdinaryTrackReader( MP4Movie theMovie, MP4Track theTrack, MP4TrackReaderPtr *outReader );
MP4Err MP4DisposeOrdinaryTrackReader( MP4TrackReaderPtr theReader );

#endif
