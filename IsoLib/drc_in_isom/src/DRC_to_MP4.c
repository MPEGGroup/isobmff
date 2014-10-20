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
#include "DRCtoMP4Options.h"
#include "Testing.h"
#include "WAVData.h"
#include "DRCData.h"
#include "InStreamDrcToFileFormatDrc.h"


MP4Err  createMP4Movie          (WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options);
MP4Err  createAudioSampleEntry  (MP4AudioSampleEntryAtomPtr *sampleEntry, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options);
MP4Err  addAudioSamples         (MP4Track trak, MP4Media media, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options);
MP4Err  addDRCMetadataTrack     (MP4Movie moov, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options);
MP4Err  addDRCSamples           (MP4Track trak, MP4Media media, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options);

int     main                ( int argc, char **argv )
{
    MP4Err              err;
    DRCtoMP4Options     options;
    WAVData             wavData;
    DRCData             drcData;
    
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
    
    logMsg(LOGLEVEL_INFO, "DRC to MP4 started.\n");
    printOptions(&options);
    
    err = initWAVDataForReading(&wavData, options.wavInputFile);                                                    if (err) goto bail;
    err = initDRCData(&drcData, options.drcBitstreamInputFile, wavData.sampleRate, wavData.channels, BLOCKLENGTH);  if (err) goto bail;
    err = createMP4Movie(&wavData, &drcData, &options);                                                             if (err) goto bail;
    
    logMsg(LOGLEVEL_INFO, "DRC to MP4 finished.");
bail:
    fflush(stdout);
	return err;
}

MP4Err  createMP4Movie      (WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options)
{
    MP4Err                      err;
    MP4Movie                    moov;
	MP4Track                    trak;
	MP4Media                    media;
    MP4Handle                   sampleEntryH;
    MP4MediaAtomPtr             mdia;
    MP4AudioSampleEntryAtomPtr  sampleEntry;
	u8                          OD_profileAndLevel;
	u8                          scene_profileAndLevel;
	u8                          audio_profileAndLevel;
	u8                          visual_profileAndLevel;
	u8                          graphics_profileAndLevel;
	u32                         initialObjectDescriptorID;
	u64                         mediaDuration;
    u32                         outReferenceNumber;
    
    
    logMsg(LOGLEVEL_INFO, "Creating MP4 movie.");
    
    err                         = MP4NoErr;
    initialObjectDescriptorID   = 1;
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
    
    err = MP4NewMovieTrack( moov, MP4NewTrackIsAudio, &trak );                                  if (err) goto bail;
    err = MP4AddTrackToMovieIOD( trak );                                                        if (err) goto bail;
	err = MP4NewTrackMedia( trak, &media, MP4AudioHandlerType, wavData->sampleRate, NULL );     if (err) goto bail;
    err = MP4NewHandle( 0, &sampleEntryH );                                                     if (err) goto bail;
    err = MP4SetMediaLanguage( media, "eng" );                                                  if (err) goto bail;
    err = MP4SetMediaExtendedLanguageTag( media, "en-US" );                                     if (err) goto bail;
    
    mdia    = (MP4MediaAtomPtr) media;
    
    err = addLoudnessInfoToTrackAtom(drcData, trak);                                if (err) goto bail;
    
    err = createAudioSampleEntry(&sampleEntry, wavData, drcData, options);          if (err) goto bail;
    err = mdia->setSampleEntry( mdia, (MP4AtomPtr) sampleEntry );                   if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Adding 'adrc' reference to audio track with id %d", 2);
    err = MP4AddTrackReferenceWithID(trak, 2, DRCTrackReferenceAtomType, &outReferenceNumber);
    if (err) goto bail;
    
	err = MP4BeginMediaEdits( media );                                              if (err) goto bail;
    err = addAudioSamples( trak, media, wavData, drcData, options );                if (err) goto bail;
    err = MP4EndMediaEdits( media );                                                if (err) goto bail;
    
	err = MP4GetMediaDuration( media, &mediaDuration );                             if (err) goto bail;
	err = MP4InsertMediaIntoTrack( trak, 0, 0, mediaDuration, 1 );                  if (err) goto bail;
    
    
    err = addDRCMetadataTrack(moov, wavData, drcData, options);                     if (err) goto bail;
    
	err = MP4WriteMovieToFile( moov, options->outputFile );                         if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Creating MP4 movie finished successfully.");
bail:
	return err;
}

MP4Err  addAudioSamples         (MP4Track trak, MP4Media media, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options)
{
    MP4Err                  err;
	MP4Handle               sampleDataH;
	MP4Handle               sampleDurationH;
	MP4Handle               sampleSizeH;
    u32                     i;
    u32                     duration;
	
    logMsg(LOGLEVEL_INFO, "Adding samples to MP4 file.");
    
    i       = 0;
	err     = MP4NoErr;
    err     = MP4NewHandle( 0, &sampleDataH );                  if (err) goto bail;
	err     = MP4NewHandle( sizeof(u32), &sampleDurationH );    if (err) goto bail;
	err     = MP4NewHandle( sizeof(u32), &sampleSizeH );        if (err) goto bail;
    
    while (readFrame(wavData, sampleDataH, &duration) == MP4NoErr)
    {
        i++;
        err = MP4GetHandleSize( sampleDataH, (u32*) *sampleSizeH ); if (err) goto bail;
        logMsg(LOGLEVEL_DEBUG, "Adding frame #%d. samples: %d", i, duration);
        *((u32*) *sampleDurationH)  = duration;
        *((u32*) *sampleSizeH)      = duration * wavData->byteDepth * wavData->channels;
		err = MP4AddMediaSamples( media, sampleDataH, 1, sampleDurationH, sampleSizeH, NULL, NULL, NULL ); if (err) goto bail;
    }
    
    logMsg(LOGLEVEL_DEBUG, "Adding samples to MP4 file finished successfully.");
bail:
	return err;
}

MP4Err  createAudioSampleEntry  (MP4AudioSampleEntryAtomPtr *sampleEntry, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options)
{
    MP4Err err;
    
    logMsg(LOGLEVEL_DEBUG, "Creating audio sample entry for raw pcm audio data");
    
    err = MP4CreateAudioSampleEntryAtom( sampleEntry ); if (err) goto bail;
    
    (*sampleEntry)->timeScale             = wavData->sampleRate;
    (*sampleEntry)->dataReferenceIndex    = 1;
    (*sampleEntry)->reserved3             = wavData->channels;
    (*sampleEntry)->reserved4             = wavData->byteDepth * 8;
    (*sampleEntry)->type                  = AudioSigned16BitLittleEndianSampleEntryType;
    
    err = addDRCDataToAudioSampleEntry(drcData, *sampleEntry); if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Creating audio sample entry finished");
bail:
	return err;
}

MP4Err  addDRCMetadataTrack     (MP4Movie moov, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options)
{
    MP4Err                          err;
    MP4Track                        trak;
    MP4Media                        media;
    MP4Handle                       sampleEntryH;
    MP4MediaAtomPtr                 mdia;
    DRCUniDrcSampleEntryAtomPtr     sampleEntry;
    u64                             mediaDuration;
    
    logMsg(LOGLEVEL_INFO, "Adding DRC meta data track.");
    
    err = MP4NoErr;
    err = MP4NewMovieTrack( moov, MP4NewTrackIsMetadata, &trak );                               if (err) goto bail;
    err = MP4NewTrackMedia( trak, &media, MP4MetaHandlerType, wavData->sampleRate, NULL );      if (err) goto bail;
    err = MP4NewHandle( 0, &sampleEntryH );                                                     if (err) goto bail;
    err = MP4SetMediaLanguage( media, "eng" );                                                  if (err) goto bail;
    err = MP4SetMediaExtendedLanguageTag( media, "en-US" );                                     if (err) goto bail;
    
    mdia    = (MP4MediaAtomPtr) media;
    
    err = MP4CreateDRCUniDrcSampleEntryAtom(&sampleEntry);                          if (err) goto bail;
    sampleEntry->dataReferenceIndex = 1;
    err = mdia->setSampleEntry( mdia, (MP4AtomPtr) sampleEntry );                   if (err) goto bail;
    
    err = MP4BeginMediaEdits( media );                                              if (err) goto bail;
    err = addDRCSamples( trak, media, wavData, drcData, options );                  if (err) goto bail;
    err = MP4EndMediaEdits( media );                                                if (err) goto bail;
    
    err = MP4GetMediaDuration( media, &mediaDuration );                             if (err) goto bail;
    err = MP4InsertMediaIntoTrack( trak, 0, 0, mediaDuration, 1 );                  if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Adding DRC meta data track finished successfully.");
bail:
    return err;
}

MP4Err  addDRCSamples         (MP4Track trak, MP4Media media, WAVData *wavData, DRCData *drcData, DRCtoMP4Options *options)
{
    MP4Err                  err;
    MP4Handle               sampleDataH;
    MP4Handle               sampleDurationH;
    MP4Handle               sampleSizeH;
    u32                     i;
    u32                     duration;
    
    logMsg(LOGLEVEL_INFO, "Adding DRC samples to MP4 file.");
    
    i           = 0;
    duration    = BLOCKLENGTH;
    err         = MP4NoErr;
    err         = MP4NewHandle( 0, &sampleDataH );                  if (err) goto bail;
    err         = MP4NewHandle( sizeof(u32), &sampleDurationH );    if (err) goto bail;
    err         = MP4NewHandle( sizeof(u32), &sampleSizeH );        if (err) goto bail;
    
    while (nextUniDRCGain(drcData, sampleDataH) == MP4NoErr)
    {
        i++;
        err = MP4GetHandleSize( sampleDataH, (u32*) *sampleSizeH ); if (err) goto bail;
        logMsg(LOGLEVEL_DEBUG, "Adding DRC sample #%d. Duration: %d. Size: %d", i, duration, *((u32*) *sampleSizeH));
        *((u32*) *sampleDurationH)  = duration;
        err = MP4AddMediaSamples( media, sampleDataH, 1, sampleDurationH, sampleSizeH, NULL, NULL, NULL ); if (err) goto bail;
    }
    
    logMsg(LOGLEVEL_DEBUG, "Adding DRC samples to MP4 file finished successfully.");
bail:
    return err;
}