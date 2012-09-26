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
	$Id: MP4TrackReader.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4TrackReader.h"
#include "MP4Impl.h"
#include <stdlib.h>

MP4Err MP4CreateOrdinaryTrackReader( MP4Movie theMovie, MP4Track theTrack, MP4TrackReaderPtr *outReader );
MP4Err MP4CreateODTrackReader( MP4Movie theMovie, MP4Track theTrack, MP4TrackReaderPtr *outReader );
/* Guido : inserted to clean-up resources */
MP4Err MP4DisposeOrdinaryTrackReader( MP4TrackReaderPtr theReader );

MP4_EXTERN ( MP4Err )
MP4CreateTrackReader( MP4Track theTrack, MP4TrackReader *outReader )
{
	MP4Err err;
	MP4Movie          theMovie;
	MP4Media          theMedia;
	u32               handlerType;
	MP4TrackReaderPtr reader;
	err = MP4NoErr;
	if ( (theTrack == 0) || (outReader == 0) )
		BAILWITHERROR( MP4BadParamErr )
	err = MP4GetTrackMovie( theTrack, &theMovie ); if (err) goto bail;
	err = MP4GetTrackMedia( theTrack, &theMedia ); if (err) goto bail;
	err = MP4GetMediaHandlerDescription( theMedia, &handlerType, NULL ); if (err) goto bail;
	switch ( handlerType )
	{
		case MP4ObjectDescriptorHandlerType:
			err = MP4CreateODTrackReader( theMovie, theTrack, &reader ); if (err) goto bail;
			break;
			
		default:
			err = MP4CreateOrdinaryTrackReader( theMovie, theTrack, &reader ); if (err) goto bail;
			break;
	}
	*outReader = (MP4TrackReader) reader;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetCurrentDecoderConfig( MP4TrackReader theReader, MP4Handle decoderConfigH )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) || (decoderConfigH == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	err = MP4GetMediaDecoderConfig( reader->media, reader->sampleDescIndex, decoderConfigH ); if (err) goto bail;
bail:
	assert( (err == 0) || (err == MP4EOF) );
	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetCurrentSampleDescription( MP4TrackReader theReader, MP4Handle sampleEntryH )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) || (sampleEntryH == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	err = MP4GetMediaSampleDescription( reader->media, reader->sampleDescIndex, sampleEntryH, NULL ); if (err) goto bail;
bail:
	assert( (err == 0) || (err == MP4EOF) );
	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetCurrentSampleDescriptionIndex( MP4TrackReader theReader, u32 *index )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	*index = reader->sampleDescIndex;
bail:
	assert( (err == 0) || (err == MP4EOF) );
	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetNextAccessUnit( MP4TrackReader theReader, MP4Handle outAccessUnit,
                                 u32 *outSize, u32 *outSampleFlags,
                                 s32 *outCTS, s32 *outDTS )
{
   return MP4TrackReaderGetNextAccessUnitWithDuration( theReader, outAccessUnit, outSize,
                                                       outSampleFlags, outCTS, outDTS, 0 );
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetNextAccessUnitWithDuration( MP4TrackReader theReader, MP4Handle outAccessUnit,
                                 u32 *outSize, u32 *outSampleFlags,
                                 s32 *outCTS, s32 *outDTS, u32 *outDuration )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) || (outAccessUnit == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	*outSize = 0;
	err = reader->getNextAccessUnit( reader, outAccessUnit, outSize,
                                     outSampleFlags, outCTS, outDTS, outDuration, 0 ); if (err) goto bail;
bail:
	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetNextAccessUnitWithPad( MP4TrackReader theReader, MP4Handle outAccessUnit,
                                 u32 *outSize, u32 *outSampleFlags,
                                 s32 *outCTS, s32 *outDTS, u8 *outPad )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) || (outAccessUnit == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	*outSize = 0;
	err = reader->getNextAccessUnit( reader, outAccessUnit, outSize,
                                     outSampleFlags, outCTS, outDTS, 0, outPad ); if (err) goto bail;
bail:
	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderSetSLConfig( MP4TrackReader theReader, MP4SLConfig slConfig )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) || (slConfig == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	err = reader->setSLConfig( reader, slConfig ); if (err) goto bail;
bail:
	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetNextPacket( MP4TrackReader theReader, MP4Handle outSample, u32 *outSize )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) || (outSample == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	*outSize = 0;
	err = reader->getNextPacket( reader, outSample, outSize ); if (err) goto bail;
bail:
	return err;
}

MP4_EXTERN ( MP4Err )
MP4TrackReaderGetCurrentSampleNumber( MP4TrackReader theReader, u32 *sampleNumber )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( (theReader == 0) || (sampleNumber == 0) )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
	*sampleNumber = reader->currentSampleNumber;
	
bail:
	return err;
}

MP4_EXTERN ( MP4Err )
MP4DisposeTrackReader( MP4TrackReader theReader )
{
	MP4Err err;
	MP4TrackReaderPtr reader;
	
	err = MP4NoErr;
	if ( theReader == 0 )
		BAILWITHERROR( MP4BadParamErr )
	reader = (MP4TrackReaderPtr) theReader;
/* Guido : inserted to clean-up resources */
	err = MP4DisposeOrdinaryTrackReader( reader ); if (err) goto bail;
	err = reader->destroy( reader ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;
	
}
