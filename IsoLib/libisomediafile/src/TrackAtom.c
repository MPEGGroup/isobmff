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
	$Id: TrackAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static MP4Err addAtom( MP4TrackAtomPtr self, MP4AtomPtr atom );

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4TrackAtomPtr self;
	u32 i;
	err = MP4NoErr;
	self = (MP4TrackAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	DESTROY_ATOM_LIST
		
	if ( self->super )
		self->super->destroy( s );
  bail:
	TEST_RETURN( err );

	return;
}

static MP4Err setMdat( struct MP4TrackAtom *self, MP4AtomPtr mdat )
{
   self->mdat = mdat;
   return MP4NoErr;
}

static MP4Err setMoov( struct MP4TrackAtom *self, MP4PrivateMovieRecordPtr moov )
{
   self->moov = moov;
   return MP4NoErr;
}

static MP4Err newMedia( struct MP4TrackAtom *self, MP4Media *outMedia, u32 mediaType, u32 timeScale, MP4Handle dataURL )
{
   MP4Err MP4CreateMediaAtom( MP4MediaAtomPtr *outAtom );
	
   MP4Err err;
   MP4Media media;
   MP4MediaAtomPtr mdia;
	
   err = MP4NoErr;
   err = MP4CreateMediaAtom( &mdia ); if (err) goto bail;
   media = (MP4Media) mdia;
   err = mdia->setupNew( mdia, (MP4AtomPtr) self, mediaType, timeScale, dataURL ); if (err) goto bail;
   err = addAtom( self, (MP4AtomPtr) mdia ); if (err) goto bail;
   *outMedia = media;
  bail:
   TEST_RETURN( err );

   return err;
}

static MP4Err adjustedDuration( u32 mediaDuration, u32 mediaTimeScale, u32 movieTimeScale, u32 *outDuration )
{
   MP4Err err;
	
   err = MP4NoErr;
	
   if ( mediaTimeScale == 0 )
      BAILWITHERROR( MP4BadParamErr );
   *outDuration = (u32) ((((u64) mediaDuration) * ((u64)movieTimeScale)) / ((u64)mediaTimeScale));
  bail:
   TEST_RETURN( err );
   return err;
}

static MP4Err mdatArrived( struct MP4TrackAtom* self, MP4AtomPtr mdat )
{
	MP4Err err;
	MP4MediaAtomPtr mdia;
	MP4MediaInformationAtomPtr minf;
	ISOMetaAtomPtr meta;

	err = MP4NoErr;

	meta = (ISOMetaAtomPtr) self->meta;
	if (meta) { 
		meta->setMdat( meta, (MP4AtomPtr) mdat ); 
	}

	mdia = (MP4MediaAtomPtr) self->trackMedia;
	if ( mdia == NULL )
	  BAILWITHERROR( MP4InvalidMediaErr );
	minf = (MP4MediaInformationAtomPtr) mdia->information;
	if (minf == NULL) 
	  BAILWITHERROR( MP4InvalidMediaErr );
	  
	err = minf->mdatArrived( minf, mdat ); if (err) goto bail;
  bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err mdatMoved( struct MP4TrackAtom* self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset )
{
	MP4Err err;
	MP4MediaAtomPtr mdia;
	ISOMetaAtomPtr meta;

	err = MP4NoErr;
	mdia = (MP4MediaAtomPtr) self->trackMedia;
	if ( mdia == NULL )
	  BAILWITHERROR( MP4InvalidMediaErr );
	err = mdia->mdatMoved( mdia, mdatBase, mdatEnd, mdatOffset ); if (err) goto bail;

	meta = (ISOMetaAtomPtr) self->meta;
	if (meta) {
		err = meta->mdatMoved( meta, mdatBase, mdatEnd, mdatOffset ); if (err) goto bail;
	}

  bail:
   TEST_RETURN( err );
   return err;
}


static MP4Err calculateDuration( struct MP4TrackAtom *self, u32 movieTimeScale )
{
   MP4Err err;
   MP4TrackHeaderAtomPtr tkhd;
   MP4EditAtomPtr        edts;
   MP4MediaAtomPtr       mdia;
   MP4MediaHeaderAtomPtr mhdr;
   u32 mediaDuration;
   u32 mediaTimeScale;
   u32 trackDuration;

   tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
   edts = (MP4EditAtomPtr) self->trackEditAtom;
   err = MP4NoErr;
   if ( tkhd == NULL )
      BAILWITHERROR( MP4InvalidMediaErr );
   mdia = (MP4MediaAtomPtr) self->trackMedia;
   if ( mdia == NULL )
      BAILWITHERROR( MP4InvalidMediaErr );
   mhdr = (MP4MediaHeaderAtomPtr) mdia->mediaHeader;
   mediaDuration = (u32) mhdr->duration;
   mediaTimeScale = (u32) mhdr->timeScale;

   trackDuration = 0;
   
   if ( edts )
   {
      err = edts->getEffectiveDuration( edts, &trackDuration ); if (err) goto bail;
   }
   
   if (trackDuration == 0) /* either no edit list at all, or an edit list with no entries */
   {
      err = adjustedDuration( mediaDuration, mediaTimeScale, movieTimeScale, &trackDuration ); if (err) goto bail;
   }
   tkhd->duration = trackDuration;
  bail:
   TEST_RETURN( err );
   return err;
}

static MP4Err getDuration( struct MP4TrackAtom *self, u32 *outDuration )
{
	MP4Err err;
	MP4TrackHeaderAtomPtr tkhd;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	
	err = MP4NoErr;
	if ( outDuration == NULL )
		BAILWITHERROR( MP4BadParamErr );
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	*outDuration = (u32) tkhd->duration;
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err setMatrix( struct MP4TrackAtom* self, u32 matrix[9] )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	memcpy( &(tkhd->qt_matrixA), matrix, sizeof(ISOMatrixRecord) );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getMatrix( struct MP4TrackAtom* self, u32 outMatrix[9] )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	memcpy( outMatrix, &(tkhd->qt_matrixA), sizeof(ISOMatrixRecord) );
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err setLayer( struct MP4TrackAtom* self, s16 layer )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	tkhd->qt_layer = layer;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getLayer( struct MP4TrackAtom* self, s16 *outLayer )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	*outLayer = (s16) tkhd->qt_layer;
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err setDimensions( struct MP4TrackAtom* self, u32 width, u32 height )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	tkhd->qt_trackWidth = width << 16;
	tkhd->qt_trackHeight = height << 16;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getDimensions( struct MP4TrackAtom* self, u32 *outWidth, u32 *outHeight )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	*outWidth = (u32) (tkhd->qt_trackWidth >> 16);
	*outHeight = (u32) (tkhd->qt_trackHeight >> 16);
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err setVolume( struct MP4TrackAtom* self, s16 volume )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	tkhd->qt_volume = volume;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getVolume( struct MP4TrackAtom* self, s16 *outVolume )
{
	MP4TrackHeaderAtomPtr tkhd;
	MP4Err err;
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	if ( tkhd == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	*outVolume = (s16) tkhd->qt_volume;
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err setEnabled( struct MP4TrackAtom *self, u32 enabled )
{
   MP4TrackHeaderAtomPtr tkhd;
   MP4Err err;

   err = MP4NoErr;	
   tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
   if ( tkhd == NULL )
      BAILWITHERROR( MP4InvalidMediaErr );

   if ( enabled )
      tkhd->flags |= 1;
   else
      tkhd->flags &= ~1;
  bail:
   TEST_RETURN( err );
   return err;
}

static MP4Err getEnabled( struct MP4TrackAtom *self, u32 *outEnabled )
{
   MP4TrackHeaderAtomPtr tkhd;
   tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
   *outEnabled = tkhd->flags & 1 ? 1 : 0;
   return MP4NoErr;
}

static MP4Err settrackfragment (struct MP4TrackAtom *self, u32 trackID, MP4AtomPtr fragment )
{
	MP4Err err;
	MP4TrackHeaderAtomPtr tkhd;
	
	err = MP4NoErr;
	tkhd = (MP4TrackHeaderAtomPtr) self->trackHeader;
	
	if (tkhd->trackID == trackID)
	{
		MP4MediaAtomPtr mdia;
		mdia = (MP4MediaAtomPtr) self->trackMedia;
		err = mdia->settrackfragment( mdia, fragment );
	}
	
	TEST_RETURN( err );

	return err;
}

static MP4Err addAtom( MP4TrackAtomPtr self, MP4AtomPtr atom )
{
   MP4Err err;
   err = MP4NoErr;
   err = MP4AddListEntry( atom, self->atomList ); if (err) goto bail;
   switch( atom->type )
   {
		case MP4TrackHeaderAtomType:
		 if ( self->trackHeader )
			BAILWITHERROR( MP4BadDataErr );
		 self->trackHeader = atom;
		 break;
			
		case MP4EditAtomType:
		 if ( self->trackEditAtom )
			BAILWITHERROR( MP4BadDataErr );
		 self->trackEditAtom = atom;
		 break;
			
		case MP4UserDataAtomType:
		 if ( self->udta )
			BAILWITHERROR( MP4BadDataErr );
		 self->udta = atom;
		 break;
			
		case MP4TrackReferenceAtomType:
		 if ( self->trackReferences )
			BAILWITHERROR( MP4BadDataErr );
		 self->trackReferences = atom;
		 break;
			
		case MP4MediaAtomType:
		 if ( self->trackMedia )
			BAILWITHERROR( MP4BadDataErr );
		 self->trackMedia = atom;
		 ((MP4MediaAtomPtr) atom)->mediaTrack = (MP4AtomPtr) self;
		 break;

		case ISOMetaAtomType:
			if ( self->meta )
				BAILWITHERROR( MP4BadDataErr )
			self->meta = atom;
			break;
		}
  bail:
   TEST_RETURN( err );

   return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
   MP4Err err;
   MP4TrackAtomPtr self = (MP4TrackAtomPtr) s;
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
   MP4TrackAtomPtr self = (MP4TrackAtomPtr) s;
   err = MP4NoErr;
	
   err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
   ADD_ATOM_LIST_SIZE( atomList );
  bail:
   TEST_RETURN( err );

   return err;
}

static MP4Err trakAtomCreateFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, /*MP4InputStreamPtr*/ char* inputStream )
{
   PARSE_ATOM_LIST(MP4TrackAtom)
bail:
   TEST_RETURN( err );

   return err;
}

MP4Err MP4CreateTrackAtom( MP4TrackAtomPtr *outAtom )
{
   MP4Err err;
   MP4TrackAtomPtr self;
	
   self = (MP4TrackAtomPtr) calloc( 1, sizeof(MP4TrackAtom) );
   TESTMALLOC( self )
   
	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type					= MP4TrackAtomType;
	self->name					= "track";
	self->createFromInputStream = trakAtomCreateFromInputStream;
	self->destroy				= destroy;
	self->addAtom				= addAtom;
	self->setMoov				= setMoov;
	self->setMdat				= setMdat;
	self->newMedia				= newMedia;
	self->setEnabled			= setEnabled;
	self->getEnabled			= getEnabled;
	err = MP4MakeLinkedList( &self->atomList ); if (err) goto bail;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->calculateDuration		= calculateDuration;
	self->getDuration			= getDuration;
	self->setMatrix				= setMatrix;
	self->getMatrix				= getMatrix;
	self->setLayer				= setLayer;
	self->getLayer				= getLayer;
	self->setDimensions			= setDimensions;
	self->getDimensions			= getDimensions;
	self->setVolume				= setVolume;
	self->getVolume				= getVolume;
	self->mdatMoved				= mdatMoved;
	self->mdatArrived 			= mdatArrived;
	self->settrackfragment		= settrackfragment;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
