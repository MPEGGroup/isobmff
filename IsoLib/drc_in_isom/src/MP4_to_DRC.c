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
 derivative works. Copyright (c) 2014.
 */
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "MP4Descriptors.h"
#include "StringUtils.h"
#include "Logger.h"
#include "MP4toDRCOptions.h"
#include "FileFormatDrcToInStreamDrc.h"
#include "WAVData.h"

MP4Err readAudioTrackAtom       (MP4Movie moov, MP4Track *trak, u32 trackNumber, StaticDrcData *staticDrcData);
MP4Err processAudioSamples      (MP4Track trak, WAVData *wavData);
MP4Err processDrcMetaTrack      (MP4Movie moov, u32 trackNumber, DrcBitstreamHandle *drcBitStreamHandle, DrcBitStreamHelper *drcBitStreamHelper);
MP4Err processDrcGainSamples    (MP4Track trak, DrcBitstreamHandle *drcBitStreamHandle, DrcBitStreamHelper *drcBitStreamHelper);
MP4Err writeDrcBitstreamToFile  (DrcBitstreamHandle *drcBitStreamHandle, char *drcBitstreamOutputFile);


int     main                ( int argc, char **argv )
{
    MP4Err                      err;
    MP4toDRCOptions             options;
    WAVData                     wavData;
    MP4Movie                    moov;
    MP4Track                    audioTrack;
    StaticDrcData               staticDrcData;
    DrcBitstreamHandle          *drcBitStreamHandle;
    DrcBitStreamHelper          drcBitStreamHelper;
    
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
    
    logMsg(LOGLEVEL_INFO, "MP4 to DRC started.\n");
    printOptions(&options);
    
    err = MP4OpenMovieFile( &moov, options.inputFile, MP4OpenMovieNormal );
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Opening movie file failed!");
        goto bail;
    }
    
    err = readAudioTrackAtom(moov, &audioTrack, options.audioTrackNumber, &staticDrcData);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Reading audio track failed!");
        goto bail;
    }
    
    err = initWAVDataForWriting(&wavData, options.wavOutputFile, staticDrcData.channelCount,
                                staticDrcData.sampleRate, staticDrcData.bytesPerSample);
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
    
    
    drcBitStreamHandle = calloc(1, sizeof(DrcBitstreamHandle));
    
    err = initDrcBitstream(&staticDrcData, drcBitStreamHandle);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Initializing DRC bitstream failed!");
        goto bail;
    }
    
    err = writeStaticDrcDataToBitstream(&staticDrcData, drcBitStreamHandle);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Writing static drc data failed!");
        goto bail;
    }
    
    err = prepareDrcBitStreamHelper(&drcBitStreamHelper, drcBitStreamHandle, &staticDrcData);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Preparing DRC bitstream for dynamic drc gains failed!");
        goto bail;
    }
    
    err = processDrcMetaTrack(moov, 2, drcBitStreamHandle, &drcBitStreamHelper);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Processing drc metadata track failed!");
        goto bail;
    }
    
    err = writeDrcBitstreamToFile(drcBitStreamHandle, options.drcOutputFile);
    if (err)
    {
        logMsg(LOGLEVEL_ERROR, "Writing drc bitstream to file failed!");
        goto bail;
    }
    
    err = MP4DisposeMovie(moov);                        if (err) goto bail;
    err = freeWAVData(&wavData);                        if (err) goto bail;
    err = freeDrcBitstreamHandle(drcBitStreamHandle);   if (err) goto bail;
    err = freeDrcBitStreamHelper(&drcBitStreamHelper);  if (err) goto bail;
    err = freeStaticDrcData(&staticDrcData);            if (err) goto bail;
    
    free(drcBitStreamHandle);
    freeOptions(&options);
    
    logMsg(LOGLEVEL_INFO, "MP4 to DRC finished.");
bail:
    fflush(stdout);
	return err;
}

MP4Err readAudioTrackAtom   (MP4Movie moov, MP4Track *trak, u32 trackNumber, StaticDrcData *staticDrcData)
{
    MP4Err                      err;
    MP4Media                    media;
    u32                         handlerType;
    u32                         outDataReferenceIndex;
    MP4Handle                   mediaDescriptionH;
    
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
    
    err = createStaticDrcDataFromAudioTrack(*trak, staticDrcData);                               if (err) goto bail;
    
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
        
        errWrite = writeFrame(wavData, packetH, unitSize);
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

MP4Err processDrcMetaTrack   (MP4Movie moov, u32 trackNumber, DrcBitstreamHandle *drcBitStreamHandle, DrcBitStreamHelper *drcBitStreamHelper)
{
    MP4Err                      err;
    MP4Track                    trak;
    MP4Media                    media;
    u32                         handlerType;
    u32                         outDataReferenceIndex;
    MP4Handle                   mediaDescriptionH;
    MP4AtomPtr                  sampleEntry;
    MP4InputStreamPtr           is;
    u32                         size;
    
    logMsg(LOGLEVEL_INFO, "Reading drc track atom from file");
    
    err = MP4NoErr;
    err = MP4GetMovieIndTrack( moov, trackNumber, &trak );                                      if (err) goto bail;
    err = MP4GetTrackMedia( trak, &media );                                                     if (err) goto bail;
    err = MP4GetMediaHandlerDescription( media, &handlerType, NULL );                           if (err) goto bail;
    err = MP4NewHandle( 0, &mediaDescriptionH );                                                if (err) goto bail;
    err = MP4GetMediaSampleDescription(media, 1, mediaDescriptionH, &outDataReferenceIndex );   if (err) goto bail;
    
    if (handlerType != MP4MetaHandlerType)
    {
        logMsg(LOGLEVEL_ERROR, "Track #%d is not an meta data track!", trackNumber);
        err = MP4InvalidMediaErr;
        if (err) goto bail;
    }
    
    err = MP4GetHandleSize( mediaDescriptionH, &size );                                         if (err) goto bail;
    err = MP4CreateMemoryInputStream( *mediaDescriptionH, size, &is );                          if (err) goto bail;
    
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &sampleEntry );                                                  if (err) goto bail;
    
    if (sampleEntry->type != DRCUniDrcSampleEntryAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "Data found in track is not UniDrc! Sampe Entry: %s", sampleEntry->name);
        err = MP4InvalidMediaErr;
        goto bail;
    }
    sampleEntry->destroy(sampleEntry);
    err = processDrcGainSamples(trak, drcBitStreamHandle, drcBitStreamHelper);                  if (err) goto bail;
    
    err = MP4DisposeHandle(mediaDescriptionH);  if (err) goto bail;
    is->destroy(is);
    logMsg(LOGLEVEL_INFO, "Reading drc track finished successfully.");
bail:
    return err;
}

MP4Err processDrcGainSamples   (MP4Track trak, DrcBitstreamHandle *drcBitStreamHandle, DrcBitStreamHelper *drcBitStreamHelper)
{
    MP4Err                      err;
    MP4Err                      errWrite;
    MP4TrackReader              reader;
    MP4Handle                   packetH;
    u32                         sampleCount;
    
    logMsg(LOGLEVEL_INFO, "Processing drc samples from track");
    
    sampleCount = 0;
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
        
        sampleCount++;
        
        err = MP4TrackReaderGetNextAccessUnitWithDuration( reader, packetH, &unitSize, &sampleFlags, &cts, &dts, &duration );
        
        logMsg(LOGLEVEL_TRACE, "Read drc sample #%d: size = %d, duration = %d, dts = %d", sampleCount, unitSize, duration, dts);
        
        if (( err != MP4EOF ) && ( err != MP4NoErr ))
        {
            logMsg(LOGLEVEL_ERROR, "Reading track data failed!");
            goto bail;
        }
        
        if (err == MP4EOF)
            break;
        
        errWrite = writeDrcGainToBitstream(packetH, drcBitStreamHandle, drcBitStreamHelper);
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
    logMsg(LOGLEVEL_DEBUG, "Processing drc samples from track finished");
bail:
    return err;
}

MP4Err writeDrcBitstreamToFile  (DrcBitstreamHandle *drcBitStreamHandle, char *drcBitstreamOutputFile)
{
    MP4Err err;
    FILE   *file;
    
    err = MP4NoErr;
    file = fopen(drcBitstreamOutputFile, "wb");
    
    if (file == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to open/write file: %s", drcBitstreamOutputFile);
        BAILWITHERROR(MP4IOErr);
    }
    
    if (drcBitStreamHandle->offsetInBits != 0)
        drcBitStreamHandle->currentBytePosition += 1;
    
    fwrite(drcBitStreamHandle->bitstreamBuffer, drcBitStreamHandle->currentBytePosition, 1, file);
    
    if (ferror (file))
    {
        logMsg(LOGLEVEL_ERROR, "Unable to write to drc output file");
        BAILWITHERROR(MP4IOErr);
    }
    
    fclose(file);
bail:
    return err;
}