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
 derivative works. Copyright (c) 2014.
 */
/*
  $Id: CompositionToDecodeAtom.c,v 1.1.1.1 2014/10/21 08:10:00 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

static void destroy(MP4AtomPtr s)
{
  MP4CompositionToDecodeAtomPtr self = (MP4CompositionToDecodeAtomPtr)s;

  if(self == NULL) return;
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4CompositionToDecodeAtomPtr self = (MP4CompositionToDecodeAtomPtr)s;
  err                                = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUT32(compositionToDTSShift);
  PUT32(leastDecodeToDisplayDelta);
  PUT32(greatestDecodeToDisplayDelta);
  PUT32(compositionStartTime);
  PUT32(compositionEndTime);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err updateFields(struct MP4Atom *s, u32 sampleCount, MP4Handle durationsH,
                           MP4Handle compositionOffsetsH)
{
  MP4Err err;
  u32 durationsCount;
  u32 onlyDuration;
  u32 offsetCount;
  s32 onlyOffset;
  s32 offset;
  s32 duration;
  s32 *offsets;
  s32 *durations;
  u32 i;

  MP4CompositionToDecodeAtomPtr self = (MP4CompositionToDecodeAtomPtr)s;
  err                                = MP4NoErr;
  onlyDuration                       = 0;
  onlyOffset                         = 0;

  offsets   = (s32 *)*compositionOffsetsH;
  durations = (s32 *)*durationsH;

  err = MP4GetHandleSize(compositionOffsetsH, &offsetCount);
  if(err) goto bail;
  offsetCount /= sizeof(u32);
  if(offsetCount == 1) onlyOffset = *(s32 *)*compositionOffsetsH;
  else if(offsetCount != sampleCount)
    BAILWITHERROR(MP4BadParamErr);

  err = MP4GetHandleSize(durationsH, &durationsCount);
  if(err) goto bail;
  durationsCount /= sizeof(u32);
  if(durationsCount == 1) onlyDuration = *(u32 *)*durationsH;
  else if(durationsCount != sampleCount)
    BAILWITHERROR(MP4BadParamErr);

  for(i = 0; i < sampleCount; i++)
  {
    if(durationsCount == 1) duration = onlyDuration;
    else
      duration = durations[i];

    if(offsetCount == 1) offset = onlyOffset;
    else
      offset = offsets[i];

    if(offset < self->leastDecodeToDisplayDelta) self->leastDecodeToDisplayDelta = offset;

    if(offset > self->greatestDecodeToDisplayDelta) self->greatestDecodeToDisplayDelta = offset;

    if(self->leastDecodeToDisplayDelta < 0)
      self->compositionToDTSShift = -1 * self->leastDecodeToDisplayDelta;

    if((self->totalDuration + offset) < self->compositionStartTime)
      self->compositionStartTime = (s32)(self->totalDuration + (s64)offset);

    if((self->totalDuration + offset) > self->compositionEndTime)
      self->compositionEndTime = (s32)(self->totalDuration + (s64)offset);

    self->totalDuration += duration;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4CompositionToDecodeAtomPtr self = (MP4CompositionToDecodeAtomPtr)s;
  err                                = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  self->size += 4 * 5;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4CompositionToDecodeAtomPtr self = (MP4CompositionToDecodeAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;
  GET32(compositionToDTSShift);
  GET32(leastDecodeToDisplayDelta);
  GET32(greatestDecodeToDisplayDelta);
  GET32(compositionStartTime);
  GET32(compositionEndTime);
  assert(self->bytesRead == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateCompositionToDecodeAtom(MP4CompositionToDecodeAtomPtr *outAtom)
{
  MP4Err err;
  MP4CompositionToDecodeAtomPtr self;

  self = (MP4CompositionToDecodeAtomPtr)calloc(1, sizeof(MP4CompositionToDecodeAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4CompositionToDecodeAtomType;
  self->name                  = "composition to decode";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->updateFields          = updateFields;

  self->compositionToDTSShift        = 0;
  self->leastDecodeToDisplayDelta    = INT_MAX; /*  2147483647; */
  self->greatestDecodeToDisplayDelta = INT_MIN; /* -2147483648; */
  self->compositionStartTime         = INT_MAX; /*  2147483647; */
  self->compositionEndTime           = INT_MIN; /* -2147483648; */
  self->totalDuration                = 0;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
