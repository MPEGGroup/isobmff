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
  $Id: ItemPropertyContainerAtom.c,v 1.1.1.1 2016/09/14 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  u32 i, j;
  u32 entry_count;
  u32 association_count;
  MP4ItemPropertyAssociationEntryPtr entry;
  MP4ItemPropertyAssociationEntryPropertyIndexPtr propertyIndex;
  MP4ItemPropertyAssociationAtomPtr self = (MP4ItemPropertyAssociationAtomPtr)s;
  err                                    = MP4NoErr;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  err = MP4GetListEntryCount(self->entries, &entry_count);
  if(err) goto bail;

  for(i = 0; i < entry_count; i++)
  {
    err = MP4GetListEntry(self->entries, i, (char **)&entry);
    if(err) goto bail;

    if(entry->propertyIndexes == NULL) BAILWITHERROR(MP4InternalErr)

    err = MP4GetListEntryCount(entry->propertyIndexes, &association_count);
    if(err) goto bail;
    for(j = 0; j < association_count; j++)
    {
      err = MP4GetListEntry(entry->propertyIndexes, j, (char **)&propertyIndex);
      if(err) goto bail;
      free(propertyIndex);
    }

    err = MP4DeleteLinkedList(entry->propertyIndexes);
    if(err) goto bail;
    free(entry);
  }

  err = MP4DeleteLinkedList(self->entries);
  if(err) goto bail;

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);
  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i, j;
  u32 entry_count;
  u32 association_count;
  u32 property_index;
  MP4ItemPropertyAssociationEntryPtr entry;
  MP4ItemPropertyAssociationEntryPropertyIndexPtr propertyIndex;
  MP4ItemPropertyAssociationAtomPtr self = (MP4ItemPropertyAssociationAtomPtr)s;

  err = MP4NoErr;
  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  err = MP4GetListEntryCount(self->entries, &entry_count);
  if(err) goto bail;

  PUT32_V(entry_count);
  for(i = 0; i < entry_count; i++)
  {
    err = MP4GetListEntry(self->entries, i, (char **)&entry);
    if(err) goto bail;
    if(self->version < 1)
    {
      PUT16_V(entry->item_ID);
    }
    else
    {
      PUT32_V(entry->item_ID);
    }

    if(entry->propertyIndexes == NULL) BAILWITHERROR(MP4InternalErr)

    err = MP4GetListEntryCount(entry->propertyIndexes, &association_count);
    if(err) goto bail;
    PUT8_V(association_count);
    for(j = 0; j < association_count; j++)
    {
      err = MP4GetListEntry(entry->propertyIndexes, j, (char **)&propertyIndex);
      if(err) goto bail;
      property_index = propertyIndex->property_index;

      if(self->flags & 1)
      {
        property_index = property_index | (propertyIndex->essential << 15);
        PUT16_V(property_index);
      }
      else
      {
        property_index = property_index | (propertyIndex->essential << 7);
        PUT8_V(property_index);
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
  u32 i;
  u32 entry_count;
  u32 association_count;
  MP4ItemPropertyAssociationEntryPtr entry;
  MP4ItemPropertyAssociationAtomPtr self = (MP4ItemPropertyAssociationAtomPtr)s;
  err                                    = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  err = MP4GetListEntryCount(self->entries, &entry_count);
  if(err) goto bail;
  self->size += 4;
  for(i = 0; i < entry_count; i++)
  {
    err = MP4GetListEntry(self->entries, i, (char **)&entry);
    if(err) goto bail;
    if(self->version < 1) self->size += 2;
    else
      self->size += 4;

    if(entry->propertyIndexes == NULL) BAILWITHERROR(MP4InternalErr)

    err = MP4GetListEntryCount(entry->propertyIndexes, &association_count);
    if(err) goto bail;
    self->size += 1;

    if(self->flags & 1) self->size += 2 * association_count;
    else
      self->size += association_count;
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i, j;
  u32 entry_count;
  u32 association_count;
  MP4ItemPropertyAssociationEntryPtr entry;
  MP4ItemPropertyAssociationEntryPropertyIndexPtr propertyIndex;
  MP4ItemPropertyAssociationAtomPtr self = (MP4ItemPropertyAssociationAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET32_V(entry_count);

  for(i = 0; i < entry_count; i++)
  {
    entry = (MP4ItemPropertyAssociationEntryPtr)calloc(1, sizeof(MP4ItemPropertyAssociationEntry));
    TESTMALLOC(entry);
    err = MP4AddListEntry(entry, self->entries);
    if(err) goto bail;
    err = MP4MakeLinkedList(&entry->propertyIndexes);
    if(err) goto bail;

    if(self->version < 1)
    {
      GET16_V(entry->item_ID);
    }
    else
    {
      GET32_V(entry->item_ID);
    }

    GET8_V(association_count);
    for(j = 0; j < association_count; j++)
    {
      propertyIndex = (MP4ItemPropertyAssociationEntryPropertyIndexPtr)calloc(
        1, sizeof(MP4ItemPropertyAssociationEntryPropertyIndex));
      TESTMALLOC(propertyIndex);
      err = MP4AddListEntry(propertyIndex, entry->propertyIndexes);
      if(err) goto bail;

      if(self->flags & 1)
      {
        GET16_V(propertyIndex->property_index);
        propertyIndex->essential = (u8)(propertyIndex->property_index & (1 << 15));
        if(propertyIndex->essential == 1)
        {
          propertyIndex->property_index &= ~(1 << 15);
        }
      }
      else
      {
        GET8_V(propertyIndex->property_index);
        propertyIndex->essential = 0;
        if(propertyIndex->property_index & (1 << 7)) propertyIndex->essential = 1;

        if(propertyIndex->essential == 1)
        {
          propertyIndex->property_index &= ~(1 << 7);
        }
      }
    }
  }

  assert(self->bytesRead == self->size);

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err addEntry(MP4ItemPropertyAssociationAtomPtr self,
                       MP4ItemPropertyAssociationEntryPtr entry)
{
  MP4Err err;
  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);

  err = MP4AddListEntry(entry, self->entries);

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateItemPropertyAssociationAtom(MP4ItemPropertyAssociationAtomPtr *outAtom)
{
  MP4Err err;
  MP4ItemPropertyAssociationAtomPtr self;

  self = (MP4ItemPropertyAssociationAtomPtr)calloc(1, sizeof(MP4ItemPropertyAssociationAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4ItemPropertyAssociationAtomType;
  self->name                  = "Item Property Association";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addEntry              = addEntry;

  err = MP4MakeLinkedList(&self->entries);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
