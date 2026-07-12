/* This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4.
 * Those intending to use this software module in hardware or software products are advised that its
 * use may infringe existing patents. The original developer of this software module and his/her
 * company, the subsequent editors and their companies, and ISO/IEC have no liability for use of
 * this software module or modifications thereof in an implementation. Copyright is not released for
 * non MPEG-4 conforming products. Apple Computer, Inc. retains full right to use the code for its
 * own purpose, assign or donate the code to a third party and to inhibit third parties from using
 * the code for non MPEG-4 conforming products. This copyright notice must be included in all copies
 * or derivative works. Copyright (c) 2016.
 */

/*
 * PixelInformationProperty ('pixi') per ISO/IEC 23008-12 clause 6.5.6.
 */

#include "isoiff.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  ISOIFF_PixelInformationPropertyAtomPtr self = (ISOIFF_PixelInformationPropertyAtomPtr)s;
  err                                         = MP4NoErr;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i;
  ISOIFF_PixelInformationPropertyAtomPtr self = (ISOIFF_PixelInformationPropertyAtomPtr)s;
  err                                         = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUT8(num_channels);
  for(i = 0; i < self->num_channels; i++)
  {
    PUT8(bits_per_channel[i]);
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOIFF_PixelInformationPropertyAtomPtr self = (ISOIFF_PixelInformationPropertyAtomPtr)s;
  err                                         = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  self->size += 1 + self->num_channels; /* num_channels + bits_per_channel[] */
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i;
  ISOIFF_PixelInformationPropertyAtomPtr self = (ISOIFF_PixelInformationPropertyAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;
  GET8(num_channels);
  if(self->num_channels > 16) BAILWITHERROR(MP4BadDataErr)
  for(i = 0; i < self->num_channels; i++)
  {
    GET8(bits_per_channel[i]);
  }
  assert(self->bytesRead == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err ISOIFF_CreatePixelInformationPropertyAtom(ISOIFF_PixelInformationPropertyAtomPtr *outAtom)
{
  MP4Err err;
  ISOIFF_PixelInformationPropertyAtomPtr self;

  self =
    (ISOIFF_PixelInformationPropertyAtomPtr)calloc(1, sizeof(ISOIFF_PixelInformationPropertyAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOIFF_4CC_pixi;
  self->name                  = "Pixel Information Property";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
