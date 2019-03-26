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

#include "PCMFormatData.h"
#include "FileFormatToPCM.h"
#include "Logger.h"

MP4Err  createSampleEntryFromBuffer    (MP4Track trak, PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr *audioSampleEntry);
MP4Err  collectSampleEntryAtoms        (MP4AudioSampleEntryAtomPtr audioSampleEntry, PCMFormatData *pcmFormatData, PCMAtoms *pcmAtoms);

MP4Err  createPCMAtomsFromAudioTrack   (MP4Track trak, PCMFormatData *pcmFormatData, PCMAtoms *pcmAtoms)
{
    MP4Err                          err;
    MP4AudioSampleEntryAtomPtr      audioSampleEntry;

    logMsg(LOGLEVEL_DEBUG, "Creating PCM Atoms from audio track");
    
    err = MP4NoErr;
    err = createSampleEntryFromBuffer(trak, pcmFormatData, &audioSampleEntry);             if (err) goto bail;

    pcmAtoms->sampleEntryAtom           = audioSampleEntry;

    pcmFormatData->channelCount         = audioSampleEntry->reserved3;
    pcmFormatData->sampleRate           = audioSampleEntry->timeScale;
    pcmFormatData->pcmSampleSize        = audioSampleEntry->reserved4 / pcmFormatData->channelCount;

    switch (audioSampleEntry->type) {
        case AudioIntegerPCMSampleEntryType:
            pcmFormatData->isInteger = 1;
            break;
        case AudioFloatPCMSampleEntryType:
            pcmFormatData->isInteger = 0;
            break;
        default:
            err = MP4BadParamErr;
            return err;
    }
    pcmAtoms->channelLayoutAtom    = NULL;
    pcmAtoms->pcmConfigAtom        = NULL;

    err = collectSampleEntryAtoms(audioSampleEntry, pcmFormatData, pcmAtoms);             if (err) goto bail;

    logMsg(LOGLEVEL_INFO, "Audio data from sample entry: channels: %d, sampleRate: %d, pcmSampleSize: %d, isInteger: %d",
           pcmFormatData->channelCount, pcmFormatData->sampleRate, pcmFormatData->pcmSampleSize, pcmFormatData->isInteger);

    logMsg(LOGLEVEL_DEBUG, "Creating PCM atoms from audio track finished.");
bail:
    return err;
}

MP4Err  createSampleEntryFromBuffer         (MP4Track trak, PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr *audioSampleEntry)
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
    err = MP4ParsePCMAtom( is, &atom ); if (err) goto bail;

    switch (atom->type) {
        case AudioIntegerPCMSampleEntryType:
            pcmFormatData->isInteger = 1;
            break;
        case AudioFloatPCMSampleEntryType:
            pcmFormatData->isInteger = 0;
            break;
        default:
            logMsg(LOGLEVEL_ERROR, "Audio sample entry type is not PCM (%s)", atom->name);
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

MP4Err  collectSampleEntryAtoms (MP4AudioSampleEntryAtomPtr audioSampleEntry, PCMFormatData *pcmFormatData, PCMAtoms *pcmAtoms)
{
    MP4Err      err;
    u32         i;
    MP4AtomPtr  atom;
    u8          pcmConfigBoxFound;

    pcmConfigBoxFound = 0;
    err = MP4NoErr;
    
    for (i = 0; i < audioSampleEntry->ExtensionAtomList->entryCount; i++)
    {
        MP4GetListEntry(audioSampleEntry->ExtensionAtomList, i, (char**) &atom);
        switch (atom->type)
        {
            case MP4ChannelLayoutAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found channel layout atom in audio sample entry");
                pcmAtoms->channelLayoutAtom = (MP4ChannelLayoutAtomPtr) atom;
                if (pcmAtoms->channelLayoutAtom->version > 0) {
                    pcmFormatData->channelCount = pcmAtoms->channelLayoutAtom->baseChannelCount; // override channel count from sample entry
                }
                break;
            case MP4PCMConfigAtomType:
                logMsg(LOGLEVEL_DEBUG, "Found PCM configuration atom in audio sample entry");
                pcmAtoms->pcmConfigAtom         = (MP4PCMConfigAtomPtr) atom;
                pcmFormatData->pcmSampleSize    = pcmAtoms->pcmConfigAtom->PCM_sample_size;
                pcmFormatData->isLittleEndian   = pcmAtoms->pcmConfigAtom->format_flags & ENDIAN_FORMAT_FLAG_LITTLE;
                pcmConfigBoxFound = 1;
                break;
            default:
                logMsg(LOGLEVEL_WARNING, "Additional atom found in sample entry box: %s", atom->name);
                break;
        }
    }
    if (pcmConfigBoxFound == 0) {
        logMsg(LOGLEVEL_ERROR, "Mandatory PCM configuration box is not present");
        err = MP4InvalidMediaErr;
    }
bail:
    return err;
}

MP4Err  freePCMAtoms                   (PCMAtoms *pcmAtoms)
{
    MP4Err err;
    
    err = MP4NoErr;
    
    pcmAtoms->sampleEntryAtom->destroy((MP4AtomPtr) pcmAtoms->sampleEntryAtom);

    err = MP4DeleteLinkedList((MP4LinkedList)pcmAtoms->channelLayoutAtom); if (err) goto bail;
    err = MP4DeleteLinkedList((MP4LinkedList)pcmAtoms->pcmConfigAtom);     if (err) goto bail;
bail:
    return err;
}
