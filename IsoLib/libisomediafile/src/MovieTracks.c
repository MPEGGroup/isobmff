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
	$Id: MovieTracks.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "MP4Impl.h"
#include "MovieTracks.h"
#include <stdio.h>
#include <string.h>

MP4_EXTERN ( MP4Err ) MP4GetTrackID( MP4Track theTrack, u32 *outTrackID )
{
   MP4Err err;
   MP4TrackAtomPtr trackAtom;
   MP4TrackHeaderAtomPtr trackHeaderAtom;
   err = MP4NoErr;
   if ( theTrack == NULL )
      BAILWITHERROR( MP4BadParamErr )
	 trackAtom = (MP4TrackAtomPtr) theTrack;
   trackHeaderAtom = (MP4TrackHeaderAtomPtr) trackAtom->trackHeader;
   *outTrackID = trackHeaderAtom->trackID;
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4GetTrackEnabled( MP4Track theTrack, u32 *outEnabled )
{
   MP4Err err;
   MP4TrackAtomPtr trackAtom;
   err = MP4NoErr;
   if ( theTrack == NULL )
      BAILWITHERROR( MP4BadParamErr )
	 trackAtom = (MP4TrackAtomPtr) theTrack;
   err = trackAtom->getEnabled( trackAtom, outEnabled ); if (err) goto bail;
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4SetTrackEnabled( MP4Track theTrack, u32 enabled )
{
   MP4Err err;
   MP4TrackAtomPtr trackAtom;
   err = MP4NoErr;
   if ( theTrack == NULL )
      BAILWITHERROR( MP4BadParamErr )
	 trackAtom = (MP4TrackAtomPtr) theTrack;
   err = trackAtom->setEnabled( trackAtom, enabled ); if (err) goto bail;
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4AddTrackReferenceWithID( MP4Track theTrack, u32 dependsOnID, u32 dependencyType, u32 *outReferenceIndex )
{
   MP4Err err;
   MP4TrackAtomPtr trak;
   MP4TrackReferenceAtomPtr tref;
   MP4TrackReferenceTypeAtomPtr dpnd;

   err = MP4NoErr;
   if ( (theTrack == NULL) || (dependsOnID == 0) )
      BAILWITHERROR( MP4BadParamErr );
   trak = (MP4TrackAtomPtr) theTrack;
   tref = (MP4TrackReferenceAtomPtr) trak->trackReferences;
   if ( tref == NULL )
   {
      MP4Err MP4CreateTrackReferenceAtom( MP4TrackReferenceAtomPtr *outAtom );
      err = MP4CreateTrackReferenceAtom( &tref ); if (err) goto bail;
      err = trak->addAtom( trak, (MP4AtomPtr) tref ); if (err) goto bail;
   }
   err = tref->findAtomOfType( tref, dependencyType, (MP4AtomPtr*) &dpnd ); if (err) goto bail;
   if ( dpnd == NULL )
   {
      MP4Err MP4CreateTrackReferenceTypeAtom( u32 atomType, MP4TrackReferenceTypeAtomPtr *outAtom );
      err = MP4CreateTrackReferenceTypeAtom( dependencyType, &dpnd ); if (err) goto bail;
      err = tref->addAtom( tref, (MP4AtomPtr) dpnd ); if (err) goto bail;
   }
   err = dpnd->addTrackID( dpnd, dependsOnID ); if (err) goto bail;
   *outReferenceIndex = dpnd->trackIDCount;
  bail:
   TEST_RETURN( err );
   return err;
}

MP4_EXTERN ( MP4Err ) MP4AddTrackReference( MP4Track theTrack, MP4Track dependsOn, u32 dependencyType, u32 *outReferenceIndex )
{
   MP4Err err;
   u32 dependsOnID;

   err = MP4NoErr;
   if ( (theTrack == NULL) || (dependsOn == NULL) )
      BAILWITHERROR( MP4BadParamErr );
   err = MP4GetTrackID( dependsOn, &dependsOnID ); if (err) goto bail;
   if ( dependsOnID == 0 )
      BAILWITHERROR( MP4InvalidMediaErr );
   err = MP4AddTrackReferenceWithID( theTrack, dependsOnID, dependencyType, outReferenceIndex ); if (err) goto bail;
  bail:
   TEST_RETURN( err );
   return err;
}

MP4_EXTERN ( MP4Err ) MP4AddTrackToMovieIOD( MP4Track theTrack )
{
   MP4Err MP4MovieAddTrackES_IDToIOD( MP4Movie theMovie, MP4Track theTrack );	
   MP4Movie itsMovie;
   MP4Err err;
   if ( theTrack == 0 )
      BAILWITHERROR( MP4BadParamErr );
   err = MP4GetTrackMovie( theTrack, &itsMovie ); if (err) goto bail;
   err = MP4MovieAddTrackES_IDToIOD( itsMovie, theTrack ); if (err) goto bail;
  bail:
   TEST_RETURN( err );
   return err;
}

MP4_EXTERN ( MP4Err ) MP4GetTrackDuration( MP4Track theTrack, u64 *outDuration )
{
   MP4Err err;
   MP4TrackAtomPtr trak;
   MP4Movie moov;
   MP4Media mdia;
   u32 ts;
   u64 duration;
   u32 trakDuration;
   
   if ( (theTrack == 0) || (outDuration == 0) )
   {
      BAILWITHERROR( MP4BadParamErr );
   }
   err = MP4GetTrackMovie( theTrack, &moov ); if (err) goto bail;
   err = MP4GetTrackMedia( theTrack, &mdia ); if (err) goto bail;
   err = MP4GetMovieTimeScale( moov, &ts ); if (err) goto bail;
   err = MP4GetMediaDuration( mdia, &duration ); if (err) goto bail;
   trak = (MP4TrackAtomPtr) theTrack;
   err = trak->calculateDuration( trak, ts ); if (err) goto bail;
   err = trak->getDuration( trak, &trakDuration ); if (err) goto bail;
   *outDuration = (u64) trakDuration;
bail:
   TEST_RETURN( err );
   return err;
}

ISO_EXTERN ( ISOErr ) MJ2SetTrackMatrix( ISOTrack theTrack, u32 matrix[9] )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	u32 aMatrix[9];
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;

	if ( matrix == NULL ) {
		/* if we are passed a NULL matrix, use the identity matrix */
		aMatrix[0] = 0x00010000;
		aMatrix[1] = 0;
		aMatrix[2] = 0;
		aMatrix[3] = 0;
		aMatrix[4] = 0x00010000;
		aMatrix[5] = 0;
		aMatrix[6] = 0;
		aMatrix[7] = 0;
		aMatrix[8] = 0x40000000;
	} else {
		memcpy( &aMatrix, matrix, sizeof(ISOMatrixRecord) );
	}
	err = trak->setMatrix( trak, aMatrix );

bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) MJ2GetTrackMatrix( ISOTrack theTrack, u32 outMatrix[9] )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;

	if ( outMatrix == NULL )
		BAILWITHERROR( MP4BadParamErr );

	err = trak->getMatrix( trak, outMatrix );
bail:
	return err;
}

ISO_EXTERN ( ISOErr ) MJ2SetTrackLayer( ISOTrack theTrack, s16 layer )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;

	err = trak->setLayer( trak, layer );

bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) MJ2GetTrackLayer( ISOTrack theTrack, s16 *outLayer )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;

	if ( outLayer == NULL )
		BAILWITHERROR( MP4BadParamErr );
	err = trak->getLayer( trak, outLayer );
bail:
	return err;
}

ISO_EXTERN ( ISOErr ) MJ2SetTrackDimensions( ISOTrack theTrack, u32 width, u32 height )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;

	err = trak->setDimensions( trak, width, height );

bail:
	TEST_RETURN( err );
	return err;

}

ISO_EXTERN ( ISOErr ) MJ2GetTrackDimensions( ISOTrack theTrack, u32 *outWidth, u32 *outHeight )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;

	if ( ( outWidth == NULL ) || ( outHeight == NULL ) )
		BAILWITHERROR( MP4BadParamErr );
	err = trak->getDimensions( trak, outWidth, outHeight );
bail:
	return err;
}

ISO_EXTERN ( ISOErr ) MJ2SetTrackVolume( ISOTrack theTrack, s16 volume )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;
	assert( trak->setVolume );
	err = trak->setVolume( trak, volume );
bail:
	return err;
}

ISO_EXTERN ( ISOErr ) MJ2GetTrackVolume( ISOTrack theTrack, s16 *outVolume )
{
	MP4Err err;
	MP4TrackAtomPtr trak;
	
	if ( theTrack == 0 ) {
		BAILWITHERROR( MP4BadParamErr );
	}
	
	trak = (MP4TrackAtomPtr) theTrack;

	if ( outVolume == NULL )
		BAILWITHERROR( MP4BadParamErr );
	err = trak->getVolume( trak, outVolume );
bail:
	return err;
}

MP4_EXTERN ( MP4Err ) MP4GetTrackReference( MP4Track theTrack, u32 referenceType, u32 referenceIndex, MP4Track *outReferencedTrack )
{
   MP4Err err;
   MP4TrackAtomPtr trak;
   MP4Movie moov;
   MP4TrackReferenceAtomPtr tref;
   MP4TrackReferenceTypeAtomPtr dpnd;
   u32 selectedTrackID;

   err = MP4NoErr;
   if ( (theTrack == NULL) || (referenceType == 0) || (referenceIndex == 0) || (outReferencedTrack == NULL) )
      BAILWITHERROR( MP4BadParamErr );
   err = MP4GetTrackMovie( theTrack, &moov ); if (err) goto bail;
   trak = (MP4TrackAtomPtr) theTrack;
   tref = (MP4TrackReferenceAtomPtr) trak->trackReferences;
   if ( tref == NULL )
      BAILWITHERROR( MP4BadParamErr );
   err = tref->findAtomOfType( tref, referenceType, (MP4AtomPtr*) &dpnd ); if (err) goto bail;
   if ( (dpnd == NULL) || (dpnd->trackIDCount < referenceIndex) )
      BAILWITHERROR( MP4BadParamErr );
   selectedTrackID = dpnd->trackIDs[ referenceIndex - 1 ];
   if ( selectedTrackID == 0 )
      BAILWITHERROR( MP4InvalidMediaErr );
   err = MP4GetMovieTrack( moov, selectedTrackID, outReferencedTrack ); if (err) goto bail;
  bail:
   TEST_RETURN( err );
   return err;
}

MP4_EXTERN ( MP4Err ) MP4GetTrackReferenceCount( MP4Track theTrack, u32 referenceType, u32 *outReferenceCount )
{
   MP4Err err;
   MP4TrackAtomPtr trak;
   MP4TrackReferenceAtomPtr tref;
   MP4TrackReferenceTypeAtomPtr dpnd;

   err = MP4NoErr;
   if ( (theTrack == NULL) || (referenceType == 0) || (outReferenceCount == NULL) )
      BAILWITHERROR( MP4BadParamErr );
   trak = (MP4TrackAtomPtr) theTrack;
   tref = (MP4TrackReferenceAtomPtr) trak->trackReferences;
   *outReferenceCount = 0;
   if ( tref != NULL )
   {
      err = tref->findAtomOfType( tref, referenceType, (MP4AtomPtr*) &dpnd );
      if ( (err == MP4NoErr) && (dpnd != NULL) )
		*outReferenceCount = dpnd->trackIDCount;
      else
		err = MP4NoErr;
   }
  bail:
   TEST_RETURN( err );
   return err;
}

MP4_EXTERN ( MP4Err ) MP4GetTrackMovie( MP4Track theTrack, MP4Movie *outMovie )
{
   MP4Err err;
   MP4TrackAtomPtr trackAtom;
   trackAtom = (MP4TrackAtomPtr) theTrack;
   err = MP4NoErr;
   if ( theTrack == NULL )
      BAILWITHERROR( MP4BadParamErr )
	*outMovie = (MP4Movie) trackAtom->moov;
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4GetTrackMedia( MP4Track theTrack, MP4Media *outMedia )
{
   MP4Err err;
   MP4TrackAtomPtr trackAtom;
   trackAtom = (MP4TrackAtomPtr) theTrack;
   err = MP4NoErr;
   if ( theTrack == NULL )
   {
      BAILWITHERROR( MP4BadParamErr );
   }
   if ( trackAtom->trackMedia )
   {
	 *outMedia = (MP4Media) trackAtom->trackMedia;
   }
   else
   {
        BAILWITHERROR( MP4InvalidMediaErr );
   }
  bail:
   TEST_RETURN( err );

   return err;
}


MP4_EXTERN ( MP4Err ) MP4GetTrackUserData( MP4Track theTrack, MP4UserData* outUserData )
{
   MP4Err err;
   MP4UserData udta;
   MP4TrackAtomPtr trackAtom;
   trackAtom = (MP4TrackAtomPtr) theTrack;
   err = MP4NoErr;
   if ( theTrack == NULL )
      BAILWITHERROR( MP4BadParamErr )
	 udta = (MP4UserData) trackAtom->udta;
   if ( trackAtom->udta == 0 )
   {
      err = MP4NewUserData( &udta ); if (err) goto bail;
      err = trackAtom->addAtom( trackAtom, (MP4AtomPtr) udta ); if (err) goto bail;
   }
   *outUserData = (MP4UserData) udta;
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4AddAtomToTrack( MP4Track theTrack, MP4GenericAtom the_atom )
{
   MP4Err err;
   MP4TrackAtomPtr trackAtom;

   trackAtom = (MP4TrackAtomPtr) theTrack;
   err = MP4NoErr;
   err = trackAtom->addAtom( trackAtom, (MP4AtomPtr) the_atom ); if (err) goto bail;

  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4NewTrackMedia( MP4Track theTrack, MP4Media *outMedia, u32 mediaType, u32 timeScale, MP4Handle dataReference )
{
   MP4Err err;
   MP4TrackAtomPtr trackAtom;
   trackAtom = (MP4TrackAtomPtr) theTrack;
   err = MP4NoErr;
   if ( theTrack == NULL )
   {
      BAILWITHERROR( MP4BadParamErr );
   }
   err = trackAtom->newMedia( trackAtom, outMedia, mediaType, timeScale, dataReference ); if (err) goto bail;

   if (((trackAtom->moov)->fileType) == ISOQuickTimeFileType) {
		MP4MediaAtomPtr mdia;
		MP4HandlerAtomPtr hdlr;
		mdia = (MP4MediaAtomPtr) trackAtom->trackMedia;
		hdlr = (MP4HandlerAtomPtr) mdia->handler;
		hdlr->setName( (MP4AtomPtr) hdlr, hdlr->nameUTF8, 1);
	}

  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4InsertMediaIntoTrack( MP4Track track, s32 trackStartTime, s32 mediaStartTime, u64 mediaDuration, s32 mediaRate )
{
   MP4Err err;
   MP4TrackAtomPtr trak;
   MP4MediaAtomPtr mdia;

   err = MP4NoErr;
   if ( (track == 0) || (mediaRate < 0) || (mediaRate > 1) )
      BAILWITHERROR( MP4BadParamErr );
   trak = (MP4TrackAtomPtr) track;
   mdia = (MP4MediaAtomPtr) trak->trackMedia;

   if ( (trackStartTime == 0) && (mediaStartTime == 0) && (mediaRate == 1) )
   {
      if ( mediaDuration == 0 )
         BAILWITHERROR( MP4BadParamErr );
   }
   else
   {
      /* need an edit list */
      MP4EditAtomPtr     edts;
      MP4EditListAtomPtr elst;
      u32 movieTimeScale, mediaTimeScale;
      
	  err = MP4GetMovieTimeScale( (MP4Movie) trak->moov, &movieTimeScale ); if (err) goto bail;
	  err = MP4GetMediaTimeScale( (MP4Media) mdia, &mediaTimeScale ); if (err) goto bail;
	  mediaDuration = (mediaDuration * movieTimeScale) / mediaTimeScale;
	
      edts = (MP4EditAtomPtr) trak->trackEditAtom;

      if ( edts == 0 )
      {
         /* no edits yet */
         MP4Err MP4CreateEditAtom( MP4EditAtomPtr *outAtom );
			
         err = MP4CreateEditAtom( &edts ); if (err) goto bail;
         err = trak->addAtom( trak, (MP4AtomPtr) edts ); if (err) goto bail;
      }
      elst = (MP4EditListAtomPtr) edts->editListAtom;
      if ( elst == 0 )
      {
         MP4Err MP4CreateEditListAtom( MP4EditListAtomPtr *outAtom );
			
         err = MP4CreateEditListAtom( &elst ); if (err) goto bail;
         err = edts->addAtom( edts, (MP4AtomPtr) elst ); if (err) goto bail;
      }
      err = elst->insertSegment( elst, trackStartTime, mediaStartTime, mediaDuration, mediaRate ); if (err) goto bail;
   }
   if ( trak->moov->inMemoryDataHandler )
   {
      if ( mdia )
      {
         MP4MediaInformationAtomPtr minf;
         minf = (MP4MediaInformationAtomPtr) mdia->information;
         if ( minf )
         {
            if ( minf->dataHandler == 0 )
            {
               minf->dataHandler = trak->moov->inMemoryDataHandler;
               if ( minf->dataEntryIndex == 0 )
               {
                  minf->dataEntryIndex = 1;
               }
            }
         }
      }
   }
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4GetTrackOffset( MP4Track track, u32 *outMovieOffsetTime )
{
   MP4Err err;
   MP4TrackAtomPtr trak;
   MP4EditAtomPtr edts;
   MP4EditListAtomPtr elst;

   err = MP4NoErr;
   if ( (track == 0) || (outMovieOffsetTime == 0) )
      BAILWITHERROR( MP4BadParamErr );

   /* see if we have an edit list */	
   trak = (MP4TrackAtomPtr) track;
   edts = (MP4EditAtomPtr) trak->trackEditAtom;

   if ( edts == 0 )
   {
      *outMovieOffsetTime = 0;
   }
   else
   {
      elst = (MP4EditListAtomPtr) edts->editListAtom;
      if ( elst == 0 )
      {
         *outMovieOffsetTime = 0;
      }
      else
      {
         err = elst->getTrackOffset( elst, outMovieOffsetTime ); if (err) goto bail;
      }
   }
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4SetTrackOffset( MP4Track track, u32 movieOffsetTime )
{
   MP4Err err;
   MP4TrackAtomPtr trak;
   MP4EditAtomPtr edts;
   MP4EditListAtomPtr elst;
   u64 trackDuration;

   err = MP4NoErr;
   if ( (track == 0) )
      BAILWITHERROR( MP4BadParamErr );

   err = MP4GetTrackDuration( track, &trackDuration ); if (err) goto bail;

   /* need an edit list */	
   trak = (MP4TrackAtomPtr) track;
   edts = (MP4EditAtomPtr) trak->trackEditAtom;

   if ( edts == 0 )
   {
      /* no edits yet */
      MP4Err MP4CreateEditAtom( MP4EditAtomPtr *outAtom );
		
      err = MP4CreateEditAtom( &edts ); if (err) goto bail;
      err = trak->addAtom( trak, (MP4AtomPtr) edts ); if (err) goto bail;
   }
   elst = (MP4EditListAtomPtr) edts->editListAtom;
   if ( elst == 0 )
   {
      MP4Err MP4CreateEditListAtom( MP4EditListAtomPtr *outAtom );
		
      err = MP4CreateEditListAtom( &elst ); if (err) goto bail;
      err = edts->addAtom( edts, (MP4AtomPtr) elst ); if (err) goto bail;
   }
   err = elst->setTrackOffset( elst, movieOffsetTime, trackDuration ); if (err) goto bail;
bail:
   TEST_RETURN( err );

   return err;
}

MP4Err MP4GetTrackReferenceType( MP4Track track, u32 atomType, MP4TrackReferenceTypeAtomPtr *outAtom )
{
   MP4Err err;
   MP4TrackReferenceAtomPtr     tref;
   MP4TrackReferenceTypeAtomPtr foundAtom;
   MP4TrackAtomPtr              trak;
	
   err = MP4NoErr;
   trak = (MP4TrackAtomPtr) track;

   if ( (track == 0) || (outAtom == 0) )
      BAILWITHERROR( MP4BadParamErr )
	 foundAtom = 0;
   tref = (MP4TrackReferenceAtomPtr) trak->trackReferences;
   if ( tref )
   {
      err = tref->findAtomOfType( tref, atomType, (MP4AtomPtr*) &foundAtom ); if (err) goto bail;
   }
   *outAtom = foundAtom;
  bail:
   TEST_RETURN( err );

   return err;
}

MP4_EXTERN ( MP4Err ) MP4TrackTimeToMediaTime( MP4Track theTrack, u64 inTrackTime, s64 *outMediaTime )
{
   MP4Err err;
   MP4Movie theMovie;
   MP4Media theMedia;
   u32 movieTimeScale;
   u32 mediaTimeScale;
   s64 mediaTime;
   MP4TrackAtomPtr    trak;
   MP4EditAtomPtr     edts;
   MP4EditListAtomPtr elst;
	
   err = MP4NoErr;
   if ( (theTrack == 0) || (outMediaTime == 0) )
      BAILWITHERROR( MP4BadParamErr )

   err = MP4GetTrackMovie( theTrack, &theMovie ); if (err) goto bail;
   err = MP4GetMovieTimeScale( theMovie, &movieTimeScale ); if (err) goto bail;
   err = MP4GetTrackMedia( theTrack, &theMedia ); if (err) goto bail;
   err = MP4GetMediaTimeScale( theMedia, &mediaTimeScale ); if (err) goto bail;
	
   if ( (movieTimeScale == 0) )
      BAILWITHERROR( MP4InvalidMediaErr )

   trak = (MP4TrackAtomPtr) theTrack;
   edts = (MP4EditAtomPtr) trak->trackEditAtom;
   if ( edts == 0 )
   {
      /* no edits */
      mediaTime = (inTrackTime / movieTimeScale) * mediaTimeScale;
      *outMediaTime = mediaTime;
   }
   else
   {
      /* edit atom is present */
      u32 editCount;
      
      elst = (MP4EditListAtomPtr) edts->editListAtom;
	  if (elst != 0) 
	  	editCount = elst->getEntryCount(elst); 
	  else editCount = 0;

      if ( editCount == 0 )
      {
	 	/* edit atom but no useful edit list, hmm... */
	 	mediaTime = (inTrackTime / movieTimeScale) * mediaTimeScale;
	 	*outMediaTime = mediaTime;
      }
      else
      {
	 	/* edit list is present and has entries */
	 	u32 mediaRate;
		 u64 prior;
	 	u64 next;
	 	err = elst->getTimeAndRate( (MP4AtomPtr) elst, inTrackTime, movieTimeScale,
				     	mediaTimeScale, &mediaTime, &mediaRate, &prior, &next ); if (err) goto bail;
	 	*outMediaTime = mediaTime;
      }
   }
  bail:
   TEST_RETURN( err );

   return err;
}

ISO_EXTERN ( MP4Err ) ISOSetTrackFragmentDefaults( MP4Track theTrack, u32 duration, u32 size, u32 is_sync, u8 pad )
{
   MP4Err MP4CreateMovieExtendsAtom( MP4MovieExtendsAtomPtr *outAtom );
   MP4Err MP4CreateTrackExtendsAtom( MP4TrackExtendsAtomPtr *outAtom );

   MP4TrackAtomPtr    trak;
   MP4Err err;
   MP4Movie theMovie;
   MP4TrackExtendsAtomPtr trex;
   MP4TrackHeaderAtomPtr tkhd;
   /* MP4SampleTableAtomPtr stbl; */
   MP4MovieExtendsAtomPtr mvex;

   err = MP4NoErr;
   trak = (MP4TrackAtomPtr) theTrack;
   err = MP4GetTrackMovie( theTrack, &theMovie ); if (err) goto bail;
   
   /* This function needs a re-write to be more object-oriented...*/
   
   { 
	   GETMOVIEATOM( theMovie );
	   if (movieAtom->mvex == NULL) {
	   	  err = MP4CreateMovieExtendsAtom( &mvex ); if (err) goto bail;
	   	  err = movieAtom->addAtom( movieAtom, (MP4AtomPtr) mvex ); if (err) goto bail;
	   }
	   else mvex = (MP4MovieExtendsAtomPtr) (movieAtom->mvex);
	   
	   err = MP4CreateTrackExtendsAtom( &trex ); if (err) goto bail;
	   tkhd = (MP4TrackHeaderAtomPtr) trak->trackHeader;
	   
	   trex->trackID = tkhd->trackID;
	   trex->default_sample_duration = duration;
	   trex->default_sample_size = size;
	   trex->default_sample_flags = ((pad & 7)<<17) | (is_sync ? 0 : fragment_difference_sample_flag );
	   
	   /* stbl = ((MP4SampleTableAtomPtr) 
	   	        ((MP4MediaInformationAtomPtr) 
	   	         ((MP4MediaAtomPtr) 
	   	          (trak->trackMedia))->information)->sampleTable);
	   
	   trex->default_sample_description_index = stbl->getCurrentSampleEntryIndex( stbl ); */
	   
	   err = mvex->addAtom( mvex, (MP4AtomPtr) trex ); if (err) goto bail;
   }
bail:
   TEST_RETURN( err );

   return err;

}
