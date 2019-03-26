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
#include "WAVToMP4Options.h"
#include "Testing.h"
#include "WAVData.h"
#include "PCMToFileFormat.h"
#include "PCMFormatData.h"

MP4Err  createMP4Movie          (WAVData *wavData, PCMFormatData *pcmFormatData, WAVToMP4Options *options);
MP4Err  createAudioSampleEntry  (MP4AudioSampleEntryAtomPtr *sampleEntry, PCMFormatData *pcmFormatData, WAVToMP4Options *options);
MP4Err  addAudioSamples         (MP4Track trak, MP4Media media, WAVData *wavData, WAVToMP4Options *options);

int     main                ( int argc, char **argv )
{
    MP4Err              err;
    WAVToMP4Options     options;
    WAVData             wavData;
    PCMFormatData       pcmFormatData;

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
    
    logMsg(LOGLEVEL_INFO, "WAV to MP4 started.");
    printOptions(&options);
    
    err = initWAVDataForReading(&wavData, options.inputFile);    if (err) goto bail;
    err = setPCMFormatDataFromWAV(&wavData, &pcmFormatData);     if (err) goto bail;
    err = createMP4Movie(&wavData, &pcmFormatData, &options);    if (err) goto bail;
    
    err = freeWAVData(&wavData);        if (err) goto bail;
    freeOptions(&options);
    logMsg(LOGLEVEL_INFO, "WAV to MP4 finished.");
bail:
    fflush(stdout);
	return err;
}

MP4Err  createMP4Movie      (WAVData *wavData, PCMFormatData *pcmFormatData, WAVToMP4Options *options)
{
    MP4Err                      err;
    MP4Movie                    moov;
	MP4Track                    trak;
	MP4Media                    media;
    MP4MediaAtomPtr             mdia;
    MP4AudioSampleEntryAtomPtr  sampleEntry;
	u8                          OD_profileAndLevel;
	u8                          scene_profileAndLevel;
	u8                          audio_profileAndLevel;
	u8                          visual_profileAndLevel;
	u8                          graphics_profileAndLevel;
	u32                         initialObjectDescriptorID;
	u64                         mediaDuration;

    logMsg(LOGLEVEL_INFO, "Creating MP4 movie.");
    
    err                         = MP4NoErr;
    initialObjectDescriptorID   = 0; /* if 0, no descriptor will be generated */
	OD_profileAndLevel          = 0xff; /* none required */
	scene_profileAndLevel       = 0xff; /* none required */
	audio_profileAndLevel       = 0xff; /* none required */
	visual_profileAndLevel      = 0xff; /* none required */
	graphics_profileAndLevel    = 0xff; /* none required */
    
    err = MP4NewMovie( &moov,
                      initialObjectDescriptorID,
                      OD_profileAndLevel,
                      scene_profileAndLevel,
                      audio_profileAndLevel,
                      visual_profileAndLevel,
                      graphics_profileAndLevel ); if (err) goto bail;
    
    err = MP4SetMovieTimeScale( moov, pcmFormatData->sampleRate );                                  if (err) goto bail;
    err = MP4NewMovieTrack( moov, MP4NewTrackIsAudio, &trak );                                      if (err) goto bail;
	err = MP4NewTrackMedia( trak, &media, MP4AudioHandlerType, pcmFormatData->sampleRate, NULL );   if (err) goto bail;
    err = MP4SetMediaLanguage( media, "und" );                                                      if (err) goto bail;

    mdia    = (MP4MediaAtomPtr) media;
    
    err = createAudioSampleEntry(&sampleEntry, pcmFormatData, options);             if (err) goto bail;
    err = mdia->setSampleEntry( mdia, (MP4AtomPtr) sampleEntry );                   if (err) goto bail;

	err = MP4BeginMediaEdits( media );                                              if (err) goto bail;

    err = addAudioSamples( trak, media, wavData, options );                         if (err) goto bail;
    err = MP4EndMediaEdits( media );                                                if (err) goto bail;

	err = MP4GetMediaDuration( media, &mediaDuration );                             if (err) goto bail;
	err = MP4InsertMediaIntoTrack( trak, 0, 0, mediaDuration, 1 );                  if (err) goto bail;

	err = MP4WriteMovieToFile( moov, options->outputFile );                         if (err) goto bail;
    
    err = MP4DisposeMovie(moov);                                                    if (err) goto bail;
    logMsg(LOGLEVEL_DEBUG, "Creating MP4 movie finished successfully.");
bail:
	return err;
}

MP4Err  addAudioSamples (MP4Track trak, MP4Media media, WAVData *wavData, WAVToMP4Options *options)
{
    MP4Err      err;
	MP4Handle   sampleDataH;
	MP4Handle   sampleDurationH;
	MP4Handle   sampleSizeH;
    u32         i;
	
    logMsg(LOGLEVEL_INFO, "Adding samples to MP4 file.");
    
    i   = 0;
	err = MP4NoErr;
    err = MP4NewHandle( 0, &sampleDataH );                  if (err) goto bail;
	err = MP4NewHandle( sizeof(u32), &sampleDurationH );    if (err) goto bail;
	err = MP4NewHandle( sizeof(u32), &sampleSizeH );        if (err) goto bail;
    
    MP4SetHandleSize( sampleDataH, (u32) wavData->totalSamplesPerChannel * wavData->channels * wavData->byteDepth );

    char* bufferPtr;
    bufferPtr = (char *) *sampleDataH;

    size_t elementsRead;
    unsigned long  int sampleCount;
    sampleCount = 0;
    do {
        elementsRead = fread(bufferPtr, wavData->channels * wavData->byteDepth, CHUNKSIZE, wavData->file);
        bufferPtr += elementsRead * wavData->channels * wavData->byteDepth;
        sampleCount += elementsRead;
    } while (elementsRead > 0);

    if (sampleCount != wavData->totalSamplesPerChannel) {
        logMsg(LOGLEVEL_ERROR, "Data size of WAV file not matching header %d %d", wavData->byteDepth * wavData->channels * wavData->totalSamplesPerChannel, sampleCount);
        err = MP4InvalidMediaErr;
        goto bail;
    }

    logMsg(LOGLEVEL_DEBUG, "Adding chunk 1 with %d samples", wavData->totalSamplesPerChannel);
    *((u32*) *sampleDurationH)  = (u32) 1;
    *((u32*) *sampleSizeH)      = (u32) wavData->byteDepth * wavData->channels;
    err = MP4AddMediaSamples( media, sampleDataH, (u32) wavData->totalSamplesPerChannel, sampleDurationH, sampleSizeH, NULL, NULL, NULL ); if (err) goto bail;
    err = MP4DisposeHandle(sampleDataH);        if (err) goto bail;
    err = MP4DisposeHandle(sampleDurationH);    if (err) goto bail;
    err = MP4DisposeHandle(sampleSizeH);        if (err) goto bail;
    logMsg(LOGLEVEL_DEBUG, "Adding samples to MP4 file finished successfully.");
bail:
	return err;
}

MP4Err  createAudioSampleEntry  (MP4AudioSampleEntryAtomPtr *sampleEntry, PCMFormatData *pcmFormatData, WAVToMP4Options *options)
{
    MP4Err err;
    
    logMsg(LOGLEVEL_DEBUG, "Creating audio sample entry for raw pcm audio data");
    
    err = MP4CreateAudioSampleEntryAtom( sampleEntry ); if (err) goto bail;
    err = addPCMDataToAudioSampleEntry (pcmFormatData, *sampleEntry);  if (err) goto bail;

    logMsg(LOGLEVEL_DEBUG, "Creating audio sample entry finished");
bail:
	return err;
}
