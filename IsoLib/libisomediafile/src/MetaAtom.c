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
 * or derivative works. Copyright (c) 1999.
 */

/*
  $Id: MetaAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include "MP4DataHandler.h"
#include "MdatDataHandler.h"
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  ISOMetaAtomPtr self;
  u32 i;

  self = (ISOMetaAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  DESTROY_ATOM_LIST

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);
  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  ISOMetaAtomPtr self = (ISOMetaAtomPtr)s;
  err                 = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  SERIALIZE_ATOM_LIST(atomList);
  assert(self->bytesWritten == self->size);

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err closeDataHandler(ISOMetaAtomPtr self)
{
  MP4Err err;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryAtomPtr dataEntryAtom;
  MP4DataHandlerPtr dhlr;

  err = MP4NoErr;

  dhlr = (MP4DataHandlerPtr)self->dataHandler;
  if(!dhlr) return MP4NoErr;

  if(self->dataEntryIndex == 0)
  {
    err = dhlr->close(self->dataHandler);
    if(err) goto bail;
  }
  else if(self->dataEntryIndex != -1)
  {
    dinf = (MP4DataInformationAtomPtr)self->dinf;
    if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
    dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
    if(dref == NULL) BAILWITHERROR(MP4InvalidMediaErr);

    err = dref->getEntry(dref, self->dataEntryIndex, &dataEntryAtom);
    if(err) goto bail;
    if(dataEntryAtom == NULL) BAILWITHERROR(MP4InvalidMediaErr)
    err = MP4DisposeDataHandler(dhlr, dataEntryAtom);
    if(err) goto bail;
  }
  self->dataHandler    = NULL;
  self->dataEntryIndex = -1;

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err openDataHandler(ISOMetaAtomPtr self, u32 dataEntryIndex)
{
  MP4Err err;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryAtomPtr dataEntryAtom;

  err = MP4NoErr;

  if(dataEntryIndex == 0)
  {
    err = MP4CreateMdatDataHandler((MP4MediaDataAtomPtr)self->mdat,
                                   (struct MP4DataHandler **)&self->dataHandler);
    if(err) goto bail;
  }
  else
  {
    dinf = (MP4DataInformationAtomPtr)self->dinf;
    if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
    dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
    if(dref == NULL) BAILWITHERROR(MP4InvalidMediaErr);

    if(dataEntryIndex > dref->getEntryCount(dref)) BAILWITHERROR(MP4BadParamErr)
    err = dref->getEntry(dref, dataEntryIndex, &dataEntryAtom);
    if(err) goto bail;
    if(dataEntryAtom == NULL) BAILWITHERROR(MP4InvalidMediaErr)

    err = MP4CreateDataHandler(self->inputStream, dataEntryAtom,
                               (struct MP4DataHandler **)&self->dataHandler);
    if(err) goto bail;
  }
  self->dataEntryIndex = dataEntryIndex;

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOMetaAtomPtr self = (ISOMetaAtomPtr)s;
  err                 = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  ADD_ATOM_LIST_SIZE(atomList);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err addAtom(ISOMetaAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  ISOItemLocationAtomPtr iloc;
  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);
  switch(atom->type)
  {
  case MP4HandlerAtomType:
    if(self->hdlr)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->hdlr = atom;
    break;

  case MP4DataInformationAtomType:
    if(self->dinf)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->dinf = atom;
    break;

  case ISOItemLocationAtomType:
    if(self->iloc)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->iloc = atom;
    iloc       = (ISOItemLocationAtomPtr)atom;
    err        = iloc->setItemsMeta(iloc, (MP4AtomPtr)self);
    if(err) goto bail;
    break;

  case ISOPrimaryItemAtomType:
    if(self->pitm)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->pitm = atom;
    break;

  case ISOItemInfoAtomType:
    if(self->iinf)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->iinf = atom;
    break;

  case ISOItemReferenceAtomType:
    if(self->iref)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->iref = atom;
    break;

  case ISOItemDataAtomType:
    if(self->idat)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->idat = atom;
    break;

  case ISOItemProtectionAtomType:
    if(self->ipro)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->ipro = atom;
    break;
  case MP4ItemPropertiesAtomType:
    if(self->iprp)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->iprp = atom;
    break;
  case MP4GroupsListBoxType:
    if(self->grpl)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    self->grpl = atom;
    break;
  }
  err = MP4AddListEntry(atom, self->atomList);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err getData(ISOMetaAtomPtr self, u32 box_type, MP4Handle data, u8 is_full_atom)
{
  MP4Err err;
  u32 i, count;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  if(self->atomList)
  {
    err = MP4GetListEntryCount(self->atomList, &count);
    if(err) goto bail;

    for(i = 0; i < count; i++)
    {
      MP4UnknownAtomPtr a;
      err = MP4GetListEntry(self->atomList, i, (char **)&a);
      if(err) goto bail;

      if(a->type == box_type)
      {
        u32 len;
        char *dataPtr;
        len     = a->dataSize;
        dataPtr = a->data;
        if(is_full_atom)
        {
          len -= 4;
          dataPtr += 4;
        }
        if(len <= 0) BAILWITHERROR(MP4BadDataErr);
        err = MP4SetHandleSize(data, len);
        if(err) goto bail;
        memcpy(*data, dataPtr, len);
        goto bail;
      }
    }
  }
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  ISOMetaAtomPtr self = (ISOMetaAtomPtr)s;
  err                 = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  while(self->bytesRead < self->size)
  {
    MP4AtomPtr atom;
    u64 currentOffset               = 0;
    u64 available                   = inputStream->available;
    u32 indent                      = inputStream->indent;
    MP4FileMappingInputStreamPtr fm = (MP4FileMappingInputStreamPtr)inputStream;
    currentOffset                   = fm->current_offset;
    err                             = MP4ParseAtom((MP4InputStreamPtr)inputStream, &atom);

    if(err == MP4BadDataErr && self->bytesRead == 12)
    {
      /* most likely we are parsing QTFF MetaBox which is a Box and not a FullBox */
      inputStream->available = available + 4;
      inputStream->indent    = indent;
      fm->current_offset     = currentOffset - 4;
      self->bytesRead        = 8;
      self->flags            = 0;
      self->version          = 0;

      err = MP4ParseAtom((MP4InputStreamPtr)inputStream, &atom);
    }
    if(err) goto bail;

    self->bytesRead += atom->size;
    if(((atom->type) == MP4FreeSpaceAtomType) || ((atom->type) == MP4SkipAtomType))
      atom->destroy(atom);
    else
    {
      err = addAtom(self, atom);
      if(err) goto bail;
    }
  }
  if(self->bytesRead != self->size) BAILWITHERROR(MP4BadDataErr)

  self->inputStream = inputStream;
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err setMdat(ISOMetaAtomPtr self, MP4AtomPtr mdat)
{
  self->mdat = mdat;
  return MP4NoErr;
}

static MP4Err mdatMoved(ISOMetaAtomPtr self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset)
{
  MP4Err err;
  ISOItemLocationAtomPtr iloc;
  err  = MP4NoErr;
  iloc = (ISOItemLocationAtomPtr)self->iloc;

  if(iloc) err = iloc->mdatMoved(iloc, mdatBase, mdatEnd, mdatOffset);

  TEST_RETURN(err);
  return err;
}

MP4Err ISOCreateMetaAtom(ISOMetaAtomPtr *outAtom)
{
  MP4Err err;
  ISOMetaAtomPtr self;

  self = (ISOMetaAtomPtr)calloc(1, sizeof(ISOMetaAtom));
  TESTMALLOC(self)

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOMetaAtomType;
  self->name                  = "meta";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  err                         = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;
  self->calculateSize    = calculateSize;
  self->serialize        = serialize;
  self->addAtom          = addAtom;
  self->setMdat          = setMdat;
  self->mdatMoved        = mdatMoved;
  self->getData          = getData;
  self->next_item_ID     = 1;
  self->openDataHandler  = openDataHandler;
  self->closeDataHandler = closeDataHandler;
  self->dataEntryIndex   = -1;
  self->relatedMeco      = NULL;
  self->iref             = NULL;
  self->iprp             = NULL;
  self->grpl             = NULL;

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
