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
 * @file SubsegmentIndexAtom.c
 * @author Ahmed Hamza <Ahmed.Hamza@InterDigital.com>
 * @date April 19, 2018
 * @brief Implements functions for reading and writing SubsegmentIndexAtom instances.
 */

#include "MP4Atoms.h"
#include <stdlib.h>

static u32 getSubsegmentCount(struct MP4SubsegmentIndexAtom *self)
{
  u32 subsegmentCount = 0;
  MP4GetListEntryCount(self->subsegmentsList, &subsegmentCount);
  return subsegmentCount;
}

static MP4Err addSubsegment(struct MP4SubsegmentIndexAtom *self, struct Subsegment *ss)
{
  MP4Err err;
  err = MP4NoErr;

  err = MP4AddListEntry(ss, self->subsegmentsList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSubsegmentRange(struct Subsegment *self, u8 level, u32 rangeSize)
{
  MP4Err err;
  SubsegmentRangePtr p;

  err = MP4NoErr;

  p = (SubsegmentRangePtr)calloc(1, sizeof(SubsegmentRange));
  TESTMALLOC(p);

  p->level     = level;
  p->rangeSize = rangeSize;

  /* add range to linked list */
  err = MP4AddListEntry(p, self->rangesList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  u32 subsegmentCount;
  u32 rangeCount;
  MP4SubsegmentIndexAtomPtr self;

  self = (MP4SubsegmentIndexAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  subsegmentCount = getSubsegmentCount(self);

  if(subsegmentCount)
  {
    u32 i;
    for(i = 0; i < subsegmentCount; i++)
    {
      SubsegmentPtr p;
      err = MP4GetListEntry(self->subsegmentsList, i, (char **)&p);
      if(err) goto bail;
      if(p)
      {

        rangeCount = p->rangeCount;
        if(rangeCount)
        {
          u32 j;
          for(j = 0; j < rangeCount; j++)
          {

            char *r;
            err = MP4GetListEntry(p->rangesList, j, &r);
            if(err) goto bail;

            if(r)
            {
              free(r);
            }
          }
        }

        MP4DeleteLinkedList(p->rangesList);

        free(p);
      }
    }
  }

  MP4DeleteLinkedList(self->subsegmentsList);

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i, j;
  u32 tmp32;

  MP4SubsegmentIndexAtomPtr self = (MP4SubsegmentIndexAtomPtr)s;
  err                            = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtom *)s, buffer);
  if(err) goto bail; /* Full Atom */
  buffer += self->bytesWritten;

  PUT32(subsegmentCount);

  for(i = 0; i < self->subsegmentCount; i++)
  {

    SubsegmentPtr ss;
    err = MP4GetListEntry(self->subsegmentsList, i, (char **)&ss);
    if(err) goto bail;

    PUT32_V(ss->rangeCount);

    for(j = 0; j < ss->rangeCount; j++)
    {

      SubsegmentRangePtr ssRange;
      err = MP4GetListEntry(ss->rangesList, i, (char **)&ssRange);
      if(err) goto bail;

      tmp32 = ssRange->level;
      tmp32 = (tmp32 << 28) | ssRange->rangeSize;

      PUT32_V(tmp32);
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  u32 i;

  MP4SubsegmentIndexAtomPtr self = (MP4SubsegmentIndexAtomPtr)s;
  err                            = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += 4;

  for(i = 0; i < self->subsegmentCount; i++)
  {
    SubsegmentPtr ss;
    self->size += 4;
    err = MP4GetListEntry(self->subsegmentsList, i, (char **)&ss);
    if(err) goto bail;

    self->size += ss->rangeCount * 4;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i, j;
  u32 tmp32;

  MP4SubsegmentIndexAtomPtr self = (MP4SubsegmentIndexAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);

  GET32(subsegmentCount);

  for(i = 0; i < self->subsegmentCount; i++)
  {

    SubsegmentPtr ss;
    ss = (SubsegmentPtr)calloc(1, sizeof(Subsegment));
    TESTMALLOC(ss);

    err = MP4MakeLinkedList(&ss->rangesList);
    if(err) goto bail;

    GET32_V(ss->rangeCount);

    for(j = 0; j < ss->rangeCount; j++)
    {

      u8 level;
      u32 rangeSize;

      GET32_V(tmp32);
      level     = (tmp32 >> 24) & 0xFF;
      rangeSize = tmp32 & 0x00FFFFFF;

      err = addSubsegmentRange(ss, level, rangeSize);
      if(err) goto bail;
    }

    err = addSubsegment(self, ss);
    if(err) goto bail;
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSubsegmentIndexAtom(MP4SubsegmentIndexAtomPtr *outAtom)
{
  MP4Err err;
  MP4SubsegmentIndexAtomPtr self;

  self = (MP4SubsegmentIndexAtomPtr)calloc(1, sizeof(MP4SubsegmentIndexAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;

  err = MP4MakeLinkedList(&self->subsegmentsList);
  if(err) goto bail;

  self->type                  = MP4SubsegmentIndexAtomType;
  self->name                  = "SubsegmentIndexBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addSubsegment         = addSubsegment;
  self->getSubsegmentCount    = getSubsegmentCount;
  self->addSubsegmentRange    = addSubsegmentRange;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
