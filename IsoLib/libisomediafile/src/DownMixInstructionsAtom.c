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
  $Id: DownMixInstructionsAtom.c,v 1.1.1.1 2014/09/18 08:10:00 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4DownMixInstructionsAtomPtr self = (MP4DownMixInstructionsAtomPtr)s;

  if(self == NULL) return;
  if(self->bs_downmix_coefficients) free(self->bs_downmix_coefficients);
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  u8 tmp8;
  MP4Err err;
  u16 i;
  u16 j;
  u32 index;
  MP4DownMixInstructionsAtomPtr self = (MP4DownMixInstructionsAtomPtr)s;
  err                                = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUT8(targetLayout);
  tmp8 = (self->reserved << 7) + self->targetChannelCount;
  PUT8_V(tmp8);
  tmp8 = (self->in_stream << 7) + self->downmix_ID;
  PUT8_V(tmp8);

  if(self->in_stream == 0)
  {
    for(i = 0; i < self->targetChannelCount; i++)
    {
      for(j = 0; j < self->baseChannelCount; j++)
      {
        index = (i * self->baseChannelCount) + j;
        if((index % 2) == 0)
        {
          tmp8 = self->bs_downmix_coefficients[index] << 4;
        }
        else
        {
          tmp8 += self->bs_downmix_coefficients[index];
          PUT8_V(tmp8);
        }
      }
    }
    if(((self->baseChannelCount * self->targetChannelCount) % 2) == 1)
    {
      tmp8 += 0x0F;
      PUT8_V(tmp8);
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
  MP4DownMixInstructionsAtomPtr self = (MP4DownMixInstructionsAtomPtr)s;
  err                                = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += 3;

  if(self->in_stream == 0)
  {
    self->size += (self->baseChannelCount * self->targetChannelCount) / 2;
    self->size += (self->baseChannelCount * self->targetChannelCount) % 2;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  u32 tmp8;
  u16 count;
  MP4Err err;
  MP4DownMixInstructionsAtomPtr self = (MP4DownMixInstructionsAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  count = 0;
  GET8(targetLayout);
  GET8_V(tmp8);
  self->reserved           = (u8)(tmp8 >> 7);
  self->targetChannelCount = tmp8 & 0x7F;
  GET8_V(tmp8);
  self->in_stream  = (u8)(tmp8 >> 7);
  self->downmix_ID = tmp8 & 0x7F;

  while(self->bytesRead < self->size)
  {
    self->bs_downmix_coefficients = realloc(self->bs_downmix_coefficients, count + 2);
    GET8_V(tmp8);
    self->bs_downmix_coefficients[count]     = (u8)(tmp8 >> 4);
    self->bs_downmix_coefficients[count + 1] = tmp8 & 0x0F;
    count += 2;
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateDownMixInstructionsAtom(MP4DownMixInstructionsAtomPtr *outAtom)
{
  MP4Err err;
  MP4DownMixInstructionsAtomPtr self;

  self = (MP4DownMixInstructionsAtomPtr)calloc(1, sizeof(MP4DownMixInstructionsAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                    = MP4DownMixInstructionsAtomType;
  self->name                    = "down mix instructions";
  self->createFromInputStream   = (cisfunc)createFromInputStream;
  self->destroy                 = destroy;
  self->calculateSize           = calculateSize;
  self->serialize               = serialize;
  self->bs_downmix_coefficients = NULL;
  self->reserved                = 0;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
