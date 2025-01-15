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
 * or derivative works. Copyright (c) 2022.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4MetadataSetupBoxPtr self = (MP4MetadataSetupBoxPtr)s;
  if(self == NULL) return;

  if(self->setup_data != NULL)
  {
    err = MP4DisposeHandle(self->setup_data);
    if(err) goto bail;
    self->setup_data = NULL;
  }

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);
  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err                  = MP4NoErr;
  MP4MetadataSetupBoxPtr self = (MP4MetadataSetupBoxPtr)s;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  if(self->setup_data != NULL)
  {
    u32 handle_size;
    char *handleBuffer;
    err = MP4GetHandleSize(self->setup_data, &handle_size);
    if(err) goto bail;
    handleBuffer = (char *)*self->setup_data;
    PUTBYTES(handleBuffer, handle_size);
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4MetadataSetupBoxPtr self = (MP4MetadataSetupBoxPtr)s;
  err                         = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  if(self->setup_data != NULL)
  {
    u32 handle_size;
    err = MP4GetHandleSize(self->setup_data, &handle_size);
    if(err) goto bail;
    self->size += handle_size;
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  long bytesToRead;
  MP4MetadataSetupBoxPtr self = (MP4MetadataSetupBoxPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  bytesToRead = (long)self->size - self->bytesRead;
  if(bytesToRead < 0) BAILWITHERROR(MP4BadDataErr);
  if(bytesToRead > 0)
  {
    err = MP4SetHandleSize(self->setup_data, bytesToRead);
    if(err) goto bail;
    GETBYTES_V_MSG(bytesToRead, (char *)*self->setup_data, "setup_data");
  }

  if(self->bytesRead != self->size)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateMetadataSetupBox(MP4MetadataSetupBoxPtr *outAtom, MP4Handle setupH)
{
  MP4Err err;
  MP4MetadataSetupBoxPtr self;

  self = (MP4MetadataSetupBoxPtr)calloc(1, sizeof(MP4MetadataSetupBox));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4MetadataSetupBoxType;
  self->name                  = "MetadataSetupBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  err = MP4NewHandle(0, &self->setup_data);
  if(err) goto bail;
  if(setupH != NULL)
  {
    u32 size1, size2;
    err = MP4HandleCat(self->setup_data, setupH);
    if(err) goto bail;
    MP4GetHandleSize(self->setup_data, &size1);
    MP4GetHandleSize(setupH, &size2);
    if(size1 != size2 || size2 == 0) BAILWITHERROR(MP4BadParamErr);
  }

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
