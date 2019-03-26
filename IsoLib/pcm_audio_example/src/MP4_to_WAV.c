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
 derivative works. Copyright (c) 2019.
 */
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "MP4Descriptors.h"
#include "StringUtils.h"
#include "Logger.h"
#include "MP4ToWAVOptions.h"
#include "WAVData.h"
#include "PCMFormatData.h"
#include "FileFormatToPCM.h"

MP4Err readAudioTrackAtom       (MP4Movie moov, MP4Track *trak, u32 trackNumber, PCMFormatData *pcmFormatData);
MP4Err processAudioSamples      (MP4Track trak, WAVData *wavData);

int     main                ( int argc, char **argv )
{
    MP4Err                      err;
    MP4ToWAVOptions             options;
    WAVData                     wavData;
    MP4Movie                    moov;
    MP4Track                    audioTrack;
    PCMFormatData               pcmFormatData;

    logOutput   = stdout;
    err         = MP4NoErr;
    
    setDefaultValues(&options);
    
    if (!parseArguments(argc, argv, &options))
    {
        logMsg(LOGLEVEL_ERROR, "Parsing options failed!");
        BAILWITHERROR(MP4BadParamErr);
    }
    
    if (options.isJustAskingForHelp) goto bail;
    logLevel    = options.debugLevel;
    
    logMsg(LOGLEVEL_INFO, "MP4 to PCM started.");
    printOptions(&options);
    
    err = MP4OpenMovieFile( &moov, options.inputFile, MP4OpenMovieNormal );
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Opening movie file failed!");
        goto bail;
    }
    
    err = readAudioTrackAtom(moov, &audioTrack, options.audioTrackNumber, &pcmFormatData);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Reading audio track failed!");
        goto bail;
    }
    
    err = initWAVDataForWriting(&wavData, options.outputFile, pcmFormatData.channelCount,
                                pcmFormatData.sampleRate, pcmFormatData.pcmSampleSize>>3);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Initializing WAV data for writing failed!");
        goto bail;
    }
    
    err = processAudioSamples(audioTrack, &wavData);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Processing audio samples failed!");
        goto bail;
    }
    
    err = closeWAVDataFile(&wavData);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Finishing WAV file failed!");
        goto bail;
    }
    
    err = MP4DisposeMovie(moov);                        if (err) goto bail;
    err = freeWAVData(&wavData);                        if (err) goto bail;

    freeOptions(&options);
    
    logMsg(LOGLEVEL_INFO, "MP4 to WAV finished.");
bail:
    fflush(stdout);
	return err;
}

MP4Err readAudioTrackAtom   (MP4Movie moov, MP4Track *trak, u32 trackNumber, PCMFormatData *pcmFormatData)
{
    MP4Err                      err;
    MP4Media                    media;
    u32                         handlerType;
    u32                         outDataReferenceIndex;
    MP4Handle                   mediaDescriptionH;
    PCMAtoms                    pcmAtoms;
    
    logMsg(LOGLEVEL_INFO, "Reading audio track atom from file");
    
    err = MP4NoErr;
    err = MP4GetMovieIndTrack( moov, trackNumber, trak );                                       if (err) goto bail;
    err = MP4GetTrackMedia( *trak, &media );                                                    if (err) goto bail;
    err = MP4GetMediaHandlerDescription( media, &handlerType, NULL );                           if (err) goto bail;
    err = MP4NewHandle( 0, &mediaDescriptionH );                                                if (err) goto bail;
    err = MP4GetMediaSampleDescription(media, 1, mediaDescriptionH, &outDataReferenceIndex );   if (err) goto bail;
    
    if (handlerType != MP4AudioHandlerType)
    {
        logMsg(LOGLEVEL_ERROR, "Track #%d is not an audio track!", trackNumber);
        err = MP4InvalidMediaErr;
        if (err) goto bail;
    }

    err = createPCMAtomsFromAudioTrack(*trak, pcmFormatData, &pcmAtoms);                               if (err) goto bail;
    
    err = MP4DisposeHandle(mediaDescriptionH);  if (err) goto bail;
    logMsg(LOGLEVEL_INFO, "Reading audio track atom finished successfully.");
bail:
    return err;
}

MP4Err processAudioSamples   (MP4Track trak, WAVData *wavData)
{
    MP4Err                      err;
    MP4Err                      errWrite;
    MP4TrackReader              reader;
    MP4Handle                   packetH;
    
    logMsg(LOGLEVEL_INFO, "Processing audio samples from track");
    
    err         = MP4NoErr;
    errWrite    = MP4NoErr;
    err         = MP4CreateTrackReader( trak, &reader );    if (err) goto bail;
    err         = MP4NewHandle( 0, &packetH );              if (err) goto bail;
    while (err != MP4EOF)
    {
        u32     unitSize;
        s32     cts;
        s32     dts;
        u32     duration;
        u32     sampleFlags;
        
        err = MP4TrackReaderGetNextAccessUnitWithDuration( reader, packetH, &unitSize, &sampleFlags, &cts, &dts, &duration );
        
        logMsg(LOGLEVEL_TRACE, "Read audio sample: size = %d, duration = %d, dts = %d", unitSize, duration, dts);
        
        if (( err != MP4EOF ) && ( err != MP4NoErr ))
        {
                logMsg(LOGLEVEL_ERROR, "Reading track data failed!");
                goto bail;
        }
        
        if (err == MP4EOF)
            break;

        errWrite = wavIO_writeRawData (wavData->wavIOHandle, *packetH, unitSize);


        if (errWrite)
        {
            logMsg(LOGLEVEL_ERROR, "Writing sample to file failed!");
            err = errWrite;
            goto bail;
        }
    }
    err = MP4NoErr;
    
    err = MP4DisposeTrackReader(reader);    if (err) goto bail;
    err = MP4DisposeHandle(packetH);        if (err) goto bail;
    logMsg(LOGLEVEL_DEBUG, "Processing audio samples from track finished");
bail:
    return err;
}
