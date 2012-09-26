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
derivative works. Copyright (c) 2002.

	Description : Tutorial for the IPMPX API version 1.0 

    This sample code protects an MPEG-4 movie with a protected audio track.
	It is produced by copying an existing input MPEG-4 movie.
	The audio track is protected by an IPMP Tool by adding IPMPX meta-data in Object Descriptors.

	Date : 20.09.02 

	Direct support : ResonateMP4 / contact@resonate-mp4.com 
*/

#include "MP4Movies.h"
#include <string.h>

/* ID of the IPMP Tool to be used, provided by the future Registration Authority (RA) */
#define kIPMPReapeaterToolID 9 

/* Content of IPMX_Data field present in the IPMP_ToolDecriptor */
static u8 ipmpxData[] = {
	0x01, /* IPMP_OpaqueData_tag : reserved for carriage of opaque data */		
    0x02, /* ipmpxDataSize */
	0x01, /* version of syntax used in the IPMP Data : shall be set to 0x01 */  
	0x08  /* the opaque data block used by the instance of the Tool */ 
};



/* function declarations */
MP4Err associateSampleDescWithIPMPToolPtr( MP4Handle outMediaSampleDescrH, 
				  						   u16 theIPMP_ToolDescriptorID );

MP4Err protectSample( MP4Handle sampleH, 
                      u64 ipmpToolID );

MP4Err protectMyAudioMovie( char *inFilename, 
                            char *outFilename, 
                            u64 ipmpToolID );

MP4Err putToolListInIOD( MP4Movie theMovie, u64 theIpmpToolID);

MP4Err addIPMPToolDescriptorUpdateToODAccessUnit( MP4Handle odAccessUnitH,
                                                  u16 ipmpToolDescriptorId,
                                                  u64 ipmpToolId );



/*=====
/
/ main function
/
/*=====*/

int main( int argc, char **argv )
{
	MP4Err err = MP4NoErr;

	if (argc != 3) 
	{
		*argv = (char*) malloc(64);
		argc = 3;
		argv[1] ="amado.mp4";
		argv[2] ="amado[k].mp4";
	}
	fprintf( stderr, "Protecting %s in %s ...\n", argv[1], argv[2] );
	
	err = protectMyAudioMovie( argv[1], argv[2], kIPMPReapeaterToolID);
	
	if ( err == MP4FileNotFoundErr || err == MP4IOErr ) 
		fprintf( stderr, "Error : %s not found ! \n", argv[1] );		
	fprintf( stderr, "protectMyMovie returns %d\n", err );
	return err;
}



/*=====
/
/ protectMyAudioMovie
/ Protect the audio media track using an IPMP Tool specified by its IPMP_ToolID
/
/*=====*/

MP4Err protectMyAudioMovie( char *inFilename, char *outFilename, u64 ipmpToolID )
{
	MP4Err err;
	MP4Movie inMovie, outMovie;
	MP4Track inTrack, outTrack;
	MP4Media inMedia, outMedia;
	MP4TrackReader reader;
	MP4Handle inSampleH, sampleDescriptionH;
    u32 unitSize, sampleNum = 1, sampleCount = 0;
	u32 duration;
	s32 cts, dts;
	MP4Handle durationH, sizeH, ctsOffsetH, isSyncH, isNotSyncH;
	u32 iod_ID = 1, handlerType, trackID;
	u16 myIPMP_ToolDescriptorID = 12;
	u8 i, ODProfile, SceneProfile, AudioProfile, VisualProfile, GraphicsProfile;
	u32 tracksCount, newTrackFlag, sampleFlags, timeScale;
	u64 mediaDuration;
	err = MP4NoErr;
    inSampleH = sampleDescriptionH = durationH = sizeH = ctsOffsetH = isSyncH = isNotSyncH = NULL;

	/* Open the input Movie */
	err = MP4OpenMovieFile( &inMovie, 
                            inFilename, 
                            MP4OpenMovieNormal); if (err) goto bail;
	err = MP4GetMovieProfilesAndLevels( inMovie, 
                                        &ODProfile, 
                                        &SceneProfile, 
                                        &AudioProfile, 
                                        &VisualProfile, 
                                        &GraphicsProfile ); if (err) goto bail;

	/* Create the output Movie with its profiles */
	err = MP4NewMovie( &outMovie, 
                       iod_ID, 
                       ODProfile, 
                       SceneProfile, 
                       AudioProfile, 
                       VisualProfile, 
                       GraphicsProfile); if (err) goto bail;
	
	/* Copy the MPEG-4 content from intput Movie to output Movie, and add IPMP data */
	err = MP4GetMovieTrackCount(inMovie, 
                                &tracksCount); if (err) goto bail;
	err = MP4NewHandle( 0, &inSampleH ); if (err) goto bail; 
    err = MP4NewHandle( sizeof(u32), &durationH ); if (err) goto bail;
    err = MP4NewHandle( sizeof(u32), &sizeH ); if (err) goto bail;
    err = MP4NewHandle( sizeof(u32), &ctsOffsetH ); if (err) goto bail;
    err = MP4NewHandle( sizeof(u32), &isSyncH ); if (err) goto bail;
    err = MP4NewHandle( 0, &isNotSyncH ); if (err) goto bail;
    *( (u32 *) *isSyncH ) = 1;

	for (i = 1 ; i < tracksCount + 1; i++) 
    {
		err = MP4GetMovieIndTrack( inMovie, i, &inTrack ); if (err) goto bail;
		err = MP4GetTrackID( inTrack, &trackID ); if (err) goto bail; 

		fprintf(stderr, "Track ID: %d \n", trackID);

		 /* Create the track reader */
        err = MP4CreateTrackReader( inTrack, 
                                    &reader ); if (err) goto bail;
		err = MP4GetTrackMedia(inTrack, &inMedia); if (err) goto bail;
		err = MP4GetMediaTimeScale(inMedia, &timeScale); if (err) goto bail; 
		err = MP4GetMediaSampleCount( inMedia, &sampleCount );

		err = MP4NewHandle( 0, &sampleDescriptionH ); if (err) goto bail; 
		err = MP4GetMediaSampleDescription( inMedia, 
                                            1, 
                                            sampleDescriptionH, 
                                            NULL ); if (err) goto bail; 

		err = MP4GetMediaHandlerDescription( inMedia, 
                                             &handlerType, 
                                             NULL ); if (err) goto bail;
		if ( handlerType == MP4AudioHandlerType ) 
			newTrackFlag = MP4NewTrackIsAudio; 
		else if ( handlerType == MP4VisualHandlerType )
			newTrackFlag = MP4NewTrackIsVisual;
		else
			newTrackFlag = 0;

        
        if ( handlerType == MP4AudioHandlerType ) {
            /* Add the IPMP_ToolDescriptorPointer to the audio Sample Description */ 
            err = associateSampleDescWithIPMPToolPtr( 	sampleDescriptionH, 
                                                        myIPMP_ToolDescriptorID); if (err) goto bail;
        }
        
        err = MP4NewMovieTrack( outMovie, 
                                newTrackFlag, 
                                &outTrack ); if (err) goto bail;	
        err = MP4NewTrackMedia( outTrack, 
                                &outMedia, 
                                handlerType, 
                                timeScale, 
                                NULL ); if (err) goto bail;
		        
        /*  == BeginMediaEdits == */
        err = MP4BeginMediaEdits( outMedia ); if (err) goto bail;
        
        for (;;) 
        {
            MP4Handle syncH;

            /* --- Get the next Access Unit --- */ 
            err = MP4TrackReaderGetNextAccessUnitWithDuration( reader, 
                                                               inSampleH, 
                                                               &unitSize, 
                                                               &sampleFlags,
                                                               &cts, 
                                                               &dts, 
                                                               &duration ); 			
            if ( err ) 
            {
                if ( err == MP4EOF ) {
                    err = MP4NoErr;
                    break;
                }
                else
                    goto bail;
            }
            
            else 
            {
                if ( handlerType == MP4AudioHandlerType ) {						
                    /* Protect the sample using the IPMP Tool whose ID is ipmpToolID */
                    err = protectSample(inSampleH, ipmpToolID);
                }
 				if ( (handlerType == MP4ObjectDescriptorHandlerType)
                    && (sampleNum == 1 ) )
                {
                    /* Add the ipmpToolDescriptorUpdate command to the first OD Access Unit */ 
                    err = addIPMPToolDescriptorUpdateToODAccessUnit( inSampleH,
                                                                     myIPMP_ToolDescriptorID,
                                                                     ipmpToolID ); if (err) goto bail;
                    err = MP4GetHandleSize( inSampleH, &unitSize);if (err) goto bail;                     
                }
                
                /* Add the sample in the media */
                if ( sampleFlags & MP4MediaSampleNotSync )
                {
                    syncH = isNotSyncH;
                }
                else
                {
                    syncH = isSyncH;
                }

                *(u32 *)( *durationH ) = (u32) duration;
                *(u32 *)( *sizeH ) = unitSize;
                *(u32 *)( *ctsOffsetH ) =  cts - dts;
                err = MP4AddMediaSamples( outMedia, 
                                          inSampleH, 
                                          1, 
                                          durationH, 
                                          sizeH, 
                                          sampleDescriptionH, 
                                          ctsOffsetH, 
                                          syncH ); if (err) goto bail; 
                
                if ( sampleDescriptionH )
                {
                /* We set the sample description handle to NULL
                    so that no more entry is later in the table by MP4AddMediaSamples() */ 
                    err = MP4DisposeHandle( sampleDescriptionH ); if (err) goto bail;
                    sampleDescriptionH = NULL;
                }	
			
				fprintf( stderr, "Sample copied : %d / %d \n", sampleNum, sampleCount );
				sampleNum++;
			
			}
								
		} /* -- End of loop on the access units of a Track -- */

		sampleNum = 1;
			
		err = MP4GetMediaDuration( outMedia, &mediaDuration ); if (err) goto bail;
		
		/* == End Media Edits == */ 
		err = MP4EndMediaEdits( outMedia );	if (err) goto bail;					
		if ( mediaDuration != 0 )
        {
            err = MP4InsertMediaIntoTrack( outTrack, 
                                           0, 
                                           0, 
                                           mediaDuration, 
                                           1 ); if (err) goto bail;
        }

		/* Add the ES_ID_Ref for BIFS and OD in the IOD */
		if ( ( handlerType == MP4SceneDescriptionHandlerType ) || 
			 ( handlerType == MP4ObjectDescriptorHandlerType ) )
		{
			err = MP4AddTrackToMovieIOD( outTrack );  if (err) goto bail;
		}
 
		/* Create the media track references for ODs */
		if ( handlerType == MP4ObjectDescriptorHandlerType ) 
		{
			u32 refCount, j;

			err = MP4GetTrackReferenceCount( inTrack, 
										  MP4ODTrackReferenceType, 
										  &refCount );  if (err) goto bail;
			for ( j = 1; j <= refCount; j++ )
			{
				MP4Track trak;
				u32 trakId, outTrackRefIndex;

				err = MP4GetTrackReference( inTrack, MP4ODTrackReferenceType, 
											j, &trak );  if (err) goto bail;
				err = MP4GetTrackID( trak, &trakId ); if (err) goto bail;
				err = MP4AddTrackReferenceWithID( outTrack, trakId, 
												  MP4ODTrackReferenceType, 
												  &outTrackRefIndex ); 
				if (err) goto bail;
			}
		}


	} /* -- End of loop on the Tracks of the Movie -- */

	/* Add the Tool List to the IOD */
	err = putToolListInIOD( outMovie, ipmpToolID); if (err) goto bail;
	
	/* Save the output protected Movie */ 
	err = MP4WriteMovieToFile( outMovie, outFilename ); if (err) goto bail;

	/* Release the Movies */  	
	err = MP4DisposeMovie( outMovie ); if (err) goto bail;
	err = MP4DisposeMovie( inMovie ); if (err) goto bail;

bail:	
    if ( inSampleH ) MP4DisposeHandle( inSampleH );
    if ( sampleDescriptionH ) MP4DisposeHandle( sampleDescriptionH );
    if ( durationH ) MP4DisposeHandle( durationH );
    if ( sizeH ) MP4DisposeHandle( sizeH );
    if ( ctsOffsetH ) MP4DisposeHandle( ctsOffsetH );
    if ( isSyncH ) MP4DisposeHandle( isSyncH );
    if ( isNotSyncH ) MP4DisposeHandle( isNotSyncH );

	return err;
}




/*=====
/
/ associateSampleDescWithIPMPToolPtr
/ Create the IPMP_ToolDecriptorPointer and add it to the media Sample Description
/
/*=====*/

MP4Err associateSampleDescWithIPMPToolPtr( MP4Handle outMediaSampleDescrH, 
										   u16 theIPMP_ToolDescriptorID) {
	
	MP4Err err = MP4NoErr;
	MP4Handle myIPMP_ToolDescriptorPtrH;

	/* Create the IPMP_ToolDecriptorPointer */
	err = MP4NewHandle( 0, 
                        &myIPMP_ToolDescriptorPtrH ); if (err) goto bail; 
	err = MP4NewIPMPDescriptorPointer( myIPMP_ToolDescriptorPtrH,
                                       0xFF, /* using IPMP extensions and not IPMP hooks */ 
                                       theIPMP_ToolDescriptorID ); if (err) goto bail;
    
    /* add it to the sample description */
    err = MP4AddDescToSampleDescription( outMediaSampleDescrH, 
                                         myIPMP_ToolDescriptorPtrH ); if (err) goto bail;

bail:
	return err;
}


/*=====
/
/ addIPMPToolDescriptorUpdateToODAccessUnit
/ Create the IPMP_ToolDecriptorUpdate command and add it to an OD access unit
/
/*=====*/

MP4Err addIPMPToolDescriptorUpdateToODAccessUnit( MP4Handle odAccessUnitH,
                                                  u16 ipmpToolDescriptorId,
                                                  u64 ipmpToolId )
{
    MP4Err err;
    MP4Handle myIpmpToolDescrH, myIpmpDataH, myIpmpToolDescrUpdateH;
    u32 ipmpxDataLength;
    
    err = MP4NoErr;
    myIpmpToolDescrH = myIpmpDataH = myIpmpToolDescrUpdateH = NULL;
    /* Set the IPMP_ToolDescriptor */
	err = MP4NewHandle( 0, &myIpmpToolDescrH ); if (err) goto bail; 
	err = MP4NewIPMPToolDescriptor( myIpmpToolDescrH,
                                    ipmpToolDescriptorId, 
                                    ipmpToolId, 
                                    0, 
                                    NULL ); if (err) goto bail;

	err = MP4NewHandle( 1, &myIpmpDataH ); if (err) goto bail; 
	ipmpxDataLength = sizeof(ipmpxData);
	err = MP4SetHandleSize( myIpmpDataH, ipmpxDataLength ); if (err) goto bail;
    memcpy( *myIpmpDataH, ipmpxData, ipmpxDataLength ); /* put the ipmp data in the handle */

	err = MP4AddIPMPDataToIPMPToolDescriptor( myIpmpToolDescrH, 
                                              myIpmpDataH ); if (err) goto bail;

		/* Set the IPMP_ToolDescriptorUpdate command */
	err = MP4NewHandle( 0, &myIpmpToolDescrUpdateH ); if (err) goto bail; 
	err = MP4NewIPMPToolDescriptorUpdate( myIpmpToolDescrUpdateH ); if (err) goto bail;
	err = MP4AddIPMPToolDescriptorToUpdate( myIpmpToolDescrUpdateH, 
                                            myIpmpToolDescrH ); if (err) goto bail;

		/* Add the IPMP command to the OD access unit */
	err = MP4HandleCat( odAccessUnitH, myIpmpToolDescrUpdateH ); if (err) goto bail;


    err = MP4NoErr;
bail:
    if ( myIpmpToolDescrH ) MP4DisposeHandle( myIpmpToolDescrH );
    if ( myIpmpDataH ) MP4DisposeHandle( myIpmpDataH );
    if ( myIpmpToolDescrUpdateH ) MP4DisposeHandle( myIpmpToolDescrUpdateH );
    return err;
}



/*=====
/
/ protectSample
/ Protect one media access unit (or sample)
/
/*=====*/

MP4Err protectSample(MP4Handle sampleH, u64 ipmpToolID) {

	/* Do nothing in this example! */
	return MP4NoErr;
}



/*=====
/
/ putToolListInIOD
/ Create the IPMP Tool List and put it in the IOD 
/
/*=====*/

MP4Err putToolListInIOD( MP4Movie theMovie, u64 theIpmpToolID) {
	
	MP4Err err = MP4NoErr;	
	MP4Handle ipmpToolH, ipmpToolListDescH;

    ipmpToolH = ipmpToolListDescH = NULL;
	/* Create the IPMP Tool */
	err = MP4NewHandle( 0, &ipmpToolH ); if (err) goto bail;
	err = MP4NewIPMPTool( ipmpToolH, theIpmpToolID, 
	                      0, NULL, NULL ); if (err) goto bail; 

	/* Create the List and add the Tool */
	err = MP4NewHandle( 0, &ipmpToolListDescH ); if (err) goto bail;
	err = MP4NewIPMPToolListDescriptor( ipmpToolListDescH ); if (err) goto bail;
	err = MP4AddToolToIPMPToolList( ipmpToolListDescH, ipmpToolH );

	/* Put the List in the IOD */
	err = MP4AddDescToMovieIOD( theMovie, ipmpToolListDescH);

bail : 
    if ( ipmpToolH ) MP4DisposeHandle( ipmpToolH );
    if ( ipmpToolListDescH ) MP4DisposeHandle( ipmpToolListDescH );
	return err;
}

