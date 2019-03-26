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

#include <math.h>
#include "PCMToFileFormat.h"
#include "Logger.h"

MP4Err  addChannelLayout                (PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr audioSampleEntry);

MP4Err  addPCMConfig                    (PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr audioSampleEntry);

MP4Err  addPCMDataToAudioSampleEntry    (PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err          err;
    err             = MP4NoErr;

    logMsg(LOGLEVEL_DEBUG, "Adding PCM data to audio sample entry atom");

    audioSampleEntry->timeScale             = pcmFormatData->sampleRate;
    audioSampleEntry->dataReferenceIndex    = 1;
    audioSampleEntry->reserved3             = pcmFormatData->channelCount;
    audioSampleEntry->reserved4             = pcmFormatData->pcmSampleSize;
    if (pcmFormatData->isInteger) {
        audioSampleEntry->type              = AudioIntegerPCMSampleEntryType;
    }
    else {
        audioSampleEntry->type              = AudioFloatPCMSampleEntryType;
    }

    err             = addChannelLayout (pcmFormatData, audioSampleEntry); if (err) goto bail;
    err             = addPCMConfig     (pcmFormatData, audioSampleEntry); if (err) goto bail;
bail:
    return err;
}


MP4Err addChannelLayout             (PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err                      err;
    MP4ChannelLayoutAtomPtr     channelLayoutAtom;
    
    err = MP4NoErr;

    logMsg(LOGLEVEL_DEBUG, "Adding channel layout to sample entry");
    
    err = MP4CreateChannelLayoutAtom(&channelLayoutAtom); if (err) goto bail;

    channelLayoutAtom->version                  = 1;
    channelLayoutAtom->stream_structure         = 1;
    channelLayoutAtom->definedLayout            = pcmFormatData->channelCount;  // This is incorrect for more than two channels
    channelLayoutAtom->definedLayouts           = NULL;
    channelLayoutAtom->omittedChannelsMap       = 0;
    channelLayoutAtom->object_count             = 0;
    channelLayoutAtom->formatOrdering           = 1;
    channelLayoutAtom->baseChannelCount         = pcmFormatData->channelCount;
    channelLayoutAtom->layoutChannelCount       = pcmFormatData->channelCount;
    channelLayoutAtom->channelOrderDefinition   = 0;
    channelLayoutAtom->omittedChannelsPresent   = 0;

    err = MP4AddListEntry(channelLayoutAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;

bail:
    return err;
}


MP4Err  addPCMConfig (PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr audioSampleEntry)
{
    MP4Err              err;
    MP4PCMConfigAtomPtr pcmConfigAtom;

    logMsg(LOGLEVEL_DEBUG, "Adding PCM configuration to sample entry");
    
    err = MP4CreatePCMConfigAtom(&pcmConfigAtom); if (err) goto bail;

    switch (pcmFormatData->pcmSampleSize) {
        case PCM_SAMPLE_SIZE_INT_16:
        case PCM_SAMPLE_SIZE_INT_24:
        case PCM_SAMPLE_SIZE_INT_32:
/*      case PCM_SAMPLE_SIZE_FLOAT_32:  */
        case PCM_SAMPLE_SIZE_FLOAT_64:
            break;
        default:
            logMsg(LOGLEVEL_ERROR, "Error: PCM sample size not supported by MPEG-4 (%d)\n", pcmFormatData->pcmSampleSize);
            err = MP4BadParamErr;
            return err;
    }
    pcmConfigAtom->PCM_sample_size  = pcmFormatData->pcmSampleSize;
    if (pcmFormatData->isLittleEndian) {
        pcmConfigAtom->format_flags = ENDIAN_FORMAT_FLAG_LITTLE;
    }
    else {
        pcmConfigAtom->format_flags = 0;
    }

    err = MP4AddListEntry(pcmConfigAtom, audioSampleEntry->ExtensionAtomList); if ( err ) goto bail;
bail:
    return err;
}

