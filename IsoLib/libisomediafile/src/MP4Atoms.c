/**
 * @file MP4Atoms.c
 * @brief Implementation of MP4 Atoms (Boxes) creation/manipulation
 * @version 0.1
 * @date 2020-12-22
 *
 * @copyright This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4
 * only for evaluation and testing purposes. Those intending to use this software module in hardware
 * or software products are advised that its use may infringe existing patents. The original
 * developer of this software module and his/her company, the subsequent editors and their
 * companies, and ISO/IEC have no liability for use of this software module or modifications thereof
 * in an implementation.
 *
 * Copyright is not released for non MPEG-4 conforming products. Apple Computer, Inc. retains full
 * right to use the code for its own purpose, assign or donate the code to a third party and to
 * inhibit third parties from using the code for non MPEG-4 conforming products. This copyright
 * notice must be included in all copies or derivative works.
 *
 * Copyright 2017 Gesellschaft zur Foerderung der angewandten Forschung e.V. acting on behalf of its
 * Fraunhofer Institute for Telecommunications, Heinrich Hertz Institute, HHI All rights reserved.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#ifndef INCLUDED_MJ2ATOMS_H
#include "MJ2Atoms.h"
#endif

MP4Err MP4CreateHEVCConfigAtom(ISOHEVCConfigAtomPtr *outAtom);

static MP4AtomPtr MP4BaseAtomClassPtr = 0;

static char *baseAtomGetName(MP4AtomPtr self) { return self->name; }

static void baseAtomDestroy(MP4AtomPtr self)
{
  free(self);
  /* if ( MP4BaseAtomClassPtr != NULL ) {
      free (MP4BaseAtomClassPtr);
      MP4BaseAtomClassPtr = NULL;
  } */
  /* There is no good time to free these static optimizations; the old code freed them and
     didn't zero them out, which meant they kept getting used. If you zero them out, they get
     remade, but there still exist pointers to the now-freed protos. See below for the same for
     fullatom. */
}

static MP4Err baseAtomCreateFromInputStream(MP4AtomPtr self, MP4AtomPtr proto,
                                            MP4InputStreamPtr inputStream)
{
  (void)inputStream;
  if(proto == NULL) return MP4BadParamErr;
  self->type = proto->type;
  memcpy(self->uuid, proto->uuid, 16);
  self->size         = proto->size;
  self->size64       = proto->size64;
  self->bytesRead    = proto->bytesRead;
  self->streamOffset = proto->streamOffset;
  return MP4NoErr;
}

MP4Err MP4CreateBaseAtom(MP4AtomPtr self)
{
  MP4Err err;
  err = MP4NoErr;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  if(MP4BaseAtomClassPtr == NULL)
  {
    MP4BaseAtomClassPtr = (MP4AtomPtr)calloc(1, sizeof(MP4Atom));
    TESTMALLOC(MP4BaseAtomClassPtr);

    MP4BaseAtomClassPtr->name                  = "base";
    MP4BaseAtomClassPtr->createFromInputStream = (cisfunc)baseAtomCreateFromInputStream;
    MP4BaseAtomClassPtr->getName               = baseAtomGetName;
    MP4BaseAtomClassPtr->destroy               = baseAtomDestroy;
    MP4BaseAtomClassPtr->super                 = MP4BaseAtomClassPtr;
  }
  *self = *MP4BaseAtomClassPtr;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4FullAtomPtr MP4FullAtomClassPtr = 0;

static void fullAtomDestroy(MP4AtomPtr self)
{
  baseAtomDestroy(self);
  /* if ( MP4FullAtomClassPtr  != NULL) {
      free (MP4FullAtomClassPtr);
      MP4FullAtomClassPtr = NULL;
  } */
  /* see above for discussion of these protos and when, if ever, they can be freed */
}

static MP4Err fullAtomCreateFromInputStream(MP4AtomPtr s, MP4AtomPtr proto,
                                            MP4InputStreamPtr inputStream)
{
  MP4FullAtomPtr self;
  MP4Err err;
  u32 val;
  self = (MP4FullAtomPtr)s;

  err = baseAtomCreateFromInputStream(s, proto, inputStream);
  if(err) goto bail;
  err = inputStream->read32(inputStream, &val, NULL);
  if(err) goto bail;
  DEBUG_SPRINTF("atom version = %d", (val >> 24));
  DEBUG_SPRINTF("atom flags = 0x%06x", (val & 0xFFFFFF));
  self->bytesRead += 4;
  self->version = val >> 24;
  self->flags   = val & 0xffffff;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateFullAtom(MP4AtomPtr s)
{
  MP4FullAtomPtr self;
  MP4Err err;
  self = (MP4FullAtomPtr)s;

  err = MP4NoErr;

  if(MP4FullAtomClassPtr == NULL)
  {
    MP4FullAtomClassPtr = (MP4FullAtomPtr)calloc(1, sizeof(MP4FullAtom));
    TESTMALLOC(MP4FullAtomClassPtr);
    err = MP4CreateBaseAtom((MP4AtomPtr)MP4FullAtomClassPtr);
    if(err) goto bail;
    MP4FullAtomClassPtr->createFromInputStream = (cisfunc)fullAtomCreateFromInputStream;
    MP4FullAtomClassPtr->destroy               = fullAtomDestroy;
    MP4FullAtomClassPtr->super                 = (MP4AtomPtr)MP4FullAtomClassPtr;
    MP4FullAtomClassPtr->name                  = "full";
    MP4FullAtomClassPtr->version               = 0;
    MP4FullAtomClassPtr->flags                 = 0;
  }
  *self = *MP4FullAtomClassPtr;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateAtom(u32 atomType, MP4AtomPtr *outAtom)
{
  MP4Err err;
  MP4AtomPtr newAtom;

  err = MP4NoErr;

  switch(atomType)
  {
  case MP4HintTrackReferenceAtomType:
  case MP4StreamDependenceAtomType:
  case MP4ODTrackReferenceAtomType:
  case MP4SyncTrackReferenceAtomType:
    err = MP4CreateTrackReferenceTypeAtom(atomType, (MP4TrackReferenceTypeAtomPtr *)&newAtom);
    break;

  case MP4FreeSpaceAtomType:
  case MP4SkipAtomType:
    err = MP4CreateFreeSpaceAtom((MP4FreeSpaceAtomPtr *)&newAtom);
    break;

  case MP4MediaDataAtomType:
    err = MP4CreateMediaDataAtom((MP4MediaDataAtomPtr *)&newAtom);
    break;

  case MP4MovieAtomType:
    err = MP4CreateMovieAtom((MP4MovieAtomPtr *)&newAtom);
    break;

  case MP4MovieHeaderAtomType:
    err = MP4CreateMovieHeaderAtom((MP4MovieHeaderAtomPtr *)&newAtom);
    break;

  case MP4MediaHeaderAtomType:
    err = MP4CreateMediaHeaderAtom((MP4MediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4VideoMediaHeaderAtomType:
    err = MP4CreateVideoMediaHeaderAtom((MP4VideoMediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4SoundMediaHeaderAtomType:
    err = MP4CreateSoundMediaHeaderAtom((MP4SoundMediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4MPEGMediaHeaderAtomType:
    err = MP4CreateMPEGMediaHeaderAtom((MP4MPEGMediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4ObjectDescriptorMediaHeaderAtomType:
    err =
      MP4CreateObjectDescriptorMediaHeaderAtom((MP4ObjectDescriptorMediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4ClockReferenceMediaHeaderAtomType:
    err = MP4CreateClockReferenceMediaHeaderAtom((MP4ClockReferenceMediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4SceneDescriptionMediaHeaderAtomType:
    err =
      MP4CreateSceneDescriptionMediaHeaderAtom((MP4SceneDescriptionMediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4HintMediaHeaderAtomType:
    err = MP4CreateHintMediaHeaderAtom((MP4HintMediaHeaderAtomPtr *)&newAtom);
    break;

  case MP4SampleTableAtomType:
    err = MP4CreateSampleTableAtom((MP4SampleTableAtomPtr *)&newAtom);
    break;

  case MP4DataInformationAtomType:
    err = MP4CreateDataInformationAtom((MP4DataInformationAtomPtr *)&newAtom);
    break;

  case MP4_FOUR_CHAR_CODE('a', 'l', 'i', 's'):
  case MP4DataEntryURLAtomType:
    err = MP4CreateDataEntryURLAtom((MP4DataEntryURLAtomPtr *)&newAtom);
    break;

  case MP4CopyrightAtomType:
    err = MP4CreateCopyrightAtom((MP4CopyrightAtomPtr *)&newAtom);
    break;

  case MP4DataEntryURNAtomType:
    err = MP4CreateDataEntryURNAtom((MP4DataEntryURNAtomPtr *)&newAtom);
    break;

  case MP4HandlerAtomType:
    err = MP4CreateHandlerAtom((MP4HandlerAtomPtr *)&newAtom);
    break;

  case MP4ObjectDescriptorAtomType:
    err = MP4CreateObjectDescriptorAtom((MP4ObjectDescriptorAtomPtr *)&newAtom);
    break;

  case MP4TrackAtomType:
    err = MP4CreateTrackAtom((MP4TrackAtomPtr *)&newAtom);
    break;

  case MP4MPEGSampleEntryAtomType:
    err = MP4CreateMPEGSampleEntryAtom((MP4MPEGSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4VisualSampleEntryAtomType:
  case ISOAVCSampleEntryAtomType:
  case ISOHEVCSampleEntryAtomType:
  case MP4H263SampleEntryAtomType:
  case ISOVVCSampleEntryAtomTypeOutOfBand:
  case ISOVVCSampleEntryAtomTypeInBand:
  case ISOVVCSubpicSampleEntryAtomType:
    err = MP4CreateVisualSampleEntryAtom((MP4VisualSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4AudioSampleEntryAtomType:
  case MP4AMRSampleEntryAtomType:
  case MP4AWBSampleEntryAtomType:
  case MP4AMRWPSampleEntryAtomType:
    err = MP4CreateAudioSampleEntryAtom((MP4AudioSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4ChannelLayoutAtomType:
    err = MP4CreateChannelLayoutAtom((MP4ChannelLayoutAtomPtr *)&newAtom);
    break;

  case MP4PCMConfigAtomType:
    err = MP4CreatePCMConfigAtom((MP4PCMConfigAtomPtr *)&newAtom);
    break;

  case MP4DownMixInstructionsAtomType:
    err = MP4CreateDownMixInstructionsAtom((MP4DownMixInstructionsAtomPtr *)&newAtom);
    break;

  case MP4TrackLoudnessInfoAtomType:
  case MP4AlbumLoudnessInfoAtomType:
    err = MP4CreateLoudnessBaseAtom((MP4LoudnessBaseAtomPtr *)&newAtom, atomType);
    break;

  case MP4LoudnessAtomType:
    err = MP4CreateLoudnessAtom((MP4LoudnessAtomPtr *)&newAtom);
    break;

  case MP4EncVisualSampleEntryAtomType:
    err = MP4CreateEncVisualSampleEntryAtom((MP4EncVisualSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4EncAudioSampleEntryAtomType:
    err = MP4CreateEncAudioSampleEntryAtom((MP4EncAudioSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4RestrictedVideoSampleEntryAtomType:
    err = MP4CreateRestrictedVideoSampleEntryAtom((MP4RestrictedVideoSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4XMLMetaSampleEntryAtomType:
    err = MP4CreateXMLMetaSampleEntryAtom((MP4XMLMetaSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4TextMetaSampleEntryAtomType:
    err = MP4CreateTextMetaSampleEntryAtom((MP4TextMetaSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4GenericSampleEntryAtomType:
    err = MP4CreateGenericSampleEntryAtom((MP4GenericSampleEntryAtomPtr *)&newAtom);
    break;

  case MP4EditAtomType:
    err = MP4CreateEditAtom((MP4EditAtomPtr *)&newAtom);
    break;

  case MP4UserDataAtomType:
    err = MP4CreateUserDataAtom((MP4UserDataAtomPtr *)&newAtom);
    break;

  case MP4DataReferenceAtomType:
    err = MP4CreateDataReferenceAtom((MP4DataReferenceAtomPtr *)&newAtom);
    break;

  case MP4SampleDescriptionAtomType:
    err = MP4CreateSampleDescriptionAtom((MP4SampleDescriptionAtomPtr *)&newAtom);
    break;

  case MP4TimeToSampleAtomType:
    err = MP4CreateTimeToSampleAtom((MP4TimeToSampleAtomPtr *)&newAtom);
    break;

  case MP4CompositionOffsetAtomType:
    err = MP4CreateCompositionOffsetAtom((MP4CompositionOffsetAtomPtr *)&newAtom);
    break;

  case MP4CompositionToDecodeAtomType:
    err = MP4CreateCompositionToDecodeAtom((MP4CompositionToDecodeAtomPtr *)&newAtom);
    break;

  case MP4SampleAuxiliaryInformationSizesAtomType:
    err = MP4CreateSampleAuxiliaryInformationSizesAtom(
      (MP4SampleAuxiliaryInformationSizesAtomPtr *)&newAtom);
    break;

  case MP4SampleAuxiliaryInformationOffsetsAtomType:
    err = MP4CreateSampleAuxiliaryInformationOffsetsAtom(
      (MP4SampleAuxiliaryInformationOffsetsAtomPtr *)&newAtom);
    break;

  case MP4ShadowSyncAtomType:
    err = MP4CreateShadowSyncAtom((MP4ShadowSyncAtomPtr *)&newAtom);
    break;

  case MP4EditListAtomType:
    err = MP4CreateEditListAtom((MP4EditListAtomPtr *)&newAtom);
    break;

  case MP4SampleToChunkAtomType:
    err = MP4CreateSampleToChunkAtom((MP4SampleToChunkAtomPtr *)&newAtom);
    break;

  case MP4SampleSizeAtomType:
  case MP4CompactSampleSizeAtomType:
    err = MP4CreateSampleSizeAtom((MP4SampleSizeAtomPtr *)&newAtom);
    break;

  case MP4ChunkLargeOffsetAtomType:
  case MP4ChunkOffsetAtomType:
    err = MP4CreateChunkOffsetAtom((MP4ChunkOffsetAtomPtr *)&newAtom);
    break;

  case MP4SyncSampleAtomType:
    err = MP4CreateSyncSampleAtom((MP4SyncSampleAtomPtr *)&newAtom);
    break;

  case MP4DegradationPriorityAtomType:
    err = MP4CreateDegradationPriorityAtom((MP4DegradationPriorityAtomPtr *)&newAtom);
    break;

  case MP4SampleDependencyAtomType:
    err = MP4CreateSampleDependencyAtom((MP4SampleDependencyAtomPtr *)&newAtom);
    break;

    /*		case MP4ChunkLargeOffsetAtomType:
                            err = MP4CreateChunkLargeOffsetAtom( (MP4ChunkLargeOffsetAtomPtr*)
       &newAtom ); break;
    */
  case MP4ESDAtomType:
    err = MP4CreateESDAtom((MP4ESDAtomPtr *)&newAtom);
    break;

  case ISOVCConfigAtomType:
    err = MP4CreateVCConfigAtom((ISOVCConfigAtomPtr *)&newAtom);
    break;

  case MP4MediaInformationAtomType:
    err = MP4CreateMediaInformationAtom((MP4MediaInformationAtomPtr *)&newAtom);
    break;

  case MP4TrackHeaderAtomType:
    err = MP4CreateTrackHeaderAtom((MP4TrackHeaderAtomPtr *)&newAtom);
    break;

  case MP4TrackReferenceAtomType:
    err = MP4CreateTrackReferenceAtom((MP4TrackReferenceAtomPtr *)&newAtom);
    break;

  case MP4MediaAtomType:
    err = MP4CreateMediaAtom((MP4MediaAtomPtr *)&newAtom);
    break;

  case MP4ExtendedLanguageTagAtomType:
    err = MP4CreateExtendedLanguageTagAtom((MP4ExtendedLanguageTagAtomPtr *)&newAtom);
    break;

  case MP4PaddingBitsAtomType:
    err = MP4CreatePaddingBitsAtom((MP4PaddingBitsAtomPtr *)&newAtom);
    break;

  /* JPEG-2000 atom ("box") types */
  case MJ2JPEG2000SignatureAtomType:
    err = MJ2CreateSignatureAtom((MJ2JPEG2000SignatureAtomPtr *)&newAtom);
    break;

  case ISOFileTypeAtomType:
    err = MJ2CreateFileTypeAtom((ISOFileTypeAtomPtr *)&newAtom);
    break;

  case MJ2ImageHeaderAtomType:
    err = MJ2CreateImageHeaderAtom((MJ2ImageHeaderAtomPtr *)&newAtom);
    break;

  case MJ2BitsPerComponentAtomType:
    err = MJ2CreateBitsPerComponentAtom((MJ2BitsPerComponentAtomPtr *)&newAtom);
    break;

  case MJ2ColorSpecificationAtomType:
    err = MJ2CreateColorSpecificationAtom((MJ2ColorSpecificationAtomPtr *)&newAtom);
    break;

  case MJ2JP2HeaderAtomType:
    err = MJ2CreateHeaderAtom((MJ2HeaderAtomPtr *)&newAtom);
    break;

  /* Movie Fragment stuff */
  case MP4MovieExtendsAtomType:
    err = MP4CreateMovieExtendsAtom((MP4MovieExtendsAtomPtr *)&newAtom);
    break;

  case MP4TrackExtensionPropertiesAtomType:
    err = MP4CreateTrackExtensionPropertiesAtom((MP4TrackExtensionPropertiesAtomPtr *)&newAtom);
    break;

  case MP4TrackExtendsAtomType:
    err = MP4CreateTrackExtendsAtom((MP4TrackExtendsAtomPtr *)&newAtom);
    break;

  case MP4MovieFragmentAtomType:
    err = MP4CreateMovieFragmentAtom((MP4MovieFragmentAtomPtr *)&newAtom);
    break;

  case MP4MovieFragmentHeaderAtomType:
    err = MP4CreateMovieFragmentHeaderAtom((MP4MovieFragmentHeaderAtomPtr *)&newAtom);
    break;

  case MP4TrackFragmentAtomType:
    err = MP4CreateTrackFragmentAtom((MP4TrackFragmentAtomPtr *)&newAtom);
    break;

  case MP4TrackFragmentHeaderAtomType:
    err = MP4CreateTrackFragmentHeaderAtom((MP4TrackFragmentHeaderAtomPtr *)&newAtom);
    break;

  case MP4TrackFragmentDecodeTimeAtomType:
    err = MP4CreateTrackFragmentDecodeTimeAtom((MP4TrackFragmentDecodeTimeAtomPtr *)&newAtom);
    break;

  case MP4TrackRunAtomType:
    err = MP4CreateTrackRunAtom((MP4TrackRunAtomPtr *)&newAtom);
    break;

  case MP4SchemeTypeAtomType:
    err = MP4CreateSchemeTypeAtom((MP4SchemeTypeAtomPtr *)&newAtom);
    break;

  case MP4OriginalFormatAtomType:
    err = MP4CreateOriginalFormatAtom((MP4OriginalFormatAtomPtr *)&newAtom);
    break;

  case MP4SchemeInfoAtomType:
    err = MP4CreateSchemeInfoAtom((MP4SchemeInfoAtomPtr *)&newAtom);
    break;

  case MP4CompatibleSchemeTypeAtomType:
    err = MP4CreateCompatibleSchemeTypeAtom((MP4CompatibleSchemeTypeAtomPtr *)&newAtom);
    break;

  case MP4RestrictedSchemeInfoAtomType:
    err = MP4CreateRestrictedSchemeInfoAtom((MP4RestrictedSchemeInfoAtomPtr *)&newAtom);
    break;

  case MP4TrackTypeAtomType:
    err = MP4CreateTrackTypeAtom((MP4TrackTypeAtomPtr *)&newAtom);
    break;

  case MP4StereoVideoAtomType:
    err = MP4CreateStereoVideoAtom((MP4StereoVideoAtomPtr *)&newAtom);
    break;

  case MP4StereoVideoGroupAtomType:
    err = MP4CreateStereoVideoGroupAtom((MP4StereoVideoGroupAtomPtr *)&newAtom);
    break;

  case MP4SegmentTypeAtomType:
    err = MP4CreateSegmentTypeAtom((MP4SegmentTypeAtomPtr *)&newAtom);
    break;
  case MP4SegmentIndexAtomType:
    err = MP4CreateSegmentIndexAtom((MP4SegmentIndexAtomPtr *)&newAtom);
    break;
  case MP4SubsegmentIndexAtomType:
    err = MP4CreateSubsegmentIndexAtom((MP4SubsegmentIndexAtomPtr *)&newAtom);
    break;
  case MP4ProducerReferenceTimeAtomType:
    err = MP4CreateProducerReferenceTimeAtom((MP4ProducerReferenceTimeAtomPtr *)&newAtom);
    break;

#ifdef ISMACrypt
  case MP4SecurityInfoAtomType:
    err = MP4CreateSecurityInfoAtom((MP4SecurityInfoAtomPtr *)&newAtom);
    break;

  case ISMAKMSAtomType:
    err = CreateISMAKMSAtom((ISMAKMSAtomPtr *)&newAtom);
    break;

  case ISMASampleFormatAtomType:
    err = CreateISMASampleFormatAtom((ISMASampleFormatAtomPtr *)&newAtom);
    break;

  case ISMASaltAtomType:
    err = CreateISMASaltAtom((ISMASaltAtomPtr *)&newAtom);
    break;
#endif
  case MP4SampleGroupDescriptionAtomType:
    err = MP4CreateSampleGroupDescriptionAtom((MP4SampleGroupDescriptionAtomPtr *)&newAtom);
    break;

  case MP4SampletoGroupAtomType:
  case MP4CompactSampletoGroupAtomType:
    err = MP4CreateSampletoGroupAtom((MP4SampletoGroupAtomPtr *)&newAtom, SAMPLE_GROUP_NORMAL);
    break;

  case ISOMetaAtomType:
    err = ISOCreateMetaAtom((ISOMetaAtomPtr *)&newAtom);
    break;

  case ISOPrimaryItemAtomType:
    err = ISOCreatePrimaryItemAtom((ISOPrimaryItemAtomPtr *)&newAtom);
    break;

  case ISOItemLocationAtomType:
    err = ISOCreateItemLocationAtom((ISOItemLocationAtomPtr *)&newAtom);
    break;

  case ISOItemProtectionAtomType:
    err = ISOCreateItemProtectionAtom((ISOItemProtectionAtomPtr *)&newAtom);
    break;

  case ISOItemInfoAtomType:
    err = ISOCreateItemInfoAtom((ISOItemInfoAtomPtr *)&newAtom);
    break;

  case ISOItemInfoEntryAtomType:
    err = ISOCreateItemInfoEntryAtom((ISOItemInfoEntryAtomPtr *)&newAtom);
    break;

  case ISOAdditionalMetaDataContainerAtomType: /* Deprecated */
    err =
      ISOCreateAdditionalMetaDataContainerAtom((ISOAdditionalMetaDataContainerAtomPtr *)&newAtom);
    break;

  case ISOMetaboxRelationAtomType: /* Deprecated */
    err = ISOCreateMetaboxRelationAtom((ISOMetaboxRelationAtomPtr *)&newAtom);
    break;

  case ISOItemDataAtomType:
    err = ISOCreateItemDataAtom((ISOItemDataAtomPtr *)&newAtom);
    break;

  case ISOItemReferenceAtomType:
    err = ISOCreateItemReferenceAtom((ISOItemReferenceAtomPtr *)&newAtom);
    break;

  case MP4AMRSpecificInfoAtomType:
    err = MP4CreateAMRSpecificInfoAtom((MP4AMRSpecificInfoAtomPtr *)&newAtom);
    break;

  case MP4AMRWPSpecificInfoAtomType:
    err = MP4CreateAMRWPSpecificInfoAtom((MP4AMRWPSpecificInfoAtomPtr *)&newAtom);
    break;

  case MP4H263SpecificInfoAtomType:
    err = MP4CreateH263SpecificInfoAtom((MP4H263SpecificInfoAtomPtr *)&newAtom);
    break;

  case MP4BitRateAtomType:
  case TGPPBitRateAtomType:
    err = MP4CreateBitRateAtom((MP4BitRateAtomPtr *)&newAtom);
    break;

  case MP4ItemPropertiesAtomType:
    err = MP4CreateItemPropertiesAtom((MP4ItemPropertiesAtomPtr *)&newAtom);
    break;

  case MP4ItemPropertyContainerAtomType:
    err = MP4CreateItemPropertyContainerAtom((MP4ItemPropertyContainerAtomPtr *)&newAtom);
    break;

  case MP4ItemPropertyAssociationAtomType:
    err = MP4CreateItemPropertyAssociationAtom((MP4ItemPropertyAssociationAtomPtr *)&newAtom);
    break;

  case ISOHEVCConfigAtomType:
    err = MP4CreateHEVCConfigAtom((ISOHEVCConfigAtomPtr *)&newAtom);
    break;

  case MP4BoxedMetadataSampleEntryType:
    err = MP4CreateMP4BoxedMetadataSampleEntry((MP4BoxedMetadataSampleEntryPtr *)&newAtom);
    break;

  case MP4MetadataKeyTableBoxType:
    err = MP4CreateMetadataKeyTableBox((MP4MetadataKeyTableBoxPtr *)&newAtom);
    break;

  case MP4MetadataKeyDeclarationBoxType:
    err = MP4CreateMetadataKeyDeclarationBox((MP4MetadataKeyDeclarationBoxPtr *)&newAtom, 0, 0);
    break;

  case MP4MetadataLocaleBoxType:
    err = MP4CreateMetadataLocaleBox((MP4MetadataLocaleBoxPtr *)&newAtom, 0);
    break;

  case MP4MetadataSetupBoxType:
    err = MP4CreateMetadataSetupBox((MP4MetadataSetupBoxPtr *)&newAtom, 0);
    break;

  case MP4MetadataGenericKeyBoxType:
    err = MP4CreateMetadataKeyBox((MP4MetadataKeyBoxPtr *)&newAtom, 0);
    break;

  case MP4GroupsListBoxType:
    err = MP4CreateGroupListBox((GroupListBoxPtr *)&newAtom);
    break;

  case MP4AlternativeEntityGroup:
    err = MP4CreateEntityToGroupBox((EntityToGroupBoxPtr *)&newAtom, 0);
    break;

  case ISOVVCConfigAtomType:
    err = MP4CreateVVCConfigAtom((ISOVVCConfigAtomPtr *)&newAtom);
    break;

  case ISOVVCNALUConfigAtomType:
    err = MP4CreateVVCNALUConfigAtom((ISOVVCNALUConfigAtomPtr *)&newAtom);
    break;

  case MP4TrackGroupAtomType:
    err = MP4CreateTrackGroupAtom((MP4TrackGroupAtomPtr *)&newAtom);
    break;

  case MP4_FOUR_CHAR_CODE('m', 's', 'r', 'c'):
  case MP4_FOUR_CHAR_CODE('a', 'l', 't', 'e'):
    err = MP4CreateTrackGroupTypeAtom(atomType, (MP4TrackGroupTypeAtomPtr *)&newAtom);
    break;

  default:
    err           = MP4CreateUnknownAtom((MP4UnknownAtomPtr *)&newAtom);
    newAtom->type = atomType;
    break;
  }
  if(err == MP4NoErr) *outAtom = newAtom;
  return err;
}

void MP4TypeToString(u32 inType, char *ioStr)
{
  u32 i;
  int ch;

  for(i = 0; i < 4; i++, ioStr++)
  {
    ch = inType >> (8 * (3 - i)) & 0xff;
    if(isprint(ch)) *ioStr = (char)ch;
    else
      *ioStr = '.';
  }
  *ioStr = 0;
}

MP4Err MP4GetListEntryAtom(MP4LinkedList list, u32 atomType, MP4AtomPtr *outItem)
{
  MP4Err err;
  u32 count, i;

  err      = MP4NoErr;
  *outItem = NULL;
  err      = MP4GetListEntryCount(list, &count);
  if(err) goto bail;
  for(i = 0; i < count; i++)
  {
    MP4AtomPtr p;
    err = MP4GetListEntry(list, i, (char **)&p);
    if(err) goto bail;
    if((p != 0) && (p->type == atomType))
    {
      *outItem = p;
      err      = MP4NoErr;
      goto bail;
    }
  }

  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4DeleteListEntryAtom(MP4LinkedList list, u32 atomType)
{
  MP4Err err;
  u32 count, i;

  err = MP4NoErr;
  err = MP4GetListEntryCount(list, &count);
  if(err) goto bail;
  for(i = 0; i < count; i++)
  {
    MP4AtomPtr p;
    err = MP4GetListEntry(list, i, (char **)&p);
    if(err) goto bail;
    if((p != 0) && (p->type == atomType))
    {
      err = MP4DeleteListEntry(list, i);
      goto bail;
    }
  }

  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4ParseAtomUsingProtoList(MP4InputStreamPtr inputStream, u32 *protoList, u32 defaultAtom,
                                  MP4AtomPtr *outAtom)
{
  MP4AtomPtr atomProto;
  MP4Err err;
  long bytesParsed;
  MP4Atom protoAtom;
  MP4AtomPtr newAtom;
  char typeString[8];
  char msgString[80];
  u64 beginAvail;
  u64 consumedBytes;
  u32 useDefaultAtom;

  atomProto   = &protoAtom;
  err         = MP4NoErr;
  bytesParsed = 0L;

  if((inputStream == NULL) || (outAtom == NULL)) BAILWITHERROR(MP4BadParamErr)
  *outAtom       = NULL;
  beginAvail     = inputStream->available;
  useDefaultAtom = 0;
  inputStream->msg(inputStream, "{");
  inputStream->indent++;
  err = MP4CreateBaseAtom(atomProto);
  if(err) goto bail;

  atomProto->streamOffset = inputStream->getStreamOffset(inputStream);

  /* atom size */
  err = inputStream->read32(inputStream, &atomProto->size, NULL);
  if(err) goto bail;
  if(atomProto->size == 0)
  {
    /* BAILWITHERROR( MP4NoQTAtomErr )  */
    u64 the_size;
    the_size = inputStream->available + 4;
    if(the_size >> 32)
    {
      atomProto->size   = 1;
      atomProto->size64 = the_size + 8;
    }
    else
      atomProto->size = (u32)the_size;
  }
  if((atomProto->size != 1) && ((atomProto->size - 4) > inputStream->available))
    BAILWITHERROR(MP4BadDataErr)
  bytesParsed += 4L;

  sprintf(msgString, "atom size is %d", atomProto->size);
  inputStream->msg(inputStream, msgString);

  /* atom type */
  err = inputStream->read32(inputStream, &atomProto->type, NULL);
  if(err) goto bail;
  bytesParsed += 4L;
  MP4TypeToString(atomProto->type, typeString);
  sprintf(msgString, "atom type is '%s' (%u)", typeString, atomProto->type);
  inputStream->msg(inputStream, msgString);
  if(atomProto->type == MP4ExtendedAtomType)
  {
    err = inputStream->readData(inputStream, 16, (char *)atomProto->uuid, NULL);
    if(err) goto bail;
    bytesParsed += 16L;
  }

  /* large atom */
  if(atomProto->size == 1)
  {
    u32 size;
    err = inputStream->read32(inputStream, &size, NULL);
    if(err) goto bail;
    /* if ( size )
            BAILWITHERROR( MP4NoLargeAtomSupportErr ) */
    atomProto->size64 = size;
    atomProto->size64 <<= 32;
    err = inputStream->read32(inputStream, &size, NULL);
    if(err) goto bail;
    atomProto->size64 |= size;
    atomProto->size = 1;
    bytesParsed += 8L;
  }

  atomProto->bytesRead = bytesParsed;
  if((atomProto->size != 1) && (((long)atomProto->size) < bytesParsed)) BAILWITHERROR(MP4BadDataErr)
  if(protoList)
  {
    while(*protoList)
    {
      if(*protoList == atomProto->type) break;
      protoList++;
    }
    if(*protoList == 0)
    {
      useDefaultAtom = 1;
    }
  }
  err = MP4CreateAtom(useDefaultAtom ? defaultAtom : atomProto->type, &newAtom);
  if(err) goto bail;
  sprintf(msgString, "atom name is '%s'", newAtom->name);
  inputStream->msg(inputStream, msgString);
  err = newAtom->createFromInputStream(newAtom, atomProto, (char *)inputStream);
  if(err) goto bail;
  consumedBytes = beginAvail - inputStream->available;
  if((atomProto->size != 1) && (consumedBytes != atomProto->size))
  {
    sprintf(msgString, "##### atom size is %d but parse used %lld bytes ####", atomProto->size,
            consumedBytes);
    inputStream->msg(inputStream, msgString);
    if(consumedBytes < atomProto->size)
    {
      u32 x;
      u32 i;
      for(i = 0; i < (atomProto->size) - consumedBytes; i++)
        inputStream->read8(inputStream, &x, NULL);
    }
  }
  else if((atomProto->size == 1) && (consumedBytes != atomProto->size64))
  {
    sprintf(msgString, "##### atom size is %lld but parse used %lld bytes ####", atomProto->size64,
            consumedBytes);
    inputStream->msg(inputStream, msgString);
    if(consumedBytes < atomProto->size64)
    {
      u32 x;
      u64 i;
      for(i = 0; i < (atomProto->size64) - consumedBytes; i++)
        inputStream->read8(inputStream, &x, NULL);
    }
  }
  *outAtom = newAtom;
  inputStream->indent--;
  inputStream->msg(inputStream, "}");
bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4ParseAtom(MP4InputStreamPtr inputStream, MP4AtomPtr *outAtom)
{
  return MP4ParseAtomUsingProtoList(inputStream, NULL, 0, outAtom);
}

MP4Err MP4ParseAtomFromHandle(MP4Handle inputHandle, MP4AtomPtr *outAtom)
{
  MP4Err err;
  MP4InputStreamPtr is = NULL;
  u32 size             = 0;
  if(inputHandle == NULL) BAILWITHERROR(MP4BadParamErr);

  err = MP4GetHandleSize(inputHandle, &size);
  if(err) goto bail;
  err = MP4CreateMemoryInputStream(*inputHandle, size, &is);
  if(err) goto bail;
  is->debugging = 0;
  err           = MP4ParseAtom(is, outAtom);
  if(err) goto bail;

bail:
  if(is) is->destroy(is);
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CalculateBaseAtomFieldSize(struct MP4Atom *self)
{
  self->size = 8;
  return MP4NoErr;
}

MP4Err MP4CalculateFullAtomFieldSize(struct MP4FullAtom *self)
{
  MP4Err err = MP4CalculateBaseAtomFieldSize((MP4AtomPtr)self);
  self->size += 4;
  return err;
}

MP4Err MP4SerializeCommonBaseAtomFields(struct MP4Atom *self, char *buffer)
{
  MP4Err err;
  u32 kept_size;
  err = MP4NoErr;

  kept_size = self->size;
  if(self->size == 1) self->size = 16;

  self->bytesWritten = 0;
  assert(self->size);
  assert(self->type);
  PUT32_V(kept_size);

  PUT32(type);

  if(kept_size == 1)
  {
    PUT64(size64);
    self->size = 1;
  }
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4SerializeCommonFullAtomFields(struct MP4FullAtom *self, char *buffer)
{
  MP4Err err;
  err = MP4SerializeCommonBaseAtomFields((MP4AtomPtr)self, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUT8(version);
  PUT24(flags);
bail:
  TEST_RETURN(err);

  return err;
}
