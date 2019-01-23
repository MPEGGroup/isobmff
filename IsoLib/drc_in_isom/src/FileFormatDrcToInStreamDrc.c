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

#include "FileFormatDrcToInStreamDrc.h"
#include "Logger.h"
#include "WAVData.h"

#include <uniDrcTables.h>


MP4Err  createSampleEntryFromBuffer         (MP4Track trak, MP4AudioSampleEntryAtomPtr *audioSampleEntry);
MP4Err  collectSampleEntryAtoms             (MP4AudioSampleEntryAtomPtr audioSampleEntry, StaticDrcData *staticDrcData);
MP4Err  getLoudnessInfoAtomFromTrack        (MP4Track trak, StaticDrcData *staticDrcData);
MP4Err  estimateBistreamBufferSize          (StaticDrcData *staticDrcData, int *outSizeInBytes);

MP4Err  writeUniDrcConfigToBitstream                (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeChannelLayoutToBitstream               (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeDownMixInstructionsToBitstream         (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeCoeffBasicsToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeInstrBasicsToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeCoeffUniDrcToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeInstrUniDrcToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeLoudnessInfoSetToBitstream             (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);
MP4Err  writeLoudnessBaseBoxToBitstream             (MP4LoudnessBaseAtomPtr loudnessBaseBox, DrcBitstreamHandle *drcBitStreamHandle);

MP4Err  writeCoeffUniDrcSequencesToBitstream        (DRCCoefficientUniDRCAtomPtr coeffUniDRCAtom, DrcBitstreamHandle *drcBitStreamHandle);


MP4Err  createStaticDrcDataFromAudioTrack   (MP4Track trak, StaticDrcData *staticDrcData)
{
    MP4Err                          err;
    MP4AudioSampleEntryAtomPtr      audioSampleEntry;

    logMsg(LOGLEVEL_DEBUG, "Creating static drc data from audio track");
    
    err = MP4NoErr;
    err = createSampleEntryFromBuffer(trak, &audioSampleEntry);             if (err) goto bail;
    
    staticDrcData->channelCount         = audioSampleEntry->reserved3;
    staticDrcData->sampleRate           = audioSampleEntry->timeScale;
    staticDrcData->bytesPerSample       = audioSampleEntry->reserved4 / 8;
    staticDrcData->drcLocation          = 1; // maybe optional later
    
    logMsg(LOGLEVEL_INFO, "Audio data from sample entry: channels: %d, sampleRate: %d, bytesPerSample: %d",
           staticDrcData->channelCount, staticDrcData->sampleRate, staticDrcData->bytesPerSample);
    
    staticDrcData->loudnessAtom         = NULL;
    staticDrcData->channelLayoutAtom    = NULL;
    
    err = MP4MakeLinkedList(&staticDrcData->downMixInstructionsAtoms);          if (err) goto bail;
    err = MP4MakeLinkedList(&staticDrcData->drcCoefficientsBasicAtoms);         if (err) goto bail;
    err = MP4MakeLinkedList(&staticDrcData->drcInstructionsBasicAtoms);         if (err) goto bail;
    err = MP4MakeLinkedList(&staticDrcData->drcCoefficientsUniDrcAtoms);        if (err) goto bail;
    err = MP4MakeLinkedList(&staticDrcData->drcInstructionsUniDrcAtoms);        if (err) goto bail;
    
    err = collectSampleEntryAtoms(audioSampleEntry, staticDrcData);             if (err) goto bail;
    err = getLoudnessInfoAtomFromTrack(trak, staticDrcData);                    if (err) goto bail;
    
    staticDrcData->sampleEntryAtom = audioSampleEntry;
    logMsg(LOGLEVEL_DEBUG, "Creating static drc data from audio track finished.");
bail:
    return err;
}

MP4Err  initDrcBitstream                    (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err      err;
    
    logMsg(LOGLEVEL_DEBUG, "Initializing DRC bitstream.");
    
    err = MP4NoErr;
    err = estimateBistreamBufferSize(staticDrcData, &drcBitStreamHandle->totalBufferSizeInBytes); if (err) goto bail;
    
    drcBitStreamHandle->currentBytePosition = 0;
    drcBitStreamHandle->offsetInBits        = 0;
    drcBitStreamHandle->bitstreamBuffer     = calloc(drcBitStreamHandle->totalBufferSizeInBytes, 1);
    
    drcBitStreamHandle->bitstream = calloc(1, sizeof(wobitbuf));
    wobitbuf_Init(drcBitStreamHandle->bitstream, drcBitStreamHandle->bitstreamBuffer, drcBitStreamHandle->totalBufferSizeInBytes * 8, 0);
    
    logMsg(LOGLEVEL_DEBUG, "Initializing DRC bitstream finished. Estimated buffer size for static DRC data: %d bytes", drcBitStreamHandle->totalBufferSizeInBytes);
bail:
    return err;
}

MP4Err  writeStaticDrcDataToBitstream       (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err      err;
    int         bitErr;
    
    logMsg(LOGLEVEL_DEBUG, "Writing static DRC data to bitstream started.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1); // uniDrcLoudnessInfoSetPresent
    if (bitErr) BAILWITHERROR(MP4IOErr);
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1); // uniDrcConfigPresent
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    err = writeUniDrcConfigToBitstream(staticDrcData, drcBitStreamHandle);      if (err) goto bail;
    
    err = writeLoudnessInfoSetToBitstream(staticDrcData, drcBitStreamHandle);   if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Writing static DRC data to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
bail:
    return err;
}

MP4Err  prepareDrcBitStreamHelper           (DrcBitStreamHelper *drcBitStreamHelper, DrcBitstreamHandle *drcBitStreamHandle, StaticDrcData *staticDrcData)
{
    MP4Err      err;
    int         bitErr;
    int         bitsRead;
    int         bytesRead;
    
    logMsg(LOGLEVEL_DEBUG, "Preparing DRC bitstream helper.");
    
    bitErr                              = 0;
    err                                 = MP4NoErr;
    drcBitStreamHelper->gainCount       = 0;
    
    drcBitStreamHelper->hUniDrcBsDecStruct     = NULL;
    drcBitStreamHelper->hUniDrcConfig          = NULL;
    drcBitStreamHelper->hLoudnessInfoSet       = NULL;
    drcBitStreamHelper->hUniDrcGain            = NULL;
    
    
    bitErr = openUniDrcBitstreamDec(&drcBitStreamHelper->hUniDrcBsDecStruct, &drcBitStreamHelper->hUniDrcConfig,
                                    &drcBitStreamHelper->hLoudnessInfoSet, &drcBitStreamHelper->hUniDrcGain);
    if (bitErr) BAILWITHERROR(MP4InternalErr);
    
    bitErr = initUniDrcBitstreamDec(drcBitStreamHelper->hUniDrcBsDecStruct, staticDrcData->sampleRate, 1024, 0, -1, NULL);
    if (bitErr) BAILWITHERROR(MP4InternalErr);
    
    bitErr = processUniDrcBitstreamDec_uniDrcConfig(drcBitStreamHelper->hUniDrcBsDecStruct,
                                                    drcBitStreamHelper->hUniDrcConfig,
                                                    drcBitStreamHelper->hLoudnessInfoSet,
                                                    drcBitStreamHandle->bitstreamBuffer,
                                                    drcBitStreamHandle->totalBufferSizeInBytes,
                                                    0, &bitsRead);
    
    bytesRead                                      = bitsRead / 8;
    drcBitStreamHandle->offsetInBits               = bitsRead - bytesRead * 8;
    drcBitStreamHandle->currentBytePosition        += bytesRead;
    
    logMsg(LOGLEVEL_DEBUG, "Preparing DRC bitstream helper finished: Static drc data size: %d bits", bitsRead);
    
    if (bitErr) BAILWITHERROR(MP4InternalErr);
    
bail:
    return err;
}

MP4Err  writeDrcGainToBitstream             (MP4Handle packetH, DrcBitstreamHandle *drcBitStreamHandle, DrcBitStreamHelper *drcBitStreamHelper)
{
    MP4Err              err;
    int                 bitErr;
    u32                 size;
    unsigned char        *buffer;
    int                 bitsRead;
    int                 bytesRead;
    int                 bytesLeft;
    unsigned char       offsetByteValue;
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    err     = MP4GetHandleSize( packetH, &size );   if (err) goto bail;
    buffer  = (unsigned char *) *packetH;
    
    drcBitStreamHandle->totalBufferSizeInBytes += size;
    drcBitStreamHandle->bitstreamBuffer = realloc(drcBitStreamHandle->bitstreamBuffer, drcBitStreamHandle->totalBufferSizeInBytes);
    
    logMsg(LOGLEVEL_TRACE, "Writing DRC gain to bitbuffer. Extending buffersizer: New size: %d", drcBitStreamHandle->totalBufferSizeInBytes);
    
    bytesLeft = drcBitStreamHandle->totalBufferSizeInBytes - drcBitStreamHandle->currentBytePosition;
    wobitbuf_Init(drcBitStreamHandle->bitstream, &drcBitStreamHandle->bitstreamBuffer[drcBitStreamHandle->currentBytePosition], bytesLeft * 8, 0);
    
    err = wobitbuf_Seek(drcBitStreamHandle->bitstream, drcBitStreamHandle->offsetInBits);
    if (err) return(err);
    
    if (drcBitStreamHelper->gainCount != 0)
    {
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1); // uniDrcLoudnessInfoSetPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
        drcBitStreamHandle->offsetInBits += 1;
    }
    
    bitErr = processUniDrcBitstreamDec_uniDrcGain(drcBitStreamHelper->hUniDrcBsDecStruct,
                                                  drcBitStreamHelper->hUniDrcConfig,
                                                  drcBitStreamHelper->hUniDrcGain,
                                                  buffer, size, 0, &bitsRead);
    
    if (bitErr > PROC_COMPLETE)
    {
        BAILWITHERROR(MP4IOErr);
    }
    
    drcBitStreamHelper->gainCount++;
    logMsg(LOGLEVEL_TRACE, "Gain #%d: %d bits", drcBitStreamHelper->gainCount, bitsRead);
    
    bytesRead                                      = bitsRead / 8;
    
    bitErr = wobitbuf_WriteBytes(drcBitStreamHandle->bitstream, buffer, bytesRead);
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    if ((bitsRead % 8) != 0)
    {
        offsetByteValue = buffer[bytesRead] >> (8 - bitsRead % 8);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, offsetByteValue, bitsRead - bytesRead * 8);
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    bitsRead += drcBitStreamHandle->offsetInBits;
    bytesRead                                      = bitsRead / 8;
    drcBitStreamHandle->offsetInBits               = bitsRead - bytesRead * 8;
    drcBitStreamHandle->currentBytePosition        += bytesRead;
    
bail:
    return err;
}

MP4Err  writeUniDrcConfigToBitstream        (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err      err;
    int         bitErr;
    
    logMsg(LOGLEVEL_DEBUG, "Writing UniDrcConfig to bitstream");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                       // sampleRatePresent
    if (bitErr) BAILWITHERROR(MP4IOErr);
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->sampleRate - 1000, 18);       // bsSampleRate
    if (bitErr) BAILWITHERROR(MP4IOErr);
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->downMixInstructionsAtoms->entryCount, 7);         // downMixInstructionsCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    if (staticDrcData->drcCoefficientsBasicAtoms->entryCount != 0)
    {
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);      // drcDescriptionBasicPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcCoefficientsBasicAtoms->entryCount, 3);      // drcCoefficientsBasicCount
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcInstructionsBasicAtoms->entryCount, 4);      // drcInstructionsBasicCount
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    else
    {
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);      // drcDescriptionBasicPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcCoefficientsUniDrcAtoms->entryCount, 3);      // drcCoefficientsUniDrcCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcInstructionsUniDrcAtoms->entryCount, 6);      // drcInstructionsUniDrcCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    
    err = writeChannelLayoutToBitstream(staticDrcData, drcBitStreamHandle);         if (err) goto bail;
    err = writeDownMixInstructionsToBitstream(staticDrcData, drcBitStreamHandle);   if (err) goto bail;
    err = writeCoeffBasicsToBitstream(staticDrcData, drcBitStreamHandle);           if (err) goto bail;
    err = writeInstrBasicsToBitstream(staticDrcData, drcBitStreamHandle);           if (err) goto bail;
    err = writeCoeffUniDrcToBitstream(staticDrcData, drcBitStreamHandle);           if (err) goto bail;
    err = writeInstrUniDrcToBitstream(staticDrcData, drcBitStreamHandle);           if (err) goto bail;
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);      // uniDrcConfigExtPresent
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    logMsg(LOGLEVEL_DEBUG, "Writing UniDrcConfig to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
bail:
    return err;
}

MP4Err  writeChannelLayoutToBitstream       (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err      err;
    int         bitErr;
    
    logMsg(LOGLEVEL_DEBUG, "Writing ChannelLayout to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->channelCount, 7);         // baseChannelCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    /* TODO: Write definedLayout if defined in ChannelLayoutAtom */
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                   // layoutSignalingPresent
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    logMsg(LOGLEVEL_DEBUG, "Writing ChannelLayout to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
bail:
    return err;
}

MP4Err  writeDownMixInstructionsToBitstream         (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err                          err;
    int                             bitErr;
    u32                             i;
    MP4DownMixInstructionsAtomPtr   downMixInstrAtom;
    
    logMsg(LOGLEVEL_DEBUG, "Writing DownMixInstructions to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    for (i = 0; i < staticDrcData->downMixInstructionsAtoms->entryCount; i++)
    {
        err = MP4GetListEntry(staticDrcData->downMixInstructionsAtoms, i, (char **) &downMixInstrAtom); if (err) goto bail;
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, downMixInstrAtom->downmix_ID, 7);                // downmixId
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, downMixInstrAtom->targetChannelCount, 7);        // targetChannelCount
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, downMixInstrAtom->targetLayout, 8);              // targetLayout
        if (bitErr) BAILWITHERROR(MP4IOErr);
        
        if (downMixInstrAtom->in_stream == 0)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                       // downmixCoefficientsPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            for (u16 x = 0; x < downMixInstrAtom->targetChannelCount; x++)
            {
                for (u16 y = 0; y < staticDrcData->channelCount; y++)
                {
                    u32 index = (x * staticDrcData->channelCount) + y;
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, downMixInstrAtom->bs_downmix_coefficients[index], 4);    // bsDownmixCoefficient
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                }
            }
        }
        else
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                       // downmixCoefficientsPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
        }
    }
    
    logMsg(LOGLEVEL_DEBUG, "Writing DownMixInstructions to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
    
bail:
    return err;
}

MP4Err  writeCoeffBasicsToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err                          err;
    int                             bitErr;
    u32                             i;
    DRCCoefficientBasicAtomPtr      coeffBasicAtom;
    
    logMsg(LOGLEVEL_DEBUG, "Writing CoefficientsBasics to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    for (i = 0; i < staticDrcData->drcCoefficientsBasicAtoms->entryCount; i++)
    {
        err = MP4GetListEntry(staticDrcData->drcCoefficientsBasicAtoms, i, (char **) &coeffBasicAtom); if (err) goto bail;
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcLocation, 4);              //drcLocation
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, coeffBasicAtom->DRC_characteristic, 7);      //drcCharacteristic
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    
    logMsg(LOGLEVEL_DEBUG, "Writing CoefficientsBasics to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
bail:
    return err;
}

MP4Err  writeInstrBasicsToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err                          err;
    int                             bitErr;
    u32                             i;
    DRCInstructionsBasicAtomPtr     instrBasicAtom;
    
    logMsg(LOGLEVEL_DEBUG, "Writing InstructionssBasics to bitstream. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    for (i = 0; i < staticDrcData->drcInstructionsBasicAtoms->entryCount; i++)
    {
        err = MP4GetListEntry(staticDrcData->drcInstructionsBasicAtoms, i, (char **) &instrBasicAtom); if (err) goto bail;
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->DRC_set_ID, 6);                      //drcSetId
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcLocation, 4);                      //drcLocation
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->downmix_ID, 7);                      //downmixId
        if (bitErr) BAILWITHERROR(MP4IOErr);
        
        if (instrBasicAtom->additional_dowmix_ID_count != 0)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                           //additionalDownmixIdPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->additional_dowmix_ID_count, 3);  //additionalDownmixIdCount
            if (bitErr) BAILWITHERROR(MP4IOErr);
            for (int j = 0; j < instrBasicAtom->additional_dowmix_ID_count; j++)
            {
                DRCInstructionsAdditionalDownMixID    *downMixID;
                err = MP4GetListEntry(instrBasicAtom->additionalDownMixIDs, j, (char **) &downMixID); if (err) goto bail;
            
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, downMixID->additional_dowmix_ID, 7);         //additionalDownmixId
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
        }
        else
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                           //additionalDownmixIdPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
        }
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->DRC_set_effect, 16);                 //drcSetEffect
        if (bitErr) BAILWITHERROR(MP4IOErr);
        
        if ((instrBasicAtom->DRC_set_effect & (1 << 10)) == 0)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->limiter_peak_target_present, 1); //limiterPeakTargetPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            if (instrBasicAtom->limiter_peak_target_present == 1)
            {
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->bs_limiter_peak_target, 8); //limiterPeakTarget
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
        }
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->DRC_set_target_loudness_present, 1); //drcSetTargetLoudnessPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
        if (instrBasicAtom->DRC_set_target_loudness_present == 1)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->bs_DRC_set_target_loudness_value_upper, 6);  //drcSetTargetLoudnessValueUpper
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                                       //drcSetTargetLoudnessValueLowerPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrBasicAtom->bs_DRC_set_target_loudness_value_lower, 6);  //drcSetTargetLoudnessValueLower
        }
    }
    
    logMsg(LOGLEVEL_DEBUG, "Writing InstructionssBasics to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
    
bail:
    return err;
}

MP4Err  writeCoeffUniDrcToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err                          err;
    int                             bitErr;
    u32                             i;
    DRCCoefficientUniDRCAtomPtr     coeffUniDrcAtom;
    
    logMsg(LOGLEVEL_DEBUG, "Writing CoefficientUniDRCs to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    for (i = 0; i < staticDrcData->drcCoefficientsUniDrcAtoms->entryCount; i++)
    {
        err = MP4GetListEntry(staticDrcData->drcCoefficientsUniDrcAtoms, i, (char **) &coeffUniDrcAtom); if (err) goto bail;
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcLocation, 4);                      //drcLocation
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, coeffUniDrcAtom->drc_frame_size_present, 1);         //drcFrameSizePresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
        
        if (coeffUniDrcAtom->drc_frame_size_present == 1)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, coeffUniDrcAtom->bs_drc_frame_size, 15);         //bsDrcFrameSize
            if (bitErr) BAILWITHERROR(MP4IOErr);
        }
        
        err = writeCoeffUniDrcSequencesToBitstream(coeffUniDrcAtom, drcBitStreamHandle); if (err) goto bail;
    }
    
    logMsg(LOGLEVEL_DEBUG, "Writing CoefficientUniDRCs to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
    
bail:
    return err;
}

MP4Err  writeCoeffUniDrcSequencesToBitstream        (DRCCoefficientUniDRCAtomPtr coeffUniDRCAtom, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err                          err;
    int                             bitErr;
    u32                             i;
    DRCCoefficientUniDRCSequence    *sequence;
    
    logMsg(LOGLEVEL_TRACE, "Writing CoefficientUniDRC sequences to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, coeffUniDRCAtom->sequences->entryCount, 6);          //sequenceCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    for (i = 0; i < coeffUniDRCAtom->sequences->entryCount; i++)
    {
        err = MP4GetListEntry(coeffUniDRCAtom->sequences, i, (char **) &sequence); if (err) goto bail;
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->gain_coding_profile, 2);               //gainCodingProfile
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->gain_interpolation_type, 1);           //gainInterpolationType
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->full_frame, 1);                        //fullFrame
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->time_alignment, 1);                    //timeAlignment
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->time_delta_min_present, 1);            //timeDeltaMinPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
  
        if (sequence->time_delta_min_present == 1)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->bs_time_delta_min, 11);            //bsTimeDeltaMin
            if (bitErr) BAILWITHERROR(MP4IOErr);
        }
        
        if (sequence->gain_coding_profile != 3)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->band_count, 4);                    //bandCount
            if (bitErr) BAILWITHERROR(MP4IOErr);
            
            if (sequence->band_count > 1)
            {
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequence->drc_band_type, 1);             //drcBandType
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
            
            for (u8 j = 0; j < sequence->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandCharacteristic  *bandCharacteristic;
                MP4GetListEntry(sequence->bandCharacteristics, j, (char **) &bandCharacteristic);
                
                
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, bandCharacteristic->DRC_characteristic, 7);  //drcCharacteristic
                if (bitErr) BAILWITHERROR(MP4IOErr);
                
            }
            
            for (u8 j = 1; j < sequence->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandIndex           *bandIndex;
                MP4GetListEntry(sequence->bandIndexes, j-1, (char **) &bandIndex);
                
                if (sequence->drc_band_type == 1)
                {
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, bandIndex->crossover_freq_index, 4);     //crossoverFreqIndex
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                }
                else
                {
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, bandIndex->start_sub_band_index, 10);    //startSubBandIndex
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                }
            }

        }
    }
    
    logMsg(LOGLEVEL_TRACE, "Writing CoefficientUniDRC sequences to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
    
bail:
    return err;
}

MP4Err  writeInstrUniDrcToBitstream                 (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err                          err;
    int                             bitErr;
    u32                             i;
    DRCInstructionsUniDRCAtomPtr    instrUniDrcAtom;
    
    logMsg(LOGLEVEL_DEBUG, "Writing InstructionsUniDRCs to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    for (i = 0; i < staticDrcData->drcInstructionsUniDrcAtoms->entryCount; i++)
    {
        err = MP4GetListEntry(staticDrcData->drcInstructionsUniDrcAtoms, i, (char **) &instrUniDrcAtom); if (err) goto bail;
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->DRC_set_ID, 6);                         //drcSetId
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->drcLocation, 4);                          //drcLocation
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->downmix_ID, 7);                         //downmixId
        if (bitErr) BAILWITHERROR(MP4IOErr);
        
        if (instrUniDrcAtom->additional_dowmix_ID_count != 0)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                               //additionalDownmixIdPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->additional_dowmix_ID_count, 3);     //additionalDownmixIdCount
            if (bitErr) BAILWITHERROR(MP4IOErr);
            for (int j = 0; j < instrUniDrcAtom->additional_dowmix_ID_count; j++)
            {
                DRCInstructionsAdditionalDownMixID    *downMixID;
                err = MP4GetListEntry(instrUniDrcAtom->additionalDownMixIDs, j, (char **) &downMixID); if (err) goto bail;
                
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, downMixID->additional_dowmix_ID, 7);             //additionalDownmixId
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
        }
        else
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                               //additionalDownmixIdPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
        }
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->DRC_set_effect, 16);                    //drcSetEffect
        if (bitErr) BAILWITHERROR(MP4IOErr);
        
        if ((instrUniDrcAtom->DRC_set_effect & (1 << 10)) == 0)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->limiter_peak_target_present, 1);    //limiterPeakTargetPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            if (instrUniDrcAtom->limiter_peak_target_present == 1)
            {
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->bs_limiter_peak_target, 8);     //limiterPeakTarget
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
        }
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->DRC_set_target_loudness_present, 1);    //drcSetTargetLoudnessPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
        if (instrUniDrcAtom->DRC_set_target_loudness_present == 1)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->bs_DRC_set_target_loudness_value_upper, 6);     //drcSetTargetLoudnessValueUpper
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                                           //drcSetTargetLoudnessValueLowerPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->bs_DRC_set_target_loudness_value_lower, 6);     //drcSetTargetLoudnessValueLower
        }
        
        if (instrUniDrcAtom->depends_on_DRC_set == 0)
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                               //dependsOnDrcSetPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->no_independent_use, 1);             //noIndependentUse
            if (bitErr) BAILWITHERROR(MP4IOErr);
        }
        else
        {
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                               //dependsOnDrcSetPresent
            if (bitErr) BAILWITHERROR(MP4IOErr);
            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, instrUniDrcAtom->depends_on_DRC_set, 6);             //dependsOnDrcSet
            if (bitErr) BAILWITHERROR(MP4IOErr);
        }
        
        if ((instrUniDrcAtom->DRC_set_effect & (1 << 10)) != 0)
        {
            u8 duckingGroupIndex[8];
            u8 offset = 0;
            for (u32 x = 0; x < instrUniDrcAtom->sequenceIndexesOfChannelGroups->entryCount; x++)
            {
                DRCInstructionsSequenceIndexOfChannelGroup  *sequenceIndexesOfChannelGroup;
                MP4GetListEntry(instrUniDrcAtom->sequenceIndexesOfChannelGroups, x, (char **) &sequenceIndexesOfChannelGroup);
                duckingGroupIndex[x] = x;
                if (sequenceIndexesOfChannelGroup->bs_sequence_index != 0)
                {
                    offset++;
                }
                else
                {
                    duckingGroupIndex[x] = duckingGroupIndex[x] - offset;
                }
            }
            
            int     repeatCount         = -1;
            int     repeatGroupIndex    = -1;
            for (u32 x = 0; x < staticDrcData->channelCount; x++)
            {
                int                                         groupIndex = -1;
                DRCInstructionsGroupIndexPerChannel         *groupIndexesPerChannel;
                DRCInstructionsChannelGroupDuckingScaling   *channelGroupDuckingScaling;
                DRCInstructionsSequenceIndexOfChannelGroup  *sequenceIndexesOfChannelGroup;
                
                MP4GetListEntry(instrUniDrcAtom->groupIndexesPerChannels, x, (char **) &groupIndexesPerChannel);
                groupIndex = groupIndexesPerChannel->channel_group_index - 1;
                MP4GetListEntry(instrUniDrcAtom->sequenceIndexesOfChannelGroups, groupIndex, (char **) &sequenceIndexesOfChannelGroup);
                
                
                if (repeatCount != -1)
                {
                    if (groupIndex == repeatGroupIndex)
                    {
                        repeatCount++;
                    }
                    else
                    {
                        if (repeatCount == 0)
                        {
                            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                           //repeatParameters
                            if (bitErr) BAILWITHERROR(MP4IOErr);
                        }
                        else
                        {
                            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                           //repeatParameters
                            if (bitErr) BAILWITHERROR(MP4IOErr);
                            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, repeatCount - 1, 5);                             //bsRepeatParametersCount
                            if (bitErr) BAILWITHERROR(MP4IOErr);
                        }
                        repeatCount = -1;
                    }
                }
                
                if (repeatCount == -1)
                {
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequenceIndexesOfChannelGroup->bs_sequence_index, 6);     //bsSequenceIndex
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                    
                    if (sequenceIndexesOfChannelGroup->bs_sequence_index != 0)
                    {
                        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);     //duckingScalingPresent
                    }
                    else
                    {
                        MP4GetListEntry(instrUniDrcAtom->channelGroupDuckingScalings, duckingGroupIndex[groupIndex], (char **) &channelGroupDuckingScaling);
                        
                        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, channelGroupDuckingScaling->ducking_scaling_present, 1);     //duckingScalingPresent
                        if (bitErr) BAILWITHERROR(MP4IOErr);
                        
                        if (channelGroupDuckingScaling->ducking_scaling_present == 1)
                        {
                            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, channelGroupDuckingScaling->bs_ducking_scaling, 4);      //duckingScaling
                            if (bitErr) BAILWITHERROR(MP4IOErr);
                        }
                    }
                    repeatCount         = 0;
                    repeatGroupIndex    = groupIndex;
                }
            }
            if (repeatCount == 0)
            {
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                           //repeatParameters
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
            else
            {
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                           //repeatParameters
                if (bitErr) BAILWITHERROR(MP4IOErr);
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, repeatCount - 1, 5);                             //bsRepeatParametersCount
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
        }
        else
        {
            u32 channelCount = staticDrcData->channelCount;
            if ((instrUniDrcAtom->downmix_ID != 0) && (instrUniDrcAtom->downmix_ID != 0x7F))
            {
                MP4DownMixInstructionsAtomPtr   downMixInstrAtom;
                for (u32 z = 0; z < staticDrcData->downMixInstructionsAtoms->entryCount; z++)
                {
                    err = MP4GetListEntry(staticDrcData->downMixInstructionsAtoms, z, (char **) &downMixInstrAtom); if (err) goto bail;
                    if (instrUniDrcAtom->downmix_ID == downMixInstrAtom->downmix_ID)
                        channelCount = downMixInstrAtom->targetChannelCount;
                }
            }
            else if (instrUniDrcAtom->downmix_ID == 0x7F)
            {
                channelCount = 1;
            }
            
            int repeatCount = -1;
            u8  repeatSequenceIndex = -1;
            for (u32 x = 0; x < channelCount; x++)
            {
                int                                         groupIndex = -1;
                DRCInstructionsGroupIndexPerChannel         *groupIndexesPerChannel;
                DRCInstructionsSequenceIndexOfChannelGroup  *sequenceIndexesOfChannelGroup;
                
                MP4GetListEntry(instrUniDrcAtom->groupIndexesPerChannels, x, (char **) &groupIndexesPerChannel);
                groupIndex = groupIndexesPerChannel->channel_group_index - 1;
                MP4GetListEntry(instrUniDrcAtom->sequenceIndexesOfChannelGroups, groupIndex, (char **) &sequenceIndexesOfChannelGroup);
                
                if (repeatCount != -1)
                {
                    if (sequenceIndexesOfChannelGroup->bs_sequence_index == repeatSequenceIndex)
                    {
                        repeatCount++;
                    }
                    else
                    {
                        if (repeatCount == 0)
                        {
                            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                           //repeatSequenceIndex
                            if (bitErr) BAILWITHERROR(MP4IOErr);
                        }
                        else
                        {
                            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                           //repeatSequenceIndex
                            if (bitErr) BAILWITHERROR(MP4IOErr);
                            bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, repeatCount - 1, 5);                             //bsRepeatSequenceCount
                            if (bitErr) BAILWITHERROR(MP4IOErr);
                        }
                        repeatCount = -1;
                    }
                }
                
                if (repeatCount == -1)
                {
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, sequenceIndexesOfChannelGroup->bs_sequence_index, 6);             //bsSequenceIndex
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                    repeatSequenceIndex = sequenceIndexesOfChannelGroup->bs_sequence_index;
                    repeatCount         = 0;
                }
            }
            
            if (repeatCount == 0)
            {
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                           //repeatSequenceIndex
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }
            else
            {
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);                                           //repeatSequenceIndex
                if (bitErr) BAILWITHERROR(MP4IOErr);
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, repeatCount - 1, 5);                             //bsRepeatSequenceCount
                if (bitErr) BAILWITHERROR(MP4IOErr);
            }

            
            for (u32 x = 0; x < instrUniDrcAtom->channelGroupGainScalings->entryCount; x++)
            {
                DRCInstructionsChannelGroupGainScaling  *channelGroupGainScaling;
                err = MP4GetListEntry(instrUniDrcAtom->channelGroupGainScalings, x, (char **) &channelGroupGainScaling); if (err) goto bail;
                
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, channelGroupGainScaling->gain_scaling_present, 1);       //gainScalingPresent
                if (bitErr) BAILWITHERROR(MP4IOErr);
                
                if (channelGroupGainScaling->gain_scaling_present == 1)
                {
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, channelGroupGainScaling->bs_attenuation_scaling, 4);         //attenuationScaling
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, channelGroupGainScaling->bs_amplification_scaling, 4);       //amplificationScaling
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                }
                
                bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, channelGroupGainScaling->gain_offset_present, 1);        //gainOffsetPresent
                if (bitErr) BAILWITHERROR(MP4IOErr);

                if (channelGroupGainScaling->gain_offset_present == 1)
                {
                    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, channelGroupGainScaling->bs_gain_offset, 6);        //bsGainOffset
                    if (bitErr) BAILWITHERROR(MP4IOErr);
                }
            }
        }
    }
    
    logMsg(LOGLEVEL_DEBUG, "Writing InstructionsUniDRCs to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
    
bail:
    return err;
}

MP4Err  writeLoudnessInfoSetToBitstream             (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err                  err;
    int                     bitErr;
    MP4LoudnessBaseAtomPtr  atom;
    
    logMsg(LOGLEVEL_DEBUG, "Writing LoudnessInfoSet to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->loudnessAtom->albumLoudnessInfoList->entryCount, 6);          //loudnessInfoAlbumCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, staticDrcData->loudnessAtom->trackLoudnessInfoList->entryCount, 6);          //loudnessInfoCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    for (u32 i = 0; i < staticDrcData->loudnessAtom->albumLoudnessInfoList->entryCount; i++)
    {
        err = MP4GetListEntry(staticDrcData->loudnessAtom->albumLoudnessInfoList, i, (char **) &atom);  if (err) goto bail;
        err = writeLoudnessBaseBoxToBitstream(atom, drcBitStreamHandle);                                if (err) goto bail;
    }
    
    for (u32 i = 0; i < staticDrcData->loudnessAtom->trackLoudnessInfoList->entryCount; i++)
    {
        err = MP4GetListEntry(staticDrcData->loudnessAtom->trackLoudnessInfoList, i, (char **) &atom);  if (err) goto bail;
        err = writeLoudnessBaseBoxToBitstream(atom, drcBitStreamHandle);                                if (err) goto bail;
    }
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);                                                                       //loudnessInfoSetExtPresent
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    logMsg(LOGLEVEL_DEBUG, "Writing LoudnessInfoSet to bitstream finsihed. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
bail:
    return err;
}

MP4Err  writeLoudnessBaseBoxToBitstream             (MP4LoudnessBaseAtomPtr loudnessBaseBox, DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err      err;
    int         bitErr;
    
    logMsg(LOGLEVEL_DEBUG, "Writing LoudnessBase to bitstream.");
    
    bitErr  = 0;
    err     = MP4NoErr;
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, loudnessBaseBox->DRC_set_ID, 6);         //drcSetId
    if (bitErr) BAILWITHERROR(MP4IOErr);
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, loudnessBaseBox->downmix_ID, 7);         //downmixId
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    if (loudnessBaseBox->bs_sample_peak_level == 0)
    {
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);         //samplePeakLevelPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    else
    {
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);         //samplePeakLevelPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, loudnessBaseBox->bs_sample_peak_level, 12);   //samplePeakLevel
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    
    if (loudnessBaseBox->bs_true_peak_level == 0)
    {
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 0, 1);         //truePeakLevelPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    else
    {
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, 1, 1);         //truePeakLevelPresent
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, loudnessBaseBox->bs_true_peak_level, 12);        //bsTruePeakLevel
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, loudnessBaseBox->measurement_system_for_TP, 4);  //measurementSystem
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, loudnessBaseBox->reliability_for_TP, 2);         //reliability
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    
    bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, loudnessBaseBox->measurement_count, 4);              //measurementCount
    if (bitErr) BAILWITHERROR(MP4IOErr);
    
    for (u32 i = 0; i < loudnessBaseBox->measurement_count; i++)
    {
        MP4LoudnessBaseMeasurement *measurement;
        MP4GetListEntry(loudnessBaseBox->measurements, i, (char**) &measurement);
        
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, measurement->method_definition, 4);              //methodDefinition
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, measurement->method_value, 8);                   //methodValue
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, measurement->measurement_system, 4);             //measurementSystem
        if (bitErr) BAILWITHERROR(MP4IOErr);
        bitErr = wobitbuf_WriteBits(drcBitStreamHandle->bitstream, measurement->reliability, 2);                    //reliability
        if (bitErr) BAILWITHERROR(MP4IOErr);
    }
    
    logMsg(LOGLEVEL_DEBUG, "Writing LoudnessBase to bitstream finished. Bits written: %d bits", drcBitStreamHandle->bitstream->bitsWritten);
bail:
    return err;
}


MP4Err  createSampleEntryFromBuffer         (MP4Track trak, MP4AudioSampleEntryAtomPtr *audioSampleEntry)
{
    MP4Err                      err;
    MP4InputStreamPtr           is;
    u32                         size;
    u32                         outDataReferenceIndex;
    MP4AtomPtr                  atom;
    MP4Handle                   mediaDescriptionH;
    MP4Media                    media;
    
    logMsg(LOGLEVEL_DEBUG, "Creating MP4AudioSampleEntryAtomPtr from buffer.");
    
    err = MP4NoErr;
    
    err = MP4NewHandle( 0, &mediaDescriptionH );                                                if (err) goto bail;
    err = MP4GetTrackMedia( trak, &media );                                                     if (err) goto bail;
    err = MP4GetMediaSampleDescription(media, 1, mediaDescriptionH, &outDataReferenceIndex );   if (err) goto bail;
    err = MP4GetHandleSize( mediaDescriptionH, &size );                                         if (err) goto bail;
    err = MP4CreateMemoryInputStream( *mediaDescriptionH, size, &is );                          if (err) goto bail;
    
    logMsg(LOGLEVEL_TRACE, "Buffersize is %d", size);
    
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &atom ); if (err) goto bail;
    
    if (atom->type != AudioSigned16BitLittleEndianSampleEntryType)
    {
        logMsg(LOGLEVEL_ERROR, "Audio type is not 16 bit signed little endian pcm data");
        err = MP4InvalidMediaErr;
        goto bail;
    }
    
    *audioSampleEntry = (MP4AudioSampleEntryAtomPtr) atom;
    
    err = MP4DisposeHandle(mediaDescriptionH);  if (err) goto bail;
    is->destroy(is);
    logMsg(LOGLEVEL_DEBUG, "Creating MP4AudioSampleEntryAtomPtr from buffer finished.");
bail:
    return err;
}

MP4Err  collectSampleEntryAtoms         (MP4AudioSampleEntryAtomPtr audioSampleEntry, StaticDrcData *staticDrcData)
{
    MP4Err      err;
    u32         i;
    MP4AtomPtr  atom;
    
    err = MP4NoErr;
    
    for (i = 0; i < audioSampleEntry->ExtensionAtomList->entryCount; i++)
    {
        MP4GetListEntry(audioSampleEntry->ExtensionAtomList, i, (char**) &atom);
        switch (atom->type)
        {
            case MP4ChannelLayoutAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found channel layout atom in audio sample entry");
                staticDrcData->channelLayoutAtom = (MP4ChannelLayoutAtomPtr) atom;
                break;
            case MP4DownMixInstructionsAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found down mix instructions atom in audio sample entry");
                err = MP4AddListEntry((MP4DownMixInstructionsAtomPtr) atom, staticDrcData->downMixInstructionsAtoms);   if (err) goto bail;
                break;
            case DRCCoefficientBasicAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found drc coefficients basic atom in audio sample entry");
                err = MP4AddListEntry((DRCCoefficientBasicAtomPtr) atom, staticDrcData->drcCoefficientsBasicAtoms);     if (err) goto bail;
                break;
            case DRCCoefficientUniDRCAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found drc coefficients uniDrc atom in audio sample entry");
                err = MP4AddListEntry((DRCCoefficientUniDRCAtomPtr) atom, staticDrcData->drcCoefficientsUniDrcAtoms);   if (err) goto bail;
                break;
            case DRCInstructionsBasicAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found drc instructions basic atom in audio sample entry");
                err = MP4AddListEntry((DRCInstructionsBasicAtomPtr) atom, staticDrcData->drcInstructionsBasicAtoms);    if (err) goto bail;
                break;
            case DRCInstructionsUniDRCAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found drc instructions uniDrc atom in audio sample entry");
                err = MP4AddListEntry((DRCInstructionsUniDRCAtomPtr) atom, staticDrcData->drcInstructionsUniDrcAtoms);  if (err) goto bail;
                break;
            default:
                logMsg(LOGLEVEL_WARNING, "Unknown atom found in sample entry box: %s", atom->name);
                break;
        }
    }
    
bail:
    return err;
}

MP4Err  getLoudnessInfoAtomFromTrack    (MP4Track trak, StaticDrcData *staticDrcData)
{
    MP4Err                      err;
    MP4UserData                 userData;
    MP4AtomPtr                  atom;
    u32                         count;
    
    logMsg(LOGLEVEL_DEBUG, "Looking for LoudnessBoxes in track user data.");
    
    err = MP4NoErr;
    
    err = MP4GetTrackUserData(trak, &userData);                             if (err) goto bail;
    
    err = MP4GetUserDataEntryCount(userData, MP4LoudnessAtomType, &count);  if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Number of LoudnessBoxes in track user data: %d", count);
    
    if (count == 0)
    {
        logMsg(LOGLEVEL_ERROR, "Loudness Atom not found in user data.");
        err = MP4InvalidMediaErr;
        goto bail;
    }
    
    err = MP4GetAtomFromUserData(userData, (MP4GenericAtom*)&atom, MP4LoudnessAtomType, 1);      if (err) goto bail;
    
    if (atom->type != MP4LoudnessAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "Parsing LoudnessAtom failed! Wrong type.");
        err = MP4InvalidMediaErr;
        goto bail;
    }
    
    staticDrcData->loudnessAtom = (MP4LoudnessAtomPtr) atom;
    
bail:
    return err;
}

MP4Err  estimateBistreamBufferSize     (StaticDrcData *staticDrcData, int *outSizeInBytes)
{
    MP4Err      err;
    int         estimatedSize;
    u32         i;
    MP4AtomPtr  atom;
    
    err             = MP4NoErr;
    estimatedSize   = 0;
    
    estimatedSize += staticDrcData->loudnessAtom->size;
    
    if (staticDrcData->channelLayoutAtom != NULL)
        estimatedSize += staticDrcData->channelLayoutAtom->size;
    
    for (i = 0; i < staticDrcData->downMixInstructionsAtoms->entryCount; i++)
    {
        MP4GetListEntry(staticDrcData->downMixInstructionsAtoms, i, (char**) &atom);
        estimatedSize +=atom->size;
    }
    
    for (i = 0; i < staticDrcData->drcCoefficientsBasicAtoms->entryCount; i++)
    {
        MP4GetListEntry(staticDrcData->drcCoefficientsBasicAtoms, i, (char**) &atom);
        estimatedSize +=atom->size;
    }
    
    for (i = 0; i < staticDrcData->drcInstructionsBasicAtoms->entryCount; i++)
    {
        MP4GetListEntry(staticDrcData->drcInstructionsBasicAtoms, i, (char**) &atom);
        estimatedSize +=atom->size;
    }
    
    for (i = 0; i < staticDrcData->drcCoefficientsUniDrcAtoms->entryCount; i++)
    {
        MP4GetListEntry(staticDrcData->drcCoefficientsUniDrcAtoms, i, (char**) &atom);
        estimatedSize +=atom->size;
    }
    
    for (i = 0; i < staticDrcData->drcInstructionsUniDrcAtoms->entryCount; i++)
    {
        MP4GetListEntry(staticDrcData->drcInstructionsUniDrcAtoms, i, (char**) &atom);
        estimatedSize +=atom->size;
    }
    
    *outSizeInBytes += estimatedSize;
    
bail:
    return err;
}

MP4Err  freeDrcBitstreamHandle              (DrcBitstreamHandle *drcBitStreamHandle)
{
    MP4Err err;
    
    err = MP4NoErr;
    
    free(drcBitStreamHandle->bitstreamBuffer);
    free(drcBitStreamHandle->bitstream);
bail:
    return err;
}

MP4Err  freeDrcBitStreamHelper              (DrcBitStreamHelper *drcBitStreamHelper)
{
    MP4Err  err;
    int     drcErr;
    
    err = MP4NoErr;
    
    drcErr = closeUniDrcBitstreamDec(&drcBitStreamHelper->hUniDrcBsDecStruct, &drcBitStreamHelper->hUniDrcConfig,
                                    &drcBitStreamHelper->hLoudnessInfoSet, &drcBitStreamHelper->hUniDrcGain);
    if (drcErr) BAILWITHERROR(MP4InternalErr);
bail:
    return err;
}

MP4Err  freeStaticDrcData                   (StaticDrcData *staticDrcData)
{
    MP4Err err;
    
    err = MP4NoErr;
    
    staticDrcData->sampleEntryAtom->destroy((MP4AtomPtr) staticDrcData->sampleEntryAtom);

    err = MP4DeleteLinkedList(staticDrcData->downMixInstructionsAtoms);          if (err) goto bail;
    err = MP4DeleteLinkedList(staticDrcData->drcCoefficientsBasicAtoms);         if (err) goto bail;
    err = MP4DeleteLinkedList(staticDrcData->drcInstructionsBasicAtoms);         if (err) goto bail;
    err = MP4DeleteLinkedList(staticDrcData->drcCoefficientsUniDrcAtoms);        if (err) goto bail;
    err = MP4DeleteLinkedList(staticDrcData->drcInstructionsUniDrcAtoms);        if (err) goto bail;
bail:
    return err;
}
