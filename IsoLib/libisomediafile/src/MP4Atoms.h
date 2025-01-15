/**
 * @file MP4Atoms.h
 * @brief MP4 Atoms (Boxes) definitions
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

#ifndef INCLUDED_MP4ATOMS_H
#define INCLUDED_MP4ATOMS_H

#include "MP4Movies.h"
#include "MP4Impl.h"
#include "MP4InputStream.h"
#include "MP4LinkedList.h"

ISOErr MP4GetCurrentTime(u64 *outTime);

enum
{
  MP4AudioSampleEntryAtomType                  = MP4_FOUR_CHAR_CODE('m', 'p', '4', 'a'),
  AudioIntegerPCMSampleEntryType               = MP4_FOUR_CHAR_CODE('i', 'p', 'c', 'm'),
  AudioFloatPCMSampleEntryType                 = MP4_FOUR_CHAR_CODE('f', 'p', 'c', 'm'),
  MP4PCMConfigAtomType                         = MP4_FOUR_CHAR_CODE('p', 'c', 'm', 'C'),
  MP4ChannelLayoutAtomType                     = MP4_FOUR_CHAR_CODE('c', 'h', 'n', 'l'),
  MP4DownMixInstructionsAtomType               = MP4_FOUR_CHAR_CODE('d', 'm', 'i', 'x'),
  MP4TrackLoudnessInfoAtomType                 = MP4_FOUR_CHAR_CODE('t', 'l', 'o', 'u'),
  MP4AlbumLoudnessInfoAtomType                 = MP4_FOUR_CHAR_CODE('a', 'l', 'o', 'u'),
  MP4LoudnessAtomType                          = MP4_FOUR_CHAR_CODE('l', 'u', 'd', 't'),
  MP4ChunkLargeOffsetAtomType                  = MP4_FOUR_CHAR_CODE('c', 'o', '6', '4'),
  MP4ChunkOffsetAtomType                       = MP4_FOUR_CHAR_CODE('s', 't', 'c', 'o'),
  MP4ClockReferenceMediaHeaderAtomType         = MP4_FOUR_CHAR_CODE('c', 'r', 'h', 'd'),
  MP4CompositionOffsetAtomType                 = MP4_FOUR_CHAR_CODE('c', 't', 't', 's'),
  MP4CompositionToDecodeAtomType               = MP4_FOUR_CHAR_CODE('c', 's', 'l', 'g'),
  MP4CopyrightAtomType                         = MP4_FOUR_CHAR_CODE('c', 'p', 'r', 't'),
  MP4DataEntryURLAtomType                      = MP4_FOUR_CHAR_CODE('u', 'r', 'l', ' '),
  MP4DataEntryURNAtomType                      = MP4_FOUR_CHAR_CODE('u', 'r', 'n', ' '),
  MP4DataInformationAtomType                   = MP4_FOUR_CHAR_CODE('d', 'i', 'n', 'f'),
  MP4DataReferenceAtomType                     = MP4_FOUR_CHAR_CODE('d', 'r', 'e', 'f'),
  MP4DegradationPriorityAtomType               = MP4_FOUR_CHAR_CODE('s', 't', 'd', 'p'),
  MP4ESDAtomType                               = MP4_FOUR_CHAR_CODE('e', 's', 'd', 's'),
  MP4EditAtomType                              = MP4_FOUR_CHAR_CODE('e', 'd', 't', 's'),
  MP4EditListAtomType                          = MP4_FOUR_CHAR_CODE('e', 'l', 's', 't'),
  MP4ExtendedAtomType                          = MP4_FOUR_CHAR_CODE('u', 'u', 'i', 'd'),
  MP4FreeSpaceAtomType                         = MP4_FOUR_CHAR_CODE('f', 'r', 'e', 'e'),
  MP4GenericSampleEntryAtomType                = MP4_FOUR_CHAR_CODE('!', 'g', 'n', 'r'),
  MP4HandlerAtomType                           = MP4_FOUR_CHAR_CODE('h', 'd', 'l', 'r'),
  MP4HintMediaHeaderAtomType                   = MP4_FOUR_CHAR_CODE('h', 'm', 'h', 'd'),
  MP4VolumetricVisualMediaHeader               = MP4_FOUR_CHAR_CODE('v', 'v', 'h', 'd'),
  MP4HintTrackReferenceAtomType                = MP4_FOUR_CHAR_CODE('h', 'i', 'n', 't'),
  MP4MPEGMediaHeaderAtomType                   = MP4_FOUR_CHAR_CODE('n', 'm', 'h', 'd'),
  MP4MPEGSampleEntryAtomType                   = MP4_FOUR_CHAR_CODE('m', 'p', '4', 's'),
  MP4MediaAtomType                             = MP4_FOUR_CHAR_CODE('m', 'd', 'i', 'a'),
  MP4MediaDataAtomType                         = MP4_FOUR_CHAR_CODE('m', 'd', 'a', 't'),
  MP4MediaHeaderAtomType                       = MP4_FOUR_CHAR_CODE('m', 'd', 'h', 'd'),
  MP4MediaInformationAtomType                  = MP4_FOUR_CHAR_CODE('m', 'i', 'n', 'f'),
  MP4ExtendedLanguageTagAtomType               = MP4_FOUR_CHAR_CODE('e', 'l', 'n', 'g'),
  MP4MovieAtomType                             = MP4_FOUR_CHAR_CODE('m', 'o', 'o', 'v'),
  MP4MovieHeaderAtomType                       = MP4_FOUR_CHAR_CODE('m', 'v', 'h', 'd'),
  MP4ObjectDescriptorAtomType                  = MP4_FOUR_CHAR_CODE('i', 'o', 'd', 's'),
  MP4ObjectDescriptorMediaHeaderAtomType       = MP4_FOUR_CHAR_CODE('o', 'd', 'h', 'd'),
  MP4ODTrackReferenceAtomType                  = MP4_FOUR_CHAR_CODE('m', 'p', 'o', 'd'),
  MP4SampleDescriptionAtomType                 = MP4_FOUR_CHAR_CODE('s', 't', 's', 'd'),
  MP4SampleSizeAtomType                        = MP4_FOUR_CHAR_CODE('s', 't', 's', 'z'),
  MP4CompactSampleSizeAtomType                 = MP4_FOUR_CHAR_CODE('s', 't', 'z', '2'),
  MP4SampleTableAtomType                       = MP4_FOUR_CHAR_CODE('s', 't', 'b', 'l'),
  MP4SampleToChunkAtomType                     = MP4_FOUR_CHAR_CODE('s', 't', 's', 'c'),
  MP4SampleAuxiliaryInformationSizesAtomType   = MP4_FOUR_CHAR_CODE('s', 'a', 'i', 'z'),
  MP4SampleAuxiliaryInformationOffsetsAtomType = MP4_FOUR_CHAR_CODE('s', 'a', 'i', 'o'),
  MP4SceneDescriptionMediaHeaderAtomType       = MP4_FOUR_CHAR_CODE('s', 'd', 'h', 'd'),
  MP4ShadowSyncAtomType                        = MP4_FOUR_CHAR_CODE('s', 't', 's', 'h'),
  MP4SkipAtomType                              = MP4_FOUR_CHAR_CODE('s', 'k', 'i', 'p'),
  MP4SoundMediaHeaderAtomType                  = MP4_FOUR_CHAR_CODE('s', 'm', 'h', 'd'),
  MP4StreamDependenceAtomType                  = MP4_FOUR_CHAR_CODE('d', 'p', 'n', 'd'),
  MP4SubSampleInformationAtomType              = MP4_FOUR_CHAR_CODE('s', 'u', 'b', 's'),
  MP4SyncSampleAtomType                        = MP4_FOUR_CHAR_CODE('s', 't', 's', 's'),
  MP4SyncTrackReferenceAtomType                = MP4_FOUR_CHAR_CODE('s', 'y', 'n', 'c'),
  MP4TimeToSampleAtomType                      = MP4_FOUR_CHAR_CODE('s', 't', 't', 's'),
  MP4TrackAtomType                             = MP4_FOUR_CHAR_CODE('t', 'r', 'a', 'k'),
  MP4TrackHeaderAtomType                       = MP4_FOUR_CHAR_CODE('t', 'k', 'h', 'd'),
  MP4TrackReferenceAtomType                    = MP4_FOUR_CHAR_CODE('t', 'r', 'e', 'f'),
  MP4TrackGroupAtomType                        = MP4_FOUR_CHAR_CODE('t', 'r', 'g', 'r'),
  MP4UserDataAtomType                          = MP4_FOUR_CHAR_CODE('u', 'd', 't', 'a'),
  MP4VideoMediaHeaderAtomType                  = MP4_FOUR_CHAR_CODE('v', 'm', 'h', 'd'),
  MP4VisualSampleEntryAtomType                 = MP4_FOUR_CHAR_CODE('m', 'p', '4', 'v'),
  MP4PaddingBitsAtomType                       = MP4_FOUR_CHAR_CODE('p', 'a', 'd', 'b'),
  MP4MovieExtendsAtomType                      = MP4_FOUR_CHAR_CODE('m', 'v', 'e', 'x'),
  MP4TrackExtensionPropertiesAtomType          = MP4_FOUR_CHAR_CODE('t', 'r', 'e', 'p'),
  MP4TrackExtendsAtomType                      = MP4_FOUR_CHAR_CODE('t', 'r', 'e', 'x'),
  MP4MovieFragmentAtomType                     = MP4_FOUR_CHAR_CODE('m', 'o', 'o', 'f'),
  MP4MovieFragmentHeaderAtomType               = MP4_FOUR_CHAR_CODE('m', 'f', 'h', 'd'),
  MP4TrackFragmentAtomType                     = MP4_FOUR_CHAR_CODE('t', 'r', 'a', 'f'),
  MP4TrackFragmentHeaderAtomType               = MP4_FOUR_CHAR_CODE('t', 'f', 'h', 'd'),
  MP4TrackFragmentDecodeTimeAtomType           = MP4_FOUR_CHAR_CODE('t', 'f', 'd', 't'),
  MP4TrackRunAtomType                          = MP4_FOUR_CHAR_CODE('t', 'r', 'u', 'n'),
  MP4ItemPropertiesAtomType                    = MP4_FOUR_CHAR_CODE('i', 'p', 'r', 'p'),
  MP4ItemPropertyContainerAtomType             = MP4_FOUR_CHAR_CODE('i', 'p', 'c', 'o'),
  MP4ItemPropertyAssociationAtomType           = MP4_FOUR_CHAR_CODE('i', 'p', 'm', 'a'),
  MP4SampleGroupDescriptionAtomType            = MP4_FOUR_CHAR_CODE('s', 'g', 'p', 'd'),
  MP4SampletoGroupAtomType                     = MP4_FOUR_CHAR_CODE('s', 'b', 'g', 'p'),
  MP4CompactSampletoGroupAtomType              = MP4_FOUR_CHAR_CODE('c', 's', 'g', 'p'),
  MP4SampleDependencyAtomType                  = MP4_FOUR_CHAR_CODE('s', 'd', 't', 'p'),
  ISOMetaAtomType                              = MP4_FOUR_CHAR_CODE('m', 'e', 't', 'a'),
  ISOPrimaryItemAtomType                       = MP4_FOUR_CHAR_CODE('p', 'i', 't', 'm'),
  ISOItemLocationAtomType                      = MP4_FOUR_CHAR_CODE('i', 'l', 'o', 'c'),
  ISOItemProtectionAtomType                    = MP4_FOUR_CHAR_CODE('i', 'p', 'r', 'o'),
  ISOItemInfoAtomType                          = MP4_FOUR_CHAR_CODE('i', 'i', 'n', 'f'),
  ISOItemInfoEntryAtomType                     = MP4_FOUR_CHAR_CODE('i', 'n', 'f', 'e'),
  ISOAdditionalMetaDataContainerAtomType       = MP4_FOUR_CHAR_CODE('m', 'e', 'c', 'o'),
  ISOMetaboxRelationAtomType                   = MP4_FOUR_CHAR_CODE('m', 'e', 'r', 'e'),
  ISOItemDataAtomType                          = MP4_FOUR_CHAR_CODE('i', 'd', 'a', 't'),
  ISOItemReferenceAtomType                     = MP4_FOUR_CHAR_CODE('i', 'r', 'e', 'f'),
  ISOVCConfigAtomType                          = MP4_FOUR_CHAR_CODE('a', 'v', 'c', 'C'),
  ISOHEVCConfigAtomType                        = MP4_FOUR_CHAR_CODE('h', 'v', 'c', 'C'),
  ISOVVCConfigAtomType                         = MP4_FOUR_CHAR_CODE('v', 'v', 'c', 'C'),
  ISOVVCNALUConfigAtomType                     = MP4_FOUR_CHAR_CODE('v', 'v', 'n', 'C'),
  ISOAVCSampleEntryAtomType                    = MP4_FOUR_CHAR_CODE('a', 'v', 'c', '1'),
  ISOHEVCSampleEntryAtomType                   = MP4_FOUR_CHAR_CODE('h', 'v', 'c', '1'),
  ISOLHEVCSampleEntryAtomType                  = MP4_FOUR_CHAR_CODE('h', 'v', 'c', '2'),
  ISOVVCSampleEntryAtomTypeOutOfBand           = MP4_FOUR_CHAR_CODE('v', 'v', 'c', '1'),
  ISOVVCSampleEntryAtomTypeInBand              = MP4_FOUR_CHAR_CODE('v', 'v', 'i', '1'),
  ISOVVCSubpicSampleEntryAtomType              = MP4_FOUR_CHAR_CODE('v', 'v', 's', '1'),
  MP4XMLMetaSampleEntryAtomType                = MP4_FOUR_CHAR_CODE('m', 'e', 't', 'x'),
  MP4TextMetaSampleEntryAtomType               = MP4_FOUR_CHAR_CODE('m', 'e', 't', 't'),
  MP4AMRSampleEntryAtomType                    = MP4_FOUR_CHAR_CODE('s', 'a', 'm', 'r'),
  MP4AMRSpecificInfoAtomType                   = MP4_FOUR_CHAR_CODE('d', 'a', 'm', 'r'),
  MP4AWBSampleEntryAtomType                    = MP4_FOUR_CHAR_CODE('s', 'a', 'w', 'b'),
  MP4AMRWPSampleEntryAtomType                  = MP4_FOUR_CHAR_CODE('s', 'a', 'w', 'p'),
  MP4AMRWPSpecificInfoAtomType                 = MP4_FOUR_CHAR_CODE('d', 'a', 'w', 'p'),
  MP4H263SampleEntryAtomType                   = MP4_FOUR_CHAR_CODE('s', '2', '6', '3'),
  MP4H263SpecificInfoAtomType                  = MP4_FOUR_CHAR_CODE('d', '2', '6', '3'),
  MP4BitRateAtomType                           = MP4_FOUR_CHAR_CODE('b', 't', 'r', 't'),
  TGPPBitRateAtomType                          = MP4_FOUR_CHAR_CODE('b', 'i', 't', 'r'),
  MP4OriginalFormatAtomType                    = MP4_FOUR_CHAR_CODE('f', 'r', 'm', 'a'),
  MP4SchemeTypeAtomType                        = MP4_FOUR_CHAR_CODE('s', 'c', 'h', 'm'),
  MP4SchemeInfoAtomType                        = MP4_FOUR_CHAR_CODE('s', 'c', 'h', 'i'),
  MP4StereoVideoAtomType                       = MP4_FOUR_CHAR_CODE('s', 't', 'v', 'i'),
  MP4CompatibleSchemeTypeAtomType              = MP4_FOUR_CHAR_CODE('c', 's', 'c', 'h'),
  MP4RestrictedSchemeInfoAtomType              = MP4_FOUR_CHAR_CODE('r', 'i', 'n', 'f'),
  MP4StereoVideoGroupAtomType                  = MP4_FOUR_CHAR_CODE('s', 't', 'e', 'r'),
  MP4TrackTypeAtomType                         = MP4_FOUR_CHAR_CODE('t', 't', 'y', 'p'),
  MP4RestrictedVideoSampleEntryAtomType        = MP4_FOUR_CHAR_CODE('r', 'e', 's', 'v'),
  MP4SegmentTypeAtomType                       = MP4_FOUR_CHAR_CODE('s', 't', 'y', 'p'),
  MP4SegmentIndexAtomType                      = MP4_FOUR_CHAR_CODE('s', 'i', 'd', 'x'),
  MP4SubsegmentIndexAtomType                   = MP4_FOUR_CHAR_CODE('s', 's', 'i', 'x'),
  MP4ProducerReferenceTimeAtomType             = MP4_FOUR_CHAR_CODE('p', 'r', 'f', 't'),
  MP4BoxedMetadataSampleEntryType              = MP4_FOUR_CHAR_CODE('m', 'e', 'b', 'x'),
  MP4MetadataKeyTableBoxType                   = MP4_FOUR_CHAR_CODE('k', 'e', 'y', 's'),
  MP4MetadataGenericKeyBoxType                 = MP4_FOUR_CHAR_CODE('!', 'k', 'e', 'y'), /* hack */
  MP4MetadataKeyDeclarationBoxType             = MP4_FOUR_CHAR_CODE('k', 'e', 'y', 'd'),
  MP4MetadataLocaleBoxType                     = MP4_FOUR_CHAR_CODE('l', 'o', 'c', 'a'),
  MP4MetadataSetupBoxType                      = MP4_FOUR_CHAR_CODE('s', 'e', 't', 'u'),
  MP4GroupsListBoxType                         = MP4_FOUR_CHAR_CODE('g', 'r', 'p', 'l'),
  MP4AlternativeEntityGroup                    = MP4_FOUR_CHAR_CODE('a', 'l', 't', 'r')

};

#ifdef ISMACrypt
enum
{
  MP4SecurityInfoAtomType         = MP4_FOUR_CHAR_CODE('s', 'i', 'n', 'f'),
  ISMAKMSAtomType                 = MP4_FOUR_CHAR_CODE('i', 'K', 'M', 'S'),
  ISMASampleFormatAtomType        = MP4_FOUR_CHAR_CODE('i', 'S', 'F', 'M'),
  ISMASaltAtomType                = MP4_FOUR_CHAR_CODE('i', 'S', 'L', 'T'),
  MP4EncAudioSampleEntryAtomType  = MP4_FOUR_CHAR_CODE('e', 'n', 'c', 'a'),
  MP4EncVisualSampleEntryAtomType = MP4_FOUR_CHAR_CODE('e', 'n', 'c', 'v'),
  ISMACryptAESCounterType         = MP4_FOUR_CHAR_CODE('i', 'A', 'E', 'C')
};
#endif

#define MP4_BASE_ATOM                                                                   \
  u32 type;                                                                             \
  u8 uuid[16];                                                                          \
  u32 size;                                                                             \
  u64 size64;                                                                           \
  u32 bytesRead;                                                                        \
  u32 bytesWritten;                                                                     \
  u64 streamOffset;                                                                     \
  char *name;                                                                           \
  struct MP4Atom *super;                                                                \
  MP4Err (*createFromInputStream)(struct MP4Atom * self, struct MP4Atom * proto,        \
                                  /*struct MP4InputStreamRecord* */ char *inputStream); \
  char *(*getName)(struct MP4Atom * self);                                              \
  MP4Err (*serialize)(struct MP4Atom * self, char *buffer);                             \
  MP4Err (*calculateSize)(struct MP4Atom * self);                                       \
  void (*destroy)(struct MP4Atom * self);

#define MP4_FULL_ATOM \
  MP4_BASE_ATOM       \
  u32 version;        \
  u32 flags;

typedef struct MP4Atom
{
  MP4_BASE_ATOM
} MP4Atom, *MP4AtomPtr;

typedef struct MP4FullAtom
{
  MP4_FULL_ATOM
} MP4FullAtom, *MP4FullAtomPtr;

typedef MP4Err (*cisfunc)(struct MP4Atom *self, struct MP4Atom *proto, char *inputStream);

typedef struct MP4PrivateMovieRecord
{
  u32 referenceCount;
  struct FileMappingObjectRecord *fileMappingObject;
  struct MP4InputStreamRecord *inputStream;
  MP4AtomPtr moovAtomPtr; /* might be a moof or a moov */
  MP4AtomPtr true_moov;

  MP4AtomPtr mdat;        /* first, primary, mdat */
  MP4LinkedList mdatList; /* all the others we find */
  /*MP4Track initialBIFS;*/
  /*MP4Track initialOD; */
  u32 fileType;    /* the file type: MPEG-4, Motion JPEG-2000, or QuickTime */
  MP4AtomPtr ftyp; /* the file type atom */
  MP4AtomPtr jp2h; /* for JPEG-2000, the JP2 header atom */
  MP4AtomPtr sgnt; /* for JPEG-2000, the signature atom */
  MP4AtomPtr meta; /* file-level meta-data */
  MP4AtomPtr meco; /* Deprecated file-level additional meta-data container */

  void *inMemoryDataHandler;
  MP4Handle prepend_handle;
  MP4LinkedList movieFragments;
} MP4PrivateMovieRecord, *MP4PrivateMovieRecordPtr;

#ifdef __cplusplus
extern "C"
{
#endif

  MP4Err MP4CreateAtom(u32 atomType, MP4AtomPtr *outAtom);
  MP4Err MP4ParseAtomFromHandle(MP4Handle inputHandle, MP4AtomPtr *outAtom);

#ifdef __cplusplus
}
#endif

MP4Err MP4CreateBaseAtom(MP4AtomPtr self);
MP4Err MP4CreateFullAtom(MP4AtomPtr s);
MP4Err MP4ParseAtom(struct MP4InputStreamRecord *inputStream, MP4AtomPtr *outAtom);
MP4Err MP4ParseAtomUsingProtoList(struct MP4InputStreamRecord *inputStream, u32 *protoList,
                                  u32 defaultAtom, MP4AtomPtr *outAtom);
MP4Err MP4SerializeCommonBaseAtomFields(struct MP4Atom *self, char *buffer);
MP4Err MP4SerializeCommonFullAtomFields(struct MP4FullAtom *self, char *buffer);
MP4Err MP4CalculateBaseAtomFieldSize(struct MP4Atom *self);
MP4Err MP4CalculateFullAtomFieldSize(struct MP4FullAtom *self);
MP4Err MP4FindGroupAtom(MP4LinkedList theList, u32 type, MP4AtomPtr *theAtom);

void MP4TypeToString(u32 inType, char *ioStr);

typedef struct MP4MediaDataAtom
{
  MP4_BASE_ATOM
  MP4Err (*addData)(struct MP4MediaDataAtom *self, MP4Handle dataH);
  MP4Err (*writeToFile)(struct MP4MediaDataAtom *self, FILE *fd);
  MP4Err (*addMdat)(struct MP4MediaDataAtom *self, struct MP4MediaDataAtom *other_mdat);

  char *data;
  u64 dataSize;
  u64 dataOffset;
  u64 allocatedSize;
} MP4MediaDataAtom, *MP4MediaDataAtomPtr;

typedef struct MP4UnknownAtom
{
  MP4_BASE_ATOM
  char *data;
  u32 dataSize;
} MP4UnknownAtom, *MP4UnknownAtomPtr;

#define COMMON_MOVIE_ATOM_FIELDS                                                              \
  MP4Err (*mdatMoved)(struct MP4MovieAtom * self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset); \
  MP4Err (*calculateDuration)(struct MP4MovieAtom * self);                                    \
                                                                                              \
  MP4AtomPtr iods;
/* the initial object descriptor */

typedef struct MP4MovieAtom
{
  MP4_BASE_ATOM
  COMMON_MOVIE_ATOM_FIELDS
  MP4Err (*setupTracks)(struct MP4MovieAtom *self, MP4PrivateMovieRecordPtr moov);
  u32 (*getTrackCount)(struct MP4MovieAtom *self);
  MP4Err (*getIndTrack)(struct MP4MovieAtom *self, u32 trackNumber, MP4AtomPtr *outTrack);
  MP4Err (*addAtom)(struct MP4MovieAtom *self, MP4AtomPtr atom);
  MP4Err (*getNextTrackID)(struct MP4MovieAtom *self, u32 *outID);
  MP4Err (*addTrack)(struct MP4MovieAtom *self, MP4AtomPtr track);
  MP4Err (*newTrack)(struct MP4MovieAtom *self, u32 newTrackFlags, MP4AtomPtr *outTrack);
  MP4Err (*newTrackWithID)(struct MP4MovieAtom *self, u32 newTrackFlags, u32 newTrackID,
                           MP4AtomPtr *outTrack);
  MP4Err (*setTimeScale)(struct MP4MovieAtom *self, u32 timeScale);
  MP4Err (*getTimeScale)(struct MP4MovieAtom *self, u32 *outTimeScale);
  MP4Err (*setMatrix)(struct MP4MovieAtom *self, u32 matrix[9]);
  MP4Err (*getMatrix)(struct MP4MovieAtom *self, u32 outMatrix[9]);
  MP4Err (*setPreferredRate)(struct MP4MovieAtom *self, u32 rate);
  MP4Err (*getPreferredRate)(struct MP4MovieAtom *self, u32 *outRate);
  MP4Err (*setPreferredVolume)(struct MP4MovieAtom *self, s16 volume);
  MP4Err (*getPreferredVolume)(struct MP4MovieAtom *self, s16 *outVolume);
  MP4Err (*settrackfragment)(struct MP4MovieAtom *self, u32 trackID, MP4AtomPtr fragment);
  MP4Err (*getTrackExtendsAtom)(struct MP4MovieAtom *self, u32 trackID, MP4AtomPtr *outTrack);
  MP4Err (*getTrackMedia)(struct MP4MovieAtom *self, u32 trackID, MP4AtomPtr *outMedia);
  MP4Err (*mdatArrived)(struct MP4MovieAtom *self, MP4AtomPtr mdat);
  MP4Err (*getSampleDescriptionIndex)(struct MP4MovieAtom *self, u32 trackID, u32 *sd_index);

  MP4AtomPtr mvhd; /* the movie header */
  MP4AtomPtr udta; /* user data */
  MP4AtomPtr mvex; /* movie extends atom */
  MP4AtomPtr meta;
  MP4AtomPtr meco; /* additional meta-data container */
  MP4PrivateMovieRecordPtr moov;
  MP4LinkedList atomList;
  MP4LinkedList trackList;
} MP4MovieAtom, *MP4MovieAtomPtr;

typedef struct MP4MovieHeaderAtom
{
  MP4_FULL_ATOM
  u64 creationTime;
  u64 modificationTime;
  u32 timeScale;
  u64 duration;
  u32 qt_preferredRate;
  u32 qt_preferredVolume;
  char qt_reserved[10];
  u32 qt_matrixA;
  u32 qt_matrixB;
  u32 qt_matrixU;
  u32 qt_matrixC;
  u32 qt_matrixD;
  u32 qt_matrixV;
  u32 qt_matrixX;
  u32 qt_matrixY;
  u32 qt_matrixW;
  u32 qt_previewTime;
  u32 qt_previewDuration;
  u32 qt_posterTime;
  u32 qt_selectionTime;
  u32 qt_selectionDuration;
  u32 qt_currentTime;
  u32 nextTrackID;
} MP4MovieHeaderAtom, *MP4MovieHeaderAtomPtr;

typedef struct MP4ObjectDescriptorAtom
{
  MP4_FULL_ATOM
  MP4Err (*setDescriptor)(struct MP4Atom *self, /*struct MP4DescriptorRecord */ char *desc);
  u32 ODSize;
  struct MP4DescriptorRecord *descriptor;
} MP4ObjectDescriptorAtom, *MP4ObjectDescriptorAtomPtr;

typedef struct MP4TrackAtom
{
  MP4_BASE_ATOM
  MP4PrivateMovieRecordPtr moov;
  MP4Err (*addAtom)(struct MP4TrackAtom *self, MP4AtomPtr atom);
  MP4Err (*setMoov)(struct MP4TrackAtom *self, MP4PrivateMovieRecordPtr moov);
  MP4Err (*setMdat)(struct MP4TrackAtom *self, MP4AtomPtr mdat);
  MP4Err (*newMedia)(struct MP4TrackAtom *self, MP4Media *outMedia, u32 mediaType, u32 timeScale,
                     MP4Handle dataURL);
  MP4Err (*setEnabled)(struct MP4TrackAtom *self, u32 enabled);
  MP4Err (*getEnabled)(struct MP4TrackAtom *self, u32 *outEnabled);
  MP4Err (*calculateDuration)(struct MP4TrackAtom *self, u32 movieTimeScale);
  MP4Err (*getDuration)(struct MP4TrackAtom *self, u32 *outDuration);
  MP4Err (*setMatrix)(struct MP4TrackAtom *self, u32 matrix[9]);
  MP4Err (*getMatrix)(struct MP4TrackAtom *self, u32 outMatrix[9]);
  MP4Err (*setLayer)(struct MP4TrackAtom *self, s16 layer);
  MP4Err (*getLayer)(struct MP4TrackAtom *self, s16 *outLayer);
  MP4Err (*setDimensions)(struct MP4TrackAtom *self, u32 width, u32 height);
  MP4Err (*getDimensions)(struct MP4TrackAtom *self, u32 *outWidth, u32 *outHeight);
  MP4Err (*setVolume)(struct MP4TrackAtom *self, s16 volume);
  MP4Err (*getVolume)(struct MP4TrackAtom *self, s16 *outVolume);
  MP4Err (*mdatMoved)(struct MP4TrackAtom *self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset);
  MP4Err (*mdatArrived)(struct MP4TrackAtom *self, MP4AtomPtr mdat);
  MP4Err (*settrackfragment)(struct MP4TrackAtom *self, u32 trackID, MP4AtomPtr fragment);

  u32 newTrackFlags;
  MP4AtomPtr mdat;
  MP4AtomPtr udta;
  MP4AtomPtr trackHeader;
  MP4AtomPtr trackMedia;
  MP4AtomPtr trackEditAtom;
  MP4AtomPtr trackReferences;
  MP4AtomPtr trackGroups;
  MP4AtomPtr meta;
  MP4AtomPtr meco; /* additional meta-data container */
  MP4LinkedList atomList;
} MP4TrackAtom, *MP4TrackAtomPtr;

typedef struct MP4TrackHeaderAtom
{
  MP4_FULL_ATOM
  u64 creationTime;
  u64 modificationTime;
  u32 trackID;
  u32 qt_reserved1;
  u64 duration;
  char qt_reserved2[8];
  u32 qt_layer;
  u32 qt_alternateGroup;
  u32 qt_volume;
  u32 qt_reserved3;
  u32 qt_matrixA;
  u32 qt_matrixB;
  u32 qt_matrixU;
  u32 qt_matrixC;
  u32 qt_matrixD;
  u32 qt_matrixV;
  u32 qt_matrixX;
  u32 qt_matrixY;
  u32 qt_matrixW;
  u32 qt_trackWidth;
  u32 qt_trackHeight;
} MP4TrackHeaderAtom, *MP4TrackHeaderAtomPtr;

typedef struct MP4TrackReferenceAtom
{
  MP4_BASE_ATOM
  MP4Err (*addAtom)(struct MP4TrackReferenceAtom *self, MP4AtomPtr atom);
  MP4Err (*findAtomOfType)(struct MP4TrackReferenceAtom *self, u32 atomType, MP4AtomPtr *outAtom);
  MP4LinkedList atomList;
} MP4TrackReferenceAtom, *MP4TrackReferenceAtomPtr;

typedef struct MP4TrackGroupAtom
{
  MP4_BASE_ATOM
  MP4Err (*addAtom)(struct MP4TrackGroupAtom *self, MP4AtomPtr atom);
  MP4Err (*findAtomOfType)(struct MP4TrackGroupAtom *self, u32 atomType, MP4AtomPtr *outAtom);
  MP4LinkedList atomList;
} MP4TrackGroupAtom, *MP4TrackGroupAtomPtr;

typedef struct MP4MediaAtom
{
  MP4_BASE_ATOM
  MP4Err (*setupNew)(struct MP4MediaAtom *self, MP4AtomPtr track, u32 mediaType, u32 timeScale,
                     MP4Handle dataHandlerH);
  MP4Err (*addSampleReference)(struct MP4MediaAtom *self, u64 dataOffset, u32 sampleCount,
                               MP4Handle durationsH, MP4Handle sizesH, MP4Handle sampleEntryH,
                               MP4Handle decodingOffsetsH, MP4Handle syncSamplesH, MP4Handle padsH);
  MP4Err (*addSamples)(struct MP4MediaAtom *self, MP4Handle sampleH, u32 sampleCount,
                       MP4Handle durationsH, MP4Handle sizesH, MP4Handle sampleEntryH,
                       MP4Handle decodingOffsetsH, MP4Handle syncSamplesH, MP4Handle padsH);
  MP4Err (*calculateDuration)(struct MP4MediaAtom *self);
  MP4Err (*mdatMoved)(struct MP4MediaAtom *self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset);
  MP4Err (*setfieldsize)(struct MP4MediaAtom *self, u32 fieldsize);
  MP4Err (*settrackfragment)(struct MP4MediaAtom *self, MP4AtomPtr fragment);
  MP4Err (*addGroupDescription)(struct MP4MediaAtom *self, u32 groupType, MP4Handle description,
                                u32 *index);
  MP4Err (*changeSamplestoGroupType)(struct MP4MediaAtom *self,
                                     sampleToGroupType_t sampleToGroupType);
  MP4Err (*mapSamplestoGroup)(struct MP4MediaAtom *self, u32 groupType, u32 group_index,
                              s32 sample_index, u32 count);
  MP4Err (*getSampleGroupMap)(struct MP4MediaAtom *self, u32 groupType, u32 sample_number,
                              u32 *group_index);
  MP4Err (*getSampleGroupSampleNumbers)(struct MP4MediaAtom *self, u32 groupType, u32 groupIndex,
                                        u32 **outSampleNumbers, u32 *outSampleCnt);
  MP4Err (*getGroupDescription)(struct MP4MediaAtom *self, u32 groupType, u32 index,
                                MP4Handle description);

  MP4Err (*setSampleDependency)(struct MP4MediaAtom *self, s32 sample_index,
                                MP4Handle dependencies);
  MP4Err (*getSampleDependency)(struct MP4MediaAtom *self, u32 sampleNumber, u8 *dependency);

  MP4Err (*extendLastSampleDuration)(struct MP4MediaAtom *self, u32 duration);
  MP4Err (*setSampleEntry)(struct MP4MediaAtom *self, MP4AtomPtr entry);
  MP4Err (*setExtendedLanguageTag)(struct MP4MediaAtom *self, MP4AtomPtr tag);

  MP4AtomPtr mediaTrack;
  MP4AtomPtr mediaHeader;
  MP4AtomPtr handler;
  MP4AtomPtr information; /* might be a minf or traf */
  MP4AtomPtr extendedLanguageTag;
  MP4LinkedList atomList;
  MP4AtomPtr true_minf;
  sampleToGroupType_t sampleToGroupType;
} MP4MediaAtom, *MP4MediaAtomPtr;

typedef struct MP4MediaHeaderAtom
{
  MP4_FULL_ATOM
  u64 creationTime;
  u64 modificationTime;
  u32 timeScale;
  u64 duration;
  u32 packedLanguage;
  u32 qt_quality;
} MP4MediaHeaderAtom, *MP4MediaHeaderAtomPtr;

typedef struct MP4HandlerAtom
{
  MP4_FULL_ATOM
  MP4Err (*setName)(struct MP4Atom *s, char *name, u32 is_qt);
  u32 nameLength;
  u32 qt_componentType;
  u32 handlerType;
  u32 qt_componentManufacturer;
  u32 qt_componentFlags;
  u32 qt_componentFlagsMask;
  char *nameUTF8;
  u32 is_qt;
} MP4HandlerAtom, *MP4HandlerAtomPtr;

typedef struct MP4ExtendedLanguageTag
{
  MP4_FULL_ATOM
  char *extended_language;
} MP4ExtendedLanguageTagAtom, *MP4ExtendedLanguageTagAtomPtr;

#define COMMON_MINF_ATOM_FIELDS                                                                   \
  MP4Err (*addSampleReference)(struct MP4MediaInformationAtom * self, u64 dataOffset,             \
                               u32 sampleCount, MP4Handle durationsH, MP4Handle sizesH,           \
                               MP4Handle sampleEntryH, MP4Handle decodingOffsetsH,                \
                               MP4Handle syncSamplesH, MP4Handle padsH);                          \
  MP4Err (*addSamples)(struct MP4MediaInformationAtom * self, MP4Handle sampleH, u32 sampleCount, \
                       MP4Handle durationsH, MP4Handle sizesH, MP4Handle sampleEntryH,            \
                       MP4Handle decodingOffsetsH, MP4Handle syncSamplesH, MP4Handle padsH);      \
  MP4Err (*mdatMoved)(struct MP4MediaInformationAtom * self, u64 mdatBase, u64 mdatEnd,           \
                      s32 mdatOffset);                                                            \
  MP4Err (*mapSamplestoGroup)(struct MP4MediaInformationAtom * self, u32 groupType,               \
                              u32 group_index, s32 sample_index, u32 count,                       \
                              sampleToGroupType_t sampleToGroupType);                             \
  MP4Err (*setSampleDependency)(struct MP4MediaInformationAtom * self, s32 sample_index,          \
                                MP4Handle dependencies);                                          \
  MP4Err (*changeSamplestoGroupType)(struct MP4MediaInformationAtom * self,                       \
                                     sampleToGroupType_t sampleToGroupType);

typedef struct MP4MediaInformationAtom
{
  MP4_FULL_ATOM
  COMMON_MINF_ATOM_FIELDS
  MP4Err (*closeDataHandler)(MP4AtomPtr self);
  MP4Err (*openDataHandler)(MP4AtomPtr self, u32 dataEntryIndex);
  MP4Err (*setupNewMedia)(struct MP4MediaInformationAtom *self, u32 mediaType, MP4Handle dataH,
                          MP4AtomPtr mdat);
  MP4Err (*getMediaDuration)(struct MP4MediaInformationAtom *self, u32 *outDuration);
  MP4Err (*setfieldsize)(struct MP4MediaInformationAtom *self, u32 fieldsize);
  MP4Err (*testDataEntry)(struct MP4MediaInformationAtom *self, u32 dataEntryIndex);
  MP4Err (*mdatArrived)(struct MP4MediaInformationAtom *self, MP4AtomPtr mdat);
  MP4Err (*addGroupDescription)(struct MP4MediaInformationAtom *self, u32 groupType,
                                MP4Handle description, u32 *index);
  MP4Err (*getSampleGroupMap)(struct MP4MediaInformationAtom *self, u32 groupType,
                              u32 sample_number, u32 *group_index);
  MP4Err (*getSampleGroupSampleNumbers)(struct MP4MediaInformationAtom *self, u32 groupType,
                                        u32 groupIndex, u32 **outSampleNumbers, u32 *outSampleCnt);
  MP4Err (*getGroupDescription)(struct MP4MediaInformationAtom *self, u32 groupType, u32 index,
                                MP4Handle description);
  MP4Err (*getSampleDependency)(struct MP4MediaInformationAtom *self, u32 sampleNumber,
                                u8 *dependency);
  MP4Err (*extendLastSampleDuration)(struct MP4MediaInformationAtom *self, u32 duration);
  MP4Err (*setSampleEntry)(struct MP4MediaInformationAtom *self, MP4AtomPtr entry);

  MP4AtomPtr dataInformation;
  MP4AtomPtr sampleTable;
  MP4AtomPtr mediaHeader;
  struct MP4InputStreamRecord *inputStream;
  void *dataHandler;
  u32 dataEntryIndex;
  MP4LinkedList atomList;
} MP4MediaInformationAtom, *MP4MediaInformationAtomPtr;

typedef struct MP4VideoMediaHeaderAtom
{
  MP4_FULL_ATOM
  u32 graphicsMode;
  u32 opColorRed;
  u32 opColorGreen;
  u32 opColorBlue;
} MP4VideoMediaHeaderAtom, *MP4VideoMediaHeaderAtomPtr;

typedef struct MP4SoundMediaHeaderAtom
{
  MP4_FULL_ATOM
  u32 balance;
  u32 reserved;
} MP4SoundMediaHeaderAtom, *MP4SoundMediaHeaderAtomPtr;

typedef struct MP4HintMediaHeaderAtom
{
  MP4_FULL_ATOM
  u32 maxPDUSize;
  u32 avgPDUSize;
  u32 maxBitrate;
  u32 avgBitrate;
  u32 slidingAverageBitrate;
} MP4HintMediaHeaderAtom, *MP4HintMediaHeaderAtomPtr;

typedef struct MP4MPEGMediaHeaderAtom
{
  MP4_FULL_ATOM
} MP4MPEGMediaHeaderAtom, *MP4MPEGMediaHeaderAtomPtr;

typedef struct MP4ObjectDescriptorMediaHeaderAtom
{
  MP4_FULL_ATOM
} MP4ObjectDescriptorMediaHeaderAtom, *MP4ObjectDescriptorMediaHeaderAtomPtr;

typedef struct MP4ClockReferenceMediaHeaderAtom
{
  MP4_FULL_ATOM
} MP4ClockReferenceMediaHeaderAtom, *MP4ClockReferenceMediaHeaderAtomPtr;

typedef struct MP4SceneDescriptionMediaHeaderAtom
{
  MP4_FULL_ATOM
} MP4SceneDescriptionMediaHeaderAtom, *MP4SceneDescriptionMediaHeaderAtomPtr;

typedef struct MP4DataInformationAtom
{
  MP4_BASE_ATOM
  MP4Err (*getOffset)(struct MP4DataInformationAtom *self, u32 dataReferenceIndex, u64 *outOffset);
  MP4Err (*addSampleReference)(struct MP4DataInformationAtom *self, u32 sampleCount,
                               u32 dataReferenceIndex, u64 dataOffset, MP4Handle sizesH);
  MP4Err (*addSamples)(struct MP4DataInformationAtom *self, u32 sampleCount, u32 dataReferenceIndex,
                       MP4Handle sampleH);
  MP4Err (*addAtom)(struct MP4DataInformationAtom *self, MP4AtomPtr atom);

  MP4AtomPtr dataReference;
  MP4LinkedList atomList;
} MP4DataInformationAtom, *MP4DataInformationAtomPtr;

#define COMMON_DATAENTRY_ATOM_FIELDS                                                              \
  MP4Err (*getOffset)(struct MP4DataEntryAtom * self, u64 * outOffset);                           \
  MP4Err (*addSampleReference)(struct MP4DataEntryAtom * self, u64 dataOffset, MP4Handle sizesH); \
  MP4Err (*addSamples)(struct MP4DataEntryAtom * self, MP4Handle sampleH);                        \
                                                                                                  \
  MP4AtomPtr mdat;                                                                                \
  u64 offset;                                                                                     \
  u32 locationLength;                                                                             \
  char *location;

typedef struct MP4DataEntryAtom
{
  MP4_FULL_ATOM
  COMMON_DATAENTRY_ATOM_FIELDS
} MP4DataEntryAtom, *MP4DataEntryAtomPtr;

typedef struct MP4DataEntryURLAtom
{
  MP4_FULL_ATOM
  COMMON_DATAENTRY_ATOM_FIELDS
} MP4DataEntryURLAtom, *MP4DataEntryURLAtomPtr;

typedef struct MP4DataEntryURNAtom
{
  MP4_FULL_ATOM
  COMMON_DATAENTRY_ATOM_FIELDS
  u32 nameLength;
  char *nameURN;
} MP4DataEntryURNAtom, *MP4DataEntryURNAtomPtr;

typedef struct MP4DataReferenceAtom
{
  MP4_FULL_ATOM
  MP4Err (*addDataEntry)(struct MP4DataReferenceAtom *self, MP4AtomPtr entry);
  MP4Err (*getOffset)(struct MP4DataReferenceAtom *self, u32 dataReferenceIndex, u64 *outOffset);
  MP4Err (*addSampleReference)(struct MP4DataReferenceAtom *self, u32 sampleCount,
                               u32 dataReferenceIndex, u64 dataOffset, MP4Handle sizesH);
  MP4Err (*addSamples)(struct MP4DataReferenceAtom *self, u32 sampleCount, u32 dataReferenceIndex,
                       MP4Handle sampleH);
  u32 (*getEntryCount)(struct MP4DataReferenceAtom *self);
  MP4Err (*getEntry)(struct MP4DataReferenceAtom *self, u32 dataReferenceIndex,
                     struct MP4DataEntryAtom **outEntry);
  MP4LinkedList atomList;

} MP4DataReferenceAtom, *MP4DataReferenceAtomPtr;

typedef struct MP4SampleAuxiliaryInformationSizesAtom
{
  MP4_FULL_ATOM
  u32 aux_info_type;
  u32 aux_info_type_parameter;

  u8 default_sample_info_size;
  u32 sample_count;
  u8 *sample_info_sizes;

  u32 totalSize;

  MP4Err (*addSizes)(MP4AtomPtr s, u32 sampleCount, MP4Handle sizesH);
  MP4Err (*mergeSizes)(MP4AtomPtr s, MP4AtomPtr otherSaiz);
} MP4SampleAuxiliaryInformationSizesAtom, *MP4SampleAuxiliaryInformationSizesAtomPtr;

typedef struct MP4SampleAuxiliaryInformationOffsetsAtom
{
  MP4_FULL_ATOM
  u32 aux_info_type;
  u32 aux_info_type_parameter;

  u32 entry_count;
  u64 *offsets;

  u64 additionalOffset;
  u64 totalOffset;
  MP4Err (*addOffsets)(MP4AtomPtr s, u32 entryCount, MP4Handle sizesH);
  MP4Err (*mdatMoved)(MP4AtomPtr s, u64 mdatBase, u64 mdatEnd, s32 mdatOffset);
  MP4Err (*mergeOffsets)(MP4AtomPtr s, MP4AtomPtr otherSaio, u64 baseOffset);
} MP4SampleAuxiliaryInformationOffsetsAtom, *MP4SampleAuxiliaryInformationOffsetsAtomPtr;

typedef struct MP4SampleTableAtom
{
  MP4_BASE_ATOM
  MP4Err (*setupNew)(struct MP4SampleTableAtom *self);
  MP4Err (*calculateDuration)(struct MP4SampleTableAtom *self, u32 *outDuration);
  MP4Err (*setSampleEntry)(struct MP4SampleTableAtom *self, MP4AtomPtr entry);
  MP4Err (*getCurrentDataReferenceIndex)(struct MP4SampleTableAtom *self,
                                         u32 *outDataReferenceIndex);
  MP4Err (*extendLastSampleDuration)(struct MP4SampleTableAtom *self, u32 duration);
  MP4Err (*addSamples)(struct MP4SampleTableAtom *self, u32 sampleCount, u64 sampleOffset,
                       MP4Handle durationsH, MP4Handle sizesH, MP4Handle compositionOffsetsH,
                       MP4Handle syncSamplesH, MP4Handle padsH);
  MP4Err (*setfieldsize)(struct MP4SampleTableAtom *self, u32 fieldsize);
  u32 (*getCurrentSampleEntryIndex)(struct MP4SampleTableAtom *self);
  MP4Err (*setDefaultSampleEntry)(struct MP4SampleTableAtom *self, u32 index);
  MP4Err (*addGroupDescription)(struct MP4SampleTableAtom *self, u32 theType,
                                MP4Handle theDescription, u32 *index);
  MP4Err (*changeSamplestoGroupType)(struct MP4SampleTableAtom *self,
                                     sampleToGroupType_t sampleToGroupType);
  MP4Err (*mapSamplestoGroup)(struct MP4SampleTableAtom *self, u32 groupType, u32 group_index,
                              s32 sample_index, u32 count, sampleToGroupType_t sampleToGroupType);
  MP4Err (*getSampleGroupMap)(struct MP4SampleTableAtom *self, u32 groupType, u32 sample_number,
                              u32 *group_index);
  MP4Err (*getSampleGroupSampleNumbers)(struct MP4SampleTableAtom *self, u32 groupType,
                                        u32 groupIndex, u32 **outSampleNumbers, u32 *outSampleCnt);
  MP4Err (*getGroupDescription)(struct MP4SampleTableAtom *self, u32 theType, u32 index,
                                MP4Handle theDescription);

  MP4Err (*setSampleDependency)(struct MP4SampleTableAtom *self, s32 sample_index,
                                MP4Handle dependencies);
  MP4Err (*getSampleDependency)(struct MP4SampleTableAtom *self, u32 sampleNumber, u8 *dependency);

  MP4Err (*getSampleAuxiliaryInformation)(struct MP4SampleTableAtom *self,
                                          u8 isUsingAuxInfoPropertiesFlag, u32 aux_info_type,
                                          u32 aux_info_type_parameter,
                                          MP4SampleAuxiliaryInformationSizesAtomPtr *saizOut,
                                          MP4SampleAuxiliaryInformationOffsetsAtomPtr *saioOut);

  MP4Err (*mergeSampleGroupDescriptions)(struct MP4SampleTableAtom *self,
                                         MP4AtomPtr otherSampleGroupDescr);

  MP4AtomPtr TimeToSample;
  MP4AtomPtr CompositionOffset;
  MP4AtomPtr CompositionToDecode;
  MP4AtomPtr SyncSample;
  MP4AtomPtr SampleDescription;
  MP4AtomPtr SampleSize;
  MP4AtomPtr SampleToChunk;
  MP4AtomPtr ChunkOffset;
  MP4AtomPtr ShadowSync;
  MP4AtomPtr DegradationPriority;
  MP4AtomPtr PaddingBits;
  MP4AtomPtr SampleDependency;

  MP4LinkedList SampleAuxiliaryInformationSizes;
  MP4LinkedList SampleAuxiliaryInformationOffsets;

  MP4LinkedList groupDescriptionList;
  MP4LinkedList sampletoGroupList;

  MP4LinkedList atomList;

  MP4AtomPtr currentSampleEntry;
  u32 currentSampleEntryIndex;
  u8 useSignedCompositionTimeOffsets;
} MP4SampleTableAtom, *MP4SampleTableAtomPtr;

typedef struct MP4TimeToSampleAtom
{
  MP4_FULL_ATOM
  MP4Err (*getTimeForSampleNumber)(MP4AtomPtr self, u32 sampleNumber, u64 *outSampleCTS,
                                   s32 *outSampleDuration);
  MP4Err (*findSamples)(MP4AtomPtr self, u64 desiredTime, s64 *outPriorSample, s64 *outExactSample,
                        s64 *outNextSample, u32 *outSampleNumber, s32 *outSampleDuration);
  MP4Err (*getTotalDuration)(struct MP4TimeToSampleAtom *self, u32 *outDuration);
  MP4Err (*addSamples)(struct MP4TimeToSampleAtom *self, u32 sampleCount, MP4Handle durationsH);
  MP4Err (*extendLastSampleDuration)(struct MP4TimeToSampleAtom *self, u32 duration);

  MP4LinkedList entryList;
  void *currentEntry;
  void *foundEntry;
  u32 foundEntryNumber;
  u32 foundEntrySampleNumber;
  u64 foundEntryTime;
} MP4TimeToSampleAtom, *MP4TimeToSampleAtomPtr;

typedef struct MP4CompositionOffsetAtom
{
  MP4_FULL_ATOM
  MP4Err (*addSamples)(struct MP4CompositionOffsetAtom *self, u32 sampleNumber, u32 sampleCount,
                       MP4Handle offsetsH);
  MP4Err (*getOffsetForSampleNumber)(MP4AtomPtr self, u32 sampleNumber, s32 *outOffset);
  MP4LinkedList entryList;
  void *currentEntry;
  u32 finalSampleNumber;
} MP4CompositionOffsetAtom, *MP4CompositionOffsetAtomPtr;

typedef struct MP4CompositionToDecodeAtom
{
  MP4_FULL_ATOM
  s32 compositionToDTSShift;
  s32 leastDecodeToDisplayDelta;
  s32 greatestDecodeToDisplayDelta;
  s32 compositionStartTime;
  s32 compositionEndTime;
  MP4Err (*updateFields)(struct MP4Atom *s, u32 sampleCount, MP4Handle durationsH,
                         MP4Handle compositionOffsetsH);
  s64 totalDuration;
} MP4CompositionToDecodeAtom, *MP4CompositionToDecodeAtomPtr;

#define COMMON_SAMPLE_ENTRY_FIELDS \
  u32 dataReferenceIndex;          \
  char reserved[6];                \
  MP4LinkedList ExtensionAtomList;

typedef struct GenericSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
} GenericSampleEntryAtom, *GenericSampleEntryAtomPtr;

typedef struct MP4GenericSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  char *data;
  u32 dataSize;
} MP4GenericSampleEntryAtom, *MP4GenericSampleEntryAtomPtr;

typedef struct MP4MPEGSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
} MP4MPEGSampleEntryAtom, *MP4MPEGSampleEntryAtomPtr;

typedef struct MP4VisualSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  char reserved2[16]; /* uint(32)[4] */
  /* u32			reserved3;         uint(32) = 0x01400f0 */
  u32 width;
  u32 height;
  u32 reserved4; /* uint(32) = 0x0048000 */
  u32 reserved5; /* uint(32) = 0x0048000 */
  u32 reserved6; /* uint(32) = 0 */
  u32 reserved7; /* uint(16) = 1 */
  u32 nameLength;
  char name31[31];
  u32 reserved8; /* uint(16) = 24 */
  s32 reserved9; /* int(16) = -1 */

} MP4VisualSampleEntryAtom, *MP4VisualSampleEntryAtomPtr;

typedef struct MP4VolumetricVisualSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  u32 nameLength;
  char name31[31];
} MP4VolumetricVisualSampleEntryAtom, *MP4VolumetricVisualSampleEntryAtomPtr;

typedef struct MP4HapticSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
} MP4HapticSampleEntryAtom, *MP4HapticSampleEntryAtomPtr;

typedef struct MP4AudioSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  char reserved2[8];      /* uint(32)[2] */
  u32 reserved3;          /* uint(16) = 2 */
  u32 reserved4;          /* uint(16) = 16 */
  u32 reserved5;          /* uint(32) = 0 */
  u32 timeScale;          /* uint(16) copied from track! */
  u32 reserved6;          /* uint(16) = 0 */
  u32 qtSamplesPerPacket; /* from SoundDescriptionV1 in QTFF */
  u32 qtbytesPerPacket;   /* from SoundDescriptionV1 in QTFF */
  u32 qtbytesPerFrame;    /* from SoundDescriptionV1 in QTFF */
  u32 qtbytesPerSample;   /* from SoundDescriptionV1 in QTFF */

} MP4AudioSampleEntryAtom, *MP4AudioSampleEntryAtomPtr;

typedef struct MP4MetadataSetupBox
{
  MP4_BASE_ATOM
  MP4Handle setup_data;
} MP4MetadataSetupBox, *MP4MetadataSetupBoxPtr;

typedef struct MP4MetadataLocaleBox
{
  MP4_BASE_ATOM
  char *locale_string;
} MP4MetadataLocaleBox, *MP4MetadataLocaleBoxPtr;

typedef struct MP4MetadataKeyDeclarationBox
{
  MP4_BASE_ATOM
  u32 key_namespace;
  MP4Handle key_value;
} MP4MetadataKeyDeclarationBox, *MP4MetadataKeyDeclarationBoxPtr;

typedef struct MP4MetadataKeyBox
{
  MP4_BASE_ATOM
  MP4Err (*addAtom)(struct MP4MetadataKeyBox *self, MP4AtomPtr atom);
  MP4MetadataKeyDeclarationBoxPtr keyDeclarationBox;
  MP4MetadataLocaleBoxPtr localeBox;
  MP4MetadataSetupBoxPtr setupBox;
  MP4LinkedList atomList;
} MP4MetadataKeyBox, *MP4MetadataKeyBoxPtr;

typedef struct MP4MetadataKeyTableBox
{
  MP4_BASE_ATOM
  MP4MetadataKeyBoxPtr (*getMetadataKeyBox)(struct MP4MetadataKeyTableBox *self, u32 local_key_id);
  MP4Err (*addMetaDataKeyBox)(struct MP4MetadataKeyTableBox *self, MP4AtomPtr atom);
  MP4LinkedList metadataKeyBoxList;
} MP4MetadataKeyTableBox, *MP4MetadataKeyTableBoxPtr;

typedef struct MP4BoxedMetadataSampleEntry
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  MP4Err (*addAtom)(struct MP4BoxedMetadataSampleEntry *self, MP4AtomPtr atom);
  MP4MetadataKeyTableBoxPtr keyTable;
} MP4BoxedMetadataSampleEntry, *MP4BoxedMetadataSampleEntryPtr;

typedef struct MP4PCMConfigAtom
{
  MP4_FULL_ATOM
  u8 format_flags;    /* uint(8) */
  u8 PCM_sample_size; /* uint(8) */
} MP4PCMConfigAtom, *MP4PCMConfigAtomPtr;

typedef struct MP4ChannelLayoutDefinedLayout
{
  u8 speaker_position; /* uint(8) */
  s16 azimuth;         /* sint(16) */
  s8 elevation;        /* sint(8) */
} MP4ChannelLayoutDefinedLayout;

typedef struct MP4ChannelLayoutAtom
{
  MP4_FULL_ATOM
  /* format in Version 0, version 1 */
  u16 channelCount;    /*    (0)               comes from the sample entry */
  u8 stream_structure; /* uint(8),  uint(4)  */
  u8 definedLayout;    /* int(8),   int(8)   */
  MP4LinkedList definedLayouts;
  u64 omittedChannelsMap; /* uint(64), uint(64) */
  u8 object_count;        /* uint(8),  uint(8)  */
  /* for version > 0 */
  u8 formatOrdering;         /*         , int(4)   */
  u16 baseChannelCount;      /*         , uint(8)  */
  u16 layoutChannelCount;    /*         , uint(8)  */
  u8 channelOrderDefinition; /*         , uint(3)  */
  u8 omittedChannelsPresent; /*         , unit(1)  */
} MP4ChannelLayoutAtom, *MP4ChannelLayoutAtomPtr;

typedef struct MP4DownMixInstructionsAtom
{
  MP4_FULL_ATOM
  u16 baseChannelCount;        /* comes from the sample entry */
  u8 targetLayout;             /* uint(8) */
  u8 reserved;                 /* uint(1) */
  u8 targetChannelCount;       /* uint(7) */
  u8 in_stream;                /* bit(1) */
  u8 downmix_ID;               /* uint(7) */
  u8 *bs_downmix_coefficients; /* bit(4)[targetChannelCount][baseChannelCount] */
} MP4DownMixInstructionsAtom, *MP4DownMixInstructionsAtomPtr;

typedef struct MP4LoudnessBaseMeasurement
{
  u8 method_definition;  /* uint(8) */
  u8 method_value;       /* uint(8) */
  u8 measurement_system; /* uint(4) */
  u8 reliability;        /* uint(4) */
} MP4LoudnessBaseMeasurement;

typedef struct MP4LoudnessBaseAtom
{
  MP4_FULL_ATOM
  u8 reserved;                  /* unit(3) = 0 */
  u8 downmix_ID;                /* uint(7) */
  u8 DRC_set_ID;                /* uint(6) */
  s16 bs_sample_peak_level;     /* int(12) */
  s16 bs_true_peak_level;       /* int(12) */
  u8 measurement_system_for_TP; /* uint(4) */
  u8 reliability_for_TP;        /* uint(4) */
  u8 measurement_count;         /* uint(8) */
  MP4LinkedList measurements;
} MP4LoudnessBaseAtom, *MP4LoudnessBaseAtomPtr;

typedef struct MP4LoudnessAtom
{
  MP4_BASE_ATOM
  MP4LinkedList trackLoudnessInfoList;
  MP4LinkedList albumLoudnessInfoList;
  MP4Err (*addAtom)(struct MP4LoudnessAtom *self, MP4AtomPtr atom);
  MP4Err (*serializeData)(struct MP4Atom *s, char *buffer);
  MP4Err (*getDataSize)(struct MP4Atom *s, u32 *dataSizeOut);
} MP4LoudnessAtom, *MP4LoudnessAtomPtr;

typedef struct MP4XMLMetaSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  char *content_encoding;
  char *xml_namespace;
  char *schema_location;
} MP4XMLMetaSampleEntryAtom, *MP4XMLMetaSampleEntryAtomPtr;

typedef struct MP4TextMetaSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS
  char *content_encoding;
  char *mime_format;
} MP4TextMetaSampleEntryAtom, *MP4TextMetaSampleEntryAtomPtr;

typedef struct MP4AMRSpecificInfoAtom
{
  MP4_BASE_ATOM
  u32 vendor;             /* uint(32) */
  u32 decoder_version;    /* uint(8)  */
  u32 mode_set;           /* uint(16) */
  u32 mode_change_period; /* uint(8)  */
  u32 frames_per_sample;  /* uint(8)  */

} MP4AMRSpecificInfoAtom, *MP4AMRSpecificInfoAtomPtr;

typedef struct MP4AMRWPSpecificInfoAtom
{
  MP4_BASE_ATOM
  u32 vendor;          /* uint(32) */
  u32 decoder_version; /* uint(8)  */

} MP4AMRWPSpecificInfoAtom, *MP4AMRWPSpecificInfoAtomPtr;

typedef struct MP4H263SpecificInfoAtom
{
  MP4_BASE_ATOM
  u32 vendor;          /* uint(32) */
  u32 decoder_version; /* uint(8)  */
  u32 H263_level;      /* uint(8)  */
  u32 H263_profile;    /* uint(8)  */
  MP4LinkedList atomList;
  MP4Err (*addAtom)(struct MP4H263SpecificInfoAtom *self, MP4AtomPtr atom);

} MP4H263SpecificInfoAtom, *MP4H263SpecificInfoAtomPtr;

typedef struct MP4BitRateAtom
{
  MP4_BASE_ATOM
  u32 buffersizeDB;
  u32 avg_bitrate; /* uint(32) */
  u32 max_bitrate; /* uint(32) */

} MP4BitRateAtom, *MP4BitRateAtomPtr;

typedef struct MP4SampleDescriptionAtom
{
  MP4_FULL_ATOM

  MP4Err (*addEntry)(struct MP4SampleDescriptionAtom *self, MP4AtomPtr entry);
  u32 (*getEntryCount)(struct MP4SampleDescriptionAtom *self);
  MP4Err (*getEntry)(struct MP4SampleDescriptionAtom *self, u32 entryNumber,
                     struct GenericSampleEntryAtom **outEntry);

  MP4LinkedList atomList;
} MP4SampleDescriptionAtom, *MP4SampleDescriptionAtomPtr;

typedef struct MP4ESDAtom
{
  MP4_FULL_ATOM
  u32 descriptorLength;
  struct MP4DescriptorRecord *descriptor;
} MP4ESDAtom, *MP4ESDAtomPtr;

typedef struct ISOVCConfigAtom
{
  MP4_BASE_ATOM
  MP4Err (*addParameterSet)(struct ISOVCConfigAtom *self, MP4Handle ps, u32 where);
  MP4Err (*getParameterSet)(struct ISOVCConfigAtom *self, MP4Handle ps, u32 where, u32 index);
  u32 profile;
  u32 profile_compatibility;
  u32 level;
  u32 length_size;
  u32 complete_rep;
  MP4LinkedList spsList;
  MP4LinkedList ppsList;
  u32 chroma_format;
  u32 bit_depth_luma_minus8;
  u32 bit_depth_chroma_minus8;
  MP4LinkedList spsextList;
} ISOVCConfigAtom, *ISOVCConfigAtomPtr;

typedef struct ISOHEVCConfigAtom
{
  MP4_BASE_ATOM
  MP4Err (*addParameterSet)(struct ISOHEVCConfigAtom *self, MP4Handle ps, u32 where);
  MP4Err (*getParameterSet)(struct ISOHEVCConfigAtom *self, MP4Handle ps, u32 where, u32 index);
  u32 general_profile_idc;
  u32 general_profile_compatibility_flags;
  u32 general_level_idc;
  u32 lengthSizeMinusOne;
  u32 complete_rep;
  u32 numOfArrays;
  struct
  {
    u32 array_completeness;
    u32 NALtype;
    MP4LinkedList nalList;
  } arrays[8];
  u32 chromaFormat;
  u32 avgFrameRate;
  u32 sps_temporal_id_nesting_flag;
  u32 bitDepthLumaMinus8;
  u32 bitDepthChromaMinus8;
} ISOHEVCConfigAtom, *ISOHEVCConfigAtomPtr;

typedef struct ISOVVCConfigAtom
{
  MP4_FULL_ATOM
  MP4Err (*addParameterSet)(struct ISOVVCConfigAtom *self, MP4Handle ps, u32 where);
  MP4Err (*getParameterSet)(struct ISOVVCConfigAtom *self, MP4Handle ps, u32 where, u32 index);

  u32 LengthSizeMinusOne;
  u32 ptl_present_flag;
  /* opi_ols_idx */
  u32 ols_idx;
  u32 num_sublayers; /* 3 bits (max value = 7) */
  u32 constant_frame_rate;
  /* one layer ? sps_chroma_format_idc : vps_ols_dpb_chroma_format[ MultiLayerOlsIdx[ ols_idx ] ] */
  u32 chroma_format_idc;
  /* one layer ? sps_bitdepth_minus8 : vps_ols_dpb_bitdepth_minus8[ MultiLayerOlsIdx[ ols_idx ] ] */
  u32 bit_depth_minus8;

  /* Profile Tile Level */
  struct PTL
  {
    u32 num_bytes_constraint_info;
    u32 general_profile_idc;
    u32 general_tier_flag;
    u32 general_level_idc;
    u32 ptl_frame_only_constraint_flag;
    u32 ptl_multi_layer_enabled_flag;

    u32 general_constraint_info_upper;
    MP4Handle general_constraint_info_lower;

    struct SubPTL
    {
      u32 ptl_sublayer_level_present_flag;
      u32 sublayer_level_idc;
    } subPTL[8];

    u32 ptl_num_sub_profiles;
    u32 *general_sub_profile_idc;
  } native_ptl;

  u32 max_picture_width;
  u32 max_picture_height;
  u32 avg_frame_rate;

  u32 num_of_arrays;
  struct
  {
    u32 array_completeness;
    u32 NAL_unit_type;
    MP4LinkedList nalList;
  } arrays[7];

} ISOVVCConfigAtom, *ISOVVCConfigAtomPtr;

typedef struct ISOVVCNALUConfigAtom
{
  MP4_FULL_ATOM

  u32 LengthSizeMinusOne;
} ISOVVCNALUConfigAtom, *ISOVVCNALUConfigAtomPtr;

typedef struct MP4SampleSizeAtom
{
  MP4_FULL_ATOM
  MP4Err (*getSampleSize)(MP4AtomPtr self, u32 sampleNumber, u32 *outSize);
  MP4Err (*getSampleSizeAndOffset)(MP4AtomPtr self, u32 sampleNumber, u32 *outSize,
                                   u32 startingSampleNumber, u32 *outOffsetSize);
  MP4Err (*addSamples)(struct MP4SampleSizeAtom *self, u32 sampleCount, MP4Handle sizesH);
  MP4Err (*setfieldsize)(struct MP4SampleSizeAtom *self, u32 fieldsize);

  u32 sampleSize;
  u32 sampleCount;
  u32 fieldsize;
  u32 allocatedSize;
  u32 *sizes;
} MP4SampleSizeAtom, *MP4SampleSizeAtomPtr;

typedef MP4SampleSizeAtom MP4CompactSampleSizeAtom;
typedef MP4SampleSizeAtomPtr MP4CompactSampleSizeAtomPtr;

typedef struct MP4PaddingBitsAtom
{
  MP4_FULL_ATOM
  MP4Err (*getSamplePad)(MP4AtomPtr self, u32 sampleNumber, u8 *outPad);
  MP4Err (*addSamplePads)(struct MP4PaddingBitsAtom *self, u32 sampleCount, MP4Handle padsH);

  u32 sampleCount;
  u8 *pads;
} MP4PaddingBitsAtom, *MP4PaddingBitsAtomPtr;

#define COMMON_CHUNK_OFFSET_FIELDS                                                     \
  u32 (*getChunkCount)(MP4AtomPtr self);                                               \
  MP4Err (*getChunkOffset)(MP4AtomPtr self, u32 chunkIndex, u64 * outOffset);          \
  MP4Err (*addOffset)(struct MP4ChunkOffsetAtom * self, u64 offset);                   \
  MP4Err (*mdatMoved)(struct MP4ChunkOffsetAtom * self, u32 firstchunk, u32 lastchunk, \
                      u64 mdatBase, u64 mdatEnd, s32 mdatOffset);                      \
  u32 entryCount;

typedef struct MP4ChunkOffsetAtom
{
  MP4_FULL_ATOM
  COMMON_CHUNK_OFFSET_FIELDS

  u32 allocatedSize;
  u64 *offsets;
} MP4ChunkOffsetAtom, *MP4ChunkOffsetAtomPtr;

/* typedef struct MP4ChunkLargeOffsetAtom
{
        MP4_FULL_ATOM
        COMMON_CHUNK_OFFSET_FIELDS

        u64 *offsets;
} MP4ChunkLargeOffsetAtom, *MP4ChunkLargeOffsetAtomPtr;
*/

typedef struct MP4SampleToChunkAtom
{
  MP4_FULL_ATOM
  MP4Err (*lookupSample)(MP4AtomPtr self, u32 sampleNumber, u32 *outChunkNumber,
                         u32 *outSampleDescriptionIndex, u32 *outFirstSampleNumberInChunk);
  MP4Err (*setEntry)(struct MP4SampleToChunkAtom *self, u32 firstChunkNumber, u32 sampleCount,
                     u32 sampleDescriptionIndex);
  u32 (*getEntryCount)(struct MP4SampleToChunkAtom *self);
  MP4Err (*mdatMoved)(struct MP4SampleToChunkAtom *self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset,
                      struct MP4SampleDescriptionAtom *stsd, struct MP4DataReferenceAtom *dref,
                      struct MP4ChunkOffsetAtom *stco);
  MP4LinkedList entryList;

  u32 foundEntryNumber;
  u32 foundEntryFirstSampleNumber;

} MP4SampleToChunkAtom, *MP4SampleToChunkAtomPtr;

typedef struct MP4SubSampleInformationAtom
{
  MP4_FULL_ATOM
  MP4Err (*addEntry)(struct MP4SubSampleInformationAtom *self, u32 sample_delta,
                     u32 subsample_count, MP4Handle subsample_size_array,
                     MP4Handle subsample_priority_array, MP4Handle subsample_discardable_array);
  MP4Err (*addEntry2)(struct MP4SubSampleInformationAtom *self, u32 sample_delta,
                      u32 subsample_count, MP4Handle subsample_size_array,
                      MP4Handle subsample_priority_array, MP4Handle subsample_discardable_array,
                      MP4Handle codec_specific_parameters_array);
  u32 entry_count;
  u32 *sample_delta;
  u32 *subsample_count;
  u32 **subsample_size;
  u32 **subsample_priority;
  u32 **discardable;
  u32 **codec_specific_parameters;
} MP4SubSampleInformationAtom, *MP4SubSampleInformationAtomPtr;

typedef struct MP4SyncSampleAtom
{
  MP4_FULL_ATOM
  MP4Err (*addSamples)(struct MP4SyncSampleAtom *self, u32 beginningSampleNumber, u32 sampleCount,
                       MP4Handle syncSamplesH);
  MP4Err (*isSyncSample)(MP4AtomPtr self, u32 sampleNumber, u32 *outSync);
  u32 entryCount;
  u32 *sampleNumbers;
  u32 nonSyncFlag;
  u32 allocatedSize;
} MP4SyncSampleAtom, *MP4SyncSampleAtomPtr;

typedef struct MP4ShadowSyncAtom
{
  MP4_FULL_ATOM
  u32 entryCount;
  void *entries;
} MP4ShadowSyncAtom, *MP4ShadowSyncAtomPtr;

typedef struct MP4DegradationPriorityAtom
{
  MP4_FULL_ATOM
  u32 entryCount;
  u16 *priorities;
} MP4DegradationPriorityAtom, *MP4DegradationPriorityAtomPtr;

typedef struct MP4FreeSpaceAtom
{
  MP4_BASE_ATOM
  char *data;
  u32 dataSize;
} MP4FreeSpaceAtom, *MP4FreeSpaceAtomPtr;

typedef struct MP4EditAtom
{
  MP4_BASE_ATOM
  MP4Err (*addAtom)(struct MP4EditAtom *self, MP4AtomPtr atom);
  MP4Err (*getEffectiveDuration)(struct MP4EditAtom *self, u32 *outDuration);

  MP4LinkedList atomList;
  MP4AtomPtr editListAtom;
} MP4EditAtom, *MP4EditAtomPtr;

typedef struct MP4EditListAtom
{
  MP4_FULL_ATOM

  MP4Err (*setTrackOffset)(struct MP4EditListAtom *self, u32 trackStartTime, u64 trackDuration);
  MP4Err (*getTrackOffset)(struct MP4EditListAtom *self, u32 *outTrackStartTime);

  MP4Err (*isEmptyEdit)(struct MP4EditListAtom *self, u32 segmentNumber, u32 *outIsEmpty);

  MP4Err (*insertSegment)(struct MP4EditListAtom *self, s32 trackStartTime, s32 mediaStartTime,
                          u64 segmentDuration, u32 mediaRate);

  MP4Err (*getEffectiveDuration)(struct MP4EditListAtom *self, u32 *outDuration);
  MP4Err (*getIndSegmentTime)(MP4AtomPtr self, u32 segmentIndex, /* one based */
                              u64 *outSegmentMovieTime, s64 *outSegmentMediaTime,
                              u64 *outSegmentDuration /* in movie's time scale */
  );
  MP4Err (*getTimeAndRate)(MP4AtomPtr self, u64 movieTime, u32 movieTimeScale, u32 mediaTimeScale,
                           s64 *outMediaTime, u32 *outMediaRate, u64 *outPriorSegmentEndTime,
                           u64 *outNextSegmentBeginTime);
  u32 (*getEntryCount)(struct MP4EditListAtom *self);
  MP4LinkedList entryList;
} MP4EditListAtom, *MP4EditListAtomPtr;

typedef struct MP4UserDataAtom
{
  MP4_BASE_ATOM
  MP4Err (*addUserData)(struct MP4UserDataAtom *self, MP4Handle userDataH, u32 userDataType,
                        u32 *outIndex);
  MP4Err (*getEntryCount)(struct MP4UserDataAtom *self, u32 userDataType, u32 *outCount);
  MP4Err (*getIndType)(struct MP4UserDataAtom *self, u32 typeIndex, u32 *outType);
  MP4Err (*getItem)(struct MP4UserDataAtom *self, MP4Handle userDataH, u32 userDataType,
                    u32 itemIndex);
  MP4Err (*getAtom)(struct MP4UserDataAtom *self, MP4AtomPtr *outatom, u32 userDataType,
                    u32 itemIndex);
  MP4Err (*deleteItem)(struct MP4UserDataAtom *self, u32 userDataType, u32 itemIndex);
  MP4Err (*getTypeCount)(struct MP4UserDataAtom *self, u32 *outCount);
  MP4LinkedList recordList;
} MP4UserDataAtom, *MP4UserDataAtomPtr;

typedef struct MP4CopyrightAtom
{
  MP4_FULL_ATOM
  u32 packedLanguageCode;
  char *notice;
} MP4CopyrightAtom, *MP4CopyrightAtomPtr;

typedef struct MP4TrackReferenceTypeAtom
{
  MP4_BASE_ATOM
  MP4Err (*addTrackID)(struct MP4TrackReferenceTypeAtom *self, u32 trackID);
  u32 trackIDCount;
  u32 *trackIDs;
} MP4TrackReferenceTypeAtom, *MP4TrackReferenceTypeAtomPtr;

typedef struct MP4TrackGroupTypeAtom
{
  MP4_FULL_ATOM
  MP4Err (*setGroupID)(struct MP4TrackGroupTypeAtom *self, u32 track_group_id);
  u32 track_group_id;
} MP4TrackGroupTypeAtom, *MP4TrackGroupTypeAtomPtr;

/* track fragment stuff */

enum
{
  fragment_difference_sample_flag = 0x10000
};

typedef struct MP4MovieFragmentAtom
{
  MP4_BASE_ATOM
  COMMON_MOVIE_ATOM_FIELDS

  MP4Err (*addAtom)(struct MP4MovieFragmentAtom *self, MP4AtomPtr atom);
  MP4Err (*mergeFragments)(struct MP4MovieFragmentAtom *self, MP4MovieAtomPtr moov);
  MP4AtomPtr mfhd; /* the movie fragment header */
  MP4PrivateMovieRecordPtr moov;
  MP4LinkedList atomList; /* the track fragments */
} MP4MovieFragmentAtom, *MP4MovieFragmentAtomPtr;

typedef struct MP4MovieFragmentHeaderAtom
{
  MP4_FULL_ATOM
  u32 sequence_number;
} MP4MovieFragmentHeaderAtom, *MP4MovieFragmentHeaderAtomPtr;

typedef struct MP4MovieExtendsAtom
{
  MP4_FULL_ATOM
  MP4LinkedList trackExtendsList;
  MP4LinkedList trackExtensionPropertiesList;
  MP4Err (*addAtom)(struct MP4MovieExtendsAtom *self, MP4AtomPtr atom);
  MP4Err (*maketrackfragments)(struct MP4MovieExtendsAtom *self, MP4MovieFragmentAtomPtr moof,
                               MP4MovieAtomPtr moov, MP4MediaDataAtomPtr mdat);
  MP4Err (*getTrackExtendsAtom)(struct MP4MovieExtendsAtom *self, u32 trackID,
                                MP4AtomPtr *outTrack);
  MP4Err (*getTrackExtensionPropertiesAtom)(struct MP4MovieExtendsAtom *self, u32 trackID,
                                            MP4AtomPtr *outProb);
  MP4Err (*setSampleDescriptionIndexes)(struct MP4MovieExtendsAtom *self, MP4AtomPtr moov);
  MP4Err (*setCompositionToDecodeProperties)(struct MP4MovieExtendsAtom *self, u32 trackID,
                                             s32 compositionToDTSShift,
                                             s32 leastDecodeToDisplayDelta,
                                             s32 greatestDecodeToDisplayDelta,
                                             s32 compositionStartTime, s32 compositionEndTime);
} MP4MovieExtendsAtom, *MP4MovieExtendsAtomPtr;

typedef struct MP4TrackExtensionPropertiesAtom
{
  MP4_FULL_ATOM
  u32 track_id;
  MP4LinkedList atomList;
  MP4Err (*addAtom)(struct MP4TrackExtensionPropertiesAtom *self, MP4AtomPtr atom);
  MP4Err (*getAtom)(struct MP4TrackExtensionPropertiesAtom *self, u32 atomType,
                    MP4AtomPtr *outAtom);
} MP4TrackExtensionPropertiesAtom, *MP4TrackExtensionPropertiesAtomPtr;

typedef struct MP4TrackExtendsAtom
{
  MP4_FULL_ATOM
  u32 trackID;
  u8 isInitialMediaDecodeTimeAdded;
  u32 baseMediaDecodeTime;
  u32 default_sample_description_index;
  u32 default_sample_duration;
  u32 default_sample_size;
  u32 default_sample_flags;
} MP4TrackExtendsAtom, *MP4TrackExtendsAtomPtr;

typedef struct MP4TrackFragmentHeaderAtom
{
  MP4_FULL_ATOM
  u32 trackID;
  u64 base_data_offset;
  u32 sample_description_index;
  u32 default_sample_duration;
  u32 default_sample_size;
  u32 default_sample_flags;
} MP4TrackFragmentHeaderAtom, *MP4TrackFragmentHeaderAtomPtr;

typedef struct MP4TrackFragmentAtom
{
  MP4_FULL_ATOM
  COMMON_MINF_ATOM_FIELDS
  MP4AtomPtr tfhd;
  MP4AtomPtr tfdt;
  MP4AtomPtr trex;

  MP4Err (*mergeRuns)(struct MP4TrackFragmentAtom *self, MP4MediaAtomPtr mdia);
  MP4Err (*calculateDataEnd)(struct MP4TrackFragmentAtom *self, u32 *outSize);
  MP4Err (*mergeSampleAuxiliaryInformation)(struct MP4TrackFragmentAtom *self,
                                            MP4MediaAtomPtr mdia);
  MP4Err (*getSampleAuxiliaryInfoFromTrackFragment)(
    struct MP4TrackFragmentAtom *self, u8 isUsingAuxInfoPropertiesFlag, u32 aux_info_type,
    u32 aux_info_type_parameter, MP4SampleAuxiliaryInformationSizesAtomPtr *saizOut,
    MP4SampleAuxiliaryInformationOffsetsAtomPtr *saioOut);

  MP4Err (*addGroupDescription)(struct MP4TrackFragmentAtom *self, u32 groupType,
                                MP4Handle description, u32 *index);
  MP4Err (*getSampleGroupMap)(struct MP4TrackFragmentAtom *self, u32 groupType, u32 sample_number,
                              u32 *group_index);
  MP4Err (*getSampleGroupSampleNumbers)(struct MP4TrackFragmentAtom *self, u32 groupType,
                                        u32 groupIndex, u32 **outSampleNumbers, u32 *outSampleCnt);
  MP4Err (*getGroupDescription)(struct MP4TrackFragmentAtom *self, u32 theType, u32 index,
                                MP4Handle theDescription);
  u32 default_sample_description_index; /* all copied from the matching trex */
  u32 default_sample_duration;
  u32 default_sample_size;
  u32 default_sample_flags;

  MP4MediaDataAtomPtr mdat;
  u32 samples_use_mdat; /* 0 -- not yet decided, 1=yes, 2=no */
  u8 useSignedCompositionTimeOffsets;

  MP4LinkedList atomList;             /* track runs */
  MP4LinkedList groupDescriptionList; /* sample group description list */
  MP4LinkedList sampletoGroupList;    /* sample to group maps */
  MP4LinkedList saizList;
  MP4LinkedList saioList;
} MP4TrackFragmentAtom, *MP4TrackFragmentAtomPtr;

enum
{
  tfhd_base_data_offset_present         = 0x01,
  tfhd_sample_description_index_present = 0x02,
  tfhd_default_sample_duration_present  = 0x08,
  tfhd_default_sample_size_present      = 0x10,
  tfhd_default_sample_flags_present     = 0x20,
  tfhd_duration_is_empty                = 0x10000,
  tfhd_default_base_is_moof             = 0x20000
};

typedef struct MP4TrackRunEntry
{
  u32 sample_duration;
  u32 sample_size;
  u32 sample_flags;
  u32 sample_composition_time_offset;
} MP4TrackRunEntry, *MP4TrackRunEntryPtr;

enum
{
  trun_data_offset_present              = 0x01,
  trun_first_sample_flags_present       = 0x04,
  trun_sample_duration_present          = 0x100,
  trun_sample_size_present              = 0x200,
  trun_sample_flags_present             = 0x400,
  trun_sample_composition_times_present = 0x800
};

#define trun_all_sample_flags                                                            \
  (trun_sample_duration_present + trun_sample_size_present + trun_sample_flags_present + \
   trun_sample_composition_times_present)

typedef struct MP4TrackFragmentDecodeTimeAtom
{
  MP4_FULL_ATOM
  u64 baseMediaDecodeTime;
} MP4TrackFragmentDecodeTimeAtom, *MP4TrackFragmentDecodeTimeAtomPtr;

typedef struct MP4ItemPropertyContainerAtom
{
  MP4_BASE_ATOM
  MP4LinkedList atomList;

  ISOErr (*addAtom)(struct MP4ItemPropertyContainerAtom *self, MP4AtomPtr atom);
} MP4ItemPropertyContainerAtom, *MP4ItemPropertyContainerAtomPtr;

typedef struct MP4ItemPropertyAssociationEntryPropertyIndex
{
  u32 essential;
  u32 property_index;
} MP4ItemPropertyAssociationEntryPropertyIndex, *MP4ItemPropertyAssociationEntryPropertyIndexPtr;

typedef struct MP4ItemPropertyAssociationEntry
{
  u32 item_ID;
  MP4LinkedList propertyIndexes;
} MP4ItemPropertyAssociationEntry, *MP4ItemPropertyAssociationEntryPtr;

typedef struct MP4ItemPropertyAssociationAtom
{
  MP4_FULL_ATOM
  MP4LinkedList entries;

  ISOErr (*addEntry)(struct MP4ItemPropertyAssociationAtom *self,
                     MP4ItemPropertyAssociationEntryPtr entry);
} MP4ItemPropertyAssociationAtom, *MP4ItemPropertyAssociationAtomPtr;

typedef struct MP4ItemPropertiesAtom
{
  MP4_BASE_ATOM
  MP4ItemPropertyContainerAtomPtr ipco;
  MP4ItemPropertyAssociationAtomPtr ipma;
  MP4LinkedList atomList;

  ISOErr (*addAtom)(struct MP4ItemPropertiesAtom *self, MP4AtomPtr atom);
  ISOErr (*addItemProperty)(struct MP4ItemPropertiesAtom *self, MP4AtomPtr itemProperty, u32 itemID,
                            u8 essential);
  ISOErr (*getPropertiesOfItem)(struct MP4ItemPropertiesAtom *self, u32 itemID,
                                MP4LinkedList *properties);
} MP4ItemPropertiesAtom, *MP4ItemPropertiesAtomPtr;

typedef struct MP4TrackRunAtom
{
  MP4_FULL_ATOM
  u32 samplecount;
  s32 data_offset;
  u32 first_sample_flags;
  MP4TrackRunEntryPtr entries;

  void (*calculateDefaults)(struct MP4TrackRunAtom *self, MP4TrackFragmentHeaderAtomPtr tfhd,
                            u32 flags_index);
  void (*setFlags)(struct MP4TrackRunAtom *self, MP4TrackFragmentHeaderAtomPtr tfhd);
} MP4TrackRunAtom, *MP4TrackRunAtomPtr;

typedef struct ISOFileTypeAtom
{
  MP4_BASE_ATOM

  ISOErr (*addStandard)(struct ISOFileTypeAtom *self, u32 standard);
  ISOErr (*setBrand)(struct ISOFileTypeAtom *self, u32 standard, u32 minorversion);
  ISOErr (*getBrand)(struct ISOFileTypeAtom *self, u32 *standard, u32 *minorversion);
  u32 (*getStandard)(struct ISOFileTypeAtom *self, u32 standard);
  u32 brand;              /* the brand of this file */
  u32 minorVersion;       /* the minor version of this file */
  u32 itemCount;          /* the number of items in the compatibility list */
  u32 *compatibilityList; /* standards this file conforms to */
} ISOFileTypeAtom, *ISOFileTypeAtomPtr;

typedef struct MP4TrackTypeAtom
{
  MP4_FULL_ATOM

  ISOErr (*addStandard)(struct MP4TrackTypeAtom *self, u32 standard);
  ISOErr (*setBrand)(struct MP4TrackTypeAtom *self, u32 standard, u32 minorversion);
  ISOErr (*getBrand)(struct MP4TrackTypeAtom *self, u32 *standard, u32 *minorversion);
  u32 (*getStandard)(struct MP4TrackTypeAtom *self, u32 standard);

  u32 majorBrand;   /* the brand of this track */
  u32 minorVersion; /* the minor version of this track */
  u32 itemCount;    /* the number of items in the compatibility list */
  u32 *compatibilityList;

} MP4TrackTypeAtom, *MP4TrackTypeAtomPtr;

typedef struct MP4SchemeTypeAtom
{
  MP4_FULL_ATOM
  u32 scheme_type;
  u32 scheme_version;

  char *scheme_url; /* if (flags & 0x000001), a scheme URL is present */

} MP4SchemeTypeAtom, *MP4SchemeTypeAtomPtr;

typedef struct MP4CompatibleSchemeTypeAtom
{
  MP4_FULL_ATOM
  u32 scheme_type;
  u32 scheme_version;

  char *scheme_url;
} MP4CompatibleSchemeTypeAtom, *MP4CompatibleSchemeTypeAtomPtr;

typedef struct MP4RestrictedSchemeInfoAtom
{
  MP4_BASE_ATOM
  MP4AtomPtr MP4OriginalFormat; /*			('frma') */
  MP4AtomPtr MP4SchemeType;     /*          ('schm') */
  MP4AtomPtr MP4SchemeInfo;     /* optional ('schi') */

  MP4LinkedList atomList; /* may contain one or more instances of CompatibleSchemeTypeBox */

  MP4Err (*addAtom)(struct MP4RestrictedSchemeInfoAtom *self, MP4AtomPtr atom);

} MP4RestrictedSchemeInfoAtom, *MP4RestrictedSchemeInfoAtomPtr;

typedef struct MP4StereoVideoAtom
{

  MP4_FULL_ATOM

  u32 reserved;               /* unsigned int(30) = 0 */
  u8 single_view_allowed;     /* unsigned int(2) */
  u32 stereo_scheme;          /* unsigned int(32)	*/
  u32 length;                 /* unsigned int(32) */
  u8 *stereo_indication_type; /* unsigned int(8)[length] */

  MP4LinkedList atomList; /* optional */

  MP4Err (*addAtom)(struct MP4StereoVideoAtom *self, MP4AtomPtr atom);

} MP4StereoVideoAtom, *MP4StereoVideoAtomPtr;

typedef struct MP4StereoVideoGroupAtom
{

  MP4_FULL_ATOM

  u32 trackGroupID; /* unsigned int(32), inherited from TrackGroupTypeAtom */

  u8 leftViewFlag;  /* unsigned int(1) */
  char reserved[4]; /* bit(31) = 0 */

} MP4StereoVideoGroupAtom, *MP4StereoVideoGroupAtomPtr;

typedef struct MP4RestrictedVideoSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_SAMPLE_ENTRY_FIELDS

  u32 restriction_type;

  MP4Err (*addSchemeInfoAtom)(struct MP4Atom *self, struct MP4Atom *theAtom);
  MP4Err (*getSchemeInfoAtom)(struct MP4Atom *self, u32 theType, struct MP4Atom **theAtom);
  MP4Err (*getRinf)(struct MP4Atom *self, struct MP4Atom **theAtom);
  MP4Err (*getScheme)(struct MP4Atom *self, u32 *sch_type, u32 *sch_version, char **sch_url);

  MP4Err (*transform)(struct MP4Atom *self, u32 sch_type, u32 sch_version, char *sch_url);
  MP4Err (*untransform)(struct MP4Atom *self);

  MP4Err (*addAtom)(struct MP4RestrictedVideoSampleEntryAtom *self, MP4AtomPtr atom);

  char reserved1[6];
  char reserved2[16]; /* uint(32)[4] */
  /* u32			reserved3;         uint(32) = 0x01400f0 */
  u32 width;
  u32 height;
  u32 reserved4; /* uint(32) = 0x0048000 */
  u32 reserved5; /* uint(32) = 0x0048000 */
  u32 reserved6; /* uint(32) = 0 */
  u32 reserved7; /* uint(16) = 1 */
  u32 nameLength;
  char name31[31];
  u32 reserved8; /* uint(16) = 24 */
  s32 reserved9; /* int(16) = -1 */

} MP4RestrictedVideoSampleEntryAtom, *MP4RestrictedVideoSampleEntryAtomPtr;

typedef struct MP4OriginalFormatAtom
{
  MP4_BASE_ATOM
  u32 original_format;

} MP4OriginalFormatAtom, *MP4OriginalFormatAtomPtr;

typedef struct MP4SchemeInfoAtom
{
  MP4_BASE_ATOM

  MP4LinkedList atomList; /* this atom may include a variable number of atoms */
  ISOErr (*addAtom)(struct MP4SchemeInfoAtom *self, MP4AtomPtr atom);

} MP4SchemeInfoAtom, *MP4SchemeInfoAtomPtr;

#ifdef ISMACrypt
typedef struct MP4SecurityInfoAtom
{
  MP4_BASE_ATOM
  MP4AtomPtr MP4OriginalFormat;
  MP4AtomPtr MP4SchemeType;
  MP4AtomPtr MP4SchemeInfo;

} MP4SecurityInfoAtom, *MP4SecurityInfoAtomPtr;

typedef struct ISMAKMSAtom
{
  MP4_FULL_ATOM
  char *kms_url;

} ISMAKMSAtom, *ISMAKMSAtomPtr;

typedef struct ISMASampleFormatAtom
{
  MP4_FULL_ATOM
  u32 selective_encryption;
  u32 key_indicator_len;
  u32 IV_len;

} ISMASampleFormatAtom, *ISMASampleFormatAtomPtr;

typedef struct ISMASaltAtom
{
  MP4_BASE_ATOM
  u64 salt;
} ISMASaltAtom, *ISMASaltAtomPtr;

#define COMMON_ENC_SAMPLE_ENTRY_FIELDS                                                        \
  COMMON_SAMPLE_ENTRY_FIELDS                                                                  \
  MP4AtomPtr SecurityInfo;                                                                    \
  u32 enc_type;                                                                               \
  MP4Err (*transform)(struct MP4Atom * self, u32 sch_type, u32 sch_version, char *sch_url);   \
  MP4Err (*untransform)(struct MP4Atom * self);                                               \
  MP4Err (*addSchemeInfoAtom)(struct MP4Atom * self, struct MP4Atom * theAtom);               \
  MP4Err (*getSchemeInfoAtom)(struct MP4Atom * self, u32 theType, struct MP4Atom * *theAtom); \
  MP4Err (*getScheme)(struct MP4Atom * self, u32 * sch_type, u32 * sch_version, char **sch_url);

typedef struct MP4EncBaseSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_ENC_SAMPLE_ENTRY_FIELDS
} MP4EncBaseSampleEntryAtom, *MP4EncBaseSampleEntryAtomPtr;

typedef struct MP4EncVisualSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_ENC_SAMPLE_ENTRY_FIELDS

  char reserved1[6];
  char reserved2[16]; /* uint(32)[4] */
  /* u32			reserved3;         uint(32) = 0x01400f0 */
  u32 width;
  u32 height;
  u32 reserved4; /* uint(32) = 0x0048000 */
  u32 reserved5; /* uint(32) = 0x0048000 */
  u32 reserved6; /* uint(32) = 0 */
  u32 reserved7; /* uint(16) = 1 */
  u32 nameLength;
  char name31[31];
  u32 reserved8; /* uint(16) = 24 */
  s32 reserved9; /* int(16) = -1 */
} MP4EncVisualSampleEntryAtom, *MP4EncVisualSampleEntryAtomPtr;

typedef struct MP4EncAudioSampleEntryAtom
{
  MP4_BASE_ATOM
  COMMON_ENC_SAMPLE_ENTRY_FIELDS

  char reserved1[6];
  char reserved2[8]; /* uint(32)[2] */
  u32 reserved3;     /* uint(16) = 2 */
  u32 reserved4;     /* uint(16) = 16 */
  u32 reserved5;     /* uint(32) = 0 */
  u32 timeScale;     /* uint(16) copied from track! */
  u32 reserved6;     /* uint(16) = 0 */
} MP4EncAudioSampleEntryAtom, *MP4EncAudioSampleEntryAtomPtr;

#endif

typedef struct sampleGroupEntry
{
  u32 groupSize;
  char *groupDescription;
} sampleGroupEntry;

#define COMMON_GROUP_ATOM_FIELDS \
  MP4_FULL_ATOM                  \
  u32 grouping_type;

typedef struct MP4SampleGroupDescriptionAtom
{
  COMMON_GROUP_ATOM_FIELDS

  u32 default_length; /* we only handle version 1 */

  u32 default_group_description_index; /* only after version 2 */

  u32 groupCount;

  sampleGroupEntry *groups;

  MP4Err (*addGroupDescription)(struct MP4SampleGroupDescriptionAtom *self,
                                MP4Handle theDescription, u32 *index);
  MP4Err (*getGroupDescription)(struct MP4SampleGroupDescriptionAtom *self, u32 index,
                                MP4Handle theDescription);
  MP4Err (*findGroupDescriptionIdx)(struct MP4SampleGroupDescriptionAtom *self, MP4Handle searchH,
                                    u32 *index);

} MP4SampleGroupDescriptionAtom, *MP4SampleGroupDescriptionAtomPtr;

typedef struct MP4CompactSampleToGroupPatternEntry
{
  u32 patternLength;
  u32 sampleCount;
} MP4CompactSampleToGroupPatternEntry, *MP4CompactSampleToGroupPatternEntryPtr;

typedef struct
{
  u32 patternCount;
  u32 totalSampleCount;
  u32 totalIndexDescriptionCount;
  u32 efficientStartIndex;
  u8 isSampleGroupCompressed;
  /* flags */
  u8 index_msb_indicates_fragment_local_description;
  u8 grouping_type_parameter_present;
  u8 pattern_size_code;
  u8 count_size_code;
  u8 index_size_code;

  MP4CompactSampleToGroupPatternEntryPtr patternEntries;
  u32 *indexDescriptionArray;

} CompressedGroupInfo, *CompressedGroupInfoPtr;

typedef struct MP4SampletoGroupAtom
{
  COMMON_GROUP_ATOM_FIELDS

  u32 *group_index; /* indexed by sample number;  we compact on write and expand on read */
  u32 sampleCount;
  u32 entryCount;
  u32 allocatedSize;
  u8 groupIsInFragment; /**< If set to 1 this group is located in traf, else it is in stbl */

  sampleToGroupType_t sampleToGroupType;

  /* these are required for CompactSampletoGroup */
  u32 groupingTypeParameter;
  u32 mapSampleToGroupIndex;
  CompressedGroupInfo compressedGroup;

  MP4Err (*addSamples)(struct MP4SampletoGroupAtom *self, u32 count);
  MP4Err (*mapSamplestoGroup)(struct MP4SampletoGroupAtom *self, u32 group_index, s32 sample_index,
                              u32 count);
  MP4Err (*changeSamplestoGroupType)(struct MP4SampletoGroupAtom *self,
                                     sampleToGroupType_t sampleToGroupType);
  MP4Err (*getSampleGroupMap)(struct MP4SampletoGroupAtom *self, u32 sampleNumber, u32 *groupIndex);
} MP4SampletoGroupAtom, *MP4SampletoGroupAtomPtr;

typedef struct ISOMetaAtom
{
  MP4_FULL_ATOM
  MP4Err (*addMetaItemExtentReference)(struct ISOMetaAtom *self, u32 item_ID, u64 dataOffset,
                                       MP4Handle sizesH);
  MP4Err (*addMetaItemExtent)(struct MP4MediaAtom *self, u32 item_ID, MP4Handle sampleH);
  MP4Err (*addAtom)(struct ISOMetaAtom *self, MP4AtomPtr atom);
  MP4Err (*setMdat)(struct ISOMetaAtom *self, MP4AtomPtr mdat);
  MP4Err (*mdatMoved)(struct ISOMetaAtom *self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset);
  MP4Err (*closeDataHandler)(struct ISOMetaAtom *self);
  MP4Err (*openDataHandler)(struct ISOMetaAtom *self, u32 dataEntryIndex);
  MP4Err (*getData)(struct ISOMetaAtom *self, u32 box_type, MP4Handle data, u8 is_full_atom);

  MP4AtomPtr hdlr;
  MP4AtomPtr dinf;
  MP4AtomPtr iloc;
  MP4AtomPtr iinf;
  MP4AtomPtr iref;
  MP4AtomPtr idat;
  MP4AtomPtr pitm;
  MP4AtomPtr ipro;
  MP4AtomPtr iprp;
  MP4AtomPtr grpl;

  MP4AtomPtr mdat;
  MP4LinkedList atomList;

  u16 next_item_ID;

  MP4AtomPtr relatedMeco;

  struct MP4InputStreamRecord *inputStream;
  void *dataHandler;
  s32 dataEntryIndex;
} ISOMetaAtom, *ISOMetaAtomPtr;

typedef struct ISOAdditionalMetaDataContainerAtom
{
  MP4_BASE_ATOM
  MP4LinkedList metaList;
  MP4LinkedList relationList;
  MP4Err (*addMeta)(struct ISOAdditionalMetaDataContainerAtom *self, MP4AtomPtr meta);
  MP4Err (*getMeta)(struct ISOAdditionalMetaDataContainerAtom *self, u32 type,
                    ISOMetaAtomPtr *outMetaPtr);
  MP4Err (*mdatMoved)(struct ISOAdditionalMetaDataContainerAtom *self, u64 mdatBase, u64 mdatEnd,
                      s32 mdatOffset);
  MP4Err (*setMdat)(struct ISOAdditionalMetaDataContainerAtom *self, MP4AtomPtr mdat);
} ISOAdditionalMetaDataContainerAtom, *ISOAdditionalMetaDataContainerAtomPtr;

typedef struct ISOMetaboxRelationAtom
{
  MP4_FULL_ATOM
  u32 first_metabox_handler_type;
  u32 second_metabox_handler_type;
  u8 metabox_relation;
} ISOMetaboxRelationAtom, *ISOMetaboxRelationAtomPtr;

typedef struct ISOItemDataAtom
{
  MP4_BASE_ATOM
  MP4Handle data;
  MP4Err (*getData)(MP4AtomPtr s, char *target, u32 offset, u32 length);
} ISOItemDataAtom, *ISOItemDataAtomPtr;

typedef struct ISOItemReferenceAtom
{
  MP4_FULL_ATOM
  MP4LinkedList atomList;
} ISOItemReferenceAtom, *ISOItemReferenceAtomPtr;

typedef struct ISOSingleItemTypeReferenceAtom
{
  MP4_BASE_ATOM
  u32 from_item_ID;
  u16 reference_count;
  u32 *to_item_IDs;
  u8 isLarge;
} ISOSingleItemTypeReferenceAtom, *ISOSingleItemTypeReferenceAtomPtr;

typedef struct ISOPrimaryItemAtom
{
  MP4_FULL_ATOM
  u32 item_ID;
} ISOPrimaryItemAtom, *ISOPrimaryItemAtomPtr;

typedef struct MetaExtentLocation
{
  u64 extent_index;
  u64 extent_offset;
  u64 extent_length;
} MetaExtentLocation, *MetaExtentLocationPtr;

typedef struct MetaItemLocation
{
  MP4AtomPtr meta;
  u16 item_ID;
  u8 construction_method;
  u16 dref_index;
  u64 base_offset;
  MP4LinkedList extentList;
} MetaItemLocation, *MetaItemLocationPtr;

typedef struct ISOItemLocationAtom
{
  MP4_FULL_ATOM
  MP4Err (*mdatMoved)(struct ISOItemLocationAtom *self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset);
  MP4Err (*setItemsMeta)(struct ISOItemLocationAtom *self, MP4AtomPtr meta);

  u8 offset_size;
  u8 length_size;
  u8 base_offset_size;
  u8 index_size;
  MP4LinkedList itemList;
} ISOItemLocationAtom, *ISOItemLocationAtomPtr;

typedef struct ISOItemProtectionAtom
{
  MP4_FULL_ATOM
  MP4LinkedList atomList;
  MP4Err (*addAtom)(struct ISOItemProtectionAtom *self, MP4AtomPtr atom);
} ISOItemProtectionAtom, *ISOItemProtectionAtomPtr;

typedef struct ISOItemInfoEntryAtom
{
  MP4_FULL_ATOM
  u32 item_ID;
  u32 protection_index;
  char *item_name;
  char *content_type;
  char *content_encoding;
  u32 extension_type;
  MP4Handle item_info_extension;
  u16 item_protection_index;
  u32 item_type;
  char *item_uri_type;
} ISOItemInfoEntryAtom, *ISOItemInfoEntryAtomPtr;

typedef struct ISOItemInfoAtom
{
  MP4_FULL_ATOM
  MP4LinkedList atomList;
  MP4Err (*addAtom)(struct ISOItemInfoAtom *self, MP4AtomPtr atom);
  MP4Err (*getEntry)(struct ISOItemInfoAtom *self, u32 itemID, ISOItemInfoEntryAtomPtr *outEntry);
} ISOItemInfoAtom, *ISOItemInfoAtomPtr;

typedef struct MP4SampleDependencyAtom
{
  MP4_FULL_ATOM

  u8 *dependency; /* indexed by sample number;  we compact on write and expand on read */
  u32 sampleCount;
  u32 allocatedSize;

  MP4Err (*addSamples)(struct MP4SampleDependencyAtom *self, u32 count);
  MP4Err (*setSampleDependency)(struct MP4SampleDependencyAtom *self, s32 sample_index,
                                MP4Handle dependencies);
  MP4Err (*getSampleDependency)(struct MP4SampleDependencyAtom *self, u32 sampleNumber,
                                u8 *dependency);
} MP4SampleDependencyAtom, *MP4SampleDependencyAtomPtr;

typedef struct SIDXReference
{
  u8 referenceType;
  u32 referencedSize;
  u32 subsegmentDuration;
  u8 startsWithSAP;
  u8 SAPType;
  u32 SAPDeltaTime;

} SIDXReference, *SIDXReferencePtr;

typedef struct SubsegmentRange
{
  u8 level;
  u32 rangeSize;

} SubsegmentRange, *SubsegmentRangePtr;

typedef struct Subsegment
{
  u32 rangeCount;
  MP4LinkedList rangesList;

  u32 (*getRangeCount)(struct Subsegment *self);
  MP4Err (*addRange)(struct Subsegment *self);

} Subsegment, *SubsegmentPtr;

typedef struct MP4SegmentTypeAtom
{
  MP4_FULL_ATOM

  ISOErr (*addStandard)(struct MP4SegmentTypeAtom *self, u32 standard);
  ISOErr (*setBrand)(struct MP4SegmentTypeAtom *self, u32 standard, u32 minorversion);
  ISOErr (*getBrand)(struct MP4SegmentTypeAtom *self, u32 *standard, u32 *minorversion);
  u32 (*getStandard)(struct MP4SegmentTypeAtom *self, u32 standard);

  u32 majorBrand;   /* the brand of this track */
  u32 minorVersion; /* the minor version of this track */
  u32 itemCount;    /* the number of items in the compatibility list */
  u32 *compatibilityList;

} MP4SegmentTypeAtom, *MP4SegmentTypeAtomPtr;

typedef struct MP4SegmentIndexAtom
{
  MP4_FULL_ATOM

  u32 referenceId;
  u32 timescale;

  u32 earliestPresentationTime;
  u32 firstOffset;

  u16 reserved1;
  u16 referenceCount;

  MP4LinkedList referencesList;

  u32 (*getReferenceCount)(struct MP4SegmentIndexAtom *self);
  MP4Err (*addReference)(struct MP4SegmentIndexAtom *self, u8 referenceType, u32 referencedSize,
                         u32 subsegmentDuration, u8 startsWithSAP, u8 SAPType, u32 SAPDeltaTime);

} MP4SegmentIndexAtom, *MP4SegmentIndexAtomPtr;

typedef struct MP4SubsegmentIndexAtom
{
  MP4_FULL_ATOM

  u32 subsegmentCount;

  MP4LinkedList subsegmentsList;

  u32 (*getSubsegmentCount)(struct MP4SubsegmentIndexAtom *self);
  MP4Err (*addSubsegment)(struct MP4SubsegmentIndexAtom *self, struct Subsegment *ss);
  MP4Err (*addSubsegmentRange)(struct Subsegment *self, u8 level, u32 rangeSize);

} MP4SubsegmentIndexAtom, *MP4SubsegmentIndexAtomPtr;

typedef struct MP4ProducerReferenceTimeAtom
{
  MP4_FULL_ATOM

  u32 referenceTrackId;
  u64 ntpTimestamp;
  u32 mediaTime;

} MP4ProducerReferenceTimeAtom, *MP4ProducerReferenceTimeAtomPtr;

typedef struct MP4VolumetricVisualMediaHeaderAtom
{
  MP4_FULL_ATOM
} MP4VolumetricVisualMediaHeaderAtom, *MP4VolumetricVisualMediaHeaderAtomPtr;

/**
 * @brief GroupListBox grpl
 *
 * The GroupsListBox includes the entity groups specified for the file. This box contains a set of
 * full boxes, each called an EntityToGroupBox, with four-character codes denoting a defined
 * grouping type.
 *
 */
typedef struct GroupListBox
{
  MP4_BASE_ATOM
  /** List containing the boxes */
  MP4LinkedList atomList;

  MP4Err (*addAtom)(struct GroupListBox *self, MP4AtomPtr atom);
  MP4Err (*findAtomOfType)(struct GroupListBox *self, u32 groupType, MP4AtomPtr *outAtom);

} GroupListBox, *GroupListBoxPtr;

/**
 * @brief EntityToGroupBox
 *
 * The EntityToGroupBox specifies an entity group.
 *
 */
typedef struct EntityToGroupBox
{
  MP4_FULL_ATOM

  /** a non-negative integer assigned to the particular grouping */
  u32 group_id;
  /** specifies the number of entity_id values mapped to this entity group */
  u32 num_entities_in_group;
  /** List containing the entity ids */
  MP4LinkedList entity_ids;

  /** ISOBMFF allows extensions to EntityToGroupBox. This buffer holds the data for it */
  char *remainingData;
  u32 remainingDataSize;

  /**
   * @brief Add an entity ID to EntityToGroupBox
   *
   * @param self pointer to EntityToGroupBox to which to add the enity ID to
   * @param entity_id The entity ID to be added
   */
  MP4Err (*addEntityId)(struct EntityToGroupBox *self, u32 entity_id);

  /**
   * @brief Get an entity ID from EntityToGroupBox
   *
   * @param self Pointer to EntityToGroupBox from which get entity ID
   * @param entity_id entity ID value
   * @param index index of entity ID
   */
  MP4Err (*getEntityId)(struct EntityToGroupBox *s, u32 *entity_id, u32 index);

} EntityToGroupBox, *EntityToGroupBoxPtr;

MP4Err MP4CreateGroupListBox(GroupListBoxPtr *outAtom);
MP4Err MP4CreateEntityToGroupBox(EntityToGroupBoxPtr *pOut, u32 type);
MP4Err MP4GetListEntryAtom(MP4LinkedList list, u32 atomType, MP4AtomPtr *outItem);
MP4Err MP4DeleteListEntryAtom(MP4LinkedList list, u32 atomType);

MP4Err sampleEntryHToAtomPtr(MP4Handle sampleEntryH, MP4AtomPtr *entryPtr, u32 defaultType);
MP4Err atomPtrToSampleEntryH(MP4Handle sampleEntryH, MP4AtomPtr entry);

MP4Err MP4CreateAudioSampleEntryAtom(MP4AudioSampleEntryAtomPtr *outAtom);
MP4Err MP4CreatePCMConfigAtom(MP4PCMConfigAtomPtr *outAtom);
MP4Err MP4CreateChannelLayoutAtom(MP4ChannelLayoutAtomPtr *outAtom);
MP4Err MP4CreateDownMixInstructionsAtom(MP4DownMixInstructionsAtomPtr *outAtom);
MP4Err MP4CreateLoudnessBaseAtom(MP4LoudnessBaseAtomPtr *outAtom, u32 type);
MP4Err MP4CreateLoudnessAtom(MP4LoudnessAtomPtr *outAtom);
/* MP4Err MP4CreateChunkLargeOffsetAtom( MP4ChunkLargeOffsetAtomPtr *outAtom ); */
MP4Err MP4CreateChunkOffsetAtom(MP4ChunkOffsetAtomPtr *outAtom);
MP4Err MP4CreateClockReferenceMediaHeaderAtom(MP4ClockReferenceMediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateCompositionOffsetAtom(MP4CompositionOffsetAtomPtr *outAtom);
MP4Err MP4CreateCompositionToDecodeAtom(MP4CompositionToDecodeAtomPtr *outAtom);
MP4Err MP4CreateCopyrightAtom(MP4CopyrightAtomPtr *outAtom);
MP4Err MP4CreateDataEntryURLAtom(MP4DataEntryURLAtomPtr *outAtom);
MP4Err MP4CreateDataEntryURNAtom(MP4DataEntryURNAtomPtr *outAtom);
MP4Err MP4CreateDataInformationAtom(MP4DataInformationAtomPtr *outAtom);
MP4Err MP4CreateDataReferenceAtom(MP4DataReferenceAtomPtr *outAtom);
MP4Err MP4CreateDegradationPriorityAtom(MP4DegradationPriorityAtomPtr *outAtom);
MP4Err MP4CreateESDAtom(MP4ESDAtomPtr *outAtom);
MP4Err MP4CreateEditAtom(MP4EditAtomPtr *outAtom);
MP4Err MP4CreateEditListAtom(MP4EditListAtomPtr *outAtom);
MP4Err MP4CreateFreeSpaceAtom(MP4FreeSpaceAtomPtr *outAtom);
MP4Err MP4CreateGenericSampleEntryAtom(MP4GenericSampleEntryAtomPtr *outAtom);
MP4Err MP4CreateHandlerAtom(MP4HandlerAtomPtr *outAtom);
MP4Err MP4CreateHintMediaHeaderAtom(MP4HintMediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateMPEGMediaHeaderAtom(MP4MPEGMediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateMPEGSampleEntryAtom(MP4MPEGSampleEntryAtomPtr *outAtom);
MP4Err MP4CreateMediaAtom(MP4MediaAtomPtr *outAtom);
MP4Err MP4CreateMediaDataAtom(MP4MediaDataAtomPtr *outAtom);
MP4Err MP4CreateMediaHeaderAtom(MP4MediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateMediaInformationAtom(MP4MediaInformationAtomPtr *outAtom);
MP4Err MP4CreateExtendedLanguageTagAtom(MP4ExtendedLanguageTagAtomPtr *outAtom);
MP4Err MP4CreateMovieAtom(MP4MovieAtomPtr *outAtom);
MP4Err MP4CreateMovieHeaderAtom(MP4MovieHeaderAtomPtr *outAtom);
MP4Err MP4CreateObjectDescriptorAtom(MP4ObjectDescriptorAtomPtr *outAtom);
MP4Err MP4CreateObjectDescriptorMediaHeaderAtom(MP4ObjectDescriptorMediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateSampleDescriptionAtom(MP4SampleDescriptionAtomPtr *outAtom);
MP4Err MP4CreateSampleSizeAtom(MP4SampleSizeAtomPtr *outAtom);
MP4Err MP4CreateSampleTableAtom(MP4SampleTableAtomPtr *outAtom);
MP4Err MP4CreateSampleToChunkAtom(MP4SampleToChunkAtomPtr *outAtom);
MP4Err
MP4CreateSampleAuxiliaryInformationSizesAtom(MP4SampleAuxiliaryInformationSizesAtomPtr *outAtom);
MP4Err MP4CreateSampleAuxiliaryInformationOffsetsAtom(
  MP4SampleAuxiliaryInformationOffsetsAtomPtr *outAtom);
MP4Err MP4CreateSceneDescriptionMediaHeaderAtom(MP4SceneDescriptionMediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateShadowSyncAtom(MP4ShadowSyncAtomPtr *outAtom);
MP4Err MP4CreateSoundMediaHeaderAtom(MP4SoundMediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateSubSampleInformationAtom(MP4SubSampleInformationAtomPtr *outAtom);
MP4Err MP4CreateSyncSampleAtom(MP4SyncSampleAtomPtr *outAtom);
MP4Err MP4CreateTimeToSampleAtom(MP4TimeToSampleAtomPtr *outAtom);
MP4Err MP4CreateTrackAtom(MP4TrackAtomPtr *outAtom);
MP4Err MP4CreateTrackHeaderAtom(MP4TrackHeaderAtomPtr *outAtom);
MP4Err MP4CreateTrackReferenceAtom(MP4TrackReferenceAtomPtr *outAtom);
MP4Err MP4CreateTrackReferenceTypeAtom(u32 atomType, MP4TrackReferenceTypeAtomPtr *outAtom);
MP4Err MP4CreateTrackGroupAtom(MP4TrackGroupAtomPtr *outAtom);
MP4Err MP4CreateTrackGroupTypeAtom(u32 atomType, MP4TrackGroupTypeAtomPtr *outAtom);
MP4Err MP4CreateUnknownAtom(MP4UnknownAtomPtr *outAtom);
MP4Err MP4CreateUserDataAtom(MP4UserDataAtomPtr *outAtom);
MP4Err MP4CreateVideoMediaHeaderAtom(MP4VideoMediaHeaderAtomPtr *outAtom);
MP4Err MP4CreateVisualSampleEntryAtom(MP4VisualSampleEntryAtomPtr *outAtom);
MP4Err MP4CreateVolumetricVisualSampleEntryAtom(MP4VolumetricVisualSampleEntryAtomPtr *outAtom);
MP4Err MP4CreateHapticSampleEntryAtom(MP4HapticSampleEntryAtomPtr *outAtom);

/* mebx stuff */
MP4Err MP4CreateMP4BoxedMetadataSampleEntry(MP4BoxedMetadataSampleEntryPtr *outAtom);
MP4Err MP4CreateMetadataKeyTableBox(MP4MetadataKeyTableBoxPtr *outAtom);
MP4Err MP4CreateMetadataKeyDeclarationBox(MP4MetadataKeyDeclarationBoxPtr *outAtom, u32 key_ns,
                                          MP4Handle key_val);
MP4Err MP4CreateMetadataLocaleBox(MP4MetadataLocaleBoxPtr *outAtom, char *locale_string);
MP4Err MP4CreateMetadataSetupBox(MP4MetadataSetupBoxPtr *outAtom, MP4Handle setupH);
MP4Err MP4CreateMetadataKeyBox(MP4MetadataKeyBoxPtr *outAtom, u32 local_key_id);

MP4Err MP4CreateXMLMetaSampleEntryAtom(MP4XMLMetaSampleEntryAtomPtr *outAtom);
MP4Err MP4CreateTextMetaSampleEntryAtom(MP4TextMetaSampleEntryAtomPtr *outAtom);

MP4Err MP4CreateMovieExtendsAtom(MP4MovieExtendsAtomPtr *outAtom);
MP4Err MP4CreateTrackExtensionPropertiesAtom(MP4TrackExtensionPropertiesAtomPtr *outAtom);
MP4Err MP4CreateTrackExtendsAtom(MP4TrackExtendsAtomPtr *outAtom);
MP4Err MP4CreateMovieFragmentAtom(MP4MovieFragmentAtomPtr *outAtom);
MP4Err MP4CreateMovieFragmentHeaderAtom(MP4MovieFragmentHeaderAtomPtr *outAtom);
MP4Err MP4CreateTrackFragmentAtom(MP4TrackFragmentAtomPtr *outAtom);
MP4Err MP4CreateTrackFragmentHeaderAtom(MP4TrackFragmentHeaderAtomPtr *outAtom);
MP4Err MP4CreateTrackFragmentDecodeTimeAtom(MP4TrackFragmentDecodeTimeAtomPtr *outAtom);
MP4Err MP4CreateTrackRunAtom(MP4TrackRunAtomPtr *outAtom);
MP4Err MP4CreatePaddingBitsAtom(MP4PaddingBitsAtomPtr *outAtom);

MP4Err MP4CreateItemPropertiesAtom(MP4ItemPropertiesAtomPtr *outAtom);
MP4Err MP4CreateItemPropertyContainerAtom(MP4ItemPropertyContainerAtomPtr *outAtom);
MP4Err MP4CreateItemPropertyAssociationAtom(MP4ItemPropertyAssociationAtomPtr *outAtom);

MP4Err MP4CreateHEVCConfigAtom(ISOHEVCConfigAtomPtr *outAtom);
MP4Err MP4CreateVVCConfigAtom(ISOVVCConfigAtomPtr *outAtom);
MP4Err MP4CreateVVCNALUConfigAtom(ISOVVCNALUConfigAtomPtr *outAtom);

MP4Err MP4CreateOriginalFormatAtom(MP4OriginalFormatAtomPtr *outAtom);
MP4Err MP4CreateSchemeInfoAtom(MP4SchemeInfoAtomPtr *outAtom);
MP4Err MP4CreateRestrictedSchemeInfoAtom(MP4RestrictedSchemeInfoAtomPtr *outAtom);
MP4Err MP4CreateSchemeTypeAtom(MP4SchemeTypeAtomPtr *outAtom);
MP4Err MP4CreateCompatibleSchemeTypeAtom(MP4CompatibleSchemeTypeAtomPtr *outAtom);
MP4Err MP4CreateRestrictedVideoSampleEntryAtom(MP4RestrictedVideoSampleEntryAtomPtr *outAtom);
MP4Err MP4CreateTrackTypeAtom(MP4TrackTypeAtomPtr *outAtom);
MP4Err MP4CreateStereoVideoAtom(MP4StereoVideoAtomPtr *outAtom);
MP4Err MP4CreateStereoVideoGroupAtom(MP4StereoVideoGroupAtomPtr *outAtom);

MP4Err MP4CreateSegmentTypeAtom(MP4SegmentTypeAtomPtr *outAtom);
MP4Err MP4CreateSegmentIndexAtom(MP4SegmentIndexAtomPtr *outAtom);
MP4Err MP4CreateSubsegmentIndexAtom(MP4SubsegmentIndexAtomPtr *outAtom);
MP4Err MP4CreateProducerReferenceTimeAtom(MP4ProducerReferenceTimeAtomPtr *outAtom);

#ifdef ISMACrypt
MP4Err MP4CreateSecurityInfoAtom(MP4SecurityInfoAtomPtr *outAtom);
MP4Err CreateISMAKMSAtom(ISMAKMSAtomPtr *outAtom);
MP4Err CreateISMASampleFormatAtom(ISMASampleFormatAtomPtr *outAtom);
MP4Err CreateISMASaltAtom(ISMASaltAtomPtr *outAtom);
MP4Err MP4CreateEncAudioSampleEntryAtom(MP4EncAudioSampleEntryAtomPtr *outAtom);
MP4Err MP4CreateEncVisualSampleEntryAtom(MP4EncVisualSampleEntryAtomPtr *outAtom);

MP4Err MP4CreateEncBaseAtom(MP4EncBaseSampleEntryAtomPtr outAtom);
#endif

MP4Err MP4CreateSampletoGroupAtom(MP4SampletoGroupAtomPtr *outAtom,
                                  sampleToGroupType_t sampleToGroupType);
MP4Err MP4CreateSampleGroupDescriptionAtom(MP4SampleGroupDescriptionAtomPtr *outAtom);

MP4Err MP4CreateSampleDependencyAtom(MP4SampleDependencyAtomPtr *outAtom);

MP4Err ISOCreateMetaAtom(ISOMetaAtomPtr *outAtom);
MP4Err ISOCreatePrimaryItemAtom(ISOPrimaryItemAtomPtr *outAtom);
MP4Err ISOCreateItemLocationAtom(ISOItemLocationAtomPtr *outAtom);
MP4Err ISOCreateItemProtectionAtom(ISOItemProtectionAtomPtr *outAtom);
MP4Err ISOCreateItemInfoAtom(ISOItemInfoAtomPtr *outAtom);
MP4Err ISOCreateItemInfoEntryAtom(ISOItemInfoEntryAtomPtr *outAtom);

MP4Err ISOCreateAdditionalMetaDataContainerAtom(ISOAdditionalMetaDataContainerAtomPtr *outAtom);
MP4Err ISOCreateMetaboxRelationAtom(ISOMetaboxRelationAtomPtr *outAtom);
MP4Err ISOCreateItemDataAtom(ISOItemDataAtomPtr *outAtom);
MP4Err ISOCreateItemReferenceAtom(ISOItemReferenceAtomPtr *outAtom);
MP4Err ISOCreateSingleItemTypeReferenceAtom(ISOSingleItemTypeReferenceAtomPtr *outAtom, u32 type,
                                            u8 isLarge);

MP4Err MP4CreateVCConfigAtom(ISOVCConfigAtomPtr *outAtom);

MP4Err MP4CreateAMRSpecificInfoAtom(MP4AMRSpecificInfoAtomPtr *outAtom);
MP4Err MP4CreateAMRWPSpecificInfoAtom(MP4AMRWPSpecificInfoAtomPtr *outAtom);
MP4Err MP4CreateH263SpecificInfoAtom(MP4H263SpecificInfoAtomPtr *outAtom);
MP4Err MP4CreateBitRateAtom(MP4BitRateAtomPtr *outAtom);

MP4Err MP4CreateVisualMediaHeaderAtom(MP4VolumetricVisualMediaHeaderAtomPtr *outAtom);

#endif
