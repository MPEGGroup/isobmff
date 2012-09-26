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
#include "MP4Movies.h"


MP4Err playMyMovie( char *filename, u32 trackNumber );

int main( int argc, char **argv )
{
	MP4Err err;
	
	err = MP4NoErr;
	err = playMyMovie( "myMovie.mp4", 1 );
	fprintf( stderr, "playMyMovie returns %d\n", err );
	return err;
}

MP4Err playMyMovie( char *filename, u32 trackNumber )
{
	u32 i;
	char *p;
	MP4Err err;
	MP4Movie moov;
	MP4Track trak;
	MP4Media media;
	MP4TrackReader reader;
	MP4Handle initialObjectDescriptorH;
	MP4Handle decoderConfigH;
	MP4Handle decoderSpecificInfoH;
	MP4Handle sampleH;
	u32 handlerType;
	u32 handleSize;
 	u32 objectTypeIndication;
	u32 streamType;
	u32 decoderBufferSize;
	
	err = MP4NoErr;
	err = MP4OpenMovieFile( &moov, filename, MP4OpenMovieDebug ); if (err) goto bail; /* MP4OpenMovieNormal MP4OpenMovieDebug MP4OpenMovieInPlace*/
 	
 	err = MP4NewHandle( 0, &initialObjectDescriptorH ); if (err) goto bail;
	err = MP4GetMovieInitialObjectDescriptor( moov, initialObjectDescriptorH ); if (err) goto bail;

	err = MP4GetMovieIndTrack( moov, trackNumber, &trak ); if (err) goto bail;
	err = MP4GetTrackMedia( trak, &media ); if (err) goto bail;
	err = MP4GetMediaHandlerDescription( media, &handlerType, NULL ); if (err) goto bail;
	err = MP4CreateTrackReader( trak, &reader ); if (err) goto bail;
	err = MP4NewHandle( 0, &decoderConfigH ); if (err) goto bail;
	err = MP4TrackReaderGetCurrentDecoderConfig( reader, decoderConfigH ); if (err) goto bail;
 	err = MP4NewHandle( 0, &decoderSpecificInfoH ); if (err) goto bail;
	err = MP4GetMediaDecoderType( media, 1, 
		&objectTypeIndication, 
		&streamType, 
		&decoderBufferSize, 
		decoderSpecificInfoH ); if (err) goto bail;
	/*
	 * check parameters in handlerType, objectTypeIndication, 
	 * streamType, profile and level (in initialObjectDescriptor) and
	 * decoderConfig and instantiate appropriate decoder
	 * (not shown)
	 */
	err = MP4DisposeHandle( initialObjectDescriptorH);

	err = MP4GetHandleSize( decoderSpecificInfoH, &handleSize ); if (err) goto bail;
	fprintf( stderr, "send decoderSpecificInfo to decoder: size = %d\n", handleSize );
	p = *decoderSpecificInfoH;
	for (i=0; i<handleSize; i++) {
		fprintf(stderr, "%3i 0x%02x\n", i, *p++); 
	}
	 
	err = MP4NewHandle( 0, &sampleH ); if (err) goto bail;	
	/* play every frame */
	for (;;)
	{
		u32 unitSize;
		s32 cts;
		s32 dts;
		u32 sampleFlags;
		
		err = MP4TrackReaderGetNextAccessUnit( reader, sampleH, &unitSize,&sampleFlags, &cts, &dts );
		if ( err )
		{
			if ( err == MP4EOF )
				err = MP4NoErr;
			break;
		}
		/*
		  send AU to decoder
		  (simulated)
		*/
		fprintf( stderr, "send AU to decoder: size = %d, cts = %d, dts = %d\n", unitSize, cts, dts );
		p = *sampleH;
		for (i=0; i<unitSize; i++) {
			fprintf(stderr, "%3i 0x%02x\n", i, *p++); 
		}
	}	
bail:
	return err;
}
