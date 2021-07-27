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
 * @file StereoVideoAtom.c
 * @author Ahmed Hamza <Ahmed.Hamza@InterDigital.com>
 * @date January 19, 2018
 * @brief Implements functions for reading and writing StereoVideoAtom instances.
 */

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4StereoVideoAtomPtr self;

  self = (MP4StereoVideoAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  DESTROY_ATOM_LIST_F(atomList);

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);

  return;
}

static MP4Err addAtom(MP4StereoVideoAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);

  err = MP4AddListEntry(atom, self->atomList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 tmp32;
  u8 tmp8;
  u32 n;
  MP4StereoVideoAtomPtr self = (MP4StereoVideoAtomPtr)s;
  err                        = MP4NoErr;

  if(self->size > 0)
  {

    err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)self, buffer);
    if(err) goto bail;
    buffer += self->bytesWritten;

    tmp32 = (self->reserved << 2) + self->single_view_allowed;
    PUT32_V(tmp32);
    PUT32(stereo_scheme);
    PUT32(length);

    for(n = 0; n < self->length; n++)
    {
      tmp8 = self->stereo_indication_type[n];
      PUT8_V(tmp8);
    }

    SERIALIZE_ATOM_LIST(atomList);

    assert(self->bytesWritten == self->size);
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4StereoVideoAtomPtr self = (MP4StereoVideoAtomPtr)s;
  err                        = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += (3 * 4) + self->length;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i;
  u32 tmp32;

  MP4StereoVideoAtomPtr self = (MP4StereoVideoAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);

  GET32_V(tmp32);
  self->reserved            = (tmp32 >> 2) & 0x3FFF;
  self->single_view_allowed = (u8)(tmp32 & 0x3);
  GET32(stereo_scheme);
  GET32(length);
  self->stereo_indication_type = calloc(self->length, sizeof(u8));
  for(i = 0; i < self->length; i++)
  {
    GET8_V(self->stereo_indication_type[i]);
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateStereoVideoAtom(MP4StereoVideoAtomPtr *outAtom)
{
  MP4Err err;
  MP4StereoVideoAtomPtr self;

  self = (MP4StereoVideoAtomPtr)calloc(1, sizeof(MP4StereoVideoAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4StereoVideoAtomType;
  self->name                  = "StereoVideoBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addAtom               = addAtom;

  err = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;

  /*
  self->stereo_indication_type = (u8*)calloc(1, sizeof(u8));
  TESTMALLOC(self->stereo_indication_type);
  */

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
