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
 derivative works. Copyright (c) 1999.
 */
/*
  $Id: SampleAuxiliaryInformationOffsetsAtom.c,v 1.1.1.1 2014/08/08 08:10:00 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)s;
  if(self == NULL) return;
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  u32 i;
  MP4Err err;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)s;
  err                                              = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  if(self->flags & 1)
  {
    PUT32(aux_info_type);
    PUT32(aux_info_type_parameter);
  }

  PUT32(entry_count);

  for(i = 0; i < self->entry_count; i++)
  {
    if(self->version == 0)
    {
      u32 temp;
      temp = (u32)(self->offsets[i] + self->additionalOffset);
      PUT32_V(temp);
    }
    else
    {
      PUT64(offsets[i]);
    }
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)s;
  err                                              = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  self->size += 0;

  if(self->flags & 1)
  {
    self->size += 8;
  }

  self->size += 4;

  if(self->version == 0)
  {
    self->size += self->entry_count * 4;
  }
  else
  {
    self->size += self->entry_count * 8;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  u32 i;
  MP4Err err;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  if(self->flags & 1)
  {
    GET32(aux_info_type);
    GET32(aux_info_type_parameter);
  }

  GET32(entry_count);

  self->offsets = calloc(self->entry_count, sizeof(u64));
  for(i = 0; i < self->entry_count; i++)
  {
    if(self->version == 0)
    {
      u32 temp;
      GET32_V(temp);
      self->offsets[i] = temp;
    }
    else
    {
      GET64(offsets[i]);
    }
  }
  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addOffsets(MP4AtomPtr s, u32 entryCount, MP4Handle sizesH)
{
  u32 i;
  u32 *sizes;
  u32 sizeCount;
  u32 onlySize;
  MP4Err err;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)s;

  err      = MP4NoErr;
  onlySize = 0;

  sizes = (u32 *)*sizesH;

  err = MP4GetHandleSize(sizesH, &sizeCount);
  if(err) goto bail;
  sizeCount /= sizeof(u32);
  if(sizeCount == 1) onlySize = *(u32 *)*sizesH;
  else if(sizeCount != entryCount)
    BAILWITHERROR(MP4BadParamErr);

  self->offsets = realloc(self->offsets, (self->entry_count + entryCount) * 8);

  for(i = 0; i < entryCount; i++)
  {
    self->offsets[self->entry_count + i] = self->totalOffset;
    if(sizeCount == 1) self->totalOffset += onlySize;
    else
      self->totalOffset += sizes[i];
  }

  self->entry_count += entryCount;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mdatMoved(MP4AtomPtr s, u64 mdatBase, u64 mdatEnd, s32 mdatOffset)
{
  MP4Err err;
  u32 j;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)s;

  err = MP4NoErr;

  for(j = 0; j <= self->entry_count; j++)
  {
    if((self->offsets[j] >= mdatBase) && (self->offsets[j] < mdatEnd))
      self->offsets[j] += mdatOffset;
  }

  TEST_RETURN(err);

  return err;
}

static MP4Err mergeOffsets(MP4AtomPtr s, MP4AtomPtr otherSaio, u64 baseOffset)
{
  u32 i;
  MP4Err err;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr other;

  err = MP4NoErr;

  self  = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)s;
  other = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)otherSaio;

  err = MP4NoErr;

  self->offsets = (u64 *)realloc(self->offsets, (self->entry_count + other->entry_count) * 8);

  for(i = 0; i < other->entry_count; i++)
  {
    self->offsets[self->entry_count + i] = baseOffset + other->offsets[i];
  }

  self->totalOffset = other->totalOffset;
  self->entry_count += other->entry_count;

  TEST_RETURN(err);

  return err;
}

MP4Err
MP4CreateSampleAuxiliaryInformationOffsetsAtom(MP4SampleAuxiliaryInformationOffsetsAtomPtr *outAtom)
{
  MP4Err err;
  MP4SampleAuxiliaryInformationOffsetsAtomPtr self;

  self = (MP4SampleAuxiliaryInformationOffsetsAtomPtr)calloc(
    1, sizeof(MP4SampleAuxiliaryInformationOffsetsAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4SampleAuxiliaryInformationOffsetsAtomType;
  self->name                  = "sample auxiliary information offsets";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->mergeOffsets          = mergeOffsets;

  self->flags            = 0;
  self->entry_count      = 0;
  self->additionalOffset = 0;
  self->totalOffset      = 0;
  self->addOffsets       = addOffsets;
  self->offsets          = NULL;
  self->mdatMoved        = mdatMoved;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
