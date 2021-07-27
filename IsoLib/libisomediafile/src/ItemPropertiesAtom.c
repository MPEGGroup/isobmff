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
  $Id: ItemPropertiesAtom.c,v 1.1.1.1 2016/09/14 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4ItemPropertiesAtomPtr self = (MP4ItemPropertiesAtomPtr)s;
  if(self == NULL) return;
  if(self->super) self->super->destroy(s);
}

static MP4Err addAtom(MP4ItemPropertiesAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);

  switch(atom->type)
  {
  case MP4ItemPropertyContainerAtomType:
    self->ipco = (MP4ItemPropertyContainerAtomPtr)atom;
    break;

  case MP4ItemPropertyAssociationAtomType:
    self->ipma = (MP4ItemPropertyAssociationAtomPtr)atom;
    break;
  }

  err = MP4AddListEntry(atom, self->atomList);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addItemProperty(struct MP4ItemPropertiesAtom *self, MP4AtomPtr itemProperty,
                              u32 itemID, u8 essential)
{
  MP4Err err;
  u32 i;
  u32 property_index;
  u32 entry_count;
  MP4ItemPropertyAssociationEntryPtr entry;
  MP4ItemPropertyAssociationEntryPtr targetEntry;
  MP4ItemPropertyAssociationEntryPropertyIndexPtr propertyIndex;

  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);

  if(self->ipma == NULL)
  {
    err = MP4CreateItemPropertyAssociationAtom(&self->ipma);
    if(err) goto bail;
    err = MP4AddListEntry(self->ipma, self->atomList);
    if(err) goto bail;
  }

  err = MP4GetListEntryCount(self->ipco->atomList, &property_index);
  if(err) goto bail;
  err = self->ipco->addAtom(self->ipco, itemProperty);
  if(err) goto bail;

  err = MP4GetListEntryCount(self->ipma->entries, &entry_count);
  if(err) goto bail;

  targetEntry = NULL;
  for(i = 0; i < entry_count; i++)
  {
    err = MP4GetListEntry(self->ipma->entries, i, (char **)&entry);
    if(err) goto bail;
    if(itemID == entry->item_ID)
    {
      targetEntry = entry;
    }
  }

  if(targetEntry == NULL)
  {
    targetEntry =
      (MP4ItemPropertyAssociationEntryPtr)calloc(1, sizeof(MP4ItemPropertyAssociationEntry));
    TESTMALLOC(targetEntry);
    targetEntry->item_ID = itemID;
    err                  = MP4AddListEntry(targetEntry, self->ipma->entries);
    if(err) goto bail;
    err = MP4MakeLinkedList(&targetEntry->propertyIndexes);
    if(err) goto bail;
  }

  propertyIndex = (MP4ItemPropertyAssociationEntryPropertyIndexPtr)calloc(
    1, sizeof(MP4ItemPropertyAssociationEntryPropertyIndex));
  TESTMALLOC(propertyIndex);
  propertyIndex->essential      = essential;
  propertyIndex->property_index = (u16)(property_index + 1);
  err                           = MP4AddListEntry(propertyIndex, targetEntry->propertyIndexes);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getPropertiesOfItem(struct MP4ItemPropertiesAtom *self, u32 itemID,
                                  MP4LinkedList *properties)
{
  MP4Err err;
  u32 i;
  u32 entry_count;
  u32 association_count;
  MP4LinkedList outProperties;
  MP4ItemPropertyAssociationEntryPtr entry;
  MP4ItemPropertyAssociationEntryPtr targetEntry;
  MP4ItemPropertyAssociationEntryPropertyIndexPtr propertyIndex;
  MP4AtomPtr property;

  err = MP4NoErr;

  err = MP4MakeLinkedList(&outProperties);
  if(err) goto bail;
  err = MP4GetListEntryCount(self->ipma->entries, &entry_count);
  if(err) goto bail;

  targetEntry = NULL;
  for(i = 0; i < entry_count; i++)
  {
    err = MP4GetListEntry(self->ipma->entries, i, (char **)&entry);
    if(err) goto bail;
    if(itemID == entry->item_ID)
    {
      targetEntry = entry;
    }
  }

  if(targetEntry != NULL)
  {
    err = MP4GetListEntryCount(targetEntry->propertyIndexes, &association_count);
    if(err) goto bail;
    for(i = 0; i < association_count; i++)
    {
      err = MP4GetListEntry(targetEntry->propertyIndexes, i, (char **)&propertyIndex);
      if(err) goto bail;
      err = MP4GetListEntry(self->ipco->atomList, propertyIndex->property_index - 1,
                            (char **)&property);
      if(err) goto bail;
      err = MP4AddListEntry(property, outProperties);
    }
  }

  *properties = outProperties;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4ItemPropertiesAtomPtr self = (MP4ItemPropertiesAtomPtr)s;
  err                           = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields((MP4AtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  SERIALIZE_ATOM_LIST(atomList);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4ItemPropertiesAtomPtr self = (MP4ItemPropertiesAtomPtr)s;
  err                           = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize((MP4AtomPtr)s);
  if(err) goto bail;
  ADD_ATOM_LIST_SIZE(atomList);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  PARSE_ATOM_LIST(MP4ItemPropertiesAtom)
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateItemPropertiesAtom(MP4ItemPropertiesAtomPtr *outAtom)
{
  MP4Err err;
  MP4ItemPropertiesAtomPtr self;

  self = (MP4ItemPropertiesAtomPtr)calloc(1, sizeof(MP4ItemPropertiesAtom));
  TESTMALLOC(self);

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4ItemPropertiesAtomType;
  self->name                  = "item properties";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addItemProperty       = addItemProperty;
  self->getPropertiesOfItem   = getPropertiesOfItem;
  self->addAtom               = addAtom;
  self->ipma                  = NULL;

  err = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;

  err = MP4CreateItemPropertyContainerAtom(&self->ipco);
  if(err) goto bail;
  err = MP4AddListEntry(self->ipco, self->atomList);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
