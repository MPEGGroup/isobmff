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
 $Id: JPEGConfigurationAtom.c,v 1.1.1.1 2016/12/30 armin Exp $
 */

#include "isoiff_jpeg.h"

#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  ISOIFF_JPEGConfigurationAtomPtr self = (ISOIFF_JPEGConfigurationAtomPtr)s;
  err                                  = MP4NoErr;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 configSize;
  ISOIFF_JPEGConfigurationAtomPtr self = (ISOIFF_JPEGConfigurationAtomPtr)s;
  err                                  = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields((MP4AtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  err = MP4GetHandleSize(self->jpegPrefix, &configSize);
  if(err) goto bail;

  PUTBYTES(*self->jpegPrefix, configSize);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  u32 configSize;
  ISOIFF_JPEGConfigurationAtomPtr self = (ISOIFF_JPEGConfigurationAtomPtr)s;
  err                                  = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize((MP4AtomPtr)s);
  if(err) goto bail;

  err = MP4GetHandleSize(self->jpegPrefix, &configSize);
  if(err) goto bail;
  self->size += configSize;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  long bytesToRead;
  ISOIFF_JPEGConfigurationAtomPtr self = (ISOIFF_JPEGConfigurationAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  bytesToRead = s->size - s->bytesRead;
  err         = MP4NewHandle(bytesToRead, &self->jpegPrefix);
  if(err) goto bail;
  GETBYTES_V(bytesToRead, *self->jpegPrefix);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err ISOIFF_CreateJPEGConfigurationAtom(ISOIFF_JPEGConfigurationAtomPtr *outAtom,
                                          MP4Handle jpegPrefix)
{
  MP4Err err;
  ISOIFF_JPEGConfigurationAtomPtr self;

  self = (ISOIFF_JPEGConfigurationAtomPtr)calloc(1, sizeof(ISOIFF_JPEGConfigurationAtom));
  TESTMALLOC(self);

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = ISOIFF_4CC_jpgC;
  self->name                  = "JPEG Configuration";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  err = MP4NewHandle(0, &self->jpegPrefix);
  if(err) goto bail;
  err = MP4HandleCat(self->jpegPrefix, jpegPrefix);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
