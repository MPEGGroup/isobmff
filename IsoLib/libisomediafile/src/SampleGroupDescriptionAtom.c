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

static void destroy(MP4AtomPtr s)
{
  u32 i;
  MP4SampleGroupDescriptionAtomPtr self;
  self = (MP4SampleGroupDescriptionAtomPtr)s;
  if(self == NULL) return;
  if(self->groups != NULL)
  {
    for(i = 0; i < self->groupCount; i++)
      free((self->groups)[i].groupDescription);

    free(self->groups);
    self->groups = NULL;
  }
  if(self->super) self->super->destroy(s);
}

static MP4Err addGroupDescription(struct MP4SampleGroupDescriptionAtom *self,
                                  MP4Handle theDescription, u32 *index)
{
  MP4Err err;
  sampleGroupEntry *p;
  u32 theSize, foundIdx;

  /* make sure we don't add duplicate descriptions */
  err = self->findGroupDescriptionIdx(self, theDescription, &foundIdx);
  if(err != MP4NotFoundErr) BAILWITHERROR(MP4BadParamErr);

  if(self->groups == NULL) self->groups = calloc(1, sizeof(sampleGroupEntry));
  else
    self->groups = realloc(self->groups, (self->groupCount + 1) * sizeof(sampleGroupEntry));
  TESTMALLOC(self->groups);
  p = &((self->groups)[self->groupCount]);

  err = MP4GetHandleSize(theDescription, &theSize);
  if(err) goto bail;

  p->groupDescription = calloc(theSize, 1);
  TESTMALLOC(p->groupDescription);
  memcpy(p->groupDescription, *theDescription, theSize);
  p->groupSize = theSize;

  self->groupCount += 1;
  *index = self->groupCount;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getGroupDescription(struct MP4SampleGroupDescriptionAtom *self, u32 index,
                                  MP4Handle theDescription)
{
  MP4Err err;
  sampleGroupEntry *p;

  if((index < 1) || (index > self->groupCount)) BAILWITHERROR(MP4BadParamErr);

  p = &((self->groups)[index - 1]);

  err = MP4SetHandleSize(theDescription, p->groupSize);
  if(err) goto bail;

  memcpy(*theDescription, p->groupDescription, p->groupSize);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err findGroupDescriptionIdx(struct MP4SampleGroupDescriptionAtom *self, MP4Handle searchH,
                                      u32 *index)
{
  MP4Err err;
  u32 size, i, temp;
  sampleGroupEntry *p;

  err = MP4GetHandleSize(searchH, &size);
  if(err) goto bail;
  if(index == NULL || self == NULL || size == 0) BAILWITHERROR(MP4BadParamErr);

  err = MP4NotFoundErr;
  for(i = 0; i < self->groupCount; ++i)
  {
    p = &((self->groups)[i]);
    if(p->groupSize != size) continue;

    temp = memcmp(p->groupDescription, *searchH, size);
    if(temp == 0)
    {
      *index = i + 1;
      err    = MP4NoErr;
      break;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i;

  MP4SampleGroupDescriptionAtomPtr self = (MP4SampleGroupDescriptionAtomPtr)s;
  err                                   = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUT32(grouping_type);
  PUT32(default_length);
  PUT32(groupCount);

  for(i = 0; i < self->groupCount; i++)
  {
    if(self->default_length == 0)
    {
      PUT32_V((self->groups)[i].groupSize);
    }
    else
    {
      assert((self->groups)[i].groupSize == self->default_length);
    }

    PUTBYTES((self->groups)[i].groupDescription, (self->groups)[i].groupSize);
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4SampleGroupDescriptionAtomPtr self = (MP4SampleGroupDescriptionAtomPtr)s;
  u32 i;

  err = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  assert(self->groupCount > 0);

  self->size += 12;

  self->default_length = (self->groups[0]).groupSize;
  self->size += self->default_length;

  for(i = 1; i < (self->groupCount); i++)
  {
    if((self->groups[i]).groupSize != self->default_length)
    {
      self->default_length = 0;
    }
    self->size += (self->groups[i]).groupSize;
  }

  if(self->default_length == 0)
  {
    self->size += (self->groupCount) * 4;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i;
  MP4SampleGroupDescriptionAtomPtr self = (MP4SampleGroupDescriptionAtomPtr)s;
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

  if(self->version >= 1)
  {
    GET32(default_length);
  }
  if(self->version >= 2)
  {
    GET32(default_group_description_index);
  }

  GET32(groupCount);

  self->groups = calloc(self->groupCount, sizeof(sampleGroupEntry));
  TESTMALLOC(self->groups);

  for(i = 0; i < self->groupCount; i++)
  {
    u32 count;
    if(self->version >= 1)
    {
      if(self->default_length == 0)
      {
        GET32_V_MSG(count, NULL);
      }
      else
      {
        count = self->default_length;
      }
    }
    else
    {
      assert(self->size > self->bytesRead);
      count = self->size - self->bytesRead;
    }

    sprintf(msgString, " entry %d, size %d", i + 1, count);
    inputStream->msg(inputStream, msgString);

    (self->groups)[i].groupDescription = malloc(count);
    TESTMALLOC((self->groups)[i].groupDescription);
    (self->groups)[i].groupSize = count;

    GETBYTES_V(count, (self->groups)[i].groupDescription);
  }

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSampleGroupDescriptionAtom(MP4SampleGroupDescriptionAtomPtr *outAtom)
{
  MP4Err err;
  MP4SampleGroupDescriptionAtomPtr self;

  self = (MP4SampleGroupDescriptionAtomPtr)calloc(1, sizeof(MP4SampleGroupDescriptionAtom));
  TESTMALLOC(self)

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                    = MP4SampleGroupDescriptionAtomType;
  self->name                    = "sample group description";
  self->version                 = 1;
  self->createFromInputStream   = (cisfunc)createFromInputStream;
  self->destroy                 = destroy;
  self->calculateSize           = calculateSize;
  self->serialize               = serialize;
  self->addGroupDescription     = addGroupDescription;
  self->getGroupDescription     = getGroupDescription;
  self->findGroupDescriptionIdx = findGroupDescriptionIdx;
  self->default_length          = 0;
  self->groupCount              = 0;
  self->groups                  = NULL;

  self->default_group_description_index = 0;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
