/* This software module was originally developed by Apple Computer, Inc.
 * in the course of development of MPEG-4.
 * This software module is an implementation of a part of one or
 * more MPEG-4 tools as specified by MPEG-4.
 * ISO/IEC gives users of MPEG-4 free license to this
 * software module or modifications thereof for use in hardware
 * or software products claiming conformance to MPEG-4 only for evaluation and testing purposes.
 * Those intending to use this software module in hardware or software
 * products are advised that its use may infringe existing patents.
 * The original developer of this software module and his/her company,
 * the subsequent editors and their companies, and ISO/IEC have no
 * liability for use of this software module or modifications thereof
 * in an implementation.
 *
 * Copyright is not released for non MPEG-4 conforming
 * products. Apple Computer, Inc. retains full right to use the code for its own
 * purpose, assign or donate the code to a third party and to
 * inhibit third parties from using the code for non
 * MPEG-4 conforming products.
 * This copyright notice must be included in all copies or
 * derivative works. */

/* Copyright© 2017 Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * acting on behalf of its Fraunhofer Institute for Telecommunications,
 * Heinrich Hertz Institute, HHI
 * All rights reserved.
 */

/*
  $Id: MP4Media.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "MP4Descriptors.h"
#include "MP4DataHandler.h"
#include <stdlib.h>
#include <string.h>

MP4Err MP4GetMediaESD(MP4Media theMedia, u32 index, MP4ES_DescriptorPtr *outESD,
                      u32 *outDataReferenceIndex);
MP4Err MP4GetMediaSampleDescIndex(MP4Media theMedia, u64 desiredTime,
                                  u32 *outSampleDescriptionIndex);

MP4Err MP4CreateMemoryInputStream(char *base, u32 size, MP4InputStreamPtr *outStream);
MP4Err MP4ParseAtomUsingProtoList(MP4InputStreamPtr inputStream, u32 *protoList, u32 defaultAtom,
                                  MP4AtomPtr *outAtom);

#ifdef ISMACrypt
u32 MP4SampleEntryProtos[] = {MP4MPEGSampleEntryAtomType,
                              MP4VisualSampleEntryAtomType,
                              MP4AudioSampleEntryAtomType,
                              MP4EncAudioSampleEntryAtomType,
                              MP4EncVisualSampleEntryAtomType,
                              MP4XMLMetaSampleEntryAtomType,
                              MP4TextMetaSampleEntryAtomType,
                              MP4AMRSampleEntryAtomType,
                              MP4AWBSampleEntryAtomType,
                              MP4AMRWPSampleEntryAtomType,
                              MP4H263SampleEntryAtomType,
                              MP4RestrictedVideoSampleEntryAtomType,
                              MP4BoxedMetadataSampleEntryType,
                              ISOAVCSampleEntryAtomType,
                              ISOHEVCSampleEntryAtomType,
                              ISOVVCSampleEntryAtomTypeInBand,
                              ISOVVCSampleEntryAtomTypeOutOfBand,
                              ISOVVCSubpicSampleEntryAtomType,
                              0};
#else
u32 MP4SampleEntryProtos[] = {MP4MPEGSampleEntryAtomType,
                              MP4VisualSampleEntryAtomType,
                              MP4AudioSampleEntryAtomType,
                              MP4XMLMetaSampleEntryAtomType,
                              MP4TextMetaSampleEntryAtomType,
                              MP4AMRSampleEntryAtomType,
                              MP4AWBSampleEntryAtomType,
                              MP4AMRWPSampleEntryAtomType,
                              MP4H263SampleEntryAtomType,
                              MP4RestrictedVideoSampleEntryAtomType,
                              MP4BoxedMetadataSampleEntryType,
                              ISOAVCSampleEntryAtomType,
                              ISOHEVCSampleEntryAtomType,
                              ISOVVCSampleEntryAtomTypeInBand,
                              ISOVVCSampleEntryAtomTypeOutOfBand,
                              ISOVVCSubpicSampleEntryAtomType,
                              0};
#endif

MP4Err sampleEntryHToAtomPtr(MP4Handle sampleEntryH, MP4AtomPtr *entryPtr, u32 defaultType)
{
  MP4Err err = MP4NoErr;
  MP4InputStreamPtr is;
  u32 size;

  is = NULL;

  if(sampleEntryH == NULL) return MP4BadParamErr;

  err = MP4GetHandleSize(sampleEntryH, &size);
  if(err) goto bail;
  err = MP4CreateMemoryInputStream(*sampleEntryH, size, &is);
  if(err) goto bail;
  is->debugging = 0;
  err           = MP4ParseAtomUsingProtoList(is, MP4SampleEntryProtos, defaultType, entryPtr);
  if(err) goto bail;

bail:
  if(is) is->destroy(is);
  TEST_RETURN(err);

  return err;
}

MP4Err atomPtrToSampleEntryH(MP4Handle sampleEntryH, MP4AtomPtr entry)
{
  MP4Err err = MP4NoErr;

  err = entry->calculateSize((MP4AtomPtr)entry);
  if(err) goto bail;
  err = MP4SetHandleSize(sampleEntryH, entry->size);
  if(err) goto bail;
  err = entry->serialize((MP4AtomPtr)entry, *sampleEntryH);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
MP4NewSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH, u32 dataReferenceIndex,
                        u32 objectTypeIndication, u32 streamType, u32 decoderBufferSize,
                        u32 maxBitrate, u32 avgBitrate, MP4Handle decoderSpecificInfoH)
{
  return MP4NewSampleDescriptionWithOCRAssociation(
    theTrack, sampleDescriptionH, dataReferenceIndex, objectTypeIndication, streamType,
    decoderBufferSize, maxBitrate, avgBitrate, decoderSpecificInfoH, 0);
}

MP4_EXTERN(MP4Err)
MP4NewSampleDescriptionWithOCRAssociation(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                          u32 dataReferenceIndex, u32 objectTypeIndication,
                                          u32 streamType, u32 decoderBufferSize, u32 maxBitrate,
                                          u32 avgBitrate, MP4Handle decoderSpecificInfoH,
                                          u32 theOCRESID)
{
  MP4Err MP4CreateMPEGSampleEntryAtom(MP4MPEGSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateVisualSampleEntryAtom(MP4VisualSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateAudioSampleEntryAtom(MP4AudioSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateESDAtom(MP4ESDAtomPtr * outAtom);
  MP4Err MP4CreateES_Descriptor(u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr * outDesc);
  MP4Err MP4CreateDecoderConfigDescriptor(u32 tag, u32 size, u32 bytesRead,
                                          MP4DescriptorPtr * outDesc);
  MP4Err MP4CreateSLConfigDescriptor(u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr * outDesc);

  MP4Err err;
  GenericSampleEntryAtomPtr entry;
  MP4ESDAtomPtr esdAtom;
  MP4ES_DescriptorPtr esd;
  MP4DecoderConfigDescriptorPtr config;
  MP4SLConfigDescriptorPtr slconfig;
  MP4TrackAtomPtr trak;
  u32 outReferenceIndex;

  entry   = NULL;
  esdAtom = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  trak = (MP4TrackAtomPtr)theTrack;
  if(trak->newTrackFlags & MP4NewTrackIsVisual)
  {
    err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&entry);
    if(err) goto bail;
  }
  else if(trak->newTrackFlags & MP4NewTrackIsAudio)
  {
    MP4AudioSampleEntryAtomPtr audioSampleEntry;
    MP4Media media;
    u32 timeScale;

    err = MP4CreateAudioSampleEntryAtom((MP4AudioSampleEntryAtomPtr *)&entry);
    if(err) goto bail;
    audioSampleEntry = (MP4AudioSampleEntryAtomPtr)entry;
    err              = MP4GetTrackMedia(theTrack, &media);
    if(err) goto bail;
    err = MP4GetMediaTimeScale(media, &timeScale);
    if(err) goto bail;
    audioSampleEntry->timeScale = (timeScale <= 0xFFFF ? timeScale : 0);
  }
  else
  {
    err = MP4CreateMPEGSampleEntryAtom((MP4MPEGSampleEntryAtomPtr *)&entry);
    if(err) goto bail;
  }
  entry->dataReferenceIndex = dataReferenceIndex;
  err                       = MP4CreateESDAtom(&esdAtom);
  if(err) goto bail;
  err = MP4CreateES_Descriptor(MP4ES_DescriptorTag, 0, 0, (MP4DescriptorPtr *)&esd);
  if(err) goto bail;
  err = MP4CreateDecoderConfigDescriptor(MP4DecoderConfigDescriptorTag, 0, 0,
                                         (MP4DescriptorPtr *)&config);
  if(err) goto bail;
  config->objectTypeIndication = objectTypeIndication;
  config->streamType           = streamType;
  config->upstream             = 0;
  config->bufferSizeDB         = decoderBufferSize;
  config->maxBitrate           = maxBitrate;
  config->avgBitrate           = avgBitrate;
  if(decoderSpecificInfoH)
  {
    MP4Err MP4CreateMemoryInputStream(char *base, u32 size, MP4InputStreamPtr *outStream);
    MP4DescriptorPtr specificInfoDesc;
    MP4InputStreamPtr is;
    u32 infoSize;

    err = MP4GetHandleSize(decoderSpecificInfoH, &infoSize);
    if(err) goto bail;
    err = MP4CreateMemoryInputStream(*decoderSpecificInfoH, infoSize, &is);
    if(err) goto bail;
    is->debugging = 0;
    err           = MP4ParseDescriptor(is, &specificInfoDesc);
    if(err) goto bail;
    config->decoderSpecificInfo = specificInfoDesc;
    if(is)
    {
      is->destroy(is);
      is = NULL;
    }
  }
  esd->decoderConfig = (MP4DescriptorPtr)config;
  err = MP4CreateSLConfigDescriptor(MP4SLConfigDescriptorTag, 0, 0, (MP4DescriptorPtr *)&slconfig);
  if(err) goto bail;
  slconfig->predefined        = SLConfigPredefinedMP4;
  slconfig->useTimestampsFlag = 1;

  /* JLF 11/00: OCR is always 0, if OCR wanted, create a sync dependancy */
  esd->OCRESID = 0;
  if(theOCRESID)
  {
    /* MP4Err MP4AddTrackReferenceWithID( MP4Track theTrack, u32 dependsOnID, u32 dependencyType,
     * u32 *outReferenceIndex); */
    /* already declared in MP4Movies.h */
    err = MP4AddTrackReferenceWithID((MP4Track)trak, theOCRESID, MP4SyncTrackReferenceAtomType,
                                     &outReferenceIndex);
    if(err) goto bail;
  }
  esd->slConfig       = (MP4DescriptorPtr)slconfig;
  esdAtom->descriptor = (MP4DescriptorPtr)esd;
  err                 = MP4AddListEntry((void *)esdAtom, entry->ExtensionAtomList);
  if(err) goto bail;
  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(esdAtom)
  {
    /* JLF 07/01: need to set the esd atom to NULL as the SampleDescEntry destructors
       attempts to delete it */
    if(entry) err = MP4DeleteListEntryAtom(entry->ExtensionAtomList, MP4ESDAtomType);
    esdAtom->destroy((MP4AtomPtr)esdAtom);
  }

  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

/* JLF: set the stream priority in the sample desc */
MP4_EXTERN(MP4Err) MP4SetSampleDescriptionPriority(MP4Handle sampleEntryH, u32 priority)
{
  MP4Err err = MP4NoErr;
  MP4ESDAtomPtr esda;
  MP4ES_Descriptor *esd;
  GenericSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  err = MP4GetListEntryAtom(entry->ExtensionAtomList, MP4ESDAtomType, (MP4AtomPtr *)&esda);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadParamErr);
  }
  esd = (MP4ES_DescriptorPtr)esda->descriptor;
  if(esd == NULL)
  {
    err = MP4InvalidMediaErr;
    goto bail;
  }
  esd->streamPriority = priority;

  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

/* JLF: add a Descriptor to the esd in the sample desc */
MP4_EXTERN(MP4Err) MP4AddDescToSampleDescription(MP4Handle sampleEntryH, MP4Handle descriptorH)
{
  MP4Err err = MP4NoErr;
  MP4ESDAtomPtr esda;
  MP4ES_Descriptor *esd;
  MP4DescriptorPtr desc;
  GenericSampleEntryAtomPtr entry = NULL;
  MP4InputStreamPtr is;
  u32 size;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  err = MP4GetListEntryAtom(entry->ExtensionAtomList, MP4ESDAtomType, (MP4AtomPtr *)&esda);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadParamErr);
  }
  esd = (MP4ES_DescriptorPtr)esda->descriptor;
  if(esd == NULL)
  {
    err = MP4InvalidMediaErr;
    goto bail;
  }
  /* then parse our descriptor... */
  err = MP4GetHandleSize(descriptorH, &size);
  if(err) goto bail;
  err = MP4CreateMemoryInputStream(*descriptorH, size, &is);
  if(err) goto bail;
  is->debugging = 0;
  err           = MP4ParseDescriptor(is, (MP4DescriptorPtr *)&desc);
  if(err) goto bail;
  is->destroy(is);

  if(!desc)
  {
    err = MP4BadParamErr;
    goto bail;
  }
  err = esd->addDescriptor((MP4DescriptorPtr)esd, desc);
  if(err) goto bail;

  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err) ISOSetSampleDescriptionDimensions(MP4Handle sampleEntryH, u16 width, u16 height)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  entry->width  = width;
  entry->height = height;

  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err) ISOSetSampleDescriptionType(MP4Handle sampleEntryH, u32 type)
{
  MP4Err err = MP4NoErr;
  MP4AtomPtr esds;
  MP4VisualSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  entry->type = type;

  type = (type & 0xFFFFFF00) | ' ';

  if(type != MP4_FOUR_CHAR_CODE('m', 'p', '4', ' '))
  {
    err = MP4GetListEntryAtom(entry->ExtensionAtomList, MP4ESDAtomType, &esds);
    if(err) goto bail;
    if(esds) esds->destroy(esds);
    err = MP4DeleteListEntryAtom(entry->ExtensionAtomList, MP4ESDAtomType);
    if(err) goto bail;
  }
  /* DO NOT set the type of a sample description away from and back to MP4, as you'll lose the esds
   */

  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetSampleDescriptionDimensions(MP4Handle sampleEntryH, u16 *width, u16 *height)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type == MP4RestrictedVideoSampleEntryAtomType)
  {
    *width  = (u16)((MP4RestrictedVideoSampleEntryAtomPtr)entry)->width;
    *height = (u16)((MP4RestrictedVideoSampleEntryAtomPtr)entry)->height;
  }
  else
  {
    *width  = (u16)entry->width;
    *height = (u16)entry->height;
  }

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err) ISOGetSampleDescriptionType(MP4Handle sampleEntryH, u32 *type)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  *type = entry->type;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISONewGeneralSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                               u32 dataReferenceIndex, u32 sampleEntryType,
                               MP4GenericAtom extensionAtom)
{
  MP4Err MP4CreateMPEGSampleEntryAtom(MP4MPEGSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateVisualSampleEntryAtom(MP4VisualSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateAudioSampleEntryAtom(MP4AudioSampleEntryAtomPtr * outAtom);

  MP4Err err;
  GenericSampleEntryAtomPtr entry = NULL;
  MP4TrackAtomPtr trak            = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  trak = (MP4TrackAtomPtr)theTrack;
  if(trak->newTrackFlags & MP4NewTrackIsVisual)
  {
    err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&entry);
    if(err) goto bail;
  }
  else if(trak->newTrackFlags & MP4NewTrackIsAudio)
  {
    MP4AudioSampleEntryAtomPtr audioSampleEntry;
    MP4Media media;
    u32 timeScale;

    err = MP4CreateAudioSampleEntryAtom((MP4AudioSampleEntryAtomPtr *)&entry);
    if(err) goto bail;
    audioSampleEntry = (MP4AudioSampleEntryAtomPtr)entry;
    err              = MP4GetTrackMedia(theTrack, &media);
    if(err) goto bail;
    err = MP4GetMediaTimeScale(media, &timeScale);
    if(err) goto bail;
    audioSampleEntry->timeScale = (timeScale <= 0xFFFF ? timeScale : 0);
  }
  else
  {
    /* this case covers MP4NewTrackIsMetadata */
    err = MP4CreateMPEGSampleEntryAtom((MP4MPEGSampleEntryAtomPtr *)&entry);
    if(err) goto bail;
  }
  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = sampleEntryType;

  if(extensionAtom)
  {
    err = MP4AddListEntry((void *)extensionAtom, entry->ExtensionAtomList);
    if(err) goto bail;
  }

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
ISOAddAtomToSampleDescription(MP4Handle sampleEntryH, MP4GenericAtom extensionAtom)
{
  MP4Err err                         = MP4NoErr;
  MP4GenericSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  if(extensionAtom)
  {
    err = MP4AddListEntry((void *)extensionAtom, entry->ExtensionAtomList);
    if(err) goto bail;
  }
  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetAtomFromSampleDescription(MP4Handle sampleEntryH, u32 atomType, MP4GenericAtom *outAtom)
{
  MP4Err err                         = MP4NoErr;
  MP4GenericSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;
  err = MP4GetListEntryAtom(entry->ExtensionAtomList, atomType, (MP4AtomPtr *)outAtom);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOAddBitrateToSampleDescription(MP4Handle sampleEntryH, u8 is_3GPP, u32 buffersizeDB,
                                 u32 maxBitrate, u32 avgBitrate)
{
  MP4Err err                         = MP4NoErr;
  MP4GenericSampleEntryAtomPtr entry = NULL;
  MP4BitRateAtomPtr rate;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  err = MP4CreateBitRateAtom(&rate);
  if(err) goto bail;
  rate->type         = (is_3GPP ? TGPPBitRateAtomType : MP4BitRateAtomType);
  rate->buffersizeDB = buffersizeDB;
  rate->max_bitrate  = maxBitrate;
  rate->avg_bitrate  = avgBitrate;

  err = MP4AddListEntry((void *)rate, entry->ExtensionAtomList);
  if(err) goto bail;

  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISONewXMLMetaDataSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                   u32 dataReferenceIndex, char *content_encoding,
                                   char *xml_namespace, char *schema_location)
{
  MP4Err err;
  MP4XMLMetaSampleEntryAtomPtr entry = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  err = MP4CreateXMLMetaSampleEntryAtom((MP4XMLMetaSampleEntryAtomPtr *)&entry);
  if(err) goto bail;

  entry->dataReferenceIndex = dataReferenceIndex;

  if(content_encoding)
  {
    entry->content_encoding = (char *)calloc(strlen(content_encoding) + 1, 1);
    TESTMALLOC(entry->content_encoding);
    strcpy(entry->content_encoding, content_encoding);
  }
  else
    entry->content_encoding = NULL;
  if(xml_namespace)
  {
    entry->xml_namespace = (char *)calloc(strlen(xml_namespace) + 1, 1);
    TESTMALLOC(entry->xml_namespace);
    strcpy(entry->xml_namespace, xml_namespace);
  }
  else
    entry->xml_namespace = NULL;
  if(schema_location)
  {
    entry->schema_location = (char *)calloc(strlen(schema_location) + 1, 1);
    TESTMALLOC(entry->schema_location);
    strcpy(entry->schema_location, schema_location);
  }
  else
    entry->schema_location = NULL;

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
ISONewTextMetaDataSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                    u32 dataReferenceIndex, char *content_encoding,
                                    char *mime_format)
{
  MP4Err err;
  MP4TextMetaSampleEntryAtomPtr entry = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  err = MP4CreateTextMetaSampleEntryAtom((MP4TextMetaSampleEntryAtomPtr *)&entry);
  if(err) goto bail;

  entry->dataReferenceIndex = dataReferenceIndex;

  if(content_encoding)
  {
    entry->content_encoding = (char *)calloc(strlen(content_encoding) + 1, 1);
    TESTMALLOC(entry->content_encoding);
    strcpy(entry->content_encoding, content_encoding);
  }
  else
    entry->content_encoding = NULL;
  if(mime_format)
  {
    entry->mime_format = (char *)calloc(strlen(mime_format) + 1, 1);
    TESTMALLOC(entry->mime_format);
    strcpy(entry->mime_format, mime_format);
  }
  else
    entry->mime_format = NULL;

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
ISONewH263SampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH, u32 dataReferenceIndex,
                            u32 vendor, u8 decoder_version, u8 H263_level, u8 H263_profile)
{
  MP4Err err;
  GenericSampleEntryAtomPtr entry                 = NULL;
  MP4H263SpecificInfoAtomPtr H263SpecificInfoAtom = NULL;
  MP4TrackAtomPtr trak                            = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  trak = (MP4TrackAtomPtr)theTrack;
  if(!(trak->newTrackFlags & MP4NewTrackIsVisual)) BAILWITHERROR(MP4BadParamErr);
  err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&entry);
  if(err) goto bail;

  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = MP4H263SampleEntryAtomType;

  err = MP4CreateH263SpecificInfoAtom(&H263SpecificInfoAtom);
  if(err) goto bail;
  H263SpecificInfoAtom->vendor          = vendor;
  H263SpecificInfoAtom->decoder_version = decoder_version;
  H263SpecificInfoAtom->H263_level      = H263_level;
  H263SpecificInfoAtom->H263_profile    = H263_profile;

  err = MP4AddListEntry((void *)H263SpecificInfoAtom, entry->ExtensionAtomList);
  if(err) goto bail;

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
ISONewAMRSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH, u32 dataReferenceIndex,
                           u8 is_WB, u32 vendor, u8 decoder_version, u16 mode_set,
                           u8 mode_change_period, u8 frames_per_sample)
{
  MP4Err err;
  MP4AudioSampleEntryAtomPtr entry;
  MP4AMRSpecificInfoAtomPtr AMRSpecificInfoAtom;
  MP4TrackAtomPtr trak;
  MP4Media media;
  u32 timeScale;

  entry = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  trak = (MP4TrackAtomPtr)theTrack;
  if(!(trak->newTrackFlags & MP4NewTrackIsAudio)) BAILWITHERROR(MP4BadParamErr);
  err = MP4CreateAudioSampleEntryAtom((MP4AudioSampleEntryAtomPtr *)&entry);
  if(err) goto bail;

  err = MP4GetTrackMedia(theTrack, &media);
  if(err) goto bail;
  err = MP4GetMediaTimeScale(media, &timeScale);
  if(err) goto bail;
  entry->timeScale = timeScale;

  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = (is_WB ? MP4AWBSampleEntryAtomType : MP4AMRSampleEntryAtomType);

  err = MP4CreateAMRSpecificInfoAtom(&AMRSpecificInfoAtom);
  if(err) goto bail;
  AMRSpecificInfoAtom->vendor             = vendor;
  AMRSpecificInfoAtom->decoder_version    = decoder_version;
  AMRSpecificInfoAtom->mode_set           = mode_set;
  AMRSpecificInfoAtom->mode_change_period = mode_change_period;
  AMRSpecificInfoAtom->frames_per_sample  = frames_per_sample;

  err = MP4AddListEntry((void *)AMRSpecificInfoAtom, entry->ExtensionAtomList);
  if(err) goto bail;

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
ISONewAMRWPSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                             u32 dataReferenceIndex, u32 vendor, u8 decoder_version)
{
  MP4Err err;
  MP4AudioSampleEntryAtomPtr entry;
  MP4AMRWPSpecificInfoAtomPtr AMRSpecificInfoAtom;
  MP4TrackAtomPtr trak;
  MP4Media media;
  u32 timeScale;

  entry = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  trak = (MP4TrackAtomPtr)theTrack;
  if(!(trak->newTrackFlags & MP4NewTrackIsAudio)) BAILWITHERROR(MP4BadParamErr);
  err = MP4CreateAudioSampleEntryAtom((MP4AudioSampleEntryAtomPtr *)&entry);
  if(err) goto bail;

  err = MP4GetTrackMedia(theTrack, &media);
  if(err) goto bail;
  err = MP4GetMediaTimeScale(media, &timeScale);
  if(err) goto bail;
  entry->timeScale = timeScale;

  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = MP4AMRWPSampleEntryAtomType;

  err = MP4CreateAMRWPSpecificInfoAtom(&AMRSpecificInfoAtom);
  if(err) goto bail;
  AMRSpecificInfoAtom->vendor          = vendor;
  AMRSpecificInfoAtom->decoder_version = decoder_version;

  err = MP4AddListEntry((void *)AMRSpecificInfoAtom, entry->ExtensionAtomList);
  if(err) goto bail;

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

/* whole loads of bitBuffer stuff for parsing parameter sets */

typedef struct
{
  u8 *ptr;
  u32 length;
  u8 *cptr;
  u8 cbyte;
  u32 curbits;
  u32 bits_left;

  u8 prevent_emulation;  /* true or false */
  u8 emulation_position; /* 0 usually, 1 after 1 zero byte, 2 after 2 zero bytes,
                                                          3 after 00 00 03, and the 3 gets stripped
                          */
} BitBuffer;

static MP4Err BitBuffer_Init(BitBuffer *bb, u8 *p, u32 length)
{
  int err = MP4NoErr;

  if(length > 0x0fffffff)
  {
    err = MP4BadParamErr;
    goto bail;
  }

  bb->ptr    = (void *)p;
  bb->length = length;

  bb->cptr    = (void *)p;
  bb->cbyte   = *bb->cptr;
  bb->curbits = 8;

  bb->bits_left = length * 8;

  bb->prevent_emulation  = 0;
  bb->emulation_position = (bb->cbyte == 0 ? 1 : 0);

bail:
  return err;
}

static u32 GetBits(BitBuffer *bb, u32 nBits, MP4Err *errout)
{
  MP4Err err = MP4NoErr;
  int myBits;
  int myValue;
  int myResidualBits;
  int leftToRead;

  myValue = 0;
  if(nBits > bb->bits_left)
  {
    err = MP4EOF;
    goto bail;
  }

  if(bb->curbits <= 0)
  {
    bb->cbyte   = *++bb->cptr;
    bb->curbits = 8;

    if(bb->prevent_emulation != 0)
    {
      if((bb->emulation_position >= 2) && (bb->cbyte == 3))
      {
        bb->cbyte = *++bb->cptr;
        bb->bits_left -= 8;
        bb->emulation_position = bb->cbyte ? 0 : 1;
        if(nBits > bb->bits_left)
        {
          err = MP4EOF;
          goto bail;
        }
      }
      else if(bb->cbyte == 0)
        bb->emulation_position += 1;
      else
        bb->emulation_position = 0;
    }
  }

  if(nBits > bb->curbits) myBits = bb->curbits;
  else
    myBits = nBits;

  myValue        = (bb->cbyte >> (8 - myBits));
  myResidualBits = bb->curbits - myBits;
  leftToRead     = nBits - myBits;
  bb->bits_left -= myBits;

  bb->curbits = myResidualBits;
  bb->cbyte   = ((bb->cbyte) << myBits) & 0xff;

  if(leftToRead > 0)
  {
    u32 newBits;
    newBits = GetBits(bb, leftToRead, &err);
    myValue = (myValue << leftToRead) | newBits;
  }

bail:
  if(errout) *errout = err;
  return myValue;
}

static MP4Err GetBytes(BitBuffer *bb, u32 nBytes, u8 *p)
{
  MP4Err err = MP4NoErr;
  unsigned int i;

  for(i = 0; i < nBytes; i++)
  {
    *p++ = (u8)GetBits(bb, 8, &err);
    if(err) break;
  }

  return err;
}

static u32 read_golomb_uev(BitBuffer *bb, MP4Err *errout)
{
  MP4Err err = MP4NoErr;

  u32 power   = 1;
  u32 value   = 0;
  u32 leading = 0;
  u32 nbits   = 0;

  leading = GetBits(bb, 1, &err);
  if(err) goto bail;

  while(leading == 0)
  {
    power = power << 1;
    nbits++;
    leading = GetBits(bb, 1, &err);
    if(err) goto bail;
  }

  if(nbits > 0)
  {
    value = GetBits(bb, nbits, &err);
    if(err) goto bail;
  }

bail:
  if(errout) *errout = err;
  return (power - 1 + value);
}

MP4_EXTERN(MP4Err)
ISONewAVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH, u32 dataReferenceIndex,
                           u32 length_size, MP4Handle first_sps, MP4Handle first_pps,
                           MP4Handle first_spsext)
{
  MP4Err MP4CreateVisualSampleEntryAtom(MP4VisualSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateVCConfigAtom(ISOVCConfigAtomPtr * outAtom);

  MP4Err err;
  GenericSampleEntryAtomPtr entry;
  ISOVCConfigAtomPtr config;
  MP4TrackAtomPtr trak;
  BitBuffer mybb;
  BitBuffer *bb;

  u32 the_size, y;
  u8 x;

  entry = NULL;

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  trak = (MP4TrackAtomPtr)theTrack;
  if(!(trak->newTrackFlags & MP4NewTrackIsVisual)) BAILWITHERROR(MP4BadParamErr);
  err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&entry);
  if(err) goto bail;

  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = ISOAVCSampleEntryAtomType;

  err                 = MP4CreateVCConfigAtom(&config);
  config->length_size = length_size;

  err = MP4AddListEntry((void *)config, entry->ExtensionAtomList);
  if(err) goto bail;

  if(!first_sps) BAILWITHERROR(MP4BadParamErr);
  err = MP4GetHandleSize(first_sps, &the_size);
  if(err) goto bail;

  bb  = &mybb;
  err = BitBuffer_Init(bb, (u8 *)*first_sps, 8 * the_size);
  if(err) goto bail;
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  if((x & 0x1F) != 7) BAILWITHERROR(MP4BadParamErr);
  /* 7 == SPS */
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  config->profile = x;
  err             = GetBytes(bb, 1, &x);
  if(err) goto bail;
  config->profile_compatibility = x;
  err                           = GetBytes(bb, 1, &x);
  if(err) goto bail;
  config->level = x;
  y             = read_golomb_uev(bb, &err);
  if(err) goto bail; /* PPS ID */
  if((config->profile == 100) || (config->profile == 110) || (config->profile == 122) ||
     (config->profile == 144))
  {
    y = read_golomb_uev(bb, &err);
    if(err) goto bail;
    config->chroma_format = y;
    y                     = read_golomb_uev(bb, &err);
    if(err) goto bail;
    config->bit_depth_luma_minus8 = y;
    y                             = read_golomb_uev(bb, &err);
    if(err) goto bail;
    config->bit_depth_chroma_minus8 = y;
  }

  if(first_sps && first_pps)
  {
    err = config->addParameterSet(config, first_sps, AVCsps);
    if(err) goto bail;
  }
  if(first_pps)
  {
    err = config->addParameterSet(config, first_pps, AVCpps);
    if(err) goto bail;
  }
  if(first_spsext)
  {
    err = config->addParameterSet(config, first_spsext, AVCspsext);
    if(err) goto bail;
  }

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err) ISOAddVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where)
{
  MP4Err err = MP4NoErr;
  ISOVCConfigAtomPtr config;
  MP4VisualSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOAVCSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  err = config->addParameterSet(config, ps, where);
  if(err) goto bail;

  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetAVCSampleDescription(MP4Handle sampleEntryH, u32 *dataReferenceIndex, u32 *length_size,
                           u32 *sps_count, u32 *pps_count, u32 *spsext_count)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOVCConfigAtomPtr config;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOAVCSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);
  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  *dataReferenceIndex = entry->dataReferenceIndex;
  *length_size        = config->length_size;
  err                 = MP4GetListEntryCount(config->spsList, sps_count);
  if(err) goto bail;
  err = MP4GetListEntryCount(config->ppsList, pps_count);
  if(err) goto bail;
  err = MP4GetListEntryCount(config->spsextList, spsext_count);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOVCConfigAtomPtr config;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOAVCSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);
  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  err = config->getParameterSet(config, ps, where, index);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetHEVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOHEVCConfigAtomPtr config;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOHEVCSampleEntryAtomType && entry->type != ISOLHEVCSampleEntryAtomType)
    BAILWITHERROR(MP4BadParamErr);
  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOHEVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  err = config->getParameterSet(config, ps, where, index);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOAddVVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where)
{
  MP4Err err = MP4NoErr;
  ISOVVCConfigAtomPtr config;
  MP4VisualSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4GenericSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOVVCSampleEntryAtomTypeOutOfBand &&
     entry->type != ISOVVCSampleEntryAtomTypeInBand)
    BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  err = config->addParameterSet(config, ps, where);
  if(err) goto bail;

  /* rewrite it... */
  err = atomPtrToSampleEntryH(sampleEntryH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetVVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOVVCConfigAtomPtr config;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOVVCSampleEntryAtomTypeOutOfBand &&
     entry->type != ISOVVCSampleEntryAtomTypeInBand)
    BAILWITHERROR(MP4BadParamErr);
  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  err = config->getParameterSet(config, ps, where, index);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetVVCNaluNums(MP4Handle sampleEntryH, u32 where, u32 *num_nalus)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOVVCConfigAtomPtr config;
  u32 i;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOVVCSampleEntryAtomTypeOutOfBand &&
     entry->type != ISOVVCSampleEntryAtomTypeInBand)
    BAILWITHERROR(MP4BadParamErr);
  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  for(i = 0; i < 7; i++)
  {
    if(config->arrays[i].NAL_unit_type == where)
    {
      err = MP4GetListEntryCount(config->arrays[i].nalList, num_nalus);
      if(err) BAILWITHERROR(MP4BadParamErr);
      break;
    }
    if(i == 6)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
  }

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetRESVSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOHEVCConfigAtomPtr configHEVC;
  ISOVCConfigAtomPtr configAVC;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != MP4RestrictedVideoSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);

  err =
    MP4GetListEntryAtom(entry->ExtensionAtomList, ISOHEVCConfigAtomType, (MP4AtomPtr *)&configHEVC);
  if(err == MP4NotFoundErr)
  {
    err =
      MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVCConfigAtomType, (MP4AtomPtr *)&configAVC);
    if(err == MP4NotFoundErr) BAILWITHERROR(MP4BadDataErr);
    err = configAVC->getParameterSet(configAVC, ps, where, index);
  }
  else
  {
    err = configHEVC->getParameterSet(configHEVC, ps, where, index);
  }

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err) ISOGetNALUnitLength(MP4Handle sampleEntryH, u32 *out)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOHEVCConfigAtomPtr configHEVC;
  ISOVCConfigAtomPtr configAVC;
  ISOVVCConfigAtomPtr configVVC;
  ISOVVCNALUConfigAtomPtr VVCNaluConfig;

  if(!out) BAILWITHERROR(MP4BadParamErr);

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err)
  {
    goto bail;
  }

  if(entry->type != MP4RestrictedVideoSampleEntryAtomType &&
     entry->type != ISOHEVCSampleEntryAtomType && entry->type != ISOAVCSampleEntryAtomType &&
     entry->type != ISOVVCSampleEntryAtomTypeOutOfBand &&
     entry->type != ISOVVCSampleEntryAtomTypeInBand)
    BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVVCNALUConfigAtomType,
                            (MP4AtomPtr *)&VVCNaluConfig);
  if(err == MP4NoErr)
  {
    *out = VVCNaluConfig->LengthSizeMinusOne + 1;
    goto bail;
  }

  err =
    MP4GetListEntryAtom(entry->ExtensionAtomList, ISOHEVCConfigAtomType, (MP4AtomPtr *)&configHEVC);
  if(err == MP4NotFoundErr)
  {
    err =
      MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVCConfigAtomType, (MP4AtomPtr *)&configAVC);
    if(err == MP4NotFoundErr)
    {
      err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVVCConfigAtomType,
                                (MP4AtomPtr *)&configVVC);
      if(err == MP4NotFoundErr) BAILWITHERROR(MP4BadDataErr);
      *out = configVVC->LengthSizeMinusOne + 1;
    }
    else
    {
      *out = configAVC->length_size;
    }
  }
  else
  {
    *out = configHEVC->lengthSizeMinusOne + 1;
  }

bail:
  if(entry)
  {
    entry->destroy((MP4AtomPtr)entry);
  }

  return err;
}

MP4_EXTERN(MP4Err) ISOGetRESVOriginalFormat(MP4Handle sampleEntryH, u32 *outOrigFmt)
{
  MP4Err err = MP4NoErr;
  MP4RestrictedVideoSampleEntryAtomPtr entry =
    NULL; /* MP4RestrictedVideoSampleEntryAtomPtr | MP4VisualSampleEntryAtomPtr */
  MP4RestrictedSchemeInfoAtomPtr rinf;
  MP4OriginalFormatAtomPtr fmt;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != MP4RestrictedVideoSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);

  err = entry->getRinf((MP4AtomPtr)entry, (MP4AtomPtr *)&rinf);
  if(err) goto bail;
  if(!rinf)
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  fmt = (MP4OriginalFormatAtomPtr)rinf->MP4OriginalFormat;
  if(!fmt)
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  *outOrigFmt = fmt->original_format;
bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetRESVSchemeType(MP4Handle sampleEntryH, u32 *schemeType, u32 *schemeVersion, char **schemeURI)
{
  MP4Err err;
  MP4RestrictedVideoSampleEntryAtomPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != MP4RestrictedVideoSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);

  err = entry->getScheme((MP4AtomPtr)entry, schemeType, schemeVersion, schemeURI);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetRESVSchemeInfoAtom(MP4Handle sampleEntryH, u32 atomType, MP4Handle outAtom)
{
  MP4Err err;
  MP4RestrictedVideoSampleEntryAtomPtr entry = NULL;
  MP4AtomPtr found;

  if(outAtom == NULL) BAILWITHERROR(MP4BadParamErr);

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != MP4RestrictedVideoSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);

  err = entry->getSchemeInfoAtom((MP4AtomPtr)entry, atomType, &found);
  if(err) goto bail;

  err = atomPtrToSampleEntryH(outAtom, found);
  if(err) goto bail;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISONewHEVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH, u32 dataReferenceIndex,
                            u32 length_size, MP4Handle first_sps, MP4Handle first_pps,
                            MP4Handle first_vps)
{
  MP4Err MP4CreateVisualSampleEntryAtom(MP4VisualSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateHEVCConfigAtom(ISOHEVCConfigAtomPtr * outAtom);

  MP4Err err;
  GenericSampleEntryAtomPtr entry;
  ISOHEVCConfigAtomPtr config;
  MP4TrackAtomPtr trak;
  BitBuffer mybb;
  BitBuffer *bb;

  u32 the_size, y, width, height;
  u8 x;
  u32 ii;
  u8 i;
  u8 sps_max_sub_layers;
  u8 sub_layer_profile_present_flag[8];
  u8 sub_layer_level_present_flag[8];

  if((theTrack == NULL) || (sampleDescriptionH == NULL))
  {
    BAILWITHERROR(MP4BadParamErr);
  }

  if(length_size != 1 && length_size != 2 && length_size != 4) BAILWITHERROR(MP4BadParamErr);

  trak = (MP4TrackAtomPtr)theTrack;
  if(!(trak->newTrackFlags & MP4NewTrackIsVisual)) BAILWITHERROR(MP4BadParamErr);
  err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&entry);
  if(err) goto bail;
  entry->super = NULL;

  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = ISOHEVCSampleEntryAtomType;

  err                        = MP4CreateHEVCConfigAtom(&config);
  config->lengthSizeMinusOne = length_size - 1;

  err = MP4AddListEntry((void *)config, entry->ExtensionAtomList);
  if(err) goto bail;

  if(!first_sps) BAILWITHERROR(MP4BadParamErr);
  err = MP4GetHandleSize(first_sps, &the_size);
  if(err) goto bail;

  bb  = &mybb;
  err = BitBuffer_Init(bb, (u8 *)*first_sps, 8 * the_size);
  if(err) goto bail;
  bb->prevent_emulation = 1;

  /* Get first header byte for nal_unit_type */
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;

  /* 33 == SPS */
  if((x >> 1) != 33) BAILWITHERROR(MP4BadParamErr);

  /* Skip second header byte */
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;

  /* sps_video_parameter_set_id (4) + sps_max_sub_layers_minus1 (3) + sps_temporal_id_nesting_flag
   * (1) */
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  sps_max_sub_layers                   = ((x & 0xf) >> 1) + 1;
  config->sps_temporal_id_nesting_flag = x & 1;

  /* profile_tier_level */
  /* general_profile_space (2) + general_tier_flag (1) + general_profile_idc (5) */
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  config->general_profile_idc = x & 0x1f;

  /* general_profile_compatibility_flag[32] */
  config->general_profile_compatibility_flags = 0;
  for(i = 0; i < 4; i++)
  {
    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    config->general_profile_compatibility_flags |= x << ((3 - i) * 8);
  }

  /* progressive_source + interlaced_source + non_packed_constraint +
           frame_only_constraint + general_reserved_zero_44bits[44] */
  for(i = 0; i < 6; i++)
  {
    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
  }

  /* general_level_idc */
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  config->general_level_idc = x;

  /* sub_layer_profile_present_flag[i] + sub_layer_level_present_flag[i] */
  for(i = 0; i < sps_max_sub_layers - 1; i++)
  {
    sub_layer_profile_present_flag[i] = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
    sub_layer_level_present_flag[i] = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
  }
  /* reserved_zero_2bits[ i ] */
  if(sps_max_sub_layers > 1)
  {
    x = (u8)GetBits(bb, 16 - (sps_max_sub_layers - 1) * 2, &err);
    if(err) goto bail;
  }

  /* We are byte-aligned at this point */
  for(i = 0; i < sps_max_sub_layers - 1; i++)
  {
    if(sub_layer_profile_present_flag[i])
    {
      for(ii = 0; ii < 11; ii++)
      {
        err = GetBytes(bb, 1, &x);
        if(err) goto bail;
      }
    }
    if(sub_layer_level_present_flag[i])
    {
      err = GetBytes(bb, 1, &x);
      if(err) goto bail;
    }
  }
  /* end profile_tier_level */

  /* sps_seq_parameter_set_id */
  y = read_golomb_uev(bb, &err);
  if(err) goto bail;
  /* chroma_format_idc */
  y = read_golomb_uev(bb, &err);
  if(err) goto bail;
  config->chromaFormat = y;
  if(y == 3)
  {
    /* separate_colour_plane_flag */
    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
  }
  /* pic_width_in_luma_samples */
  width = read_golomb_uev(bb, &err);
  if(err) goto bail;
  ((MP4VisualSampleEntryAtomPtr)entry)->width = width;

  /* pic_height_in_luma_samples */
  height = read_golomb_uev(bb, &err);
  if(err) goto bail;
  ((MP4VisualSampleEntryAtomPtr)entry)->height = height;
  ((MP4TrackAtomPtr)theTrack)->setDimensions((MP4TrackAtomPtr)theTrack, width, height);

  /* conformance_window_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  if(x)
  {
    /* conf_win_[left|right|top|bottom]_offset */
    for(i = 0; i < 4; i++)
    {
      x = (u8)read_golomb_uev(bb, &err);
      if(err) goto bail;
    }
  }

  /* bit_depth_luma_minus8 */
  y = read_golomb_uev(bb, &err);
  if(err) goto bail;
  config->bitDepthLumaMinus8 = y;

  /* bit_depth_chroma_minus8 */
  y = read_golomb_uev(bb, &err);
  if(err) goto bail;
  config->bitDepthChromaMinus8 = y;

  if(first_vps)
  {
    err = config->addParameterSet(config, first_vps, 32);
    if(err) goto bail;
  }
  if(first_sps)
  {
    err = config->addParameterSet(config, first_sps, 33);
    if(err) goto bail;
  }
  if(first_pps)
  {
    err = config->addParameterSet(config, first_pps, 34);
    if(err) goto bail;
  }

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

MP4_EXTERN(MP4Err)
ISONewMebxSampleDescription(MP4BoxedMetadataSampleEntryPtr *outSE, u32 dataReferenceIndex)
{
  MP4Err err;
  MP4BoxedMetadataSampleEntryPtr mebx;
  MP4MetadataKeyTableBoxPtr keytable;

  err = MP4CreateMP4BoxedMetadataSampleEntry(&mebx);
  if(err) goto bail;
  mebx->dataReferenceIndex = dataReferenceIndex;

  err = MP4CreateMetadataKeyTableBox(&keytable);
  if(err) goto bail;

  err = mebx->addAtom(mebx, (MP4AtomPtr)keytable);
  if(err) goto bail;

  *outSE = mebx;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOAddMebxMetadataToSampleEntry(MP4BoxedMetadataSampleEntryPtr mebx, u32 desired_local_key_id,
                                u32 *out_local_key_id, u32 key_namespace, MP4Handle key_value,
                                char *locale_string, MP4Handle setupInfo)
{
  MP4Err err;
  u32 valueSize, n;
  MP4MetadataKeyTableBoxPtr keytable;
  MP4MetadataKeyBoxPtr keyb;
  MP4MetadataKeyDeclarationBoxPtr keyd;

  if(mebx == NULL) BAILWITHERROR(MP4BadParamErr);
  if(mebx->keyTable == NULL) BAILWITHERROR(MP4BadDataErr);

  keytable = mebx->keyTable;

  /* search for an unused key */
  keyb = NULL;
  for(n = desired_local_key_id; n < 0xFFFFFFFF; n++)
  {
    keyb = keytable->getMetadataKeyBox(keytable, n);
    if(keyb == NULL)
    {
      err = MP4CreateMetadataKeyBox(&keyb, n);
      if(err) goto bail;
      *out_local_key_id = n;
      break;
    }
  }
  if(keyb == NULL) BAILWITHERROR(MP4BadDataErr);

  /* in case key_value is not set, set it based on the namespace and key */
  if(key_value == NULL)
  {
    if(key_namespace != MP4KeyNamespace_me4c && key_namespace != MP4KeyNamespace_uiso)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    else
    {
      err = MP4NewHandle(4, &key_value);
      if(err) return err;
      (*key_value)[0] = (desired_local_key_id >> 24) & 0xFF;
      (*key_value)[1] = (desired_local_key_id >> 16) & 0xFF;
      (*key_value)[2] = (desired_local_key_id >> 8) & 0xFF;
      (*key_value)[3] = desired_local_key_id & 0xFF;
    }
  }
  else
  {
    err = MP4GetHandleSize(key_value, &valueSize);
    if(err) BAILWITHERROR(MP4BadParamErr);
    if(key_namespace == MP4KeyNamespace_me4c || key_namespace == MP4KeyNamespace_uiso)
    {
      if(desired_local_key_id != (u32)MP4_FOUR_CHAR_CODE((*key_value)[0], (*key_value)[1],
                                                         (*key_value)[2], (*key_value)[3]) ||
         valueSize != 4)
      {
        BAILWITHERROR(MP4BadParamErr);
      }
    }
  }

  /* keyd - MetadataKeyDeclarationBox */
  err = MP4CreateMetadataKeyDeclarationBox(&keyd, key_namespace, key_value);
  if(err) goto bail;
  err = keyb->addAtom(keyb, (MP4AtomPtr)keyd);
  if(err) goto bail;

  /* loca - MetadataLocaleBox */
  if(locale_string != NULL)
  {
    MP4MetadataLocaleBoxPtr loca;
    err = MP4CreateMetadataLocaleBox(&loca, locale_string);
    if(err) goto bail;
    err = keyb->addAtom(keyb, (MP4AtomPtr)loca);
    if(err) goto bail;
  }

  /* setu - MetadataSetupBox */
  if(setupInfo != NULL)
  {
    MP4MetadataSetupBoxPtr setu;
    if(key_namespace == MP4KeyNamespace_uiso) BAILWITHERROR(MP4BadParamErr); /* setu not allowed */
    err = MP4CreateMetadataSetupBox(&setu, setupInfo);
    if(err) goto bail;
    err = keyb->addAtom(keyb, (MP4AtomPtr)setu);
    if(err) goto bail;
  }

  keytable->addMetaDataKeyBox(keytable, (MP4AtomPtr)keyb);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetMebxHandle(MP4BoxedMetadataSampleEntryPtr mebxSE, MP4Handle sampleDescriptionH)
{
  MP4Err err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)mebxSE);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetMebxMetadataCount(MP4Handle sampleEntryH, u32 *key_cnt)
{
  MP4Err err;
  MP4BoxedMetadataSampleEntryPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, 0);
  if(err) goto bail;

  if(entry->keyTable == NULL) BAILWITHERROR(MP4BadDataErr);

  err = MP4GetListEntryCount(entry->keyTable->metadataKeyBoxList, key_cnt);

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetMebxMetadataConfig(MP4Handle sampleEntryH, u32 cnt, u32 *local_key_id, u32 *key_namespace,
                         MP4Handle key_value, char **locale_string, MP4Handle setupInfo)
{
  MP4Err err;
  MP4MetadataKeyBoxPtr key;
  u32 size;
  MP4BoxedMetadataSampleEntryPtr entry = NULL;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, 0);
  if(err) goto bail;

  if(entry->keyTable == NULL) BAILWITHERROR(MP4BadDataErr);

  err = MP4GetListEntry(entry->keyTable->metadataKeyBoxList, cnt, (char **)&key);
  if(err) goto bail;

  /* set output values */
  *local_key_id  = key->type;
  *key_namespace = key->keyDeclarationBox->key_namespace;
  if(key->keyDeclarationBox->key_value != NULL && key_value != NULL)
  {
    err = MP4GetHandleSize(key->keyDeclarationBox->key_value, &size);
    if(err) goto bail;
    err = MP4SetHandleSize(key_value, size);
    if(err) goto bail;
    memcpy(*key_value, *(key->keyDeclarationBox->key_value), size);
  }
  if(key->localeBox != NULL && locale_string != NULL)
  {
    *locale_string = key->localeBox->locale_string;
  }
  if(key->setupBox != NULL && setupInfo != NULL)
  {
    err = MP4GetHandleSize(key->setupBox->setup_data, &size);
    if(err) goto bail;
    err = MP4SetHandleSize(setupInfo, size);
    if(err) goto bail;
    memcpy(*setupInfo, *(key->setupBox->setup_data), size);
  }

bail:
  TEST_RETURN(err);
  return err;
}

MP4_EXTERN(MP4Err)
ISONewVVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH, u32 dataReferenceIndex,
                           u32 length_size, MP4Handle first_sps, MP4Handle first_pps)
{
  MP4Err MP4CreateVisualSampleEntryAtom(MP4VisualSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateVVCConfigAtom(ISOVVCConfigAtomPtr * outAtom);

  MP4Err err = MP4NoErr;
  GenericSampleEntryAtomPtr entry;
  ISOVVCConfigAtomPtr config;
  MP4TrackAtomPtr trak;
  BitBuffer mybb;
  BitBuffer *bb;
  u32 the_size, ue, ui, y, uvBits, tmpWidthVal, tmpHeightVal, width, height;
  s32 i;
  u8 x;
  u8 gciBuffer[10];

  u32 CtbSize, sps_num_subpics_minus1, sps_independent_subpics_flag, sps_subpic_same_size_flag,
    sps_subpic_id_len_minus1, gci_present_flag, gci_num_reserved_bits;

  if((theTrack == NULL) || (sampleDescriptionH == NULL)) BAILWITHERROR(MP4BadParamErr);
  if(length_size != 1 && length_size != 2 && length_size != 4) BAILWITHERROR(MP4BadParamErr);
  if(!first_sps) BAILWITHERROR(MP4BadParamErr);

  trak = (MP4TrackAtomPtr)theTrack;
  if(!(trak->newTrackFlags & MP4NewTrackIsVisual)) BAILWITHERROR(MP4BadParamErr);
  err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&entry);
  if(err) goto bail;
  entry->super              = NULL;
  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = ISOVVCSampleEntryAtomTypeOutOfBand;

  /* list vvcC to vvc1 */
  err = MP4CreateVVCConfigAtom(&config);
  err = MP4AddListEntry((void *)config, entry->ExtensionAtomList);
  if(err) goto bail;

  config->LengthSizeMinusOne = length_size - 1;

  err = MP4GetHandleSize(first_sps, &the_size);
  if(err) goto bail;

  bb  = &mybb;
  err = BitBuffer_Init(bb, (u8 *)*first_sps, 8 * the_size);
  if(err) goto bail;
  bb->prevent_emulation = 1;

  /* Get first two bytes for nal_unit_type */
  err = GetBytes(bb, 1, &x);
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  /* SPS == 15 */
  if((x >> 3) != 15) BAILWITHERROR(MP4BadParamErr);

  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  /* sps_max_sublayers_minus1 */
  config->num_sublayers     = ((x & 0xff) >> 5) + 1;
  config->chroma_format_idc = (x & 0x1f) >> 3;
  CtbSize                   = 1 << (((x & 0x06) >> 1) + 5);
  config->ptl_present_flag  = x & 0x01;

  if(config->ptl_present_flag)
  {
    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    config->native_ptl.general_profile_idc = (x & 0xff) >> 1;
    config->native_ptl.general_tier_flag   = x & 0x01;

    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    config->native_ptl.general_level_idc = x;

    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
    config->native_ptl.ptl_frame_only_constraint_flag = x;

    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
    config->native_ptl.ptl_multi_layer_enabled_flag = x;

    /* general_constraints_info */
    {
      /* gci_present_flag */
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      gci_present_flag = x;
      if(!gci_present_flag)
      {
        config->native_ptl.general_constraint_info_upper = 0;
        config->native_ptl.num_bytes_constraint_info     = 1;
      }
      else
      {
        x = (u8)GetBits(bb, 5, &err);
        if(err) goto bail;
        config->native_ptl.general_constraint_info_upper = x | (gci_present_flag << 5);
        config->native_ptl.num_bytes_constraint_info     = 1;
        for(ui = 0; ui < 9; ui++)
        {
          err = GetBytes(bb, 1, &x);
          if(err) goto bail;
          gciBuffer[ui] = x;
          config->native_ptl.num_bytes_constraint_info += 1;
        }
        gci_num_reserved_bits = (x & 0x1f) << 3;
        x                     = (u8)GetBits(bb, 3, &err);
        gci_num_reserved_bits |= x;
        gciBuffer[config->native_ptl.num_bytes_constraint_info - 1] = (u8)x << 5;
        config->native_ptl.num_bytes_constraint_info += ((gci_num_reserved_bits + 3) / 8);

        assert(config->native_ptl.num_bytes_constraint_info == 11);

        err = MP4NewHandle(config->native_ptl.num_bytes_constraint_info,
                           &config->native_ptl.general_constraint_info_lower);
        /* todo fix bug */
        memcpy((*config->native_ptl.general_constraint_info_lower), gciBuffer,
               config->native_ptl.num_bytes_constraint_info);
      }
    }
    /* byte_alligned */
    while(bb->curbits % 8 != 0)
    {
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
    }
    assert(bb->curbits % 8 == 0);

    for(i = config->num_sublayers - 2; i >= 0; i--)
    {
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      config->native_ptl.subPTL[i].ptl_sublayer_level_present_flag = x;
    }
    for(ui = config->num_sublayers; ui <= 8 && config->num_sublayers > 1; ui++)
    {
      /* ptl_reserved_zero_bit, for byte aligned */
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
    }
    assert(bb->curbits % 8 == 0);

    for(i = config->num_sublayers - 2; i >= 0; i--)
    {
      if(config->native_ptl.subPTL[i].ptl_sublayer_level_present_flag)
      {
        err = GetBytes(bb, 1, &x);
        if(err) goto bail;
        config->native_ptl.subPTL[i].sublayer_level_idc = x;
      }
    }

    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    config->native_ptl.ptl_num_sub_profiles = x;

    for(ui = 0; ui < config->native_ptl.ptl_num_sub_profiles; ui++)
    {
      y = (u32)GetBits(bb, 32, &err);
      if(err) goto bail;
      config->native_ptl.general_sub_profile_idc[ui] = y;
    }
  }

  /* sps_gdr_enabled_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  /* sps_ref_pic_resampling_enabled_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  if(x)
  {
    /* sps_res_change_in_clvs_allowed_flag */
    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
  }

  /* sps_pic_width_max_in_luma_samples */
  width = read_golomb_uev(bb, &err);
  if(err) goto bail;
  config->max_picture_width                   = width;
  ((MP4VisualSampleEntryAtomPtr)entry)->width = width;

  /* sps_pic_height_max_in_luma_samples */
  height = read_golomb_uev(bb, &err);
  if(err) goto bail;
  config->max_picture_height                   = height;
  ((MP4VisualSampleEntryAtomPtr)entry)->height = height;
  ((MP4TrackAtomPtr)theTrack)->setDimensions((MP4TrackAtomPtr)theTrack, width, height);

  /* sps_conformance_window_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  if(x)
  {
    for(ui = 0; ui < 4; ui++)
    {
      /* win_offset */
      ue = read_golomb_uev(bb, &err);
      if(err) goto bail;
    }
  }

  /* sps_subpic_info_present_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  if(x)
  {
    sps_num_subpics_minus1       = 0;
    sps_independent_subpics_flag = 0;
    sps_subpic_same_size_flag    = 0;

    /* sps_num_subpics_minus1 */
    ue = read_golomb_uev(bb, &err);
    if(err) goto bail;
    sps_num_subpics_minus1 = ue;
    if(sps_num_subpics_minus1 > 0)
    {
      /* sps_independent_subpics_flag & sps_subpic_same_size_flag */
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      sps_independent_subpics_flag = x;
      x                            = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      sps_subpic_same_size_flag = x;
    }

    for(ui = 0; sps_num_subpics_minus1 > 0 && ui <= sps_num_subpics_minus1; ui++)
    {
      if(!sps_subpic_same_size_flag || ui == 0)
      {
        if(ui > 0 && config->max_picture_width > CtbSize)
        {
          tmpWidthVal = (config->max_picture_width + CtbSize - 1) / CtbSize;
          uvBits      = 0;
          while(tmpWidthVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
        if(ui > 0 && config->max_picture_height > CtbSize)
        {
          tmpHeightVal = (config->max_picture_height + CtbSize - 1) / CtbSize;
          uvBits       = 0;
          while(tmpHeightVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
        if(ui < sps_num_subpics_minus1 && config->max_picture_width > CtbSize)
        {
          tmpWidthVal = (config->max_picture_width + CtbSize - 1) / CtbSize;
          uvBits      = 0;
          while(tmpWidthVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
        if(ui < sps_num_subpics_minus1 && config->max_picture_height > CtbSize)
        {
          tmpHeightVal = (config->max_picture_height + CtbSize - 1) / CtbSize;
          uvBits       = 0;
          while(tmpHeightVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
      }
      if(!sps_independent_subpics_flag)
      {
        x = (u8)GetBits(bb, 2, &err);
        if(err) goto bail;
      }
    }
    /* sps_subpic_id_len_minus1 */
    ue = read_golomb_uev(bb, &err);
    if(err) goto bail;
    sps_subpic_id_len_minus1 = ue;
    /* sps_subpic_id_mapping_explicitly_signalled_flag */
    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
    if(x)
    {
      /* sps_subpic_id_mapping_present_flag */
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      if(x)
      {
        for(ui = 0; ui <= sps_num_subpics_minus1; ui++)
        {
          /* sps_subpic_id */
          y = GetBits(bb, sps_subpic_id_len_minus1 + 1, &err);
          if(err) goto bail;
        }
      }
    }
  }

  /* sps_bitdepth_minus8 */
  ue = read_golomb_uev(bb, &err);
  if(err) goto bail;
  config->bit_depth_minus8 = ue;

  /* add sps */
  err = config->addParameterSet(config, first_sps, 15);
  if(err) goto bail;

  /* add pps */
  if(first_pps)
  {
    err = MP4GetHandleSize(first_pps, &the_size);
    if(err) goto bail;
    err = BitBuffer_Init(bb, (u8 *)*first_pps, 8 * the_size);
    if(err) goto bail;
    bb->prevent_emulation = 1;
    /* Get first two bytes for nal_unit_type */
    err = GetBytes(bb, 1, &x);
    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    /* PPS == 16 */
    if((x >> 3) != 16) BAILWITHERROR(MP4BadParamErr);
    err = config->addParameterSet(config, first_pps, 16);
    if(err) goto bail;
  }

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

MP4_EXTERN(MP4Err)
ISONewVVCSubpicSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                 u32 dataReferenceIndex, u32 width, u32 height, u32 length_size)
{
  MP4Err MP4CreateVisualSampleEntryAtom(MP4VisualSampleEntryAtomPtr * outAtom);
  MP4Err MP4CreateVVCNALUConfigAtom(ISOVVCNALUConfigAtomPtr * outAtom);

  MP4Err err = MP4NoErr;
  GenericSampleEntryAtomPtr entry;
  ISOVVCNALUConfigAtomPtr NALUConfig;
  MP4TrackAtomPtr trak;

  if((theTrack == NULL) || (sampleDescriptionH == NULL)) BAILWITHERROR(MP4BadParamErr);
  if(length_size != 1 && length_size != 2 && length_size != 4) BAILWITHERROR(MP4BadParamErr);

  trak = (MP4TrackAtomPtr)theTrack;
  if(!(trak->newTrackFlags & MP4NewTrackIsVisual)) BAILWITHERROR(MP4BadParamErr);
  err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&entry);
  if(err) goto bail;

  ((MP4VisualSampleEntryAtomPtr)entry)->width  = width;
  ((MP4VisualSampleEntryAtomPtr)entry)->height = height;
  ((MP4TrackAtomPtr)theTrack)->setDimensions((MP4TrackAtomPtr)theTrack, width, height);
  entry->super              = NULL;
  entry->dataReferenceIndex = dataReferenceIndex;
  entry->type               = ISOVVCSubpicSampleEntryAtomType;

  err                            = MP4CreateVVCNALUConfigAtom(&NALUConfig);
  NALUConfig->LengthSizeMinusOne = length_size - 1;

  /* list vvnC to vvs1 */
  err = MP4AddListEntry((void *)NALUConfig, entry->ExtensionAtomList);
  if(err) goto bail;

  err = atomPtrToSampleEntryH(sampleDescriptionH, (MP4AtomPtr)entry);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetVVCSampleDescription(MP4Handle sampleEntryH, u32 *dataReferenceIndex, u32 *length_size,
                           u32 naluType, u32 *count)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOVVCConfigAtomPtr config;
  u32 i;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOVVCSampleEntryAtomTypeOutOfBand &&
     entry->type != ISOVVCSampleEntryAtomTypeInBand)
    BAILWITHERROR(MP4BadParamErr);
  err = MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVVCConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  *dataReferenceIndex = entry->dataReferenceIndex;
  *length_size        = config->LengthSizeMinusOne + 1;

  for(i = 0; i < 7; i++)
  {
    if(config->arrays[i].NAL_unit_type == naluType)
    {
      err = MP4GetListEntryCount(config->arrays[i].nalList, count);
      if(err) goto bail;
      break;
    }
    if(i == 6)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
  }

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}

MP4_EXTERN(MP4Err)
ISOGetVVCSubpicSampleDescription(MP4Handle sampleEntryH, u32 *dataReferenceIndex, u32 *length_size)
{
  MP4Err err                        = MP4NoErr;
  MP4VisualSampleEntryAtomPtr entry = NULL;
  ISOVVCNALUConfigAtomPtr config;

  err = sampleEntryHToAtomPtr(sampleEntryH, (MP4AtomPtr *)&entry, MP4VisualSampleEntryAtomType);
  if(err) goto bail;

  if(entry->type != ISOVVCSubpicSampleEntryAtomType) BAILWITHERROR(MP4BadParamErr);
  err =
    MP4GetListEntryAtom(entry->ExtensionAtomList, ISOVVCNALUConfigAtomType, (MP4AtomPtr *)&config);
  if(err == MP4NotFoundErr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  *dataReferenceIndex = entry->dataReferenceIndex;
  *length_size        = config->LengthSizeMinusOne + 1;

bail:
  if(entry) entry->destroy((MP4AtomPtr)entry);
  return err;
}
