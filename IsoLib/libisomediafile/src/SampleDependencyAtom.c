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

#define allocation_size 4096

static void destroy(MP4AtomPtr s)
{
  MP4SampleDependencyAtomPtr self;
  self = (MP4SampleDependencyAtomPtr)s;
  if(self == NULL) return;
  if(self->dependency != NULL)
  {
    free(self->dependency);
    self->dependency = NULL;
  }
  if(self->super) self->super->destroy(s);
}

static MP4Err ensureSize(struct MP4SampleDependencyAtom *self, u32 newSize)
{
  MP4Err err;

  err = MP4NoErr;

  if(newSize > self->allocatedSize)
  {
    self->allocatedSize += allocation_size;
    if(newSize > self->allocatedSize) self->allocatedSize = newSize;

    if(self->dependency != NULL)
      self->dependency = (u8 *)realloc(self->dependency, self->allocatedSize);
    else
      self->dependency = (u8 *)calloc(self->allocatedSize, 1);

    TESTMALLOC(self->dependency);
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSamples(struct MP4SampleDependencyAtom *self, u32 count)
{
  MP4Err err;
  u8 *p;
  u32 j;

  err = MP4NoErr;
  err = ensureSize(self, (self->sampleCount + count) * sizeof(u8));
  if(err) goto bail;

  p = &((self->dependency)[self->sampleCount]);
  for(j = 0; j < count; j++)
    *p++ = 0;
  self->sampleCount += count;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setSampleDependency(struct MP4SampleDependencyAtom *self, s32 sample_index,
                                  MP4Handle dependencies)
{
  MP4Err err;
  u32 i;
  u8 *p;
  u8 *dependency;
  u32 count;

  err = MP4GetHandleSize(dependencies, &count);
  if(err) goto bail;
  dependency = (u8 *)*dependencies;

  if(sample_index < 0)
  {
    p = &((self->dependency)[self->sampleCount + sample_index]);
    if(count > ((u32)(-sample_index)))
    {
      err = MP4BadParamErr;
      goto bail;
    }
  }
  else
  {
    p = &((self->dependency)[sample_index]);
    if(count + sample_index > self->sampleCount)
    {
      err = MP4BadParamErr;
      goto bail;
    }
  }
  for(i = 0; i < count; i++)
    *p++ = *dependency++;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleDependency(struct MP4SampleDependencyAtom *self, u32 sampleNumber,
                                  u8 *dependency)
{
  MP4Err err;

  err = MP4NoErr;

  if(sampleNumber < 1) BAILWITHERROR(MP4BadParamErr);
  if(sampleNumber > self->sampleCount) *dependency = 0;
  else
    *dependency = (self->dependency)[sampleNumber - 1];

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;

  MP4SampleDependencyAtomPtr self = (MP4SampleDependencyAtomPtr)s;
  err                             = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUTBYTES(self->dependency, self->sampleCount);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4SampleDependencyAtomPtr self = (MP4SampleDependencyAtomPtr)s;

  err = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += self->sampleCount;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4SampleDependencyAtomPtr self = (MP4SampleDependencyAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  self->sampleCount = (self->size) - (self->bytesRead);
  err               = ensureSize(self, self->sampleCount);
  if(err) goto bail;

  GETBYTES_V_MSG(self->sampleCount, (char *)(self->dependency), "dependency");

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSampleDependencyAtom(MP4SampleDependencyAtomPtr *outAtom)
{
  MP4Err err;
  MP4SampleDependencyAtomPtr self;

  self = (MP4SampleDependencyAtomPtr)calloc(1, sizeof(MP4SampleDependencyAtom));
  TESTMALLOC(self)

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4SampleDependencyAtomType;
  self->name                  = "sample dependency";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->setSampleDependency   = setSampleDependency;
  self->addSamples            = addSamples;
  self->getSampleDependency   = getSampleDependency;
  self->dependency            = NULL;
  self->sampleCount           = 0;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
