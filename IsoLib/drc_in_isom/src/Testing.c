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

#include "Logger.h"
#include "DRCAtoms.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>

#define CHECK( pt1, pt2, varname ) \
if (pt1->varname != pt2->varname) { \
logMsg(LOGLEVEL_ERROR, "Compareing %s->%s failed! Original: %d, Parsed: %d", #pt1, #varname, pt1->varname, pt2->varname); \
err = MP4BadDataErr; goto bail; }

u16  getTestInt(u8 length)
{
    u8 boundaryTest;
    
    boundaryTest = rand() % 5;
    if (boundaryTest == 0)
        return 0;
    
    if (boundaryTest == 4)
        return (pow(2, length) -1);
    
    return rand() % ((int) pow(2, length));
}


MP4Err testChannelLayoutAtom()
{
    MP4Err                      err;
    MP4ChannelLayoutAtomPtr     atom;
    MP4Handle                   dataH;
    u32                         size;
    char                        *buffer;
    MP4AtomPtr                  parsedAtom;
    MP4ChannelLayoutAtomPtr     parsedDRCAtom;
    MP4InputStreamPtr           is;
    
    err = MP4NoErr;
    MP4CreateChannelLayoutAtom(&atom);
    
    atom->stream_structure          = getTestInt(1) + 1;
    if (atom->stream_structure & 1) /* channelStructured */
    {
        atom->definedLayout          = getTestInt(8);
        if (atom->definedLayout == 0)
        {
            atom->channelCount          = getTestInt(4); //actually u16
            for (u8 i = 0; i < atom->channelCount; i++)
            {
                MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
                definedLayoutStruct = calloc(1, sizeof(MP4ChannelLayoutDefinedLayout));
/*   this code does not implement the latest syntax.
                definedLayoutStruct->explicit_position = getTestInt(1);
                if (definedLayoutStruct->explicit_position)
                {
                    definedLayoutStruct->elevation = getTestInt(7);
                    definedLayoutStruct->azimuth = getTestInt(8);
                }
                else
*/
                {
                    definedLayoutStruct->speaker_position = getTestInt(7);
                }
                MP4AddListEntry(definedLayoutStruct, atom->definedLayouts);
            }
        }
        else
        {
            atom->omittedChannelsMap = getTestInt(16);
            atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
            atom->omittedChannelsMap += getTestInt(16);
            atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
            atom->omittedChannelsMap += getTestInt(16);
            atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
            atom->omittedChannelsMap += getTestInt(16);
        }
    }
    
    if (atom->stream_structure & 2) /* objectStructured */
    {
        atom->object_count += getTestInt(8);
    }
    
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );          if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );  if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != MP4ChannelLayoutAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(MP4ChannelLayoutAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedDRCAtom = (MP4ChannelLayoutAtomPtr) parsedAtom;
    
    CHECK(atom, parsedDRCAtom, stream_structure);
    if (atom->stream_structure & 1) /* channelStructured */
    {
        CHECK(atom, parsedDRCAtom, definedLayout);
        if (atom->definedLayout == 0)
        {
            for (u8 i = 0; i < atom->channelCount; i++)
            {
                MP4ChannelLayoutDefinedLayout *definedLayoutStructOrig;
                MP4ChannelLayoutDefinedLayout *definedLayoutStructParsed;
                MP4GetListEntry(atom->definedLayouts, i, (char**) &definedLayoutStructOrig);
                MP4GetListEntry(parsedDRCAtom->definedLayouts, i, (char**) &definedLayoutStructParsed);
                /*   this code does not implement the latest syntax.
                CHECK(definedLayoutStructOrig, definedLayoutStructParsed, explicit_position);
                if (definedLayoutStructOrig->explicit_position)
                {
                    CHECK(definedLayoutStructOrig, definedLayoutStructParsed, elevation);
                    CHECK(definedLayoutStructOrig, definedLayoutStructParsed, azimuth);
                }
                else
*/
                {
                    CHECK(definedLayoutStructOrig, definedLayoutStructParsed, speaker_position);
                }
            }
        }
        else
        {
            CHECK(atom, parsedDRCAtom, omittedChannelsMap);
        }
    }
    
    if (atom->stream_structure & 2) /* objectStructured */
    {
        CHECK(atom, parsedDRCAtom, object_count);
    }
    
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "MP4ChannelLayoutAtom  test successfull!");
bail:
    return err;
}

MP4Err testDownMixInstructionsAtom()
{
    MP4Err                              err;
    MP4DownMixInstructionsAtomPtr       atom;
    MP4Handle                           dataH;
    u32                                 size;
    char                                *buffer;
    MP4AtomPtr                          parsedAtom;
    MP4DownMixInstructionsAtomPtr       parsedDRCAtom;
    MP4InputStreamPtr                   is;
    
    err = MP4NoErr;
    MP4CreateDownMixInstructionsAtom(&atom);
    
    atom->targetLayout          = getTestInt(8);
    atom->targetChannelCount    = getTestInt(4); // actually u7!
    atom->in_stream             = getTestInt(1);
    atom->downmix_ID            = getTestInt(7);
    atom->baseChannelCount      = getTestInt(4); //Not actually u4! Decreased for testing
    
    if (atom->in_stream == 0)
    {
        for (u8 i = 0; i < atom->targetChannelCount; i++)
        {
            for (u8 j = 0; j < atom->baseChannelCount; j++)
            {
                atom->bs_downmix_coefficients = realloc(atom->bs_downmix_coefficients, i * atom->baseChannelCount + j + 1);
                atom->bs_downmix_coefficients[i * atom->baseChannelCount + j] = getTestInt(4);
            }
        }
    }
    
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );            if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );      if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != MP4DownMixInstructionsAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(MP4DownMixInstructionsAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedDRCAtom = (MP4DownMixInstructionsAtomPtr) parsedAtom;
    
    CHECK(atom, parsedDRCAtom, targetLayout);
    CHECK(atom, parsedDRCAtom, reserved);
    CHECK(atom, parsedDRCAtom, targetChannelCount);
    CHECK(atom, parsedDRCAtom, in_stream);
    CHECK(atom, parsedDRCAtom, downmix_ID);
    
    if (atom->in_stream == 0)
    {
        for (u16 i = 0; i < atom->targetChannelCount; i++)
        {
            for (u16 j = 0; j < atom->baseChannelCount; j++)
            {
                u32 index = (i * atom->baseChannelCount) + j;
                CHECK(atom, parsedDRCAtom, bs_downmix_coefficients[index]);
            }
        }
        if (((atom->baseChannelCount * atom->targetChannelCount) % 2) == 1)
        {
            if (parsedDRCAtom->bs_downmix_coefficients[atom->baseChannelCount * atom->targetChannelCount] != 0x0F)
            {
                err = MP4BadDataErr; goto bail;
            }
        }
    }
    
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "MP4DownMixInstructionsAtom  test successfull!");
bail:
    return err;
}

MP4Err testLoudnessBaseAtom()
{
    MP4Err                              err;
    MP4LoudnessBaseAtomPtr              atom;
    MP4Handle                           dataH;
    u32                                 size;
    char                                *buffer;
    MP4AtomPtr                          parsedAtom;
    MP4LoudnessBaseAtomPtr              parsedDRCAtom;
    MP4InputStreamPtr                   is;
    u32                                 type;
    
    err = MP4NoErr;
    
    type = MP4AlbumLoudnessInfoAtomType;
    if (getTestInt(1) == 0)
        type = MP4TrackLoudnessInfoAtomType;

    MP4CreateLoudnessBaseAtom(&atom, type);
    

    atom->downmix_ID                        = getTestInt(7);
    atom->DRC_set_ID                        = getTestInt(6);
    atom->bs_sample_peak_level              = getTestInt(12);
    atom->bs_sample_peak_level              = atom->bs_sample_peak_level << 4;
    atom->bs_sample_peak_level              = atom->bs_sample_peak_level >> 4; //get sign right
    atom->bs_true_peak_level                = getTestInt(12);
    atom->bs_true_peak_level                = atom->bs_true_peak_level << 4;
    atom->bs_true_peak_level                = atom->bs_true_peak_level >> 4; //get sign right
    atom->measurement_system_for_TP         = getTestInt(4);
    atom->reliability_for_TP                = getTestInt(4);
    atom->measurement_count                 = getTestInt(8);
    
    for (u8 i = 0; i < atom->measurement_count; i++)
    {
        MP4LoudnessBaseMeasurement *measurement;
        measurement = calloc(1, sizeof(MP4LoudnessBaseMeasurement));
        measurement->method_definition          = getTestInt(8);
        measurement->method_value               = getTestInt(8);
        measurement->measurement_system         = getTestInt(4);
        measurement->reliability                = getTestInt(4);
        MP4AddListEntry(measurement, atom->measurements);
    }

    
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );            if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );      if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != type)
    {
        logMsg(LOGLEVEL_ERROR, "(MP4LoudnessBaseAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedDRCAtom = (MP4LoudnessBaseAtomPtr) parsedAtom;
    
    CHECK(atom, parsedDRCAtom, reserved);
    CHECK(atom, parsedDRCAtom, downmix_ID);
    CHECK(atom, parsedDRCAtom, DRC_set_ID);
    CHECK(atom, parsedDRCAtom, bs_sample_peak_level);
    CHECK(atom, parsedDRCAtom, bs_true_peak_level);
    CHECK(atom, parsedDRCAtom, measurement_system_for_TP);
    CHECK(atom, parsedDRCAtom, reliability_for_TP);
    CHECK(atom, parsedDRCAtom, measurement_count);
    
    for (u8 i = 0; i < atom->measurement_count; i++)
    {
        MP4LoudnessBaseMeasurement *measurementOrig;
        MP4LoudnessBaseMeasurement *measurementParsed;
        MP4GetListEntry(atom->measurements, i, (char**) &measurementOrig);
        MP4GetListEntry(parsedDRCAtom->measurements, i, (char**) &measurementParsed);
        CHECK(measurementOrig, measurementParsed, method_definition);
        CHECK(measurementOrig, measurementParsed, method_value);
        CHECK(measurementOrig, measurementParsed, measurement_system);
        CHECK(measurementOrig, measurementParsed, reliability);
    }
    
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "MP4LoudnessBaseAtom  test successfull!");
bail:
    return err;
}

MP4Err testDRCCoefficientBasicAtom()
{
    MP4Err                      err;
    DRCCoefficientBasicAtomPtr  atom;
    MP4Handle                   dataH;
    u32                         size;
    char                        *buffer;
    MP4AtomPtr                  parsedAtom;
    DRCCoefficientBasicAtomPtr  parsedDRCAtom;
    MP4InputStreamPtr           is;
    
    err = MP4NoErr;
    MP4CreateDRCCoefficientBasicAtom(&atom);
    
    atom->DRC_location          = getTestInt(5);
    atom->DRC_location          = atom->DRC_location << 3;
    atom->DRC_location          = atom->DRC_location >> 3; // get sign right
    atom->DRC_characteristic    = getTestInt(7);
    
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );          if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );  if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != DRCCoefficientBasicAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(testDRCCoefficientBasicAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedDRCAtom = (DRCCoefficientBasicAtomPtr) parsedAtom;
    
    CHECK(atom, parsedDRCAtom, reserved);
    CHECK(atom, parsedDRCAtom, DRC_location);
    CHECK(atom, parsedDRCAtom, DRC_characteristic);
    
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "DRCCoefficientBasicAtom test successfull!");
bail:
    return err;
}

MP4Err testDRCCoefficientUniDRCAtom()
{
    MP4Err                          err;
    DRCCoefficientUniDRCAtomPtr     atom;
    MP4Handle                       dataH;
    u32                             size;
    char                            *buffer;
    MP4AtomPtr                      parsedAtom;
    DRCCoefficientUniDRCAtomPtr     parsedDRCAtom;
    MP4InputStreamPtr               is;
    
    err = MP4NoErr;
    MP4CreateDRCCoefficientUniDRCAtom(&atom);
    
    atom->DRC_location                  = getTestInt(5);
    atom->DRC_location                  = atom->DRC_location << 3;
    atom->DRC_location                  = atom->DRC_location >> 3; // get sign right
    atom->drc_frame_size_present        = getTestInt(1);
    
    if (atom->drc_frame_size_present)
    {
        atom->bs_drc_frame_size         = getTestInt(15);
    }
    
    atom->sequence_count                = getTestInt(6);
    
    for (u8 i = 0; i < atom->sequence_count; i++)
    {
        DRCCoefficientUniDRCSequence *sequence;
        sequence = calloc(1, sizeof(DRCCoefficientUniDRCSequence));
        
        sequence->reserved1                 = 0;
        sequence->gain_coding_profile       = getTestInt(2);
        sequence->gain_interpolation_type   = getTestInt(1);
        sequence->full_frame                = getTestInt(1);
        sequence->time_alignment            = getTestInt(1);
        sequence->time_delta_min_present    = getTestInt(1);
        
        if (sequence->time_delta_min_present == 1)
        {
            sequence->reserved2             = 0;
            sequence->bs_time_delta_min     = getTestInt(11);
        }
        
        if (sequence->gain_coding_profile != 3)
        {
            sequence->reserved3             = 0;
            sequence->band_count            = getTestInt(4);
            sequence->drc_band_type         = getTestInt(1);
            
            err = MP4MakeLinkedList(&sequence->bandCharacteristics); if ( err ) goto bail;
            for (u8 j = 0; j < sequence->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandCharacteristic  *bandCharacteristic;
                bandCharacteristic = calloc(1, sizeof(DRCCoefficientUniDRCSequenceBandCharacteristic));
                bandCharacteristic->reserved            = 0;
                bandCharacteristic->DRC_characteristic  = getTestInt(7);
                err = MP4AddListEntry(bandCharacteristic, sequence->bandCharacteristics); if ( err ) goto bail;
            }
            
            err = MP4MakeLinkedList(&sequence->bandIndexes); if ( err ) goto bail;
            for (u8 j = 0; j < sequence->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandIndex *bandIndex;
                bandIndex = calloc(1, sizeof(DRCCoefficientUniDRCSequenceBandIndex));
                if (sequence->drc_band_type == 1)
                {
                    bandIndex->reserved1            = 0;
                    bandIndex->crossover_freq_index = getTestInt(4);
                }
                else
                {
                    bandIndex->reserved2            = 0;
                    bandIndex->start_sub_band_index = getTestInt(10);
                }
                err = MP4AddListEntry(bandIndex, sequence->bandIndexes); if ( err ) goto bail;
            }
        }
        err = MP4AddListEntry(sequence, atom->sequences); if ( err ) goto bail;
    }

    
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );            if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );      if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != DRCCoefficientUniDRCAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(testDRCCoefficientUniDRCAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedDRCAtom = (DRCCoefficientUniDRCAtomPtr) parsedAtom;
    
    CHECK(atom, parsedDRCAtom, reserved1);
    CHECK(atom, parsedDRCAtom, DRC_location);
    CHECK(atom, parsedDRCAtom, drc_frame_size_present);
    
    if (atom->drc_frame_size_present)
    {
        CHECK(atom, parsedDRCAtom, reserved2);
        CHECK(atom, parsedDRCAtom, bs_drc_frame_size);
    }
    
    CHECK(atom, parsedDRCAtom, reserved3);
    CHECK(atom, parsedDRCAtom, sequence_count);
    
    for (u8 i = 0; i < atom->sequence_count; i++)
    {
        DRCCoefficientUniDRCSequence    *sequenceOrig;
        DRCCoefficientUniDRCSequence    *sequenceParsed;
        MP4GetListEntry(atom->sequences, i, (char**) &sequenceOrig);
        MP4GetListEntry(parsedDRCAtom->sequences, i, (char**) &sequenceParsed);
        
        CHECK(sequenceOrig, sequenceParsed, reserved1);
        CHECK(sequenceOrig, sequenceParsed, gain_coding_profile);
        CHECK(sequenceOrig, sequenceParsed, gain_interpolation_type);
        CHECK(sequenceOrig, sequenceParsed, full_frame);
        CHECK(sequenceOrig, sequenceParsed, time_alignment);
        CHECK(sequenceOrig, sequenceParsed, time_delta_min_present);
        
        if (sequenceOrig->time_delta_min_present == 1)
        {
            CHECK(sequenceOrig, sequenceParsed, reserved2);
            CHECK(sequenceOrig, sequenceParsed, bs_time_delta_min);
        }
        
        if (sequenceOrig->gain_coding_profile != 3)
        {
            CHECK(sequenceOrig, sequenceParsed, reserved3);
            CHECK(sequenceOrig, sequenceParsed, band_count);
            CHECK(sequenceOrig, sequenceParsed, drc_band_type);
            
            for (u8 j = 0; j < sequenceOrig->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandCharacteristic *bandCharacteristicOrig;
                DRCCoefficientUniDRCSequenceBandCharacteristic *bandCharacteristicParsed;
                MP4GetListEntry(sequenceOrig->bandCharacteristics, j, (char**) &bandCharacteristicOrig);
                MP4GetListEntry(sequenceParsed->bandCharacteristics, j, (char**) &bandCharacteristicParsed);
                CHECK(bandCharacteristicOrig, bandCharacteristicParsed, reserved);
                CHECK(bandCharacteristicOrig, bandCharacteristicParsed, DRC_characteristic);
            }
            
            for (u8 j = 0; j < sequenceOrig->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandIndex *bandIndexOrig;
                DRCCoefficientUniDRCSequenceBandIndex *bandIndexParsed;
                MP4GetListEntry(sequenceOrig->bandIndexes, j, (char**) &bandIndexOrig);
                MP4GetListEntry(sequenceParsed->bandIndexes, j, (char**) &bandIndexParsed);
                
                if (sequenceOrig->drc_band_type == 1)
                {
                    CHECK(bandIndexOrig, bandIndexParsed, reserved1);
                    CHECK(bandIndexOrig, bandIndexParsed, crossover_freq_index);
                }
                else
                {
                    CHECK(bandIndexOrig, bandIndexParsed, reserved2);
                    CHECK(bandIndexOrig, bandIndexParsed, start_sub_band_index);
                }
            }
        }
    }
    
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "DRCCoefficientUniDRCAtom test successfull!");
    
bail:
    return err;
}

MP4Err testDRCInstructionsBasicAtom()
{
    MP4Err                          err;
    DRCInstructionsBasicAtomPtr     atom;
    MP4Handle                       dataH;
    u32                             size;
    char                            *buffer;
    MP4AtomPtr                      parsedAtom;
    DRCInstructionsBasicAtomPtr     parsedDRCAtom;
    MP4InputStreamPtr               is;
    
    err = MP4NoErr;
    MP4CreateDRCInstructionsBasicAtom(&atom);
    
    atom->DRC_set_ID                    = getTestInt(6);
    while (atom->DRC_set_ID == 0) atom->DRC_set_ID = getTestInt(6);
    
    atom->DRC_location                  = getTestInt(5);
    atom->DRC_location                  = atom->DRC_location << 3;
    atom->DRC_location                  = atom->DRC_location >> 3; // get sign right
    atom->downmix_ID                    = getTestInt(7);
    
    atom->additional_dowmix_ID_count    = getTestInt(3);
    
    for (u8 i = 0; i < atom->additional_dowmix_ID_count; i++)
    {
        DRCInstructionsAdditionalDownMixID  *downMixID;
        downMixID = calloc(1, sizeof(DRCInstructionsAdditionalDownMixID));
        
        downMixID->reserved             = 0;
        downMixID->additional_dowmix_ID = getTestInt(7);
        err = MP4AddListEntry(downMixID, atom->additionalDownMixIDs); if ( err ) goto bail;
    }
    
    atom->DRC_set_effect = getTestInt(16);
    
    if ((atom->DRC_set_effect & (1 << 10)) == 0)
    {
        atom->limiter_peak_target_present   = getTestInt(1);
        if (atom->limiter_peak_target_present == 1)
        {
            atom->bs_limiter_peak_target   = getTestInt(8);
        }
    }
    
    atom->DRC_set_target_loudness_present   = getTestInt(1);
    if (atom->DRC_set_target_loudness_present == 1)
    {
        atom->bs_DRC_set_target_loudness_value_upper    = getTestInt(6);
        atom->bs_DRC_set_target_loudness_value_lower    = getTestInt(6);
    }
    
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );            if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );      if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != DRCInstructionsBasicAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(test DRCInstructionsBasicAtomPtr) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedDRCAtom = (DRCInstructionsBasicAtomPtr) parsedAtom;
    
    CHECK(atom, parsedDRCAtom, reserved1);
    CHECK(atom, parsedDRCAtom, DRC_set_ID);
    CHECK(atom, parsedDRCAtom, DRC_location);
    CHECK(atom, parsedDRCAtom, downmix_ID);
    CHECK(atom, parsedDRCAtom, additional_dowmix_ID_count);
    
    for (u8 i = 0; i < atom->additional_dowmix_ID_count; i++)
    {
        DRCInstructionsAdditionalDownMixID  *downMixIDOrig;
        DRCInstructionsAdditionalDownMixID  *downMixIDParsed;
        MP4GetListEntry(atom->additionalDownMixIDs, i, (char**) &downMixIDOrig);
        MP4GetListEntry(parsedDRCAtom->additionalDownMixIDs, i, (char**) &downMixIDParsed);
        CHECK(downMixIDOrig, downMixIDParsed, reserved);
        CHECK(downMixIDOrig, downMixIDParsed, additional_dowmix_ID);
    }
    
    CHECK(atom, parsedDRCAtom, DRC_set_effect);
    
    if ((atom->DRC_set_effect & (1 << 10)) == 0)
    {
        CHECK(atom, parsedDRCAtom, reserved2);
        CHECK(atom, parsedDRCAtom, limiter_peak_target_present);
        if (atom->limiter_peak_target_present == 1)
        {
            CHECK(atom, parsedDRCAtom, bs_limiter_peak_target);
        }
    }
    
    CHECK(atom, parsedDRCAtom, DRC_set_target_loudness_present);
    if (atom->DRC_set_target_loudness_present == 1)
    {
        CHECK(atom, parsedDRCAtom, reserved4);
        CHECK(atom, parsedDRCAtom, bs_DRC_set_target_loudness_value_upper);
        CHECK(atom, parsedDRCAtom, bs_DRC_set_target_loudness_value_lower);
    }
    
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "DRCInstructionsBasicAtom test successfull!");
bail:
    return err;
}

MP4Err testDRCInstructionsUniDRCAtom()
{
    MP4Err                          err;
    DRCInstructionsUniDRCAtomPtr     atom;
    MP4Handle                       dataH;
    u32                             size;
    char                            *buffer;
    MP4AtomPtr                      parsedAtom;
    DRCInstructionsUniDRCAtomPtr     parsedDRCAtom;
    MP4InputStreamPtr               is;
    
    err = MP4NoErr;
    MP4CreateDRCInstructionsUniDRCAtom(&atom);
    
    atom->DRC_set_ID                    = getTestInt(6);
    while (atom->DRC_set_ID == 0) atom->DRC_set_ID = getTestInt(6);
    
    atom->DRC_location                  = getTestInt(5);
    atom->DRC_location                  = atom->DRC_location << 3;
    atom->DRC_location                  = atom->DRC_location >> 3; // get sign right
    atom->downmix_ID                    = getTestInt(7);
    
    atom->additional_dowmix_ID_count    = getTestInt(3);
    
    for (u8 i = 0; i < atom->additional_dowmix_ID_count; i++)
    {
        DRCInstructionsAdditionalDownMixID  *downMixID;
        downMixID = calloc(1, sizeof(DRCInstructionsAdditionalDownMixID));
        
        downMixID->reserved             = 0;
        downMixID->additional_dowmix_ID = getTestInt(7);
        err = MP4AddListEntry(downMixID, atom->additionalDownMixIDs); if ( err ) goto bail;
    }
    
    atom->DRC_set_effect = getTestInt(16);
    
    if ((atom->DRC_set_effect & (1 << 10)) == 0)
    {
        atom->limiter_peak_target_present   = getTestInt(1);
        if (atom->limiter_peak_target_present == 1)
        {
            atom->bs_limiter_peak_target   = getTestInt(8);
        }
    }
    
    atom->DRC_set_target_loudness_present   = getTestInt(1);
    if (atom->DRC_set_target_loudness_present == 1)
    {
        atom->bs_DRC_set_target_loudness_value_upper    = getTestInt(6);
        atom->bs_DRC_set_target_loudness_value_lower    = getTestInt(6);
    }
    
    atom->depends_on_DRC_set                = getTestInt(6);
    
    if (atom->depends_on_DRC_set == 0)
        atom->no_independent_use = getTestInt(1);
    
    atom->channel_count                     = getTestInt(8);
    
    u8 biggestIndex = 0;
    for (u8 i = 0; i < atom->channel_count; i++)
    {
        DRCInstructionsGroupIndexPerChannel  *item;
        item = calloc(1, sizeof(DRCInstructionsGroupIndexPerChannel));
        
        item->channel_group_index                   = i+1;
        if (biggestIndex < item->channel_group_index)
            biggestIndex = item->channel_group_index;
        
        err = MP4AddListEntry(item, atom->groupIndexesPerChannels); if ( err ) goto bail;
    }
    
    for (u8 i = 0; i < biggestIndex -1; i++)
    {
        DRCInstructionsSequenceIndexOfChannelGroup  *item;
        item = calloc(1, sizeof(DRCInstructionsSequenceIndexOfChannelGroup));
        
        item->reserved              = 0;
        item->bs_sequence_index     = getTestInt(6);
        err = MP4AddListEntry(item, atom->sequenceIndexesOfChannelGroups); if ( err ) goto bail;
    }
    
    
    if ((atom->DRC_set_effect & (1 << 10)) != 0)
    {
        for (u8 i = 0; i < biggestIndex -1; i++)
        {
            DRCInstructionsChannelGroupDuckingScaling  *channelGroupDuckingScaling;
            channelGroupDuckingScaling = calloc(1, sizeof(DRCInstructionsChannelGroupDuckingScaling));

            channelGroupDuckingScaling->reserved1                   = 0;
            channelGroupDuckingScaling->ducking_scaling_present     = getTestInt(1);
            if (channelGroupDuckingScaling->ducking_scaling_present == 1)
            {
                channelGroupDuckingScaling->reserved2               = 0;
                channelGroupDuckingScaling->bs_ducking_scaling      = getTestInt(4);
            }
            err = MP4AddListEntry(channelGroupDuckingScaling, atom->channelGroupDuckingScalings); if ( err ) goto bail;
        }
    }
    else
    {
        for (u8 i = 0; i < biggestIndex -1; i++)
        {
            DRCInstructionsChannelGroupGainScaling  *channelGroupGainScaling;
            channelGroupGainScaling = calloc(1, sizeof(DRCInstructionsChannelGroupGainScaling));
            channelGroupGainScaling->reserved1                  = 0;
            channelGroupGainScaling->gain_scaling_present       = getTestInt(1);
            if (channelGroupGainScaling->gain_scaling_present == 1)
            {
                channelGroupGainScaling->bs_attenuation_scaling        = getTestInt(4);
                channelGroupGainScaling->bs_amplification_scaling      = getTestInt(4);
            }
            channelGroupGainScaling->reserved2                  = 0;
            channelGroupGainScaling->gain_offset_present        = getTestInt(1);
            if (channelGroupGainScaling->gain_offset_present == 1)
            {
                channelGroupGainScaling->reserved3              = 0;
                channelGroupGainScaling->bs_gain_offset         = getTestInt(6);
            }
            err = MP4AddListEntry(channelGroupGainScaling, atom->channelGroupGainScalings); if ( err ) goto bail;
        }
    }
    
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );        if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );  if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDRCAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != DRCInstructionsUniDRCAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(test DRCInstructionsUniDRCAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedDRCAtom = (DRCInstructionsUniDRCAtomPtr) parsedAtom;
    
    CHECK(atom, parsedDRCAtom, reserved1);
    CHECK(atom, parsedDRCAtom, DRC_set_ID);
    CHECK(atom, parsedDRCAtom, DRC_location);
    CHECK(atom, parsedDRCAtom, downmix_ID);
    CHECK(atom, parsedDRCAtom, additional_dowmix_ID_count);
    
    for (u8 i = 0; i < atom->additional_dowmix_ID_count; i++)
    {
        DRCInstructionsAdditionalDownMixID  *downMixIDOrig;
        DRCInstructionsAdditionalDownMixID  *downMixIDParsed;
        MP4GetListEntry(atom->additionalDownMixIDs, i, (char**) &downMixIDOrig);
        MP4GetListEntry(parsedDRCAtom->additionalDownMixIDs, i, (char**) &downMixIDParsed);
        CHECK(downMixIDOrig, downMixIDParsed, reserved);
        CHECK(downMixIDOrig, downMixIDParsed, additional_dowmix_ID);
    }
    
    CHECK(atom, parsedDRCAtom, DRC_set_effect);
    
    if ((atom->DRC_set_effect & (1 << 10)) == 0)
    {
        CHECK(atom, parsedDRCAtom, reserved2);
        CHECK(atom, parsedDRCAtom, limiter_peak_target_present);
        if (atom->limiter_peak_target_present == 1)
        {
            CHECK(atom, parsedDRCAtom, bs_limiter_peak_target);
        }
    }
    
    CHECK(atom, parsedDRCAtom, DRC_set_target_loudness_present);
    if (atom->DRC_set_target_loudness_present == 1)
    {
        CHECK(atom, parsedDRCAtom, reserved4);
        CHECK(atom, parsedDRCAtom, bs_DRC_set_target_loudness_value_upper);
        CHECK(atom, parsedDRCAtom, bs_DRC_set_target_loudness_value_lower);
    }
    
    
    CHECK(atom, parsedDRCAtom, reserved5);
    CHECK(atom, parsedDRCAtom, depends_on_DRC_set);
    
    if (atom->depends_on_DRC_set == 0)
    {
        CHECK(atom, parsedDRCAtom, no_independent_use);
    }
    else
    {
        CHECK(atom, parsedDRCAtom, reserved6);
    }
    
    CHECK(atom, parsedDRCAtom, channel_count);
    
    for (u8 i = 0; i < atom->channel_count; i++)
    {
        DRCInstructionsGroupIndexPerChannel  *channelSequenceIndexOrig;
        DRCInstructionsGroupIndexPerChannel  *channelSequenceIndexParsed;
        MP4GetListEntry(atom->groupIndexesPerChannels, i, (char**) &channelSequenceIndexOrig);
        MP4GetListEntry(parsedDRCAtom->groupIndexesPerChannels, i, (char**) &channelSequenceIndexParsed);
        CHECK(channelSequenceIndexOrig, channelSequenceIndexParsed, channel_group_index);
    }
    
    
    if ((atom->DRC_set_effect & (1 << 10)) != 0)
    {
        for (u8 i = 0; i < atom->channel_count; i++)
        {
            DRCInstructionsChannelGroupDuckingScaling  *channelGroupDuckingScalingOrig;
            DRCInstructionsChannelGroupDuckingScaling  *channelGroupDuckingScalingParsed;
            MP4GetListEntry(atom->channelGroupDuckingScalings, i, (char**) &channelGroupDuckingScalingOrig);
            MP4GetListEntry(parsedDRCAtom->channelGroupDuckingScalings, i, (char**) &channelGroupDuckingScalingParsed);
            CHECK(channelGroupDuckingScalingOrig, channelGroupDuckingScalingParsed, reserved1);
            CHECK(channelGroupDuckingScalingOrig, channelGroupDuckingScalingParsed, ducking_scaling_present);
            if (channelGroupDuckingScalingOrig->ducking_scaling_present == 1)
            {
                CHECK(channelGroupDuckingScalingOrig, channelGroupDuckingScalingParsed, reserved2);
                CHECK(channelGroupDuckingScalingOrig, channelGroupDuckingScalingParsed, bs_ducking_scaling);
            }
        }
    }
    else
    {
        for (u8 i = 0; i < atom->channel_count; i++)
        {
            DRCInstructionsChannelGroupGainScaling  *channelGroupGainScalingOrig;
            DRCInstructionsChannelGroupGainScaling  *channelGroupGainScalingParsed;
            MP4GetListEntry(atom->channelGroupGainScalings, i, (char**) &channelGroupGainScalingOrig);
            MP4GetListEntry(parsedDRCAtom->channelGroupGainScalings, i, (char**) &channelGroupGainScalingParsed);
            CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, reserved1);
            CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, gain_scaling_present);
            if (channelGroupGainScalingOrig->gain_scaling_present == 1)
            {
                CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, bs_attenuation_scaling);
                CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, bs_amplification_scaling);
            }
            CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, reserved2);
            CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, gain_offset_present);
            if (channelGroupGainScalingOrig->gain_offset_present == 1)
            {
                CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, reserved3);
                CHECK(channelGroupGainScalingOrig, channelGroupGainScalingParsed, bs_gain_offset);
            }
        }
    }

    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "DRCInstructionsUniDRCAtom test successfull!");
bail:
    return err;
}

MP4Err testAll(u32 iterations)
{
    MP4Err err = MP4NoErr;
    srand((unsigned int) time(NULL));
    for (u32 i = 0; i < iterations; i++)
    {
        err = testChannelLayoutAtom();          if (err) goto bail;
        err = testDownMixInstructionsAtom();    if (err) goto bail;
        err = testLoudnessBaseAtom();           if (err) goto bail;

        err = testDRCCoefficientBasicAtom();    if (err) goto bail;
        err = testDRCCoefficientUniDRCAtom();   if (err) goto bail;
        err = testDRCInstructionsBasicAtom();   if (err) goto bail;
        err = testDRCInstructionsUniDRCAtom();  if (err) goto bail;
    }
bail:
    return err;
}
