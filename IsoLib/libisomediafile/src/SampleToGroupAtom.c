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

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

#define allocation_size 8192

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr self;
  err  = MP4NoErr;
  self = (MP4SampletoGroupAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  if(self->group_index != NULL)
  {
    free(self->group_index);
    self->group_index = NULL;
  }
  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err ensureSize(struct MP4SampletoGroupAtom *self, u32 newSize)
{
  MP4Err err;

  err = MP4NoErr;

  if(newSize > self->allocatedSize)
  {
    self->allocatedSize += allocation_size;
    if(newSize > self->allocatedSize) self->allocatedSize = newSize;

    if(self->group_index != NULL)
      self->group_index = (u32 *)realloc(self->group_index, self->allocatedSize);
    else
      self->group_index = (u32 *)calloc(self->allocatedSize, 1);

    TESTMALLOC(self->group_index);
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSamples(struct MP4SampletoGroupAtom *self, u32 count)
{
  MP4Err err;
  u32 *p;
  u32 j;

  err = MP4NoErr;
  err = ensureSize(self, (self->sampleCount + count) * sizeof(u32));
  if(err) goto bail;

  p = &((self->group_index)[self->sampleCount]);
  for(j = 0; j < count; j++)
    *p++ = 0;
  self->sampleCount += count;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mapSamplestoGroup(struct MP4SampletoGroupAtom *self, u32 group_index,
                                s32 sample_index, u32 count)
{
  MP4Err err;
  u32 i;
  u32 *p;

  err = MP4NoErr;

  if(sample_index < 0)
  {
    p = &((self->group_index)[self->sampleCount + sample_index]);
    if(count > ((u32)(-sample_index)))
    {
      err = MP4BadParamErr;
      goto bail;
    }
  }
  else
  {
    p = &((self->group_index)[sample_index]);
    if(count + sample_index > self->sampleCount)
    {
      err = MP4BadParamErr;
      goto bail;
    }
  }
  for(i = 0; i < count; i++)
    *p++ = group_index;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupMap(struct MP4SampletoGroupAtom *self, u32 sampleNumber,
                                u32 *groupIndex)
{
  MP4Err err;

  err = MP4NoErr;

  if(sampleNumber < 1) BAILWITHERROR(MP4BadParamErr);
  if(sampleNumber > self->sampleCount) *groupIndex = 0;
  else
    *groupIndex = (self->group_index)[sampleNumber - 1];

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i;
  u32 cur_index, cur_count, entryCount;

  MP4SampletoGroupAtomPtr self = (MP4SampletoGroupAtomPtr)s;
  err                          = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUT32(grouping_type);
  PUT32(entryCount);

  cur_index  = (self->group_index)[0];
  cur_count  = 1;
  entryCount = 0;

  for(i = 1; i < self->sampleCount; i++)
  {
    if((self->group_index)[i - 1] != (self->group_index)[i])
    {
      PUT32_V(cur_count);
      PUT32_V(cur_index);
      cur_count = 1;
      cur_index = (self->group_index)[i];
      entryCount++;
    }
    else
      cur_count++;
  }
  PUT32_V(cur_count);
  PUT32_V(cur_index);
  entryCount++;

  assert(entryCount == self->entryCount);
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr self = (MP4SampletoGroupAtomPtr)s;
  u32 entryCount, i;

  err = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  entryCount = 1;

  for(i = 1; i < (self->sampleCount); i++)
  {
    if((self->group_index)[i - 1] != (self->group_index)[i]) entryCount++;
  }
  self->size += (entryCount * 8) + 8;
  self->entryCount = entryCount;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i, j, size;
  u32 *p;
  MP4SampletoGroupAtomPtr self = (MP4SampletoGroupAtomPtr)s;
  char typeString[8];
  char msgString[80];

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET32(grouping_type);
  MP4TypeToString(self->grouping_type, typeString);
  sprintf(msgString, " grouping type is '%s'", typeString);
  inputStream->msg(inputStream, msgString);

  GET32(entryCount);
  size = 0;

  for(i = 0; i < self->entryCount; i++)
  {
    u32 count, index;
    GET32_V_MSG(count, NULL);
    GET32_V_MSG(index, NULL);
    sprintf(msgString, " entry %d, count %d index %d", i + 1, count, index);
    inputStream->msg(inputStream, msgString);

    err = ensureSize(self, (size + count) * sizeof(u32));
    if(err) goto bail;

    p = &((self->group_index)[size]);
    for(j = 0; j < count; j++)
      *p++ = index;
    size += count;
  }
  self->sampleCount = size;

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSampletoGroupAtom(MP4SampletoGroupAtomPtr *outAtom)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr self;

  self = (MP4SampletoGroupAtomPtr)calloc(1, sizeof(MP4SampletoGroupAtom));
  TESTMALLOC(self)

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4SampletoGroupAtomType;
  self->name                  = "sample to group";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->mapSamplestoGroup     = mapSamplestoGroup;
  self->addSamples            = addSamples;
  self->getSampleGroupMap     = getSampleGroupMap;
  self->group_index           = NULL;
  self->sampleCount           = 0;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
