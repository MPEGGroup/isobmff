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
 * or derivative works. Copyright (c) 2026.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err                        = MP4NoErr;
  MP4T35MetadataSampleEntryPtr self = (MP4T35MetadataSampleEntryPtr)s;
  if(self == NULL) return;

  if(self->t35_identifier)
  {
    free(self->t35_identifier);
    self->t35_identifier = NULL;
  }

  DESTROY_ATOM_LIST_F(ExtensionAtomList);
  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);
  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4T35MetadataSampleEntryPtr self = (MP4T35MetadataSampleEntryPtr)s;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUTBYTES(self->reserved, 6);
  PUT16(dataReferenceIndex);

  if(self->t35_identifier_length < 1 || self->t35_identifier_length > 255 ||
     self->t35_identifier == NULL)
  {
    err = MP4BadParamErr;
    goto bail;
  }

  /* Write t35_identifier_length */
  PUT8(t35_identifier_length);

  /* Write t35_identifier byte array */
  PUTBYTES(self->t35_identifier, self->t35_identifier_length);

  SERIALIZE_ATOM_LIST(ExtensionAtomList);
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4T35MetadataSampleEntryPtr self = (MP4T35MetadataSampleEntryPtr)s;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  self->size += (6 + 2); /* reserved + dataReferenceIndex */

  if(self->t35_identifier_length < 1 || self->t35_identifier_length > 255 ||
     self->t35_identifier == NULL)
  {
    err = MP4BadParamErr;
    goto bail;
  }

  /* Add t35_identifier_length size */
  self->size += 1;
  /* Add t35_identifier size */
  self->size += self->t35_identifier_length;

  ADD_ATOM_LIST_SIZE(ExtensionAtomList);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4T35MetadataSampleEntryPtr self = (MP4T35MetadataSampleEntryPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GETBYTES(6, reserved);
  GET16(dataReferenceIndex);

  /* Read t35_identifier_length */
  err = inputStream->read8(inputStream, &self->t35_identifier_length, NULL);
  if(err) goto bail;
  ++self->bytesRead;
  if(self->t35_identifier_length == 0) BAILWITHERROR(MP4BadDataErr);
  if(self->t35_identifier_length > self->size - self->bytesRead) BAILWITHERROR(MP4BadDataErr);

  /* Read t35_identifier */
  self->t35_identifier = (u8 *)calloc(self->t35_identifier_length, 1);
  TESTMALLOC(self->t35_identifier);
  err = inputStream->readData(inputStream, self->t35_identifier_length,
                              (char *)self->t35_identifier, NULL);
  if(err) goto bail;
  self->bytesRead += self->t35_identifier_length;

  GETATOM_LIST(ExtensionAtomList);
  if(self->bytesRead != self->size) BAILWITHERROR(MP4BadDataErr)

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateT35MetadataSampleEntry(MP4T35MetadataSampleEntryPtr *outAtom)
{
  MP4Err err;
  MP4T35MetadataSampleEntryPtr self;

  self = (MP4T35MetadataSampleEntryPtr)calloc(1, sizeof(MP4T35MetadataSampleEntry));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4T35MetadataSampleEntryType;
  self->name                  = "T35MetadataSampleEntry";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  self->dataReferenceIndex = 1;
  memset(self->reserved, 0, 6);

  self->t35_identifier_length = 0;
  self->t35_identifier        = NULL;

  // Typically for MP4HumanReadableStreamDescriptionAtom.
  err = MP4MakeLinkedList(&self->ExtensionAtomList);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
