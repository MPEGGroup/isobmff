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
  MP4MetadataLocaleBoxPtr self;
  self = (MP4MetadataLocaleBoxPtr)s;

  if(self == NULL) return;

  if(self->locale_string)
  {
    free(self->locale_string);
    self->locale_string = NULL;
  }

  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  u32 strLen;
  MP4Err err                   = MP4NoErr;
  MP4MetadataLocaleBoxPtr self = (MP4MetadataLocaleBoxPtr)s;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  strLen = (u32)strlen(self->locale_string) + 1;
  PUTBYTES(self->locale_string, strLen);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4MetadataLocaleBoxPtr self = (MP4MetadataLocaleBoxPtr)s;
  err                          = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  if((self->locale_string) && (strlen(self->locale_string) > 0))
  {
    self->size += (u32)strlen(self->locale_string) + 1; /* strlen ignores \0 */
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  long bytesToRead;
  MP4MetadataLocaleBoxPtr self = (MP4MetadataLocaleBoxPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  bytesToRead = (long)self->size - self->bytesRead;
  if(bytesToRead < 0) BAILWITHERROR(MP4BadDataErr);
  if(bytesToRead > 0)
  {
    self->locale_string = (char *)calloc(1, bytesToRead);
    if(self->locale_string == NULL) BAILWITHERROR(MP4NoMemoryErr);
    GETBYTES(bytesToRead, locale_string);
  }

  if(self->bytesRead != self->size) BAILWITHERROR(MP4BadDataErr)

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateMetadataLocaleBox(MP4MetadataLocaleBoxPtr *outAtom, char *locale_string)
{
  MP4Err err;
  MP4MetadataLocaleBoxPtr self;

  self = (MP4MetadataLocaleBoxPtr)calloc(1, sizeof(MP4MetadataLocaleBox));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4MetadataLocaleBoxType;
  self->name                  = "MetadataLocaleBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  if(locale_string != NULL)
  {
    u32 str_size        = (u32)strlen(locale_string);
    self->locale_string = (char *)calloc(1, str_size + 1);
    TESTMALLOC(self->locale_string);
    memcpy(self->locale_string, locale_string, str_size);
    self->locale_string[str_size] = '\0';
  }

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
