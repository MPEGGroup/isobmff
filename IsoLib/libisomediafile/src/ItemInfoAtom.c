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
derivative works. Copyright (c) 1999.
*/
/*
  $Id: ItemInfoAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  ISOItemInfoAtomPtr self;
  u32 i;
  err  = MP4NoErr;
  self = (ISOItemInfoAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  DESTROY_ATOM_LIST

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err addAtom(ISOItemInfoAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;

  err = MP4AddListEntry(atom, self->atomList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getEntry(ISOItemInfoAtomPtr self, u32 itemID, ISOItemInfoEntryAtomPtr *outEntry)
{
  MP4Err err;
  u32 i;

  err = MP4NoErr;

  for(i = 0; i < self->atomList->entryCount; i++)
  {
    ISOItemInfoEntryAtomPtr entry;
    err = MP4GetListEntry(self->atomList, i, (char **)&entry);
    if(err) goto bail;
    if(entry->type == ISOItemInfoEntryAtomType)
    {
      if(entry->item_ID == itemID) *outEntry = entry;
    }
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  ISOItemInfoAtomPtr self = (ISOItemInfoAtomPtr)s;
  u32 countBase;

  err = MP4NoErr;
  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  err = MP4GetListEntryCount(self->atomList, &countBase);
  if(err) goto bail;

  if(self->version == 0)
  {
    PUT16_V(countBase);
  }
  else
  {
    PUT32_V(countBase);
  }

  SERIALIZE_ATOM_LIST(atomList); /* should be sorted by item_id! */
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOItemInfoAtomPtr self = (ISOItemInfoAtomPtr)s;
  err                     = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  self->size += 2;

  if(self->version != 0)
  {
    self->size += 2;
  }

  ADD_ATOM_LIST_SIZE(atomList);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  u32 in_count;
  u32 list_count;
  MP4Err err;
  ISOItemInfoAtomPtr self = (ISOItemInfoAtomPtr)s;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr);
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  if(self->version == 0)
  {
    GET16_V(in_count);
  }
  else
  {
    GET32_V(in_count);
  }

  while(self->bytesRead < self->size)
  {
    MP4AtomPtr atom;
    err = MP4ParseAtom((MP4InputStreamPtr)inputStream, &atom);
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

  err = MP4GetListEntryCount(self->atomList, &list_count);
  if(err) goto bail;
  if(list_count != in_count)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err ISOCreateItemInfoAtom(ISOItemInfoAtomPtr *outAtom)
{
  MP4Err err;
  ISOItemInfoAtomPtr self;

  self = (ISOItemInfoAtomPtr)calloc(1, sizeof(ISOItemInfoAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOItemInfoAtomType;
  self->name                  = "item info";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  err                         = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;
  self->calculateSize = calculateSize;
  self->serialize     = serialize;
  self->addAtom       = addAtom;
  self->getEntry      = getEntry;
  *outAtom            = self;
bail:
  TEST_RETURN(err);

  return err;
}
