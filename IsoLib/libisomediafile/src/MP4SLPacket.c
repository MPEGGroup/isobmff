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
	$Id: MP4SLPacket.c,v 1.2 2002/10/01 12:49:19 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "MovieTracks.h"
#include "MP4Descriptors.h"
#include <string.h>
#include <stdlib.h>

MP4Err rewriteODFrame( MP4Track track,
                       MP4Handle sampleH,
                       u64 cts,
                       MP4TrackReferenceTypeAtomPtr mpod, MP4SLConfig slconfig );

MP4Err rewriteIPMPDescriptorPointers( MP4DescriptorPtr desc,
                                      MP4TrackReferenceTypeAtomPtr mpod );


MP4Err rewriteODFrame( MP4Track track, MP4Handle sampleH, u64 cts, MP4TrackReferenceTypeAtomPtr mpod, MP4SLConfig slconfig )
{
	MP4Err makeESD( MP4Movie theMovie, u32 trackNumber, u64 cts, MP4SLConfig slconfig, MP4DescriptorPtr *outDesc );
	MP4Err MP4ParseCommand( MP4InputStreamPtr inputStream, MP4DescriptorPtr *outDesc );
	
	MP4Err err;
	u32 unitSize;
	MP4InputStreamPtr is;
	MP4Movie theMovie;
	MP4LinkedList descList;
	u32 commandSize;
	u32 numCmds;
	char *buffer;
	u32 i;
	u32 j;

	err = MP4NoErr;
	err = MP4GetHandleSize( sampleH, &unitSize ); if (err) goto bail;
	err = MP4GetTrackMovie( track, &theMovie ); if (err) goto bail;
	err = MP4CreateMemoryInputStream( *sampleH, unitSize, &is ); if (err) goto bail;
	err = MP4MakeLinkedList( &descList ); if (err) goto bail;
	is->debugging = 0;
	commandSize = 0;
	numCmds = 0;
	while ( is->available > 0 )
	{
		MP4DescriptorPtr             desc;
		MP4ObjectDescriptorUpdatePtr odUpdate;
		MP4ESDescriptorUpdatePtr     esUpdate;
		u32                          odCount;
		u32                          esCount;
		
		/* err = MP4ParseDescriptor( is, &desc ); if (err) goto bail; */
		err = MP4ParseCommand( is, &desc ); if (err) goto bail;
		numCmds += 1;
		switch ( desc->tag )
		{
			case MP4ObjectDescriptorUpdateTag:
				odUpdate = (MP4ObjectDescriptorUpdatePtr) desc;
				err = MP4GetListEntryCount( odUpdate->objectDescriptors, &odCount ); if (err) goto bail;
				for ( i = 0; i < odCount; i++ )
				{
					MP4ObjectDescriptorPtr od;
					err = MP4GetListEntry( odUpdate->objectDescriptors, i, (char **) &od ); if (err) goto bail;
                    /* JB_RESO 09/02 rewrite IPMPDescriptorPointers if needed */
                    err = rewriteIPMPDescriptorPointers( (MP4DescriptorPtr) od,
                                                         mpod ); if (err) goto bail;

					err = MP4GetListEntryCount( od->ES_ID_RefDescriptors, &esCount ); if (err) goto bail;
					for( j = 0; j < esCount; j++ )
					{
						MP4ES_ID_RefDescriptorPtr ref;
						MP4DescriptorPtr esd;
						int trackNumber;
						err = MP4GetListEntry( od->ES_ID_RefDescriptors, j, (char**) &ref ); if (err) goto bail;
						if ( ref->refIndex > mpod->trackIDCount )
							BAILWITHERROR( MP4InvalidMediaErr );
						trackNumber = mpod->trackIDs[ ref->refIndex - 1 ];
						err = makeESD( theMovie, trackNumber, cts, slconfig, &esd ); if (err) goto bail;
                        /* JB_RESO 09/02 rewrite IPMPDescriptorPointers if needed */
                        err = rewriteIPMPDescriptorPointers( (MP4DescriptorPtr) esd,
                                                             mpod ); if (err) goto bail;
						err = od->addDescriptor( (MP4DescriptorPtr) od, esd ); if (err) goto bail;
					}
					/* --awm to avoid putting the ES_ID_Ref descriptors in the output */
					DESTROY_DESCRIPTOR_LIST_V( od->ES_ID_RefDescriptors );
					od->ES_ID_RefDescriptors = NULL;
				}
				break;
				
			case MP4ESDescriptorUpdateTag:
				esUpdate = (MP4ESDescriptorUpdatePtr) desc;
				err = MP4GetListEntryCount( esUpdate->ES_ID_RefDescriptors, &esCount ); if (err) goto bail;
				for( j = 0; j < esCount; j++ )
				{
					MP4ES_ID_RefDescriptorPtr ref;
					MP4DescriptorPtr esd;
					int trackNumber;
					err = MP4GetListEntry( esUpdate->ES_ID_RefDescriptors, j, (char **) &ref ); if (err) goto bail;
					if ( ref->refIndex > mpod->trackIDCount )
						BAILWITHERROR( MP4InvalidMediaErr );
					trackNumber = mpod->trackIDs[ ref->refIndex - 1 ];
					err = makeESD( theMovie, trackNumber, cts, slconfig, &esd ); if (err) goto bail;
                    /* JB_RESO 09/02 rewrite IPMPDescriptorPointers if needed */
                    err = rewriteIPMPDescriptorPointers( (MP4DescriptorPtr) esd,
                                                         mpod ); if (err) goto bail;
					err = esUpdate->addDescriptor( (MP4DescriptorPtr) esUpdate, esd ); if (err) goto bail;
				}
				/* --awm to avoid putting the ES_ID_Ref descriptors in the output */
				DESTROY_DESCRIPTOR_LIST_V( esUpdate->ES_ID_RefDescriptors );
				esUpdate->ES_ID_RefDescriptors = NULL;
				break;
			
			default:
				break;
		}
		err = MP4AddListEntry( desc, descList ); if (err) goto bail;
		err = desc->calculateSize( desc ); if (err) goto bail;
		commandSize += desc->size;
	}

	err = MP4SetHandleSize( sampleH, commandSize ); if (err) goto bail;
	buffer = *sampleH;
	
	for ( i = 0; i < numCmds; i++ )
	{
		MP4DescriptorPtr desc;
		err = MP4GetListEntry( descList, i, (char **) &desc ); if (err) goto bail;
		err = desc->serialize( desc, buffer ); if (err) goto bail;
		buffer += desc->size;
	}
	
	/* MP4DeleteLinkedList( descList ); */
	/* Just deleting the list still leaves the allocated descriptors hanging around -- dws */
	DESTROY_DESCRIPTOR_LIST_V( descList );
bail:
	if (is) {
		is->destroy( is );
		is = NULL;
	}
	TEST_RETURN( err );

	return err;
}

static void put32( u32 val, char *buf )
{
	buf[3] = val         & 0xFF;
	buf[2] = (val >> 8)  & 0xFF;
	buf[1] = (val >> 16) & 0xFF;
	buf[0] = (val >> 24) & 0xFF; 
}

MP4_EXTERN ( MP4Err )
MP4GetElementaryStreamPacket( MP4Media theMedia, MP4Handle outPacket, u32 *outSize,
				   			  u32 sequenceNumber, u64 desiredDecodingTime, u64 *outActualTime, u64 *outDuration )
{
	MP4Err    err;
	MP4Handle sampleH;
	u32       sampleSize;
	u32       sampleFlags;
	u32       packetSize;
	u32       handlerType;
	MP4Track  track;
	char      header[ 40 ];
	u32       headerBytes;
	u8        val;
	u32       cts;
	u32       dts;
	u64       sampleCTS;

	err = MP4NoErr;
	err = MP4NewHandle( 4096, &sampleH ); if (err) goto bail;
	err = MP4GetMediaTrack( theMedia, &track ); if (err) goto bail;

	/* first get the sample */
	err = MP4GetMediaSample( theMedia, sampleH, &sampleSize,
							 desiredDecodingTime, outActualTime, &sampleCTS, outDuration,
							 NULL, NULL, &sampleFlags ); if (err) goto bail;
	
	/* if OD stream, rewrite media */
	err = MP4GetMediaHandlerDescription( theMedia, &handlerType, NULL ); if (err) goto bail;

	if ( handlerType == MP4ClockReferenceHandlerType )
		BAILWITHERROR( MP4NotImplementedErr )

	if ( handlerType == MP4ObjectDescriptorHandlerType )
	{
		MP4TrackReferenceTypeAtomPtr mpod;

		err = MP4GetTrackReferenceType( track, MP4ODTrackReferenceAtomType, &mpod ); if (err) goto bail;
		if ( mpod )
		{
			u32 sz;
			err = rewriteODFrame( track, sampleH, *outActualTime, mpod, 0 ); if (err) goto bail;
			err = MP4GetHandleSize( sampleH, &sz ); if (err) goto bail;
			sampleSize = sz;
		}
	}

	/* make packet header */
	dts = (u32) (*outActualTime & 0xFFFFFFFF);
	cts = (u32) (sampleCTS & 0xFFFFFFFF);
	
	/* note that we hardwire seqnum to 5 bits */
	val = (sequenceNumber & 0x1F) << 3;
	if ( (sampleFlags & MP4MediaSampleNotSync) == 0 )
		val |= (1 << 2);
	if ( sampleFlags & MP4MediaSampleHasCTSOffset )
		val |= (1 << 1);
	val |= 1; /* we always have CTS */
	headerBytes = 0;
	header[ headerBytes++ ] = val;

	if ( sampleFlags & MP4MediaSampleHasCTSOffset )
	{
		put32( dts, header + headerBytes );
		headerBytes += 4;
	}
	put32( cts, header + headerBytes );
	headerBytes += 4;
	put32( sampleSize, header + headerBytes );
	headerBytes += 4;

	/* size output handle */
	packetSize = sampleSize + headerBytes;
	err = MP4SetHandleSize( outPacket, packetSize ); if (err) goto bail;

	/* copy header and sample into output handle */
	memcpy( *outPacket, header, headerBytes );
	memcpy( *outPacket + headerBytes, *sampleH, sampleSize );
	*outSize = packetSize;
	MP4DisposeHandle( sampleH );
bail:
	TEST_RETURN( err );

	if ( err )
	{
		if ( sampleH )
			MP4DisposeHandle( sampleH );
	}
	return err;
}

/**
* JB_RESO 09/02 rewrite the IPMPDescriptorPointers
*/
MP4Err rewriteIPMPDescriptorPointers( MP4DescriptorPtr desc,
                                      MP4TrackReferenceTypeAtomPtr mpod )
{
    u32 k;
    u32 ipmpDescPointersCount;
    MP4Err err;
    MP4LinkedList ipmpDescPointersList;

    err = MP4NoErr;
    /* if ( desc->name == "MP4ES_Descriptor" ) fb_reso */ 
    if ( desc->tag == MP4ES_DescriptorTag ) /* fb_reso */ 
    { 
        ipmpDescPointersList = ((MP4ES_DescriptorPtr) desc)->IPMPDescriptorPointers;
    }
    /* else if ( desc->name = "MP4ObjectDescriptor" ) fb_reso */
    else if ( desc->tag == MP4ObjectDescriptorTag ) /* fb_reso */
    {
        ipmpDescPointersList = ((MP4ObjectDescriptorPtr) desc)->IPMPDescriptorPointers;
    }
    else 
        BAILWITHERROR( MP4BadParamErr );

    err = MP4GetListEntryCount( ipmpDescPointersList,
                                &ipmpDescPointersCount ); if (err) goto bail; 
    for ( k = 0; k < ipmpDescPointersCount; k++ )
    {
        MP4IPMPDescriptorPointerPtr ipmpDescPtr;
        err = MP4GetListEntry( ipmpDescPointersList,
                               k, 
                               (char **) &ipmpDescPtr ); if (err) goto bail;
        if ( (ipmpDescPtr->ipmpEsId != 0) 
             || (ipmpDescPtr->ipmpEsId != 0xFFFF) )
        {
            /* rewrite the good ID */
            ipmpDescPtr->ipmpEsId
                = mpod->trackIDs[ ipmpDescPtr->ipmpEsId - 1 ];
        }
    }

bail:
    return err;
}

