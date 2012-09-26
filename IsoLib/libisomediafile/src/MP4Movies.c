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
derivative works. Copyright (c) 1999, 2000.
*/
/*
	$Id: MP4Movies.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "MJ2Atoms.h"
#include "MP4InputStream.h"
#include "MP4Impl.h"
#include "MovieTracks.h"
#include "FileMappingObject.h"
#include "MP4Descriptors.h"
#include "MdatDataHandler.h"

#ifdef macintosh
#include <OSUtils.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

MP4Err makeESD( MP4Movie theMovie, u32 trackNumber, u64 cts, MP4SLConfig slconfig, MP4DescriptorPtr *outDesc );
MP4Err MP4MovieCreateDescriptors( MP4Movie theMovie );
MP4Err MP4MovieAddTrackES_IDToIOD( MP4Movie theMovie, MP4Track theTrack );	
MP4Err MP4GetMovieObjectDescriptorUsingSLConfig( MP4Movie theMovie, MP4SLConfig slconfig, MP4Handle outDescriptorH );

MP4_EXTERN ( MP4Err ) MergeMovieFragments( MP4PrivateMovieRecordPtr movie );
MP4_EXTERN ( MP4Err ) MergeMovieData( MP4PrivateMovieRecordPtr movie );

MP4_EXTERN ( MP4Err )
MP4DisposeMovie( MP4Movie theMovie )
{
	GETMOOV( theMovie );
	
	if ( moov->inputStream )
		moov->inputStream->destroy( moov->inputStream );
		
	if ( moov->fileMappingObject )
	{
		moov->fileMappingObject->close( moov->fileMappingObject );
		moov->fileMappingObject->destroy( moov->fileMappingObject );
	}

	if ( moov->inMemoryDataHandler )
	{
		MP4DataHandlerPtr dh = (MP4DataHandlerPtr)moov->inMemoryDataHandler;

		dh->close( dh );
	}

	if ( moov->moovAtomPtr )
		moov->moovAtomPtr->destroy( moov->moovAtomPtr );
		
	if (( moov->true_moov ) && ( moov->true_moov != moov->moovAtomPtr)) 
		moov->true_moov->destroy( moov->true_moov );
	
	if ( moov->mdat )
		moov->mdat->destroy( moov->mdat );
		
	if ( moov->jp2h )
		moov->jp2h->destroy( moov->jp2h );
		
	if ( moov->ftyp )
		moov->ftyp->destroy( moov->ftyp );
	
	if ( moov->sgnt )
		moov->sgnt->destroy( moov->sgnt );
		
	if ( moov->prepend_handle) 
		MP4DisposeHandle( moov->prepend_handle );
	
	if ( moov )
		free( moov );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err
parseMovie( MP4Movie *theMovie, MP4PrivateMovieRecordPtr moov, int openMovieFlags )
{
	MP4Err err;
	MP4AtomPtr anAtomPtr;
	int debug;
	int finished;
	
	err = MP4NoErr;
	if ( (moov == NULL) || (moov->inputStream == NULL) )
		BAILWITHERROR( MP4BadParamErr );

	debug = openMovieFlags & MP4OpenMovieDebug;

	moov->inputStream->debugging = debug;
	for ( finished = 0; !finished; )
	{
		err = MP4ParseAtom( moov->inputStream, &anAtomPtr );
		if ( err == MP4EOF )
		{
			err = MP4NoErr;
			finished = 1;
			break;
		}
		if ( err ) goto bail;
		switch( anAtomPtr->type ) 
		{
			case MP4MovieAtomType:
				if ( moov->moovAtomPtr ) BAILWITHERROR( MP4BadDataErr )
				else
				{
					MP4MovieAtomPtr m = (MP4MovieAtomPtr) anAtomPtr;
					m->setupTracks( m, moov );
					moov->moovAtomPtr = anAtomPtr;					
				}
			  break;
			case MP4MediaDataAtomType:
				{
					MP4MediaDataAtomPtr mdat;
					mdat = (MP4MediaDataAtomPtr) anAtomPtr;
					/* open in place here is not ideal, as the mdat is read and discarded, but it is
					    a start.  note that movies opened in place cannot be written back to disc */
					if ((openMovieFlags & MP4OpenMovieInPlace) || (mdat->dataSize == 0))
						anAtomPtr->destroy( anAtomPtr );
					else if ( moov->mdat == NULL )
						moov->mdat = anAtomPtr;
					else { 
						if (moov->mdatList == NULL) {
							err = MP4MakeLinkedList( &moov->mdatList ); if (err) goto bail;
						}
						err = MP4AddListEntry( anAtomPtr, moov->mdatList ); if (err) goto bail;
					}
				}
			  break;
			case MP4MovieFragmentAtomType:
			  	if (moov->movieFragments == NULL) {
					err = MP4MakeLinkedList( &moov->movieFragments ); if (err) goto bail;
				}
				err = MP4AddListEntry( anAtomPtr, moov->movieFragments ); if (err) goto bail;
			  break;
			case ISOFileTypeAtomType:
				if ( moov->ftyp ) BAILWITHERROR( MP4BadDataErr )
				else moov->ftyp = anAtomPtr;
			  break;
			case MJ2JPEG2000SignatureAtomType:
				if ( moov->sgnt ) BAILWITHERROR( MP4BadDataErr )
				else moov->sgnt = anAtomPtr;
			  break;
			case MJ2JP2HeaderAtomType:
				if ( moov->jp2h ) BAILWITHERROR( MP4BadDataErr )
				else moov->jp2h = anAtomPtr;
			  break;
			case MP4FreeSpaceAtomType:
			case MP4SkipAtomType:
				anAtomPtr->destroy( anAtomPtr );
			  break;
			case ISOMetaAtomType:
				if ( moov->meta ) BAILWITHERROR( MP4BadDataErr )
				else moov->meta = anAtomPtr;
			  break;
			default:
				anAtomPtr->destroy( anAtomPtr );
				/* TODO: Save other top-level atoms -- if any, but this should be it */
				break;
		}
		if ( moov->inputStream->available == 0 )
			finished = 1;
	}
	/* only mp4v1 files have no ftyp, and they have to have a moov */
	if ((moov->moovAtomPtr == NULL) && (moov->ftyp == NULL))
		BAILWITHERROR( MP4BadDataErr )
		
	err = MergeMovieFragments( moov ); if (err) goto bail;
	
	if ( moov->mdat != NULL ) {
		err = MergeMovieData( moov ); if (err) goto bail;
	}
	
	if ( err == MP4NoErr )
		*theMovie = (MP4Movie) moov;
bail:
	TEST_RETURN( err );

	if ( err != MP4NoErr )
	{
		if ( moov ) {
			if (moov->inputStream) {
				moov->inputStream->destroy(moov->inputStream);
			}
			if (moov->fileMappingObject) {
				moov->fileMappingObject->close(moov->fileMappingObject);
				moov->fileMappingObject->destroy(moov->fileMappingObject);
			}
			free( moov );
		}
	}
	return err;
}

MP4_EXTERN ( MP4Err ) MergeMovieFragments( MP4PrivateMovieRecordPtr movie )
{
	u32 i, seq;
	MP4Err err;
	u32 fragmentCount;
	MP4MovieAtomPtr moov;
	err = MP4NoErr;
		
	if (movie->movieFragments == NULL) goto bail;
	seq = 0;
	
	MP4GetListEntryCount( movie->movieFragments, &fragmentCount );
	for( i = 0; i < fragmentCount; i++ )
	{
		MP4MovieFragmentAtomPtr moof;
		MP4MovieFragmentHeaderAtomPtr mfhd;
		
		err = MP4GetListEntry( movie->movieFragments, i, (char **) &moof ); if (err) goto bail;
		
		mfhd = (MP4MovieFragmentHeaderAtomPtr) moof->mfhd;
		if (mfhd == NULL) BAILWITHERROR( MP4BadDataErr );
		if (i==0) seq = mfhd->sequence_number;
			else { 
				if (mfhd->sequence_number <= seq) BAILWITHERROR( MP4BadDataErr );
				seq = mfhd->sequence_number;
			}
				
		
		moof->mergeFragments( moof, (MP4MovieAtomPtr) (movie->moovAtomPtr) );
	}
	DESTROY_ATOM_LIST_V( movie->movieFragments )
	moov = (MP4MovieAtomPtr) movie->moovAtomPtr;
	err = moov->calculateDuration( moov );
bail:
	TEST_RETURN( err );

	return err;	
}

MP4_EXTERN ( MP4Err ) MergeMovieData( MP4PrivateMovieRecordPtr movie )
{
	u32 i;
	MP4Err err;
	MP4MovieAtomPtr moov;
	MP4MediaDataAtomPtr mdat;
	MP4MediaDataAtomPtr secondary_mdat;
	u32 mdatCount;
	u32 offset; 
	ISOMetaAtomPtr meta;
	
	err = MP4NoErr;
		
	moov = (MP4MovieAtomPtr) movie->moovAtomPtr;
	mdat = (MP4MediaDataAtomPtr) movie->mdat;
	if (mdat == NULL) goto bail;
	
	offset = (u32) mdat->dataOffset;
	
	if (moov) {
		moov->mdatArrived( moov, (MP4AtomPtr) mdat );
		moov->mdatMoved( moov, offset, mdat->dataSize + offset, -((s32) offset) );
	}
	
	meta = (ISOMetaAtomPtr) movie->meta;
	if (meta) { 
		err = meta->setMdat( meta, (MP4AtomPtr) mdat ); if (err) goto bail;
		err = meta->mdatMoved( meta, offset, mdat->dataSize + offset, -((s32) offset) ); if (err) goto bail;
	}
	
	if (movie->mdatList != NULL) {
		err = MP4GetListEntryCount( movie->mdatList, &mdatCount );
		
		for ( i = 0; i < mdatCount; i++ )
		{			
			err = MP4GetListEntry( movie->mdatList, i, (char**) &secondary_mdat ); if (err) goto bail;
			offset = (u32) secondary_mdat->dataOffset;
			if (moov) {
				err = moov->mdatMoved( moov, offset, secondary_mdat->dataSize + offset,
											 (s32) (mdat->dataSize - offset) );
			}
			if (meta) {
				err = meta->mdatMoved( meta, offset, secondary_mdat->dataSize + offset,
											 (s32) (mdat->dataSize - offset) );
			}
			err = mdat->addMdat( mdat, secondary_mdat ); if (err) goto bail;
		}
		DESTROY_ATOM_LIST_V( movie->mdatList ) 
	}

bail:
	TEST_RETURN( err );

	return err;	
}

MP4_EXTERN ( MP4Err )
MP4NewMovieFromHandle( MP4Movie *theMovie, MP4Handle movieH, u32 newMovieFlags )
{
	MP4Err err;
	MP4PrivateMovieRecordPtr moov;
	u32 handleSize;

	err = MP4NoErr;
	moov = (MP4PrivateMovieRecordPtr) calloc( 1, sizeof(MP4PrivateMovieRecord) );
	TESTMALLOC( moov );
	moov->referenceCount = 1;
	moov->prepend_handle = NULL;
	moov->moovAtomPtr = NULL;
	err = MP4GetHandleSize( movieH, &handleSize ); if (err) goto bail;
	err = MP4CreateMemoryInputStream( *movieH, handleSize, &moov->inputStream ); if (err) goto bail;
	err = parseMovie( theMovie, moov, newMovieFlags ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	return err;
}

MP4_EXTERN ( MP4Err )
MP4OpenMovieFile( MP4Movie *theMovie, const char *movieURL, int openMovieFlags )
{
	MP4Err err;
	MP4PrivateMovieRecordPtr moov;
	
	err = MP4NoErr;
	moov = (MP4PrivateMovieRecordPtr) calloc( 1, sizeof(MP4PrivateMovieRecord) );
	TESTMALLOC( moov );
	moov->referenceCount = 1;
	moov->prepend_handle = NULL;
	moov->moovAtomPtr = NULL;
	err = MP4CreateFileMappingObject( (char*) movieURL, (struct FileMappingObjectRecord **) &moov->fileMappingObject ); if (err) goto bail;
	err = MP4CreateFileMappingInputStream( moov->fileMappingObject, &moov->inputStream ); if (err) goto bail;
	err = parseMovie( theMovie, moov, openMovieFlags ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	return err;
}



/* */

MP4_EXTERN ( MP4Err )
MP4GetMovieTrackCount( MP4Movie theMovie, u32* outTrackCount )
{
	GETMOVIEATOM( theMovie );
	*outTrackCount = movieAtom->getTrackCount( movieAtom );
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetMovieDuration( MP4Movie theMovie, u64* outDuration )
{
	GETMOVIEHEADERATOM( theMovie );
	if ( outDuration == 0 )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	if ( movieHeaderAtom == 0 )
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	err = movieAtom->calculateDuration( movieAtom ); if (err) goto bail;
	*outDuration = movieHeaderAtom->duration;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetMovieTimeScale( MP4Movie theMovie, u32* outTimeScale )
{
	GETMOVIEHEADERATOM( theMovie );
	if ( outTimeScale == 0 )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	if ( movieHeaderAtom == 0 )
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	*outTimeScale = movieHeaderAtom->timeScale;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetMovieUserData( MP4Movie theMovie, MP4UserData* outUserData )
{
	MP4UserData udta;
	GETMOVIEATOM( theMovie );
	if ( outUserData == 0 )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	udta = (MP4UserData) movieAtom->udta;
	if ( movieAtom->udta == 0 )
	{
		err = MP4NewUserData( &udta ); if (err) goto bail;
		err = movieAtom->addAtom( movieAtom, (MP4AtomPtr) udta ); if (err) goto bail;
	}
	*outUserData = (MP4UserData) udta;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err ) MP4AddAtomToMovie( MP4Movie theMovie, MP4GenericAtom the_atom )
{
	GETMOVIEATOM( theMovie );

   err = MP4NoErr;
   err = movieAtom->addAtom( movieAtom, (MP4AtomPtr) the_atom ); if (err) goto bail;

  bail:
   TEST_RETURN( err );

   return err;
}

MP4Err makeESD( MP4Movie theMovie, u32 trackNumber, u64 cts, MP4SLConfig inSLConfig, MP4DescriptorPtr *outDesc )
{
	MP4Err MP4GetMediaSampleDescIndex( MP4Media theMedia, u64 desiredTime, u32 *outSampleDescriptionIndex );
	MP4Err MP4GetMediaESD( MP4Media theMedia, u32 index, MP4ES_DescriptorPtr *outESD, u32 *outDataReferenceIndex );
	MP4Err MP4CreateSLConfigDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
    MP4Err MP4CreateES_Descriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );

	MP4Err err;
	MP4ES_DescriptorPtr esd;
	MP4ES_DescriptorPtr esdInFile;
	MP4SLConfigDescriptorPtr slconfig;
	MP4TrackReferenceTypeAtomPtr dpnd;
	MP4Track theTrack;
	MP4Media theMedia;
	u32 sampleDescIndex;
	
	err = MP4GetMovieTrack( theMovie, trackNumber, &theTrack ); if (err) goto bail;
	err = MP4GetTrackMedia( theTrack, &theMedia ); if (err) goto bail;
	err = MP4GetMediaSampleDescIndex( theMedia, cts, &sampleDescIndex ); if (err) goto bail;
	err = MP4GetMediaESD( theMedia, sampleDescIndex, &esdInFile, 0 ); if (err) goto bail;
	/* 
	err = MP4CreateES_Descriptor( MP4ES_DescriptorTag, 0, 0, (MP4DescriptorPtr*) &esd ); if (err) goto bail;
	*esd = *esdInFile; 
	*/
	/* We don't want a shallow copy, we want a deep copy */
	err = MP4CopyDescriptor( (MP4DescriptorPtr) esdInFile, (MP4DescriptorPtr*) &esd ); if (err) goto bail;
	
	esd->ESID = trackNumber;

	/* stream dependancy */
	err = MP4GetTrackReferenceType( theTrack, MP4StreamDependenceAtomType, &dpnd ); if (err) goto bail;
	if ( dpnd && (dpnd->trackIDCount) )
		esd->dependsOnES = dpnd->trackIDs[0];

	/* JLF 11/00 */
	/* OCR dependancy */
	err = MP4GetTrackReferenceType( theTrack, MP4SyncTrackReferenceAtomType, &dpnd ); if (err) goto bail;
	if ( dpnd && (dpnd->trackIDCount) )
		esd->OCRESID = dpnd->trackIDs[0];
		
	/*
		handle SLConfig
	*/
	if ( inSLConfig )
	{
        u32 movieTimeScale;
        u32 mediaTimeScale;

    	slconfig = (MP4SLConfigDescriptorPtr) malloc( sizeof(MP4SLConfigDescriptor) );
        if ( slconfig == NULL )
        {
           BAILWITHERROR( MP4NoMemoryErr );
        }
        *slconfig = *( (MP4SLConfigDescriptorPtr) inSLConfig);
        err = MP4GetMovieTimeScale( theMovie, &movieTimeScale ); if (err) goto bail;
        err = MP4GetMediaTimeScale( theMedia, &mediaTimeScale ); if (err) goto bail;    		
        slconfig->timestampResolution          = mediaTimeScale;
        slconfig->OCRResolution                = movieTimeScale;
        slconfig->timeScale                    = mediaTimeScale;
        slconfig->AUDuration                   = mediaTimeScale;
        slconfig->CUDuration                   = mediaTimeScale;
	}
	else
	{
    	slconfig = (MP4SLConfigDescriptorPtr) esd->slConfig;
    	if ( slconfig == NULL )
    		BAILWITHERROR( MP4InvalidMediaErr );
    	if ( slconfig->predefined == SLConfigPredefinedMP4 )
    	{
    		u32 movieTimeScale;
    		u32 mediaTimeScale;
    		
    		err = MP4GetMovieTimeScale( theMovie, &movieTimeScale ); if (err) goto bail;
    		err = MP4GetMediaTimeScale( theMedia, &mediaTimeScale ); if (err) goto bail;    		
    		err = MP4CreateSLConfigDescriptor( MP4SLConfigDescriptorTag, 0, 0,
                                               (MP4DescriptorPtr *) &slconfig ); if (err) goto bail;
            slconfig->predefined                   = 0;
            slconfig->useAccessUnitStartFlag       = 0;
            slconfig->useAccessUnitEndFlag         = 0;
            slconfig->useRandomAccessPointFlag     = 1;
            slconfig->useRandomAccessUnitsOnlyFlag = 0;
            slconfig->usePaddingFlag               = 0;
            slconfig->useTimestampsFlag            = 1;
            slconfig->useIdleFlag                  = 0;
            slconfig->durationFlag                 = 0;
            slconfig->timestampResolution          = mediaTimeScale;
            slconfig->OCRResolution                = movieTimeScale;
            slconfig->timestampLength              = 32;
            slconfig->OCRLength                    = 0;
            slconfig->AULength                     = 0;
            slconfig->instantBitrateLength         = 0;
            slconfig->degradationPriorityLength    = 0;
            slconfig->AUSeqNumLength               = 0;
            slconfig->packetSeqNumLength           = 5;
            slconfig->timeScale                    = mediaTimeScale;
            slconfig->AUDuration                   = mediaTimeScale;
            slconfig->CUDuration                   = mediaTimeScale;
            slconfig->startDTS                     = 0;
            slconfig->startCTS                     = 0;
    	}
	}
	
	if (esd->slConfig) {
		esd->slConfig->destroy( esd->slConfig );
	}
	esd->slConfig = (MP4DescriptorPtr) slconfig;
	
	*outDesc = (MP4DescriptorPtr) esd;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetMovieIODInlineProfileFlag( MP4Movie theMovie, u8* outFlag )
{
	MP4InitialObjectDescriptorPtr iod;
	GETIODATOM( theMovie );

	if ( outFlag == 0 )
	{
		 BAILWITHERROR( MP4BadParamErr );
	}
	if ( iodAtom->ODSize == 0)
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	iod = (MP4InitialObjectDescriptorPtr) iodAtom->descriptor;
	if ( iod == 0 )
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	/* jlf 12/00: escape sequence for OD */
	if (iod->tag == MP4_OD_Tag) return MP4HasRootOD;

	*outFlag = (iod->inlineProfileFlag != 0);
bail:
	TEST_RETURN( err );
	return err;	
}

MP4_EXTERN ( MP4Err )
MP4SetMovieIODInlineProfileFlag( MP4Movie theMovie, u8 theFlag )
{
	MP4InitialObjectDescriptorPtr iod;
	GETIODATOM( theMovie );

	if ( iodAtom->ODSize == 0)
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	iod = (MP4InitialObjectDescriptorPtr) iodAtom->descriptor;
	if ( iod == 0 )
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	/* jlf 12/00: escape sequence for OD */
	if ((iod->tag == MP4_OD_Tag) || (iod->tag == MP4ObjectDescriptorTag)) return MP4HasRootOD;
	iod->inlineProfileFlag = (theFlag != 0);

bail:
	TEST_RETURN( err );
	return err;	
}

MP4_EXTERN ( MP4Err )
MP4GetMovieProfilesAndLevels( MP4Movie theMovie, u8 *outOD, u8 *outScene, 
                    u8 *outAudio, u8 *outVisual, u8 *outGraphics  )
{
	MP4InitialObjectDescriptorPtr iodDesc;
	GETIODATOM( theMovie );

	if ( iodAtom->ODSize == 0)
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	iodDesc = (MP4InitialObjectDescriptorPtr) iodAtom->descriptor;
	if ( iodDesc == 0 )
	{
	    BAILWITHERROR( MP4InvalidMediaErr );
	}
	/* jlf 12/00: escape sequence for OD */
	if ((iodDesc->tag == MP4_OD_Tag) || (iodDesc->tag == MP4ObjectDescriptorTag)) return MP4HasRootOD;

	if ( outOD )
	{
	    *outOD = (u8) iodDesc->OD_profileAndLevel;
	}
	if ( outScene )
	{
	    *outScene = (u8) iodDesc->scene_profileAndLevel;
	}
	if ( outAudio )
	{
	    *outAudio = (u8) iodDesc->audio_profileAndLevel;
	}
	if ( outVisual )
	{
	    *outVisual = (u8) iodDesc->visual_profileAndLevel;
	}
	if ( outGraphics )
	{
	    *outGraphics = (u8) iodDesc->graphics_profileAndLevel;
	}
bail:
	TEST_RETURN( err );
	return err;	
}

MP4_EXTERN ( MP4Err )
MP4GetMovieInitialObjectDescriptor( MP4Movie theMovie, MP4Handle outDescriptorH )
{
    return MP4GetMovieInitialObjectDescriptorUsingSLConfig( theMovie, 0, outDescriptorH );
}

MP4_EXTERN ( MP4Err )
MP4GetMovieInitialObjectDescriptorUsingSLConfig( MP4Movie theMovie, MP4SLConfig slconfig, MP4Handle outDescriptorH )
{
	MP4Err MP4CreateSLConfigDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
	MP4Err MP4GetMediaESD( MP4Media theMedia, u32 index, MP4ES_DescriptorPtr *outESD, u32 *outDataReferenceIndex );
	MP4Err MP4GetMovieObjectDescriptorUsingSLConfig( MP4Movie theMovie, MP4SLConfig slconfig, MP4Handle outDescriptorH );
	
	MP4InitialObjectDescriptorPtr iodDesc;
    MP4LinkedList incDescriptors;
	GETIODATOM( theMovie );

	if (iodAtom && ( iodAtom->ODSize ))
	{
		u32 count;
		u32 i;
		u32 trackCount;

		/* jlf 11/00 : if this is an OD, extract directly the OD. */
		if (iodAtom->descriptor->tag == MP4_OD_Tag) {
			return MP4GetMovieObjectDescriptorUsingSLConfig(theMovie, slconfig, outDescriptorH);
		}

		err = MP4GetMovieTrackCount( theMovie, &trackCount ); if (err) goto bail;
		iodDesc = (MP4InitialObjectDescriptorPtr) iodAtom->descriptor;
		err = MP4GetListEntryCount( iodDesc->ES_ID_IncDescriptors, &count ); if (err) goto bail;
		
		/*
			get and rewrite ES_Descriptors, placing each in iodDesc
		*/
		for ( i = 0; i < count; i++ )
		{
			
			MP4ES_ID_IncDescriptorPtr inc;
			MP4ES_DescriptorPtr       esd;
			
			err = MP4GetListEntry( iodDesc->ES_ID_IncDescriptors, i, (char **) &inc ); if (err) goto bail;				
			err = makeESD( theMovie, inc->trackID, 0, slconfig, (MP4DescriptorPtr *) &esd ); if (err) goto bail;			
			/* add esd to iodDesc */
			err = iodDesc->addDescriptor( (MP4DescriptorPtr) iodDesc, (MP4DescriptorPtr) esd ); if (err) goto bail;
		}
		/*
          err = MP4DeleteLinkedList( iodDesc->ES_ID_IncDescriptors ); if (err) goto bail;
        */
        incDescriptors = iodDesc->ES_ID_IncDescriptors;
		iodDesc->ES_ID_IncDescriptors = NULL;
		iodDesc->tag = MP4InitialObjectDescriptorTag;
		err = iodDesc->calculateSize( (MP4DescriptorPtr) iodDesc ); if (err) goto bail;
		err = MP4SetHandleSize( outDescriptorH, iodDesc->size ); if (err) goto bail;
		err = iodDesc->serialize((MP4DescriptorPtr) iodDesc, *outDescriptorH ); if (err) goto bail;
        err = iodDesc->removeESDS( (MP4DescriptorPtr) iodDesc ); if (err) goto bail;
        iodDesc->ES_ID_IncDescriptors = incDescriptors;
        iodDesc->tag = MP4_IOD_Tag;
	}
	else err = MP4BadDataErr;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetMovieIndTrack( MP4Movie theMovie, u32 trackIndex, MP4Track *outTrack )
{
	MP4AtomPtr aTrack;
	GETMOVIEATOM( theMovie );
	if ( (trackIndex == 0) || (trackIndex > movieAtom->getTrackCount(movieAtom)) )
		BAILWITHERROR( MP4BadParamErr )
	err = movieAtom->getIndTrack( movieAtom, trackIndex, &aTrack ); if (err) goto bail;
	if ( aTrack == NULL )
		BAILWITHERROR( MP4BadDataErr )
	*outTrack = (MP4Track) aTrack;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetMovieTrack( MP4Movie theMovie, u32 trackID, MP4Track *outTrack )
{
	u32 i;
	u32 trackCount;
	
	MP4Err err;
	err = MP4GetMovieTrackCount( theMovie, &trackCount ); if (err) goto bail;
	if ( trackCount == 0 )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	for ( i = 1; i <= trackCount; i++ )
	{
		MP4Track t;
		u32  id;
		err = MP4GetMovieIndTrack( theMovie, i, &t ); if (err) goto bail;
		err = MP4GetTrackID( t, &id ); if (err) goto bail;
		if ( id == trackID )
		{
			*outTrack = t;
			break;
		}
		else if ( i == trackCount )
			err = MP4BadParamErr;
	}
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4SetMovieTimeScale( MP4Movie theMovie, u32 timeScale )
{
	GETMOVIEATOM( theMovie );
	if ( timeScale == 0 )
		BAILWITHERROR( MP4BadParamErr );
	assert( movieAtom->setTimeScale );
	err = movieAtom->setTimeScale( movieAtom, timeScale ); if (err) goto bail;
bail:
	return err;
}

ISO_EXTERN ( ISOErr )
MJ2SetMovieMatrix( ISOMovie theMovie, u32 matrix[9] )
{
	u32 aMatrix[9];
	GETMOVIEATOM( theMovie );
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
	err = movieAtom->setMatrix( movieAtom, aMatrix );
bail:
	return err;
}

ISO_EXTERN ( ISOErr )
MJ2GetMovieMatrix( ISOMovie theMovie, u32 outMatrix[9] )
{
	GETMOVIEATOM( theMovie );
	if ( outMatrix == NULL )
		BAILWITHERROR( MP4BadParamErr );
	assert( movieAtom->getMatrix );
	err = movieAtom->getMatrix( movieAtom, outMatrix );
bail:
	return err;
}

ISO_EXTERN ( ISOErr )
MJ2SetMoviePreferredRate( ISOMovie theMovie, u32 rate )
{
	GETMOVIEATOM( theMovie );
	assert( movieAtom->setPreferredRate );
	err = movieAtom->setPreferredRate( movieAtom, rate );
bail:
	return err;
}

ISO_EXTERN ( ISOErr )
MJ2GetMoviePreferredRate( ISOMovie theMovie, u32 *outRate )
{
	GETMOVIEATOM( theMovie );
	if ( outRate == NULL )
		BAILWITHERROR( MP4BadParamErr );
	assert( movieAtom->getPreferredRate );
	err = movieAtom->getPreferredRate( movieAtom, outRate );
bail:
	return err;
}

ISO_EXTERN ( ISOErr )
MJ2SetMoviePreferredVolume( ISOMovie theMovie, s16 volume )
{
	GETMOVIEATOM( theMovie );
	assert( movieAtom->setPreferredVolume );
	err = movieAtom->setPreferredVolume( movieAtom, volume );
bail:
	return err;
}

ISO_EXTERN ( ISOErr )
MJ2GetMoviePreferredVolume( ISOMovie theMovie, s16 *outVolume )
{
	GETMOVIEATOM( theMovie );
	if ( outVolume == NULL )
		BAILWITHERROR( MP4BadParamErr );
	assert( movieAtom->getPreferredVolume );
	err = movieAtom->getPreferredVolume( movieAtom, outVolume );
bail:
	return err;
}

ISO_EXTERN ( ISOErr )
ISOSetMovieBrand( ISOMovie theMovie, u32 brand, u32 minorversion )
{
	ISOFileTypeAtomPtr ftyp;
	GETMOOV( theMovie );

	ftyp = (ISOFileTypeAtomPtr)moov->ftyp;
	if (ftyp)
		err = ftyp->setBrand( ftyp, brand, minorversion );
		else BAILWITHERROR( MP4NoQTAtomErr )
bail:
	TEST_RETURN( err );
	return err;	
}

ISO_EXTERN ( ISOErr )
ISOSetMovieCompatibleBrand( ISOMovie theMovie, u32 brand )
{
	ISOFileTypeAtomPtr ftyp;
	GETMOOV( theMovie );
	ftyp = (ISOFileTypeAtomPtr) moov->ftyp;
	if (ftyp)
		err = ftyp->addStandard( ftyp, brand );
		else BAILWITHERROR( MP4NoQTAtomErr )
bail:
	TEST_RETURN( err );
	return err;	
}

ISO_EXTERN ( ISOErr )
ISOGetMovieBrand( ISOMovie theMovie, u32* brand, u32* minorversion )
{
	ISOFileTypeAtomPtr ftyp;
	GETMOOV( theMovie );

	ftyp = (ISOFileTypeAtomPtr)moov->ftyp;
	if (ftyp)
		err = ftyp->getBrand( ftyp, brand, minorversion );
		else BAILWITHERROR( MP4NoQTAtomErr )
bail:
	TEST_RETURN( err );
	return err;	
}

ISO_EXTERN ( u32 )
ISOIsMovieCompatibleBrand( ISOMovie theMovie, u32 brand )
{
	ISOFileTypeAtomPtr ftyp;
	u32 outval = 0;
	
	GETMOOV( theMovie );
	ftyp = (ISOFileTypeAtomPtr) moov->ftyp;
	if (ftyp)
		outval = ftyp->getStandard( ftyp, brand );
		else BAILWITHERROR( MP4NoQTAtomErr )
bail:
	TEST_RETURN( err );
	return outval;	
}

MP4_EXTERN ( MP4Err )
MP4NewMovieTrack( MP4Movie theMovie, u32 newTrackFlags, MP4Track *outTrack )
{
	MP4TrackAtomPtr trak;
	GETMOVIEATOM( theMovie );
	err = movieAtom->newTrack( movieAtom, newTrackFlags, (MP4AtomPtr*) &trak ); if (err) goto bail;
	err = trak->setMdat( trak, moov->mdat ); if (err) goto bail;
	trak->moov = moov;
	*outTrack = (MP4Track) trak;
	
bail:
	TEST_RETURN( err );
	return err;
}

MP4_EXTERN ( MP4Err )
MP4NewMovieTrackWithID( MP4Movie theMovie, u32 newTrackFlags, u32 newTrackID, MP4Track *outTrack )
{
	MP4TrackAtomPtr trak;
	GETMOVIEATOM( theMovie );
	err = movieAtom->newTrackWithID( movieAtom, newTrackFlags, newTrackID, (MP4AtomPtr*) &trak ); if (err) goto bail;
	err = trak->setMdat( trak, moov->mdat ); if (err) goto bail;
	trak->moov = moov;
	*outTrack = (MP4Track) trak;
bail:
	TEST_RETURN( err );
	return err;
}

#if 0
MP4_EXTERN ( MP4Err )
MP4GetMovieInitialBIFSTrack( MP4Movie theMovie, MP4Track *outBIFSTrack )
{
	GETMOVIEATOM( theMovie );
	if ( moov->initialBIFS == NULL )
	{
		err = MP4NewMovieTrack( theMovie, MP4NewTrackIsVisual, &moov->initialBIFS ); if (err) goto bail;
	}
	*outBIFSTrack = moov->initialBIFS;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4SetMovieInitialBIFSTrack( MP4Movie theMovie, MP4Track theBIFSTrack )
{
	GETMOVIEATOM( theMovie );
	moov->initialBIFS = theBIFSTrack;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4SetMovieInitialODTrack( MP4Movie theMovie, MP4Track theODTrack )
{
	GETMOVIEATOM( theMovie );
	moov->initialOD = theODTrack;
bail:
	TEST_RETURN( err );

	return err;
}
#endif

MP4_EXTERN ( MP4Err )
MP4NewMovie( MP4Movie *outMovie, u32 initialODID,
						   u8 OD_profileAndLevel, u8 scene_profileAndLevel,
                           u8 audio_profileAndLevel, u8 visual_profileAndLevel,
                           u8 graphics_profileAndLevel )
{
	return MP4NewMovieExt( outMovie, initialODID, 
			OD_profileAndLevel, scene_profileAndLevel,
			audio_profileAndLevel, visual_profileAndLevel,
			graphics_profileAndLevel,
			"",
			(u8) ( ((OD_profileAndLevel == 0) && (scene_profileAndLevel == 0) &&
			 (audio_profileAndLevel == 0) && (visual_profileAndLevel == 0) &&
			 (graphics_profileAndLevel == 0)) ? 1 : 0 ) );
			 
#if 0	
	MP4Err MP4CreateInitialObjectDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
	MP4Err MP4CreateMediaDataAtom( MP4MediaDataAtomPtr *outAtom );
	MP4Err MP4CreateMovieAtom( MP4MovieAtomPtr *outAtom );
	MP4Err MP4CreateMovieHeaderAtom( MP4MovieHeaderAtomPtr *outAtom );
	MP4Err MP4CreateObjectDescriptorAtom( MP4ObjectDescriptorAtomPtr *outAtom );
	   
	u64 now;
	MP4Err err;
	MP4PrivateMovieRecordPtr movie;
	MP4MovieAtomPtr            moov;
	MP4MovieHeaderAtomPtr      mvhd;
	MP4ObjectDescriptorAtomPtr iods;
	MP4MediaDataAtomPtr        mdat;
	MP4InitialObjectDescriptorPtr iod;
	ISOFileTypeAtomPtr		   ftyp;

	movie = (MP4PrivateMovieRecordPtr) calloc( 1, sizeof(MP4PrivateMovieRecord) );
	if ( movie == NULL )
	{
		err = MP4NoMemoryErr;
		goto bail;
	}
	movie->referenceCount     = 1;
	movie->prepend_handle	  = NULL;
	err = MP4GetCurrentTime( &now ); if (err) goto bail;
	err = MP4CreateMovieAtom( (MP4MovieAtomPtr *) &movie->moovAtomPtr ); if (err) goto bail;
	moov = (MP4MovieAtomPtr) movie->moovAtomPtr;
	err = MP4CreateMovieHeaderAtom( &mvhd ); if (err) goto bail;
	mvhd->nextTrackID = 1;
	mvhd->creationTime = now;
	mvhd->modificationTime = now;
	err = moov->addAtom( moov, (MP4AtomPtr) mvhd ); if (err) goto bail;
	moov->setTimeScale( moov, 600 );
	err = MP4CreateObjectDescriptorAtom( &iods ); if (err) goto bail;
	err = MP4CreateInitialObjectDescriptor( MP4_IOD_Tag, 0, 0, (MP4DescriptorPtr*)&iod ); if (err) goto bail;
	iod->objectDescriptorID = initialODID;
	iod->OD_profileAndLevel = OD_profileAndLevel;
	iod->scene_profileAndLevel = scene_profileAndLevel;
	iod->audio_profileAndLevel = audio_profileAndLevel;
	iod->visual_profileAndLevel = visual_profileAndLevel;
	iod->graphics_profileAndLevel = graphics_profileAndLevel;
	err = iods->setDescriptor( (MP4AtomPtr) iods, (char*) iod ); if (err) goto bail;
	err = moov->addAtom( moov, (MP4AtomPtr) iods ); if (err) goto bail;
	err = MP4CreateMediaDataAtom( &mdat );
	movie->mdat = (MP4AtomPtr) mdat;
	{
	   MP4DataHandlerPtr dh;
	      err = MP4CreateMdatDataHandler( mdat, &dh ); if (err) goto bail;
	      movie->inMemoryDataHandler = dh;
	}
	movie->fileType = ISOMPEG4FileType;
	
	err = MJ2CreateFileTypeAtom( &ftyp ); if (err) goto bail;
	err = ftyp->setBrand( ftyp, ISOMpeg4V2Brand, 0 );  if (err) goto bail;
	err = ftyp->addStandard( ftyp, ISOISOBrand );  if (err) goto bail;
	err = ftyp->calculateSize( (MP4AtomPtr) ftyp ); if (err) goto bail;
	movie->ftyp = (MP4AtomPtr) ftyp;
	
	*outMovie = (MP4Movie) movie;
bail:
	TEST_RETURN( err );

	return err;
#endif	
}

MP4Err MP4MovieCreateDescriptors( MP4Movie theMovie )
{
	MP4Err MP4CreateES_ID_IncDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
	MP4ObjectDescriptorAtomPtr iods;
	MP4DescriptorPtr desc;
    MP4PrivateMovieRecordPtr movieRecord;
	
	GETMOVIEATOM( theMovie );
	
	/* return with no error for non-MPEG-4 movies */
    movieRecord = (MP4PrivateMovieRecordPtr) theMovie;
	if (movieRecord->fileType != ISOMPEG4FileType)
		return ISONoErr;

	err = movieAtom->calculateDuration( movieAtom ); if (err) goto bail;

	iods = (MP4ObjectDescriptorAtomPtr) movieAtom->iods;
	if ( iods == NULL )
		BAILWITHERROR(  MP4NoErr  );   /* was MP4InvalidMediaErr -- iods is now optional */
	desc = (MP4DescriptorPtr) iods->descriptor;
	if ( desc == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	if ( (desc->tag != MP4_IOD_Tag) && (desc->tag != MP4_OD_Tag))
		BAILWITHERROR( MP4InvalidMediaErr );

#if 0
	if ( moov->initialBIFS )
	{
		MP4ES_ID_IncDescriptorPtr inc;
		err = MP4CreateES_ID_IncDescriptor( MP4ES_ID_IncDescriptorTag, 0, 0, (MP4DescriptorPtr*) &inc ); if (err) goto bail;
		err = MP4GetTrackID( moov->initialBIFS, &inc->trackID ); if (err) goto bail;
		err = iod->addDescriptor( (MP4DescriptorPtr) iod, (MP4DescriptorPtr) inc ); if (err) goto bail;
	}
	if ( moov->initialOD )
	{
		MP4ES_ID_IncDescriptorPtr inc;
		err = MP4CreateES_ID_IncDescriptor( MP4ES_ID_IncDescriptorTag, 0, 0, (MP4DescriptorPtr*) &inc ); if (err) goto bail;
		err = MP4GetTrackID( moov->initialOD, &inc->trackID ); if (err) goto bail;
		err = iod->addDescriptor( (MP4DescriptorPtr) iod, (MP4DescriptorPtr) inc ); if (err) goto bail;
	}
#endif
bail:
	TEST_RETURN( err );

	return err;	
}

MP4Err MP4MovieAddTrackES_IDToIOD( MP4Movie theMovie, MP4Track theTrack )
{
	MP4Err MP4CreateES_ID_IncDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
	MP4ObjectDescriptorAtomPtr iods;
	MP4InitialObjectDescriptorPtr iod;
	MP4ES_ID_IncDescriptorPtr inc;
	
	GETMOVIEATOM( theMovie );
	if ( theTrack == 0 )
		BAILWITHERROR( MP4BadParamErr );
	iods = (MP4ObjectDescriptorAtomPtr) movieAtom->iods;
	if ( iods == NULL )
		BAILWITHERROR( MP4InvalidMediaErr );
	iod = (MP4InitialObjectDescriptorPtr) iods->descriptor;
	if ( (iod == NULL) || (iod->ES_ID_IncDescriptors == NULL) )
		BAILWITHERROR( MP4InvalidMediaErr );

	err = MP4CreateES_ID_IncDescriptor( MP4ES_ID_IncDescriptorTag, 0, 0, (MP4DescriptorPtr*) &inc );
    if (err) goto bail;
	err = MP4GetTrackID( theTrack, &inc->trackID );
    if (err) goto bail;
	err = iod->addDescriptor( (MP4DescriptorPtr) iod, (MP4DescriptorPtr) inc );
    if (err) goto bail;
bail:
	TEST_RETURN( err );
	return err;	
}

#define MAC_EPOCH 2082758400;

MP4Err MP4GetCurrentTime( u64 *outTime )
{
	MP4Err err;
	unsigned long calctime;
	u64 ret;
	err = MP4NoErr;
	if ( outTime == NULL )
		BAILWITHERROR( MP4BadParamErr )
#ifdef macintosh
	GetDateTime( &calctime );
#else
	{
		time_t t;
		t = time( NULL );
		calctime = t + MAC_EPOCH;
	}
#endif
	ret = calctime;
	*outTime = ret;
bail:
	TEST_RETURN( err );

	return err;
}

ISO_EXTERN ( MP4Err )
NewMPEG21( MP4Movie *outMovie )
{
	return ISONewMetaMovie( outMovie, ISOMPEG21Brand, ISOMPEG21Brand, 0);
}

ISO_EXTERN (MP4Err)
ISONewMetaMovie( MP4Movie *outMovie, u32 handlertype, u32 brand, u32 minorversion )
{
	MP4Err MP4CreateMediaDataAtom( MP4MediaDataAtomPtr *outAtom );
	MP4Err err;
	MP4PrivateMovieRecordPtr movie;
	ISOFileTypeAtomPtr		ftyp;
	MP4MediaDataAtomPtr     mdat;
	ISOMeta					meta;
		
	movie = (MP4PrivateMovieRecordPtr) calloc( 1, sizeof(MP4PrivateMovieRecord) );
	TESTMALLOC(movie);
	movie->referenceCount     = 1;
	movie->prepend_handle	  = NULL;

	err = MP4CreateMediaDataAtom( &mdat );
	movie->mdat = (MP4AtomPtr) mdat;
	{
	   MP4DataHandlerPtr dh;
	      err = MP4CreateMdatDataHandler( mdat, &dh ); if (err) goto bail;
	      movie->inMemoryDataHandler = dh;
	}
	movie->fileType = ISOMPEG21FileType;
	err = MJ2CreateFileTypeAtom( &ftyp ); if (err) goto bail;
	err = ftyp->setBrand( ftyp, brand, minorversion );  if (err) goto bail;
	err = ftyp->calculateSize( (MP4AtomPtr) ftyp ); if (err) goto bail;
	movie->ftyp = (MP4AtomPtr) ftyp;
	
	err = ISONewFileMeta( (MP4Movie) movie, handlertype, &meta); if (err) goto bail;

	*outMovie = (MP4Movie) movie;
bail:
	TEST_RETURN( err );

	return err;	
}

/* Til end of file: JLF, 11/00 : modif to support OD in the iods atom (creation and extraction) */

MP4_EXTERN ( MP4Err )
MP4NewMovieExt( MP4Movie *outMovie, u32 initialODID,
						   u8 OD_profileAndLevel, u8 scene_profileAndLevel,
                           u8 audio_profileAndLevel, u8 visual_profileAndLevel,
                           u8 graphics_profileAndLevel,
						   char *url,
						   u8 IsExchangeFile)
{
	MP4Err MP4CreateInitialObjectDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
	MP4Err MP4CreateObjectDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
	MP4Err MP4CreateMediaDataAtom( MP4MediaDataAtomPtr *outAtom );
	MP4Err MP4CreateMovieAtom( MP4MovieAtomPtr *outAtom );
	MP4Err MP4CreateMovieHeaderAtom( MP4MovieHeaderAtomPtr *outAtom );
	MP4Err MP4CreateObjectDescriptorAtom( MP4ObjectDescriptorAtomPtr *outAtom );
	   
	u64 now;
	MP4Err err;
	MP4PrivateMovieRecordPtr movie;
	MP4MovieAtomPtr            moov;
	MP4MovieHeaderAtomPtr      mvhd;
	MP4ObjectDescriptorAtomPtr iods;
	MP4MediaDataAtomPtr        mdat;
	MP4InitialObjectDescriptorPtr iod;
	MP4ObjectDescriptorPtr od;
	ISOFileTypeAtomPtr		ftyp;

	movie = (MP4PrivateMovieRecordPtr) calloc( 1, sizeof(MP4PrivateMovieRecord) );
	if ( movie == NULL )
	{
		err = MP4NoMemoryErr;
		goto bail;
	}
	movie->referenceCount     = 1;
	movie->prepend_handle	  = NULL;
	err = MP4GetCurrentTime( &now ); if (err) goto bail;
	err = MP4CreateMovieAtom( (MP4MovieAtomPtr *) &movie->moovAtomPtr ); if (err) goto bail;
	moov = (MP4MovieAtomPtr) movie->moovAtomPtr;
	err = MP4CreateMovieHeaderAtom( &mvhd ); if (err) goto bail;
	mvhd->nextTrackID = 1;
	mvhd->creationTime = now;
	mvhd->modificationTime = now;
	err = moov->addAtom( moov, (MP4AtomPtr) mvhd ); if (err) goto bail;
	moov->setTimeScale( moov, 600 );
	
	if (initialODID != 0) {
		err = MP4CreateObjectDescriptorAtom( &iods ); if (err) goto bail;

		/* here we have fun */
		if (IsExchangeFile) {
			err = MP4CreateObjectDescriptor(MP4_OD_Tag, 0, 0, (MP4DescriptorPtr*)&od ); if (err) goto bail;
			od->objectDescriptorID = initialODID;
			if (strlen(url)) {
				od->URLStringLength = strlen(url);
				od->URLString = (char*) malloc(sizeof(char) * od->URLStringLength);
				strcpy(od->URLString, url);
			} else {
				od->URLStringLength = 0;
			}
			err = iods->setDescriptor( (MP4AtomPtr) iods, (char*) od ); if (err) goto bail;
		} else {
			err = MP4CreateInitialObjectDescriptor( MP4_IOD_Tag, 0, 0, (MP4DescriptorPtr*)&iod ); if (err) goto bail;
			iod->objectDescriptorID = initialODID;
			iod->OD_profileAndLevel = OD_profileAndLevel;
			iod->scene_profileAndLevel = scene_profileAndLevel;
			iod->audio_profileAndLevel = audio_profileAndLevel;
			iod->visual_profileAndLevel = visual_profileAndLevel;
			iod->graphics_profileAndLevel = graphics_profileAndLevel;
			if (strlen(url)) {
				iod->URLStringLength = strlen(url);
				iod->URLString = (char*) malloc(sizeof(char) * iod->URLStringLength);
				strcpy(iod->URLString, url);
			} else {
				iod->URLStringLength = 0;
			}
			err = iods->setDescriptor( (MP4AtomPtr) iods, (char*) iod ); if (err) goto bail;
		}
		err = moov->addAtom( moov, (MP4AtomPtr) iods ); if (err) goto bail;
	}
	err = MP4CreateMediaDataAtom( &mdat );
	movie->mdat = (MP4AtomPtr) mdat;
	{
	   MP4DataHandlerPtr dh;
	      err = MP4CreateMdatDataHandler( mdat, &dh ); if (err) goto bail;
	      movie->inMemoryDataHandler = dh;
	}
	movie->fileType = ISOMPEG4FileType;

	err = MJ2CreateFileTypeAtom( &ftyp ); if (err) goto bail;
	err = ftyp->setBrand( ftyp, ISOMpeg4V2Brand, 0 );  if (err) goto bail;
	err = ftyp->addStandard( ftyp, ISOISOBrand );  if (err) goto bail;
	err = ftyp->calculateSize( (MP4AtomPtr) ftyp ); if (err) goto bail;
	movie->ftyp = (MP4AtomPtr) ftyp;

	*outMovie = (MP4Movie) movie;
bail:
	TEST_RETURN( err );

	return err;	
}


/* jlf 11/00: Handle ObjectDescriptor in the iods atom for exchange files */
MP4Err 
MP4GetMovieObjectDescriptorUsingSLConfig( MP4Movie theMovie, MP4SLConfig slconfig, MP4Handle outDescriptorH )
{
	MP4Err MP4CreateSLConfigDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc );
	MP4Err MP4GetMediaESD( MP4Media theMedia, u32 index, MP4ES_DescriptorPtr *outESD, u32 *outDataReferenceIndex );
	
	MP4ObjectDescriptorPtr odDesc;
    MP4LinkedList incDescriptors;
	GETIODATOM( theMovie );

	if ( iodAtom->ODSize ) {
		u32 count;
		u32 i;
		u32 trackCount;

		err = MP4GetMovieTrackCount( theMovie, &trackCount ); if (err) goto bail;
		odDesc = (MP4ObjectDescriptorPtr) iodAtom->descriptor;
		
		/* check that there's no ES_ID_Ref */
		err = MP4GetListEntryCount( odDesc->ES_ID_RefDescriptors, &count ); if (err) goto bail;
		if (count) BAILWITHERROR( MP4BadDataErr );

		err = MP4GetListEntryCount( odDesc->ES_ID_IncDescriptors, &count ); if (err) goto bail;
		
		/*
			get and rewrite ES_Descriptors, placing each in odDesc
		*/
		for ( i = 0; i < count; i++ )
		{
			
			MP4ES_ID_IncDescriptorPtr inc;
			MP4ES_DescriptorPtr       esd;
			
			err = MP4GetListEntry( odDesc->ES_ID_IncDescriptors, i, (char **) &inc); if (err) goto bail;				
			err = makeESD( theMovie, inc->trackID, 0, slconfig, (MP4DescriptorPtr *) &esd ); if (err) goto bail;			
			/* add esd to odDesc */
			err = odDesc->addDescriptor( (MP4DescriptorPtr) odDesc, (MP4DescriptorPtr) esd ); if (err) goto bail;
		}
        incDescriptors = odDesc->ES_ID_IncDescriptors;
		odDesc->ES_ID_IncDescriptors = NULL;
		odDesc->tag = MP4ObjectDescriptorTag;
		err = odDesc->calculateSize( (MP4DescriptorPtr) odDesc ); if (err) goto bail;
		err = MP4SetHandleSize( outDescriptorH, odDesc->size ); if (err) goto bail;
		err = odDesc->serialize((MP4DescriptorPtr) odDesc, *outDescriptorH ); if (err) goto bail;
        err = odDesc->removeESDS( (MP4DescriptorPtr) odDesc ); if (err) goto bail;
        odDesc->ES_ID_IncDescriptors = incDescriptors;
        odDesc->tag = MP4_OD_Tag;
	}
bail:
	TEST_RETURN( err );

	return err;
}


/* JLF 11/00 : add descriptor to IOD/OD of the movie */
MP4_EXTERN ( MP4Err ) 
MP4AddDescToMovieIOD(MP4Movie theMovie, MP4Handle descriptorH)
{
	MP4Err MP4CreateMemoryInputStream( char *base, u32 size, MP4InputStreamPtr *outStream );

	MP4InitialObjectDescriptorPtr iod;
	MP4ObjectDescriptorPtr od;
	MP4InputStreamPtr is;
	MP4DescriptorPtr desc;
	u32 size;

	GETIODATOM( theMovie );

	/* parse our descriptor... */
	err = MP4GetHandleSize( descriptorH, &size ); if (err) goto bail;
	err = MP4CreateMemoryInputStream(*descriptorH, size, &is ); if (err) goto bail;
	is->debugging = 0;
	err = MP4ParseDescriptor( is, (MP4DescriptorPtr *)&desc); if (err) goto bail;
	is->destroy(is);

	/* quick check for the desc type */
	switch (desc->tag) {
	case MP4ForbiddenZeroDescriptorTag:
	case MP4ObjectDescriptorTag:
	case MP4InitialObjectDescriptorTag:
	case MP4ES_DescriptorTag:
	case MP4DecoderConfigDescriptorTag:
	case MP4DecSpecificInfoDescriptorTag:
	case MP4SLConfigDescriptorTag:
	case MP4IPMP_DescriptorTag:
	case MPM4QoS_DescriptorTag:
	case MP4ES_ID_IncDescriptorTag:
	case MP4ES_ID_RefDescriptorTag:
	case MP4_IOD_Tag:
	case MP4_OD_Tag:
		return MP4BadParamErr;
	}

	switch (iodAtom->descriptor->tag) {
	case MP4_IOD_Tag:
		iod = (MP4InitialObjectDescriptorPtr) iodAtom->descriptor;
		err = iod->addDescriptor((MP4DescriptorPtr) iod, desc);
		if (err) goto bail;
		break;

	case MP4_OD_Tag:
		od = (MP4ObjectDescriptorPtr) iodAtom->descriptor;
		err = od->addDescriptor((MP4DescriptorPtr) od, desc);
		if (err) goto bail;
		break;
	default:
		return MP4InvalidMediaErr;
	}

bail:
	return err;
}
