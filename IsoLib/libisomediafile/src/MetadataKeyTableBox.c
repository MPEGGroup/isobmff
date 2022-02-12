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
  MP4MetadataKeyTableBoxPtr self;
  u32 i = 0;
  self  = (MP4MetadataKeyTableBoxPtr)s;

  if(self == NULL) return;

  if(self->metadataKeyBoxList)
  {
    u32 keyboxCnt;
    err = MP4GetListEntryCount(self->metadataKeyBoxList, &keyboxCnt);
    if(err) goto bail;
    for(i = 0; i < keyboxCnt; i++)
    {
      MP4AtomPtr a;
      err = MP4GetListEntry(self->metadataKeyBoxList, i, (char **)&a);
      if(err) goto bail;
      if(a) a->destroy(a);
    }
    err = MP4DeleteLinkedList(self->metadataKeyBoxList);
    if(err) goto bail;
  }

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);
  return;
}

static MP4Err addMetaDataKeyBox(MP4MetadataKeyTableBoxPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  err = MP4AddListEntry(atom, self->metadataKeyBoxList);
  if(err) goto bail;
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err                     = MP4NoErr;
  MP4MetadataKeyTableBoxPtr self = (MP4MetadataKeyTableBoxPtr)s;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  if(self->metadataKeyBoxList)
  {
    u32 count, i;
    MP4AtomPtr key;
    err = MP4GetListEntryCount(self->metadataKeyBoxList, &count);
    if(err) goto bail;
    for(i = 0; i < count; i++)
    {
      err = MP4GetListEntry(self->metadataKeyBoxList, i, (char **)&key);
      if(err) goto bail;
      if(key)
      {
        if(self->bytesWritten + key->size > self->size)
        {
          err = MP4IOErr;
          goto bail;
        }
        err = key->serialize(key, buffer);
        if(err) goto bail;
        self->bytesWritten += key->bytesWritten;
        buffer += key->bytesWritten;
      }
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
  MP4MetadataKeyTableBoxPtr self = (MP4MetadataKeyTableBoxPtr)s;
  err                            = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  if(self->metadataKeyBoxList)
  {
    u32 count, i;
    err = MP4GetListEntryCount(self->metadataKeyBoxList, &count);
    if(err) goto bail;
    for(i = 0; i < count; i++)
    {
      MP4AtomPtr key;
      err = MP4GetListEntry(self->metadataKeyBoxList, i, (char **)&key);
      if(err) goto bail;
      if(key)
      {
        err = key->calculateSize(key);
        if(err) goto bail;
        self->size += key->size;
      }
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4MetadataKeyTableBoxPtr self = (MP4MetadataKeyTableBoxPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  while(self->bytesRead < self->size)
  {
    MP4AtomPtr atom;
    err = MP4ParseAtom(inputStream, &atom);
    if(err) goto bail;
    self->bytesRead += atom->size;
    err = self->addMetaDataKeyBox(self, atom);
    if(err) goto bail;
  }

  if(self->bytesRead != self->size) BAILWITHERROR(MP4BadDataErr)

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateMetadataKeyTableBox(MP4MetadataKeyTableBoxPtr *outAtom)
{
  MP4Err err;
  MP4MetadataKeyTableBoxPtr self;

  self = (MP4MetadataKeyTableBoxPtr)calloc(1, sizeof(MP4MetadataKeyTableBox));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4MetadataKeyTableBoxType;
  self->name                  = "MetadataKeyTableBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addMetaDataKeyBox     = addMetaDataKeyBox;

  err = MP4MakeLinkedList(&self->metadataKeyBoxList);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
