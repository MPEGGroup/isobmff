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
  MP4MetadataKeyDeclarationBoxPtr self = (MP4MetadataKeyDeclarationBoxPtr)s;
  if(self == NULL) return;

  if(self->key_value != NULL)
  {
    err = MP4DisposeHandle(self->key_value);
    if(err) goto bail;
    self->key_value = NULL;
  }

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);
  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err                           = MP4NoErr;
  MP4MetadataKeyDeclarationBoxPtr self = (MP4MetadataKeyDeclarationBoxPtr)s;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUT32(key_namespace);

  if(self->key_value != NULL)
  {
    u32 key_value_size;
    char *handleBuffer;
    err = MP4GetHandleSize(self->key_value, &key_value_size);
    if(err) goto bail;

    if(key_value_size != 4 &&
       (self->key_namespace == MP4KeyNamespace_me4c || self->key_namespace == MP4KeyNamespace_uiso))
    {
      BAILWITHERROR(MP4BadDataErr); /* me4c and uiso require value to be 4 bytes */
    }

    handleBuffer = (char *)*self->key_value;
    PUTBYTES(handleBuffer, key_value_size);
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4MetadataKeyDeclarationBoxPtr self = (MP4MetadataKeyDeclarationBoxPtr)s;
  err                                  = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  self->size += 4; /* key_namespace */
  if(self->key_value != NULL)
  {
    u32 key_value_size;
    err = MP4GetHandleSize(self->key_value, &key_value_size);
    if(err) goto bail;
    self->size += key_value_size;
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  long bytesToRead;
  MP4MetadataKeyDeclarationBoxPtr self = (MP4MetadataKeyDeclarationBoxPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET32(key_namespace);

  bytesToRead = (long)self->size - self->bytesRead;
  if(bytesToRead < 0) BAILWITHERROR(MP4BadDataErr);
  if(bytesToRead > 0)
  {
    err = MP4SetHandleSize(self->key_value, bytesToRead);
    if(err) goto bail;
    GETBYTES_V_MSG(bytesToRead, (char *)*self->key_value, "key_value");
  }

  if(self->bytesRead != self->size)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateMetadataKeyDeclarationBox(MP4MetadataKeyDeclarationBoxPtr *outAtom, u32 key_ns,
                                          MP4Handle key_val)
{
  MP4Err err;
  MP4MetadataKeyDeclarationBoxPtr self;

  self = (MP4MetadataKeyDeclarationBoxPtr)calloc(1, sizeof(MP4MetadataKeyDeclarationBox));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4MetadataKeyDeclarationBoxType;
  self->name                  = "MetadataKeyDeclarationBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->key_namespace         = key_ns;

  err = MP4NewHandle(0, &self->key_value);
  if(err) goto bail;
  if(key_val != NULL)
  {
    u32 size1, size2;
    MP4GetHandleSize(key_val, &size2);

    if(size2 != 4 &&
       (self->key_namespace == MP4KeyNamespace_me4c || self->key_namespace == MP4KeyNamespace_uiso))
    {
      BAILWITHERROR(MP4BadParamErr); /* me4c and uiso require value to be 4 bytes */
    }
    err = MP4HandleCat(self->key_value, key_val);
    if(err) goto bail;
    MP4GetHandleSize(self->key_value, &size1);
    if(size1 != size2 || size2 == 0) BAILWITHERROR(MP4BadParamErr);
  }

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
