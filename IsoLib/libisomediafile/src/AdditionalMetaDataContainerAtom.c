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
  ISOAdditionalMetaDataContainerAtomPtr self;
  err  = MP4NoErr;
  self = (ISOAdditionalMetaDataContainerAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  DESTROY_ATOM_LIST_F(metaList);
  DESTROY_ATOM_LIST_F(relationList);

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err addAtom(ISOAdditionalMetaDataContainerAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;

  switch(atom->type)
  {
  case ISOMetaAtomType:
    err = MP4AddListEntry(atom, self->metaList);
    if(err) goto bail;
    break;

  case ISOMetaboxRelationAtomType:
    err = MP4AddListEntry(atom, self->relationList);
    if(err) goto bail;
    break;
  default:
    break;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  ISOAdditionalMetaDataContainerAtomPtr self = (ISOAdditionalMetaDataContainerAtomPtr)s;
  err                                        = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);

  if(err)
  {
    goto bail;
  }

  buffer += self->bytesWritten;

  SERIALIZE_ATOM_LIST(metaList);
  SERIALIZE_ATOM_LIST(relationList);
  assert(self->bytesWritten == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOAdditionalMetaDataContainerAtomPtr self = (ISOAdditionalMetaDataContainerAtomPtr)s;
  err                                        = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);

  if(err)
  {
    goto bail;
  }

  ADD_ATOM_LIST_SIZE(metaList);
  ADD_ATOM_LIST_SIZE(relationList);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  PARSE_ATOM_LIST(ISOAdditionalMetaDataContainerAtom);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addMeta(struct ISOAdditionalMetaDataContainerAtom *self, MP4AtomPtr meta)
{
  u32 i;
  MP4Err err;
  ISOMetaAtomPtr metaPtr;

  err = MP4NoErr;

  metaPtr = (ISOMetaAtomPtr)meta;

  for(i = 0; i < self->metaList->entryCount; i++)
  {
    ISOMetaAtomPtr tempMetaPtr;
    err = MP4GetListEntry(self->metaList, i, (char **)&tempMetaPtr);
    if(err) goto bail;
    if(tempMetaPtr->type == metaPtr->type) BAILWITHERROR(MP4BadParamErr);
  }

  err = MP4AddListEntry(metaPtr, self->metaList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getMeta(struct ISOAdditionalMetaDataContainerAtom *self, u32 type,
                      ISOMetaAtomPtr *outMetaPtr)
{
  u32 i;
  MP4Err err;
  MP4HandlerAtomPtr hdlr;

  err         = MP4NoErr;
  *outMetaPtr = NULL;

  for(i = 0; i < self->metaList->entryCount; i++)
  {
    ISOMetaAtomPtr tempMetaPtr;
    err = MP4GetListEntry(self->metaList, i, (char **)&tempMetaPtr);
    if(err) goto bail;
    hdlr = (MP4HandlerAtomPtr)tempMetaPtr->hdlr;
    if(hdlr->handlerType == type)
    {
      *outMetaPtr = tempMetaPtr;
      break;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setMdat(struct ISOAdditionalMetaDataContainerAtom *self, MP4AtomPtr mdat)
{
  u32 i;
  MP4Err err;

  err = MP4NoErr;

  for(i = 0; i < self->metaList->entryCount; i++)
  {
    ISOMetaAtomPtr meta;
    err = MP4GetListEntry(self->metaList, i, (char **)&meta);
    if(err) goto bail;
    err = meta->setMdat(meta, mdat);
    if(err) goto bail;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mdatMoved(struct ISOAdditionalMetaDataContainerAtom *self, u64 mdatBase, u64 mdatEnd,
                        s32 mdatOffset)
{
  u32 i;
  MP4Err err;

  err = MP4NoErr;

  for(i = 0; i < self->metaList->entryCount; i++)
  {
    ISOMetaAtomPtr meta;
    err = MP4GetListEntry(self->metaList, i, (char **)&meta);
    if(err) goto bail;
    err = meta->mdatMoved(meta, mdatBase, mdatEnd, mdatOffset);
    if(err) goto bail;
  }
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err ISOCreateAdditionalMetaDataContainerAtom(ISOAdditionalMetaDataContainerAtomPtr *outAtom)
{
  MP4Err err;
  ISOAdditionalMetaDataContainerAtomPtr self;

  self =
    (ISOAdditionalMetaDataContainerAtomPtr)calloc(1, sizeof(ISOAdditionalMetaDataContainerAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOAdditionalMetaDataContainerAtomType;
  self->name                  = "Additional Metadata Container";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addMeta               = addMeta;
  self->getMeta               = getMeta;
  self->mdatMoved             = mdatMoved;
  self->setMdat               = setMdat;

  err = MP4MakeLinkedList(&self->metaList);
  if(err) goto bail;
  err = MP4MakeLinkedList(&self->relationList);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
