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

#include "InStreamDrcToFileFormatDrc.h"
#include "Logger.h"

#include <uniDrc.h>
#include <math.h>
#include <uniDrcTables.h>
#include <uniDrcBitstreamDecoder_api.h>

#define DRC_LOCATION -1

extern const float downmixCoeff[];
extern const float downmixCoeffLfe[];

MP4Err  addChannelLayout                (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry);
MP4Err  addDownMixInstructions          (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry);

MP4Err  addCoefficientBasics            (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry);
MP4Err  addInstructionsBasics           (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry);
MP4Err  addCoefficientUniDRC            (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry);
MP4Err  addInstructionsUniDRC           (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry);

MP4Err  addCoeffUniDRCSequences         (DRCCoefficientUniDRCAtomPtr coeffUniDRCAtom, DrcCoefficientsUniDrc *coeffUniDrc);
MP4Err  addCoeffUniDRCSeqBandInfos      (DRCCoefficientUniDRCSequence *sequence, GainSetParams *sequenceParams);

MP4Err  addInstrAdditionalDownMixIDs    (DRCInstructionsBasicAtomPtr instrBasicAtom, DrcInstructionsBasic *instrBasic);

MP4Err  createAndFillLoudnessBox        (MP4LoudnessBaseAtomPtr *outatom, u32 type, LoudnessInfo *loudnessInfo);

void    encDownmixCoefficient           (const float bsDownmixCoefficient, int* code);
int     encDuckingScaling               (const float scaling);
MP4Err  encMethodValue                  (const int methodDefinition, const float methodValue, int *code);


MP4Err  addDRCDataToAudioSampleEntry    (DRCData *drcData, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err                      err;
    UniDrcConfig                *uniDrcConfig;
    
    err             = MP4NoErr;
    uniDrcConfig    = (UniDrcConfig*) drcData->hUniDrcConfig;
    
    logMsg(LOGLEVEL_DEBUG, "Adding DRC data to audio sample entry atom");
    
    err             = addChannelLayout        (uniDrcConfig, audioSampleEntry); if (err) goto bail;
    err             = addDownMixInstructions  (uniDrcConfig, audioSampleEntry); if (err) goto bail;
    err             = addCoefficientBasics    (uniDrcConfig, audioSampleEntry); if (err) goto bail;
    err             = addInstructionsBasics   (uniDrcConfig, audioSampleEntry); if (err) goto bail;
    err             = addCoefficientUniDRC    (uniDrcConfig, audioSampleEntry); if (err) goto bail;
    err             = addInstructionsUniDRC   (uniDrcConfig, audioSampleEntry); if (err) goto bail;
bail:
    return err;
}

MP4Err  addLoudnessInfoToTrackAtom      (DRCData *drcData, MP4Track track)
{
    MP4Err                      err;
    MP4UserDataAtomPtr          userDataAtom;
    MP4LoudnessAtomPtr          loudnessAtom;
    LoudnessInfoSet             *loudnessInfoSet;
    MP4Handle                   loudnessAtomDataH;
    char                        *buffer;
    u32                         outIndex;
    MP4TrackAtomPtr             trackAtom;
    u32                         dataSize;
    
    logMsg(LOGLEVEL_DEBUG, "Adding loudness info to track");
    
    err             = MP4NoErr;
    trackAtom       = (MP4TrackAtomPtr) track;
    loudnessInfoSet = (LoudnessInfoSet *) drcData->hLoudnessInfoSet;
    err             = MP4CreateLoudnessAtom(&loudnessAtom); if (err) goto bail;
    
    for (int i = 0; i < loudnessInfoSet->loudnessInfoCount; i++)
    {
        MP4LoudnessBaseAtomPtr  baseAtom;
        err =   createAndFillLoudnessBox(&baseAtom, MP4TrackLoudnessInfoAtomType, &loudnessInfoSet->loudnessInfo[i]); if (err) goto bail;
        err =   loudnessAtom->addAtom(loudnessAtom, (MP4AtomPtr) baseAtom); if (err) goto bail;
    }
    
    for (int i = 0; i < loudnessInfoSet->loudnessInfoAlbumCount; i++)
    {
        MP4LoudnessBaseAtomPtr  baseAtom;
        err =   createAndFillLoudnessBox(&baseAtom, MP4AlbumLoudnessInfoAtomType, &loudnessInfoSet->loudnessInfoAlbum[i]); if (err) goto bail;
        err =   loudnessAtom->addAtom(loudnessAtom, (MP4AtomPtr) baseAtom); if (err) goto bail;
    }
    
    err     = MP4CreateUserDataAtom(&userDataAtom);                                                         if (err) goto bail;
    
    err     = MP4NewHandle( 0, &loudnessAtomDataH );                                                        if (err) goto bail;
    
    err     = loudnessAtom->getDataSize((MP4AtomPtr) loudnessAtom, &dataSize);                              if (err) goto bail;
    err     = MP4SetHandleSize( loudnessAtomDataH, dataSize );                                              if (err) goto bail;
    buffer  = (char *) *loudnessAtomDataH;
    err     = loudnessAtom->serializeData((MP4AtomPtr) loudnessAtom, buffer);                               if (err) goto bail;
    
    err     = userDataAtom->addUserData(userDataAtom, loudnessAtomDataH, MP4LoudnessAtomType, &outIndex);   if (err) goto bail;
    err     = trackAtom->addAtom(trackAtom, (MP4AtomPtr) userDataAtom);                                     if (err) goto bail;
    
    err     = MP4DisposeHandle(loudnessAtomDataH);                                                          if (err) goto bail;
    loudnessAtom->destroy((MP4AtomPtr) loudnessAtom);
bail:
    return err;
}

MP4Err  createAndFillLoudnessBox        (MP4LoudnessBaseAtomPtr *outatom, u32 type, LoudnessInfo *loudnessInfo)
{
    MP4Err                      err;
    
    logMsg(LOGLEVEL_DEBUG, "Creating and filling a loudness base box");
    
    err     = MP4NoErr;
    err     = MP4CreateLoudnessBaseAtom(outatom, type);     if (err) goto bail;
    
    (*outatom)->downmix_ID                  = loudnessInfo->downmixId;
    (*outatom)->DRC_set_ID                  = loudnessInfo->drcSetId;
    
    if (loudnessInfo->samplePeakLevelPresent == 0)
        (*outatom)->bs_sample_peak_level        = 0;
    else
        (*outatom)->bs_sample_peak_level        = (20.0f - loudnessInfo->samplePeakLevel) / 0.03125f; // convert back
    
    if (loudnessInfo->truePeakLevelPresent == 0)
        (*outatom)->bs_true_peak_level          = 0;
    else
        (*outatom)->bs_true_peak_level          = (20.0f - loudnessInfo->truePeakLevel) / 0.03125f; // convert back
    
    (*outatom)->measurement_system_for_TP   = loudnessInfo->truePeakLevelMeasurementSystem;
    (*outatom)->reliability_for_TP          = loudnessInfo->truePeakLevelReliability;
    (*outatom)->measurement_count           = loudnessInfo->measurementCount;
    
    for (u8 i = 0; i < (*outatom)->measurement_count; i++)
    {
        MP4LoudnessBaseMeasurement *measurement;
        LoudnessMeasure            *loudnessMeasure;
        int                         tmp;
        
        measurement         = calloc(1, sizeof(MP4LoudnessBaseMeasurement));
        loudnessMeasure     = &loudnessInfo->loudnessMeasure[i];
        
        
        measurement->method_definition          = loudnessMeasure->methodDefinition;
        
        err = encMethodValue(loudnessMeasure->methodDefinition, loudnessMeasure->methodValue, &tmp); if (err) goto bail;
        measurement->method_value               = tmp;
        
        measurement->measurement_system         = loudnessMeasure->measurementSystem;
        measurement->reliability                = loudnessMeasure->reliability;
        
        MP4AddListEntry(measurement, (*outatom)->measurements);
    }
bail:
    return err;
}

MP4Err addChannelLayout             (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    u8                          i;
    MP4Err                      err;
    MP4ChannelLayoutAtomPtr     channelLayoutAtom;
    
    err = MP4NoErr;
    if (uniDrcConfig->channelLayout.layoutSignalingPresent == 0)
        goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Adding channel layout to sample entry");
    
    err = MP4CreateChannelLayoutAtom(&channelLayoutAtom); if (err) goto bail;
    
    channelLayoutAtom->stream_structure     = 1;
    channelLayoutAtom->definedLayout        = uniDrcConfig->channelLayout.definedLayout;
    
    if (uniDrcConfig->channelLayout.definedLayout == 0)
    {
        for (i = 0; i < uniDrcConfig->channelLayout.baseChannelCount; i++)
        {
            MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
            definedLayoutStruct = calloc(1, sizeof(MP4ChannelLayoutDefinedLayout));
            
            definedLayoutStruct->speaker_position   = uniDrcConfig->channelLayout.speakerPosition[i];
            MP4AddListEntry(definedLayoutStruct, channelLayoutAtom->definedLayouts);
        }
    }
    
    err = MP4AddListEntry(channelLayoutAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;
    
bail:
    return err;
}

MP4Err addDownMixInstructions       (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    u8                              i;
    u8                              j;
    u8                              k;
    u16                             index;
    MP4Err                          err;
    DownmixInstructions             *downMixIntr;
    int                              tmp;
    
    err = MP4NoErr;
    
    logMsg(LOGLEVEL_DEBUG, "Adding down mix instructions to sample entry");
    
    for (i = 0; i < uniDrcConfig->downmixInstructionsCount; i++)
    {
        MP4DownMixInstructionsAtomPtr   downMixInstrAtom;
        err = MP4CreateDownMixInstructionsAtom(&downMixInstrAtom); if (err) goto bail;
        downMixIntr = &uniDrcConfig->downmixInstructions[i];
        
        downMixInstrAtom->baseChannelCount      = uniDrcConfig->channelLayout.baseChannelCount;
        downMixInstrAtom->targetLayout          = downMixIntr->targetLayout;
        downMixInstrAtom->targetChannelCount    = downMixIntr->targetChannelCount;
        downMixInstrAtom->in_stream             = 1;
        
        if (downMixIntr->downmixCoefficientsPresent == 1)
            downMixInstrAtom->in_stream         = 0;
        
        downMixInstrAtom->downmix_ID            = downMixIntr->downmixId;
        
        downMixInstrAtom->bs_downmix_coefficients = calloc(downMixIntr->targetChannelCount * uniDrcConfig->channelLayout.baseChannelCount, sizeof(u8));
        
        if (downMixIntr->downmixCoefficientsPresent == 1)
        {
            for (j = 0; j < downMixIntr->targetChannelCount; j++)
                for (k = 0; k < uniDrcConfig->channelLayout.baseChannelCount; k++)
                {
                    index = j * uniDrcConfig->channelLayout.baseChannelCount + k;
                    encDownmixCoefficient(downMixIntr->downmixCoefficient[index], &tmp); // uniDrcParser:864 convert back to 4 bit code
                    downMixInstrAtom->bs_downmix_coefficients[index] = (u8) tmp;
                }
        }
         err = MP4AddListEntry(downMixInstrAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;
    }
    
bail:
    return err;
}

MP4Err  addCoefficientBasics            (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err                          err;
    
    logMsg(LOGLEVEL_DEBUG, "Adding coefficient basics to sample entry");
    
    err         = MP4NoErr;
    for (int i = 0; i < uniDrcConfig->drcCoefficientsBasicCount; i++)
    {
        DRCCoefficientBasicAtomPtr  coeffBasicAtom;
        DrcCoefficientsBasic        *coeffBasic;
        
        err         = MP4CreateDRCCoefficientBasicAtom(&coeffBasicAtom); if (err) goto bail;
        coeffBasic  = &uniDrcConfig->drcCoefficientsBasic[i];
        
        coeffBasicAtom->DRC_location        = DRC_LOCATION;
        coeffBasicAtom->DRC_characteristic  = coeffBasic->drcCharacteristic;
        
        err = MP4AddListEntry(coeffBasicAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;
    }
    
bail:
    return err;
}

MP4Err  addInstructionsBasics           (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err                          err;
    
    logMsg(LOGLEVEL_DEBUG, "Adding instructions basics to sample entry");
    
    err         = MP4NoErr;
    for (int i = 0; i < uniDrcConfig->drcInstructionsBasicCount; i++)
    {
        DRCInstructionsBasicAtomPtr  instrBasicAtom;
        DrcInstructionsBasic        *instrBasic;
        
        err         = MP4CreateDRCInstructionsBasicAtom(&instrBasicAtom); if (err) goto bail;
        instrBasic  = &uniDrcConfig->drcInstructionsBasic[i];
        
        instrBasicAtom->DRC_set_ID                  = instrBasic->drcSetId;
        instrBasicAtom->DRC_location                = DRC_LOCATION;
        instrBasicAtom->downmix_ID                  = instrBasic->downmixId[0];
        instrBasicAtom->additional_dowmix_ID_count  = instrBasic->downmixIdCount - 1;
        
        err = addInstrAdditionalDownMixIDs(instrBasicAtom, instrBasic); if (err) goto bail;
        
        instrBasicAtom->DRC_set_effect = instrBasic->drcSetEffect;
        
        if ((instrBasicAtom->DRC_set_effect & (1 << 10)) == 0)
        {
            instrBasicAtom->limiter_peak_target_present   = instrBasic->limiterPeakTargetPresent;
            if (instrBasicAtom->limiter_peak_target_present == 1)
            {
                instrBasicAtom->bs_limiter_peak_target   = - instrBasic->limiterPeakTarget / 0.125f; // Convert back
            }
        }
        
        instrBasicAtom->DRC_set_target_loudness_present   = instrBasic->drcSetTargetLoudnessPresent;
        if (instrBasicAtom->DRC_set_target_loudness_present == 1)
        {
            instrBasicAtom->bs_DRC_set_target_loudness_value_upper    = instrBasic->drcSetTargetLoudnessValueUpper;
            instrBasicAtom->bs_DRC_set_target_loudness_value_lower    = instrBasic->drcSetTargetLoudnessValueLower;
        }
        
        err = MP4AddListEntry(instrBasicAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;
    }
    
bail:
    return err;
}

MP4Err  addInstrAdditionalDownMixIDs    (DRCInstructionsBasicAtomPtr instrBasicAtom, DrcInstructionsBasic *instrBasic)
{
    MP4Err      err;
    err         = MP4NoErr;
    
    logMsg(LOGLEVEL_DEBUG, "Adding additional downmix ids to instruction basics atom");
    
    for (int j = 1; j < instrBasic->downmixIdCount; j++)
    {
        DRCInstructionsAdditionalDownMixID    *downMixID;
        downMixID       = calloc(1, sizeof(DRCInstructionsAdditionalDownMixID));

        downMixID->reserved                 = 0;
        downMixID->additional_dowmix_ID     = instrBasic->downmixId[j];
        
        err = MP4AddListEntry(downMixID, instrBasicAtom->additionalDownMixIDs); if ( err ) goto bail;
    }
    
bail:
    return err;
}

MP4Err  addCoefficientUniDRC            (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err                          err;
    
    logMsg(LOGLEVEL_DEBUG, "Adding coefficient uni drcs to sample entry");
    
    err         = MP4NoErr;
    for (int i = 0; i < uniDrcConfig->drcCoefficientsUniDrcCount; i++)
    {
        DRCCoefficientUniDRCAtomPtr  coeffUniDRCAtom;
        DrcCoefficientsUniDrc        *coeffUniDrc;
        
        err             = MP4CreateDRCCoefficientUniDRCAtom(&coeffUniDRCAtom); if (err) goto bail;
        coeffUniDrc     = &uniDrcConfig->drcCoefficientsUniDrc[i];
        
        coeffUniDRCAtom->DRC_location               = DRC_LOCATION;
        coeffUniDRCAtom->drc_frame_size_present     = coeffUniDrc->drcFrameSizePresent;
        
        if (coeffUniDrc->drcFrameSizePresent == 1)
            coeffUniDRCAtom->bs_drc_frame_size      = coeffUniDrc->drcFrameSize - 1;
        
        coeffUniDRCAtom->sequence_count             = coeffUniDrc->gainSetCount;
        
        err = addCoeffUniDRCSequences(coeffUniDRCAtom, coeffUniDrc); if (err) goto bail;
        
        err = MP4AddListEntry(coeffUniDRCAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;
    }
    
bail:
    return err;
}

MP4Err  addCoeffUniDRCSequences         (DRCCoefficientUniDRCAtomPtr coeffUniDRCAtom, DrcCoefficientsUniDrc *coeffUniDrc)
{
    MP4Err      err;
    err         = MP4NoErr;
    
    logMsg(LOGLEVEL_DEBUG, "Adding coefficient drc sequences to coefficient uni drc atom");
    
    for (int j = 0; j < coeffUniDrc->gainSetCount; j++)
    {
        DRCCoefficientUniDRCSequence    *sequence;
        GainSetParams                  *sequenceParams;
        
        sequence        = calloc(1, sizeof(DRCCoefficientUniDRCSequence));
        sequenceParams  = &coeffUniDrc->gainSetParams[j];
        
        
        sequence->reserved1                 = 0;
        sequence->gain_coding_profile       = sequenceParams->gainCodingProfile;
        sequence->gain_interpolation_type   = sequenceParams->gainInterpolationType;
        sequence->full_frame                = sequenceParams->fullFrame;
        sequence->time_alignment            = sequenceParams->timeAlignment;
        sequence->time_delta_min_present    = sequenceParams->timeDeltaMinPresent;
        
        if (sequence->time_delta_min_present == 1)
        {
            sequence->reserved2             = 0;
            sequence->bs_time_delta_min     = sequenceParams->timeDeltaMin;
        }
        
        if (sequence->gain_coding_profile != 3)
        {
            sequence->reserved3             = 0;
            sequence->band_count            = sequenceParams->bandCount;
            sequence->drc_band_type         = sequenceParams->drcBandType;
            
            err = addCoeffUniDRCSeqBandInfos(sequence, sequenceParams); if ( err ) goto bail;
        }
        
        err = MP4AddListEntry(sequence, coeffUniDRCAtom->sequences); if ( err ) goto bail;
    }
    
bail:
    return err;
}

MP4Err  addCoeffUniDRCSeqBandInfos      (DRCCoefficientUniDRCSequence *sequence, GainSetParams *sequenceParams)
{
    MP4Err      err;
    
    logMsg(LOGLEVEL_DEBUG, "Adding coefficient uni drcs band infos to coefficient uni drc atom");
    
    err = MP4NoErr;
    err = MP4MakeLinkedList(&sequence->bandCharacteristics);    if ( err ) goto bail;
    err = MP4MakeLinkedList(&sequence->bandIndexes);            if ( err ) goto bail;
    for (u8 j = 0; j < sequence->band_count; j++)
    {
        DRCCoefficientUniDRCSequenceBandCharacteristic  *bandCharacteristic;
        GainParams                                      *gainParams;
        
        gainParams          = &sequenceParams->gainParams[j];
        bandCharacteristic  = calloc(1, sizeof(DRCCoefficientUniDRCSequenceBandCharacteristic));
        
        bandCharacteristic->reserved            = 0;
        bandCharacteristic->DRC_characteristic  = gainParams->drcCharacteristic;

        
        err = MP4AddListEntry(bandCharacteristic, sequence->bandCharacteristics);   if ( err ) goto bail;
    }
    
    for (u8 j = 1; j < sequence->band_count; j++)
    {
        DRCCoefficientUniDRCSequenceBandIndex           *bandIndex;
        GainParams                                      *gainParams;
        
        gainParams          = &sequenceParams->gainParams[j];
        bandIndex           = calloc(1, sizeof(DRCCoefficientUniDRCSequenceBandIndex));

        if (sequence->drc_band_type == 1)
        {
            bandIndex->reserved1                = 0;
            bandIndex->crossover_freq_index     = gainParams->crossoverFreqIndex;
        }
        else
        {
            bandIndex->reserved2                = 0;
            bandIndex->start_sub_band_index     = gainParams->startSubBandIndex;
        }
        
        err = MP4AddListEntry(bandIndex, sequence->bandIndexes);    if ( err ) goto bail;
    }
bail:
    return err;
}

MP4Err  addInstructionsUniDRC           (UniDrcConfig *uniDrcConfig, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err                          err;
    
    logMsg(LOGLEVEL_DEBUG, "Adding instruction uni drcs to sample entry");
    
    err         = MP4NoErr;
    for (int i = 0; i < uniDrcConfig->drcInstructionsUniDrcCount; i++)
    {
        DRCInstructionsUniDRCAtomPtr  instrUniDRCAtom;
        DrcInstructionsUniDrc        *instrUniDrc;
        u8                           channelGroupCount;
        u8                           uniqueIndex[8];
        float                        uniqueScaling[8];
        
        for (u8 x = 0; x < 8; x++)
        {
            uniqueIndex[x] = 0;
            uniqueScaling[x] = -10.0;
        }
        
        err             = MP4CreateDRCInstructionsUniDRCAtom(&instrUniDRCAtom); if (err) goto bail;
        instrUniDrc     = &uniDrcConfig->drcInstructionsUniDrc[i];
        
        instrUniDRCAtom->DRC_set_ID                  = instrUniDrc->drcSetId;
        instrUniDRCAtom->DRC_location                = DRC_LOCATION;
        instrUniDRCAtom->downmix_ID                  = instrUniDrc->downmixId[0];
        instrUniDRCAtom->additional_dowmix_ID_count  = instrUniDrc->downmixIdCount;
        
        for (int j = 1; j < instrUniDrc->downmixIdCount; j++)
        {
            DRCInstructionsAdditionalDownMixID    *downMixID;
            downMixID       = calloc(1, sizeof(DRCInstructionsAdditionalDownMixID));
            
            downMixID->reserved                 = 0;
            downMixID->additional_dowmix_ID     = instrUniDrc->downmixId[j];
            
            err = MP4AddListEntry(downMixID, instrUniDRCAtom->additionalDownMixIDs); if ( err ) goto bail;
        }
        
        instrUniDRCAtom->DRC_set_effect = instrUniDrc->drcSetEffect;
        
        if ((instrUniDRCAtom->DRC_set_effect & (1 << 10)) == 0)
        {
            instrUniDRCAtom->limiter_peak_target_present   = instrUniDrc->limiterPeakTargetPresent;
            if (instrUniDRCAtom->limiter_peak_target_present == 1)
            {
                instrUniDRCAtom->bs_limiter_peak_target   = (u8) (- instrUniDrc->limiterPeakTarget / 0.125f); // Convert back
            }
        }
        
        instrUniDRCAtom->DRC_set_target_loudness_present   = instrUniDrc->drcSetTargetLoudnessPresent;
        if (instrUniDRCAtom->DRC_set_target_loudness_present == 1)
        {
            instrUniDRCAtom->bs_DRC_set_target_loudness_value_upper    = instrUniDrc->drcSetTargetLoudnessValueUpper;
            instrUniDRCAtom->bs_DRC_set_target_loudness_value_lower    = instrUniDrc->drcSetTargetLoudnessValueLower;
        }
        
        instrUniDRCAtom->depends_on_DRC_set         = instrUniDrc->dependsOnDrcSet;
        
        if (instrUniDRCAtom->depends_on_DRC_set == 0)
            instrUniDRCAtom->no_independent_use     = instrUniDrc->noIndependentUse;
        
        instrUniDRCAtom->channel_count              = uniDrcConfig->channelLayout.baseChannelCount;
        channelGroupCount                           = 0;
        
        
        if ((instrUniDRCAtom->DRC_set_effect & (1 << 10)) != 0)
        {
            for (u8 m = 0; m < instrUniDRCAtom->channel_count; m++)
            {
                int found         = -1;
                u8 sequenceIndex = instrUniDrc->gainSetIndex[m] + 1;
                float duckingModifier = -10.0;
                
                DuckingModifiers *duckingModifierForChannel = &instrUniDrc->duckingModifiersForChannel[m];
                if (duckingModifierForChannel->duckingScalingPresent == 1)
                    duckingModifier = duckingModifierForChannel->duckingScaling;
                    
                
                for (int u = 0; u < channelGroupCount; u++)
                {
                    if ((sequenceIndex == uniqueIndex[u]) && (duckingModifier == uniqueScaling[u]))
                    {
                        found = u;
                    }
                }
                
                if (found == -1)
                {
                    uniqueIndex[channelGroupCount]      = sequenceIndex;
                    uniqueScaling[channelGroupCount]    = duckingModifier;
                    found = channelGroupCount;
                    channelGroupCount++;
                }
                
                DRCInstructionsGroupIndexPerChannel *groupIndexesPerChannel;
                groupIndexesPerChannel = calloc(1, sizeof(DRCInstructionsGroupIndexPerChannel));
                
                groupIndexesPerChannel->channel_group_index = found + 1;
                
                err = MP4AddListEntry(groupIndexesPerChannel, instrUniDRCAtom->groupIndexesPerChannels); if ( err ) goto bail;
            }
            
            for (u8 x = 0; x < channelGroupCount; x++)
            {
                DRCInstructionsSequenceIndexOfChannelGroup *sequenceIndexeOfChannelGroup;
                sequenceIndexeOfChannelGroup = calloc(1, sizeof(DRCInstructionsGroupIndexPerChannel));
                
                sequenceIndexeOfChannelGroup->bs_sequence_index = uniqueIndex[x];
                
                err = MP4AddListEntry(sequenceIndexeOfChannelGroup, instrUniDRCAtom->sequenceIndexesOfChannelGroups); if ( err ) goto bail;
            }
            
            for (u8 x = 0; x < instrUniDrc->nDrcChannelGroups; x++)
            {
                DRCInstructionsChannelGroupDuckingScaling  *channelGroupDuckingScaling;
                DuckingModifiers                           *duckingModifiers;
                channelGroupDuckingScaling = calloc(1, sizeof(DRCInstructionsChannelGroupDuckingScaling));
                duckingModifiers = &instrUniDrc->duckingModifiersForChannelGroup[x];
                
                
                channelGroupDuckingScaling->reserved1                   = 0;
                channelGroupDuckingScaling->ducking_scaling_present     = duckingModifiers->duckingScalingPresent;
                if (channelGroupDuckingScaling->ducking_scaling_present == 1)
                {
                    channelGroupDuckingScaling->reserved2               = 0;
                    channelGroupDuckingScaling->bs_ducking_scaling      = encDuckingScaling(duckingModifiers->duckingScaling); // Convert back to 4 bits
                }
                err = MP4AddListEntry(channelGroupDuckingScaling, instrUniDRCAtom->channelGroupDuckingScalings); if ( err ) goto bail;
            }
        }
        else
        {
            if ((instrUniDrc->downmixId[0] != 0) && (instrUniDrc->downmixId[0] != 0x7F))
            {
                for (u32 dmixIndex = 0; dmixIndex < uniDrcConfig->downmixInstructionsCount; dmixIndex++)
                {
                    DownmixInstructions *downMixIntr;
                    downMixIntr = &uniDrcConfig->downmixInstructions[dmixIndex];
                    if (instrUniDrc->downmixId[0] == downMixIntr->downmixId)
                        instrUniDRCAtom->channel_count = downMixIntr->targetChannelCount;
                }
            }
            else if (instrUniDrc->downmixId[0] == 0x7F)
            {
                instrUniDRCAtom->channel_count = 1;
            }
            
            for (u8 m = 0; m < instrUniDRCAtom->channel_count; m++)
            {
                int found         = -1;
                u8 sequenceIndex = instrUniDrc->gainSetIndex[m] + 1;
                
                for (int u = 0; u < channelGroupCount; u++)
                {
                    if (sequenceIndex == uniqueIndex[u])
                    {
                        found = u + 1;
                    }
                }
                
                if (found == -1)
                {
                    uniqueIndex[channelGroupCount] = sequenceIndex;
                    channelGroupCount++;
                    found = channelGroupCount;
                }
                
                DRCInstructionsGroupIndexPerChannel *groupIndexesPerChannel;
                groupIndexesPerChannel = calloc(1, sizeof(DRCInstructionsGroupIndexPerChannel));
                
                groupIndexesPerChannel->channel_group_index = found;
                
                err = MP4AddListEntry(groupIndexesPerChannel, instrUniDRCAtom->groupIndexesPerChannels); if ( err ) goto bail;
            }
            
            for (u8 x = 0; x < channelGroupCount; x++)
            {
                DRCInstructionsSequenceIndexOfChannelGroup *sequenceIndexeOfChannelGroup;
                sequenceIndexeOfChannelGroup = calloc(1, sizeof(DRCInstructionsGroupIndexPerChannel));
                
                sequenceIndexeOfChannelGroup->bs_sequence_index = uniqueIndex[x];
                
                err = MP4AddListEntry(sequenceIndexeOfChannelGroup, instrUniDRCAtom->sequenceIndexesOfChannelGroups); if ( err ) goto bail;
            }
            
            for (u8 x = 0; x < instrUniDrc->nDrcChannelGroups; x++)
            {
                DRCInstructionsChannelGroupGainScaling  *channelGroupGainScaling;
                GainModifiers                           *gainModifiers;
                
                channelGroupGainScaling = calloc(1, sizeof(DRCInstructionsChannelGroupGainScaling));
                gainModifiers           = &instrUniDrc->gainModifiersForChannelGroup[x];
                
                channelGroupGainScaling->reserved1                  = 0;
                channelGroupGainScaling->gain_scaling_present       = gainModifiers->gainScalingPresent[0]; // This should be per band!
                if (channelGroupGainScaling->gain_scaling_present == 1)
                {
                    channelGroupGainScaling->bs_attenuation_scaling        = (u8) (gainModifiers->attenuationScaling[0] / 0.125f); //Convert back
                    channelGroupGainScaling->bs_amplification_scaling      = (u8) (gainModifiers->amplificationScaling[0] / 0.125f);
                }
                channelGroupGainScaling->reserved2                  = 0;
                channelGroupGainScaling->gain_offset_present        = gainModifiers->gainOffsetPresent[0];
                if (channelGroupGainScaling->gain_offset_present == 1)
                {
                    channelGroupGainScaling->reserved3              = 0;
                    channelGroupGainScaling->bs_gain_offset         = (u8) ((gainModifiers->gainOffset[0] / 0.25f) - 1);
                }
                err = MP4AddListEntry(channelGroupGainScaling, instrUniDRCAtom->channelGroupGainScalings); if ( err ) goto bail;
            }
        }
        
        err = MP4AddListEntry(instrUniDRCAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;
    }
    
bail:
    return err;
}

void encDownmixCoefficient(const float bsDownmixCoefficient, int* code)
{
    int i;
    float coeffDb;
    
    logMsg(LOGLEVEL_TRACE, "Encoding downmix coefficient: %f", bsDownmixCoefficient);
    
    coeffDb = 20.0f * (float)log10(bsDownmixCoefficient);
    
    if (coeffDb < downmixCoeff[14])
    {
        *code = 15;
    }
    else
    {
        i = 0;
        while (coeffDb < downmixCoeff[i]) i++;
        if ((i>0) && (coeffDb > 0.5f * (downmixCoeff[i-1] + downmixCoeff[i]))) i--;
        *code = i;
    }
    
    logMsg(LOGLEVEL_TRACE, "Encoding downmix coefficient finished, code: %d", *code);
}

int encDuckingScaling(const float scaling)
{
    float   delta;
    int     mu;
    int     bits;
    
    logMsg(LOGLEVEL_TRACE, "Encoding ducking scaling: %f", scaling);
    
    delta = scaling - 1.0f;
    
    if (delta > 0.0f)
    {
        mu      = - 1 + (int) (0.5f + 8.0f * delta);
        bits    = 0;
    }
    else
    {
        mu      = - 1 + (int) (0.5f - 8.0f * delta);
        bits    = 1 << 3;
    }
    if (mu != -1)
    {
        mu      = min(7, mu);
        mu      = max(0, mu);
        bits    += mu;
    }
    
    logMsg(LOGLEVEL_TRACE, "Encoding ducking scaling finished, result: %d", bits);
    return bits;
}

MP4Err encMethodValue(const int methodDefinition, const float methodValue, int *code)
{
    MP4Err  err;
    int     bits;
    
    logMsg(LOGLEVEL_TRACE, "Encoding method value: methodDefinition: %d, methodValue: %f", methodDefinition, methodValue);
    
    err = MP4NoErr;
    switch (methodDefinition)
    {
        case METHOD_DEFINITION_UNKNOWN_OTHER:
        case METHOD_DEFINITION_PROGRAM_LOUDNESS:
        case METHOD_DEFINITION_ANCHOR_LOUDNESS:
        case METHOD_DEFINITION_MAX_OF_LOUDNESS_RANGE:
        case METHOD_DEFINITION_MOMENTARY_LOUDNESS_MAX:
        case METHOD_DEFINITION_SHORT_TERM_LOUDNESS_MAX:
            bits = ((int) (0.5f + 4.0f * (methodValue + 57.75f) + 10000.0f)) - 10000;
            bits = min(0x0FF, bits);
            bits = max(0x0, bits);
            break;
        case METHOD_DEFINITION_LOUDNESS_RANGE:
            if (methodValue < 0.0f)
                bits = 0;
            else if (methodValue <= 32.0f)
                bits = (int) (4.0f * methodValue + 0.5f);
            else if (methodValue <= 70.0f)
                bits = ((int)(2.0f * (methodValue - 32.0f) + 0.5f)) + 128;
            else if (methodValue < 121.0f)
                bits = ((int) ((methodValue - 70.0f) + 0.5f)) + 204;
            else
                bits = 255;
            break;
        case METHOD_DEFINITION_MIXING_LEVEL:
            bits = (int)(0.5f + methodValue - 80.0f);
            bits = min(0x1F, bits);
            bits = max(0x0, bits);
            break;
        case METHOD_DEFINITION_ROOM_TYPE:
            bits = (int)(0.5f + methodValue);
            if (bits > 0x2)
            {
                logMsg(LOGLEVEL_ERROR, "methodDefinition %d has unexpected methodValue: %d\n", methodDefinition, bits);
                BAILWITHERROR(MP4InternalErr);
            }
            bits = min(0x2, bits);
            bits = max(0x0, bits);
            break;
        case METHOD_DEFINITION_SHORT_TERM_LOUDNESS:
            bits = ((int) (0.5f + 2.0f * (methodValue + 116.f) + 10000.0f)) - 10000;
            bits = min(0x0FF, bits);
            bits = max(0x0, bits);
            break;
        default:
        {
            logMsg(LOGLEVEL_ERROR, "methodDefinition unexpected value: %d\n", methodDefinition);
            BAILWITHERROR(MP4InternalErr);
        }
    }
    *code = bits;
    
    logMsg(LOGLEVEL_TRACE, "Encoding method value finished: code: %d", bits);
    
bail:
    return err;
}
