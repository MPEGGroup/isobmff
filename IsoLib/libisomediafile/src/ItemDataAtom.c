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

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  ISOItemDataAtomPtr self;

  err  = MP4NoErr;
  self = (ISOItemDataAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  err = MP4DisposeHandle(self->data);
  if(err) goto bail;

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  ISOItemDataAtomPtr self = (ISOItemDataAtomPtr)s;
  u32 dataSize;
  err = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  err = MP4GetHandleSize(self->data, &dataSize);
  if(err) goto bail;
  PUTBYTES((char *)*self->data, dataSize);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  u32 size;
  ISOItemDataAtomPtr self = (ISOItemDataAtomPtr)s;
  err                     = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  err = MP4GetHandleSize(self->data, &size);
  if(err) goto bail;

  self->size += size;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  long bytesToRead;
  ISOItemDataAtomPtr self = (ISOItemDataAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  bytesToRead = self->size - self->bytesRead;

  err = MP4SetHandleSize(self->data, bytesToRead);
  if(err) goto bail;
  if(bytesToRead > 0)
  {
    GETBYTES_V(bytesToRead, (char *)*self->data);
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getData(MP4AtomPtr s, char *target, u32 offset, u32 length)
{
  MP4Err err;
  ISOItemDataAtomPtr self = (ISOItemDataAtomPtr)s;
  char *dataBuffer;
  u32 dataSize;

  err        = MP4NoErr;
  dataBuffer = (char *)*self->data;

  err = MP4GetHandleSize(self->data, &dataSize);
  if(err) goto bail;

  if((dataSize - offset) < length) BAILWITHERROR(MP4BadParamErr);

  memcpy(target, dataBuffer + offset, length);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err ISOCreateItemDataAtom(ISOItemDataAtomPtr *outAtom)
{
  MP4Err err;
  ISOItemDataAtomPtr self;

  self = (ISOItemDataAtomPtr)calloc(1, sizeof(ISOItemDataAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOItemDataAtomType;
  self->name                  = "item data";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->getData               = getData;

  err = MP4NewHandle(0, &self->data);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
