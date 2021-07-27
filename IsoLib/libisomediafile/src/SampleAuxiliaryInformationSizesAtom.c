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
  $Id: SampleAuxiliaryInformationSizesAtom.c,v 1.1.1.1 2014/08/08 08:10:00 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4SampleAuxiliaryInformationSizesAtomPtr self = (MP4SampleAuxiliaryInformationSizesAtomPtr)s;
  if(self == NULL) return;
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  u32 i;
  MP4Err err;
  MP4SampleAuxiliaryInformationSizesAtomPtr self = (MP4SampleAuxiliaryInformationSizesAtomPtr)s;
  err                                            = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  if(self->flags & 1)
  {
    PUT32(aux_info_type);
    PUT32(aux_info_type_parameter);
  }

  PUT8(default_sample_info_size);
  PUT32(sample_count);

  if(self->default_sample_info_size == 0)
  {
    for(i = 0; i < self->sample_count; i++)
    {
      PUT8(sample_info_sizes[i]);
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
  MP4SampleAuxiliaryInformationSizesAtomPtr self = (MP4SampleAuxiliaryInformationSizesAtomPtr)s;
  err                                            = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  if(self->flags & 1)
  {
    self->size += 8;
  }

  self->size += 5;

  if(self->default_sample_info_size == 0)
  {
    self->size += self->sample_count;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  u32 i;
  MP4Err err;
  MP4SampleAuxiliaryInformationSizesAtomPtr self = (MP4SampleAuxiliaryInformationSizesAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  if(self->flags & 1)
  {
    GET32(aux_info_type);
    GET32(aux_info_type_parameter);
  }

  GET8(default_sample_info_size);
  GET32(sample_count);

  if(self->default_sample_info_size == 0)
  {
    self->sample_info_sizes = calloc(self->sample_count, sizeof(u8));
    for(i = 0; i < self->sample_count; i++)
    {
      GET8(sample_info_sizes[i]);
    }
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSizes(MP4AtomPtr s, u32 sampleCount, MP4Handle sizesH)
{
  u32 i;
  u32 *sizes;
  u32 sizeCount;
  u32 onlySize;
  MP4Err err;
  MP4SampleAuxiliaryInformationSizesAtomPtr self = (MP4SampleAuxiliaryInformationSizesAtomPtr)s;

  err      = MP4NoErr;
  onlySize = 0;

  if(self->default_sample_info_size == 0)
  {
    sizes = (u32 *)*sizesH;

    err = MP4GetHandleSize(sizesH, &sizeCount);
    if(err) goto bail;
    sizeCount /= sizeof(u32);
    if(sizeCount == 1) onlySize = *(u32 *)*sizesH;
    else if(sizeCount != sampleCount)
      BAILWITHERROR(MP4BadParamErr);

    self->sample_info_sizes = realloc(self->sample_info_sizes, self->sample_count + sampleCount);

    for(i = 0; i < sampleCount; i++)
    {
      if(sizeCount == 1) self->sample_info_sizes[self->sample_count + i] = (u8)onlySize;
      else
        self->sample_info_sizes[self->sample_count + i] = (u8)sizes[i];
    }
  }

  self->sample_count += sampleCount;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mergeSizes(MP4AtomPtr s, MP4AtomPtr otherSaiz)
{
  u32 i;
  MP4Err err;
  MP4SampleAuxiliaryInformationSizesAtomPtr self;
  MP4SampleAuxiliaryInformationSizesAtomPtr other;

  err = MP4NoErr;

  self  = (MP4SampleAuxiliaryInformationSizesAtomPtr)s;
  other = (MP4SampleAuxiliaryInformationSizesAtomPtr)otherSaiz;

  if(self->default_sample_info_size != 0)
  {
    if(other->default_sample_info_size != 0)
    {
      if(self->default_sample_info_size == other->default_sample_info_size)
      {
        self->sample_count += other->sample_count;
        goto bail;
      }
    }

    self->sample_info_sizes = realloc(self->sample_info_sizes, self->sample_count);
    for(i = 0; i < self->sample_count; i++)
    {
      self->sample_info_sizes[i] = (u8)self->default_sample_info_size;
    }
    self->default_sample_info_size = 0;
  }

  self->sample_info_sizes =
    realloc(self->sample_info_sizes, self->sample_count + other->sample_count);
  for(i = 0; i < other->sample_count; i++)
  {
    if(other->default_sample_info_size != 0)
      self->sample_info_sizes[self->sample_count + i] = (u8)other->default_sample_info_size;
    else
      self->sample_info_sizes[self->sample_count + i] = other->sample_info_sizes[i];
  }
  self->sample_count += other->sample_count;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err
MP4CreateSampleAuxiliaryInformationSizesAtom(MP4SampleAuxiliaryInformationSizesAtomPtr *outAtom)
{
  MP4Err err;
  MP4SampleAuxiliaryInformationSizesAtomPtr self;

  self = (MP4SampleAuxiliaryInformationSizesAtomPtr)calloc(
    1, sizeof(MP4SampleAuxiliaryInformationSizesAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4SampleAuxiliaryInformationSizesAtomType;
  self->name                  = "sample auxiliary information sizes";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->mergeSizes            = mergeSizes;

  self->flags        = 0;
  self->sample_count = 0;

  self->addSizes = addSizes;

  self->sample_info_sizes = NULL;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
