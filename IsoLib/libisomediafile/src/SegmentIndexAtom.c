/*
 *This software module was originally developed by InterDigital, Inc.
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
 * products. InterDigital, Inc. retains full right to use the code for its own
 * purpose, assign or donate the code to a third party and to
 * inhibit third parties from using the code for non
 * MPEG-4 conforming products.
 *
 * This copyright notice must be included in all copies or
 * derivative works.
 */

/**
 * @file SegmentIndexAtom.c
 * @author Ahmed Hamza <Ahmed.Hamza@InterDigital.com>
 * @date April 19, 2018
 * @brief Implements functions for reading and writing SegmentIndexAtom instances.
 */

#include "MP4Atoms.h"
#include <stdlib.h>

static u32 getReferenceCount(struct MP4SegmentIndexAtom *self)
{
  u32 referenceCount = 0;
  MP4GetListEntryCount(self->referencesList, &referenceCount);
  return referenceCount;
}

static MP4Err addReference(struct MP4SegmentIndexAtom *self, u8 referenceType, u32 referencedSize,
                           u32 subsegmentDuration, u8 startsWithSAP, u8 SAPType, u32 SAPDeltaTime)
{
  MP4Err err;

  SIDXReferencePtr p;
  SIDXReferencePtr last;

  u32 referenceCount;
  u8 addnew;

  err = MP4NoErr;

  referenceCount = getReferenceCount(self);
  addnew         = 1;

  if(referenceCount > 0)
  {
    /* get last refernece */
    err = MP4GetListEntry(self->referencesList, referenceCount - 1, (char **)&last);
    if(err) goto bail;
  }

  if(addnew)
  {

    p = (SIDXReferencePtr)calloc(1, sizeof(SIDXReference));
    TESTMALLOC(p);

    p->referenceType      = referenceType;
    p->referencedSize     = referencedSize;
    p->subsegmentDuration = subsegmentDuration;
    p->startsWithSAP      = startsWithSAP;
    p->SAPType            = SAPType;
    p->SAPDeltaTime       = SAPDeltaTime;
    self->referenceCount++;
    /* add reference to linked list */
    err = MP4AddListEntry(p, self->referencesList);
    if(err) goto bail;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  u32 referenceCount;
  MP4SegmentIndexAtomPtr self;

  self = (MP4SegmentIndexAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  referenceCount = getReferenceCount(self);

  if(referenceCount)
  {
    u32 i;
    for(i = 0; i < referenceCount; i++)
    {
      char *p;
      err = MP4GetListEntry(self->referencesList, i, &p);
      if(err) goto bail;
      if(p)
      {
        free(p);
      }
    }
  }

  MP4DeleteLinkedList(self->referencesList);

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i;
  u32 tmp32;

  MP4SegmentIndexAtomPtr self = (MP4SegmentIndexAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtom *)s, buffer);
  if(err) goto bail; /* Full Atom */
  buffer += self->bytesWritten;

  PUT32(referenceId);
  PUT32(timescale);

  PUT32(earliestPresentationTime);
  PUT32(firstOffset);

  PUT16(reserved1);
  PUT16(referenceCount);

  for(i = 0; i < self->referenceCount; i++)
  {

    SIDXReferencePtr reference;
    err = MP4GetListEntry(self->referencesList, i, (char **)&reference);
    if(err) goto bail;

    tmp32 = (reference->referenceType << 31) | (reference->referencedSize);
    PUT32_V(tmp32);

    PUT32_V(reference->subsegmentDuration);

    tmp32 = (reference->startsWithSAP << 31) | (reference->SAPType << 28) | reference->SAPDeltaTime;
    PUT32_V(tmp32);
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  u32 i;
  MP4SegmentIndexAtomPtr self = (MP4SegmentIndexAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += (4 * 2) + (4 * 2) + (2 * 2);

  self->size += (4 * self->referenceCount);

  for(i = 0; i < self->referenceCount; i++)
  {
    self->size += 12;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i;
  u32 tmp32;

  MP4SegmentIndexAtomPtr self = (MP4SegmentIndexAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);

  GET32(referenceId);
  GET32(timescale);

  GET32(earliestPresentationTime);
  GET32(firstOffset);

  GET16(reserved1);
  GET16(referenceCount);

  for(i = 0; i < self->referenceCount; i++)
  {

    u8 referenceType;
    u32 referencedSize;
    u32 subsegmentDuration;
    u8 startsWithSAP;
    u8 SAPType;
    u32 SAPDeltaTime;

    GET32_V(tmp32);
    referenceType  = (tmp32 >> 31) & 0x01;
    referencedSize = tmp32 & 0x7FFF;
    GET32_V(subsegmentDuration);
    GET32_V(tmp32);
    startsWithSAP = (tmp32 >> 31) & 0x01;
    SAPType       = (tmp32 >> 28) & 0x07;
    SAPDeltaTime  = tmp32 & 0x0FFFFFFF;

    err = addReference(self, referenceType, referencedSize, subsegmentDuration, startsWithSAP,
                       SAPType, SAPDeltaTime);
    if(err) goto bail;
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSegmentIndexAtom(MP4SegmentIndexAtomPtr *outAtom)
{
  MP4Err err;
  MP4SegmentIndexAtomPtr self;

  self = (MP4SegmentIndexAtomPtr)calloc(1, sizeof(MP4SegmentIndexAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;

  err = MP4MakeLinkedList(&self->referencesList);
  if(err) goto bail;

  self->type                  = MP4SegmentIndexAtomType;
  self->name                  = "SegmentIndexBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addReference          = addReference;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
