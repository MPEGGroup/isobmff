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
derivative works. Copyright (c) 1999, 2000.
*/
/*
  $Id: ISMASecurity.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

#ifdef ISMACrypt

u32 NoProtos[] = {0};

MP4_EXTERN(MP4Err)
ISMATransformSampleEntry(u32 newTrackFlags, MP4Handle insampleEntryH, u8 selective_encryption,
                         u8 key_indicator_length, u8 IV_length, char *kms_URL,
                         MP4Handle outsampleEntryH)
{
  return ISMATransformSampleEntrySalt(newTrackFlags, insampleEntryH, selective_encryption,
                                      key_indicator_length, IV_length, kms_URL, 0, outsampleEntryH);
}

MP4_EXTERN(MP4Err)
ISMATransformSampleEntrySalt(u32 newTrackFlags, MP4Handle insampleEntryH, u8 selective_encryption,
                             u8 key_indicator_length, u8 IV_length, char *kms_URL, u64 salt,
                             MP4Handle outsampleEntryH)
{
  MP4Err MP4CreateMemoryInputStream(char *base, u32 size, MP4InputStreamPtr *outStream);
  MP4Err MP4ParseAtomUsingProtoList(MP4InputStreamPtr inputStream, u32 * protoList, u32 defaultAtom,
                                    MP4AtomPtr * outAtom);

  u32 atomType;
  MP4Err err;
  MP4InputStreamPtr is;
  u32 size;
  MP4EncBaseSampleEntryAtomPtr entry;
  ISMAKMSAtomPtr kms;
  ISMASampleFormatAtomPtr fmt;
  ISMASaltAtomPtr slt;
  char *kms_url_copy = NULL;

  err   = MP4NoErr;
  entry = NULL;
  kms   = NULL;
  fmt   = NULL;
  slt   = NULL;
  is    = NULL;

  if(newTrackFlags & ISONewTrackIsVisual) atomType = MP4EncVisualSampleEntryAtomType;
  else if(newTrackFlags & ISONewTrackIsAudio)
    atomType = MP4EncAudioSampleEntryAtomType;
  else
  {
    err = MP4NotImplementedErr;
    goto bail;
  }

  err = MP4GetHandleSize(insampleEntryH, &size);
  if(err) goto bail;
  err = MP4CreateMemoryInputStream(*insampleEntryH, size, &is);
  if(err) goto bail;
  is->debugging = 0;
  err           = MP4ParseAtomUsingProtoList(is, NoProtos, atomType, (MP4AtomPtr *)&entry);
  if(err) goto bail;

  err = CreateISMAKMSAtom(&kms);
  if(err) goto bail;

  if(kms_URL)
  {
    kms_url_copy = (char *)calloc(1, strlen(kms_URL) + 1);
    TESTMALLOC(kms_url_copy);
    memcpy(kms_url_copy, kms_URL, strlen(kms_URL) + 1);
    kms->kms_url = kms_url_copy;
  }
  else
    kms->kms_url = NULL;

  err = CreateISMASampleFormatAtom(&fmt);
  if(err) goto bail;

  fmt->selective_encryption = selective_encryption;
  fmt->key_indicator_len    = key_indicator_length;
  fmt->IV_len               = IV_length;

  err = entry->transform((MP4AtomPtr)entry, ISMACryptAESCounterType, 1, "http://www.isma.tv");
  if(err) goto bail;
  err = entry->addSchemeInfoAtom((MP4AtomPtr)entry, (MP4AtomPtr)kms);
  if(err) goto bail;
  kms = NULL;

  err = entry->addSchemeInfoAtom((MP4AtomPtr)entry, (MP4AtomPtr)fmt);
  if(err) goto bail;
  fmt = NULL;

  if(salt != 0)
  {
    err = CreateISMASaltAtom(&slt);
    if(err) goto bail;
    slt->salt = salt;
    err       = entry->addSchemeInfoAtom((MP4AtomPtr)entry, (MP4AtomPtr)slt);
    if(err) goto bail;
    slt = NULL;
  }

  err = entry->calculateSize((MP4AtomPtr)entry);
  if(err) goto bail;
  err = MP4SetHandleSize(outsampleEntryH, entry->size);
  if(err) goto bail;
  err = entry->serialize((MP4AtomPtr)entry, *outsampleEntryH);
  if(err) goto bail;

bail:
  if(is)
  {
    is->destroy(is);
    is = NULL;
  }

  if(entry) entry->destroy((MP4AtomPtr)entry);
  if(kms) kms->destroy((MP4AtomPtr)kms);
  if(fmt) fmt->destroy((MP4AtomPtr)fmt);
  if(slt) slt->destroy((MP4AtomPtr)slt);

  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
ISMAUnTransformSampleEntry(MP4Handle insampleEntryH, u8 *selective_encryption,
                           u8 *key_indicator_length, u8 *IV_length, char **kms_URL,
                           MP4Handle outsampleEntryH)
{
  return ISMAUnTransformSampleEntrySalt(insampleEntryH, selective_encryption, key_indicator_length,
                                        IV_length, kms_URL, NULL, outsampleEntryH);
}

MP4_EXTERN(MP4Err)
ISMAUnTransformSampleEntrySalt(MP4Handle insampleEntryH, u8 *selective_encryption,
                               u8 *key_indicator_length, u8 *IV_length, char **kms_URL, u64 *salt,
                               MP4Handle outsampleEntryH)
{
  MP4Err err;
  MP4InputStreamPtr is;
  u32 size;
  MP4EncBaseSampleEntryAtomPtr entry;
  ISMAKMSAtomPtr kms;
  ISMASaltAtomPtr slt;
  ISMASampleFormatAtomPtr fmt;
  char *kms_url_copy;
  u32 sch_type, sch_version;

  err   = MP4NoErr;
  entry = NULL;
  is    = NULL;

  err = MP4GetHandleSize(insampleEntryH, &size);
  if(err) goto bail;
  err = MP4CreateMemoryInputStream(*insampleEntryH, size, &is);
  if(err) goto bail;
  is->debugging = 0;
  err           = MP4ParseAtom(is, (MP4AtomPtr *)&entry);
  if(err) goto bail;

  if((entry->type != MP4EncAudioSampleEntryAtomType) &&
     (entry->type != MP4EncVisualSampleEntryAtomType))
  {
    err = MP4NotImplementedErr;
    goto bail;
  }

  err = entry->getScheme((MP4AtomPtr)entry, &sch_type, &sch_version, NULL);
  if(err) goto bail;
  if((sch_type != ISMACryptAESCounterType) || (sch_version != 1))
  {
    err = MP4NotImplementedErr;
    goto bail;
  }

  err = entry->getSchemeInfoAtom((MP4AtomPtr)entry, ISMAKMSAtomType, (MP4AtomPtr *)&kms);
  if(err) goto bail;
  if(kms_URL)
  {
    kms_url_copy = (char *)calloc(1, strlen(kms->kms_url) + 1);
    TESTMALLOC(kms_url_copy);
    memcpy(kms_url_copy, kms->kms_url, strlen(kms->kms_url) + 1);
    *kms_URL = kms_url_copy;
  }

  err = entry->getSchemeInfoAtom((MP4AtomPtr)entry, ISMASaltAtomType, (MP4AtomPtr *)&slt);
  if(err) goto bail;
  if(salt)
  {
    err = entry->getSchemeInfoAtom((MP4AtomPtr)entry, ISMASaltAtomType, (MP4AtomPtr *)&slt);
    if(err) goto bail;
    if(slt) *salt = slt->salt;
    else
      *salt = 0;
  }

  err = entry->getSchemeInfoAtom((MP4AtomPtr)entry, ISMASampleFormatAtomType, (MP4AtomPtr *)&fmt);
  if(err) goto bail;
  *selective_encryption = (u8)fmt->selective_encryption;
  *key_indicator_length = (u8)fmt->key_indicator_len;
  *IV_length            = (u8)fmt->IV_len;

  err = entry->untransform((MP4AtomPtr)entry);
  if(err) goto bail;

  err = entry->calculateSize((MP4AtomPtr)entry);
  if(err) goto bail;
  err = MP4SetHandleSize(outsampleEntryH, entry->size);
  if(err) goto bail;
  err = entry->serialize((MP4AtomPtr)entry, *outsampleEntryH);
  if(err) goto bail;

bail:
  if(is)
  {
    is->destroy(is);
    is = NULL;
  }

  if(entry) entry->destroy((MP4AtomPtr)entry);

  TEST_RETURN(err);

  return err;
}

#endif
