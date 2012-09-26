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
	$Id: MP4OrdinaryTrackReader.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4TrackReader.h"
#include "MP4Impl.h"
#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void put32( u32 val, char *buf )
{
   buf[3] = val         & 0xFF;
   buf[2] = (val >> 8)  & 0xFF;
   buf[1] = (val >> 16) & 0xFF;
   buf[0] = (val >> 24) & 0xFF; 
}

static MP4Err destroy(struct MP4TrackReaderStruct* self )
{
   MP4DisposeHandle( self->sampleH );
   self->sampleH = NULL;
   free( self );
   return MP4NoErr;
}

static MP4Err getNextSegment( MP4TrackReaderPtr self )
{
   MP4Err err;
   MP4EditListAtomPtr elst;
   u64 segmentDuration;
   u64 segmentMediaDuration;
   u32 segmentEndSampleNumber;
	
   err = MP4NoErr;
	
   elst = (MP4EditListAtomPtr) self->elst;
   if ( elst == NULL )
      BAILWITHERROR( MP4InvalidMediaErr );
	
   for ( ;; )
   {
      u32 empty;
      err = elst->isEmptyEdit( elst, self->nextSegment, &empty ); if (err) goto bail;
      if ( empty )
      {
         if ( self->nextSegment == self->trackSegments )
         {
            BAILWITHERROR( MP4InvalidMediaErr );
         }
         else
            self->nextSegment++;
      }
      else
         break;
   }
   err = elst->getIndSegmentTime( (MP4AtomPtr) elst,
                                  self->nextSegment,
                                  &self->segmentMovieTime,
                                  &self->segmentMediaTime,
                                  &segmentDuration ); if (err) goto bail;

   err = MP4MediaTimeToSampleNum( self->media,
                                  self->segmentMediaTime,
                                  &self->nextSampleNumber, NULL, NULL, NULL ); if (err) goto bail;

   self->segmentEndTime = self->mediaTimeScale * (self->segmentMovieTime + segmentDuration)
      / self->movieTimeScale;

   segmentMediaDuration = (self->mediaTimeScale * segmentDuration)
      / self->movieTimeScale;

   err = MP4MediaTimeToSampleNum( self->media,
                                  self->segmentMediaTime + segmentMediaDuration,
                                  &segmentEndSampleNumber, NULL, NULL, NULL ); if (err) goto bail;

   if ( segmentEndSampleNumber < self->nextSampleNumber )
      BAILWITHERROR( MP4InvalidMediaErr );

   self->segmentSampleCount = (segmentEndSampleNumber - self->nextSampleNumber);
   self->segmentBeginTime = (self->mediaTimeScale * self->segmentMovieTime) / self->movieTimeScale;
  bail:
   TEST_RETURN( err );

   return err;
}


static MP4Err getNextAccessUnit( struct MP4TrackReaderStruct* self, MP4Handle outAccessUnit,
                                 u32 *outSize, u32 *outSampleFlags,
                                 s32 *outCTS, s32 *outDTS, u32 *outDuration, u8* outPad )
{
   MP4Err    err;
   u32       sampleFlags;
   u64       sampleDTS;
   s32       sampleCTSOffset;
   u64       duration;


   err = MP4NoErr;

   if ( self->nextSegment > self->trackSegments )
   {
      BAILWITHERROR( MP4EOF )
   }
   if ( (self->elst == 0) && (self->nextSampleNumber > self->segmentSampleCount) )
   {
      BAILWITHERROR( MP4EOF )
   }

   /* first get the sample */
   err = MP4GetIndMediaSampleWithPad( self->media,
                               self->nextSampleNumber,
                               outAccessUnit, outSize,
                               &sampleDTS, &sampleCTSOffset,
                               &duration, &sampleFlags, &self->sampleDescIndex, outPad ); if (err) goto bail;
   self->currentSampleNumber = self->nextSampleNumber;
   
   if ( self->isODTrack )
   {
      MP4Err MP4GetTrackReferenceType( MP4Track track,
                                       u32 atomType,
                                       MP4TrackReferenceTypeAtomPtr *outAtom );
      MP4Err rewriteODFrame( MP4Track track,
                             MP4Handle sampleH,
                             u64 cts,
                             MP4TrackReferenceTypeAtomPtr mpod, MP4SLConfig slconfig );

      MP4TrackReferenceTypeAtomPtr mpod;
      err = MP4GetTrackReferenceType( self->track,
                                      MP4ODTrackReferenceAtomType,
                                      &mpod ); if (err) goto bail;
      if ( mpod )
      {
         u32 sz;
         err = rewriteODFrame( self->track, outAccessUnit, sampleDTS, mpod, self->slconfig ); if (err) goto bail;
         err = MP4GetHandleSize( outAccessUnit, &sz ); if (err) goto bail;
         *outSize = sz;
      }
   }
   self->nextSampleNumber += 1;
   *outSampleFlags = sampleFlags;
   *outDTS = (u32)(sampleDTS - self->segmentMediaTime + self->segmentBeginTime);

   if ( sampleFlags & MP4MediaSampleHasCTSOffset )
      *outCTS = (u32) ((*outDTS + sampleCTSOffset) & 0xFFFFFFFF);
   else
      *outCTS = *outDTS;

   if ( (self->elst) && ((duration + *outDTS) >= self->segmentEndTime) )
   {
	  if (outDuration) *outDuration = (u32) (self->segmentEndTime - *outDTS);
	  
      self->nextSegment += 1;
      if ( self->nextSegment <= self->trackSegments )
      {
         err = getNextSegment( self ); if (err) goto bail;
      }
   }
   else if ( outDuration )
   {
      *outDuration = (u32)duration;
   }
  bail:
   return err;
}

static MP4Err setSLConfig( struct MP4TrackReaderStruct* self, MP4SLConfig slconfig )
{
    self->slconfig = slconfig;
    return MP4NoErr;
}

static MP4Err getNextPacket( MP4TrackReaderPtr self, MP4Handle outPacket, u32 *outSize )
{
   MP4Err    err;
   u32       sampleSize;
   u32       sampleFlags;
   u32       packetSize;
   char      header[ 40 ];
   u32       headerBytes;
   u8        val;
   s32       packetCTS;
   s32       packetDTS;

   err = MP4NoErr;
   if ( self->slconfig )
   {
        /* 
          We don't support packetization with 
          arbitrary SLConfigs yet!!!
        */
        BAILWITHERROR( MP4NotImplementedErr );
   }
   err = getNextAccessUnit( self, self->sampleH, &sampleSize,
                            &sampleFlags, &packetCTS, &packetDTS, 0, NULL ); if (err) goto bail;
	
   /* make packet header */
   self->sequenceNumber += 1;
   self->sequenceNumber &= 0x1F;
	
   /* note that we hardwire seqnum to 5 bits */
   val = (self->sequenceNumber & 0x1F) << 3;
   if ( (sampleFlags & MP4MediaSampleNotSync) == 0 )
      val |= (1 << 2);
   if ( sampleFlags & MP4MediaSampleHasCTSOffset )
      val |= (1 << 1);
   val |= 1; /* we always have CTS */
   headerBytes = 0;
   header[ headerBytes++ ] = val;

   if ( sampleFlags & MP4MediaSampleHasCTSOffset )
   {
      put32( packetDTS, header + headerBytes );
      headerBytes += 4;
   }
   put32( packetCTS, header + headerBytes );
   headerBytes += 4;

   /* size output handle */
   packetSize = sampleSize + headerBytes;
   err = MP4SetHandleSize( outPacket, packetSize ); if (err) goto bail;

   /* copy header and sample into output handle */
   memcpy( *outPacket, header, headerBytes );
   memcpy( *outPacket + headerBytes, *self->sampleH, sampleSize );
   *outSize = packetSize;
bail:
   return err;
}

static MP4Err setupReader( MP4TrackReaderPtr self )
{
   MP4Err err;
   MP4TrackAtomPtr    trak;
   MP4EditAtomPtr     edts;
   u32 editCount;
   u32 sampleCount;

   err = MP4NoErr;
   trak = (MP4TrackAtomPtr) self->track;
   edts = (MP4EditAtomPtr)  trak->trackEditAtom;
   
   editCount = 0;
   if (edts != 0) {
   	    MP4EditListAtomPtr elst;
   		elst = (MP4EditListAtomPtr) (edts->editListAtom);
   		if (elst != 0)
   			editCount = elst->getEntryCount( elst );
   	}
   	
   if ( editCount == 0 )
   {
      err = MP4GetMediaSampleCount( self->media, &sampleCount ); if (err) goto bail;
      self->elst                     = 0;
      self->trackSegments            = 1;
      self->nextSegment              = 0;
      self->segmentSampleCount       = sampleCount;
      self->nextSampleNumber         = 1;
      self->segmentMovieTime         = 0;
      self->sampleDescIndex          = 1;
      self->segmentBeginTime         = 0;
	  self->segmentMediaTime		 = 0;
   }
   else
   {
      self->elst = (MP4EditListAtomPtr) edts->editListAtom;
      self->trackSegments            = self->elst->getEntryCount( self->elst );
      self->nextSegment              = 1;
      self->sampleDescIndex          = 1;
      err = getNextSegment( self ); if (err) goto bail;
   }
   self->currentSampleNumber = 0;
	
  bail:
   TEST_RETURN( err );

   return err;
}

MP4Err MP4CreateOrdinaryTrackReader( MP4Movie theMovie, MP4Track theTrack, MP4TrackReaderPtr *outReader )
{
   MP4Err err;
   MP4TrackReaderPtr self;
	
   err = MP4NoErr;
   self = (MP4TrackReaderPtr) calloc( 1, sizeof(struct MP4TrackReaderStruct) );
   TESTMALLOC( self )
   self->movie   = theMovie;
   self->track   = theTrack;
   self->destroy = destroy;
   self->getNextPacket = getNextPacket;
   self->getNextAccessUnit = getNextAccessUnit;
   self->setSLConfig = setSLConfig;
   err = MP4NewHandle( 4096, &self->sampleH ); if (err) goto bail;
   err = MP4GetTrackMedia( theTrack, &self->media ); if (err) goto bail;
   err = MP4CheckMediaDataReferences( self->media ); if (err) goto bail;
   err = MP4GetMovieTimeScale( theMovie, &self->movieTimeScale ); if (err) goto bail;
   err = MP4GetMediaTimeScale( self->media, &self->mediaTimeScale ); if (err) goto bail;
   err = setupReader( self ); if (err) goto bail;
   *outReader = self;
  bail:
   TEST_RETURN( err );

   return err;
}
/* Guido : inserted to clean-up resources */
MP4Err MP4DisposeOrdinaryTrackReader( MP4TrackReaderPtr self )
{
   MP4Err err;
   MP4Media theMedia;
   MP4MediaInformationAtomPtr minf;

   err = MP4NoErr;
   if (self == NULL)
   {
      BAILWITHERROR( MP4BadParamErr );
   }
   theMedia = self->media;
   if (theMedia == NULL)
   {
      BAILWITHERROR( MP4BadParamErr );
   }
   minf = (MP4MediaInformationAtomPtr)((MP4MediaAtomPtr) theMedia)->information;
   if ( minf == NULL )
   {
      BAILWITHERROR( MP4InvalidMediaErr );
   }
   err = minf->closeDataHandler( (MP4AtomPtr) minf ); if (err) goto bail;

  bail:
   TEST_RETURN( err );

   return err;
}

