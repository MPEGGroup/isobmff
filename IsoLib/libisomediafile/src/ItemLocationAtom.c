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
  $Id: ItemLocationAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  ISOItemLocationAtomPtr self;
  u32 i, j;

  err  = MP4NoErr;
  self = (ISOItemLocationAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  if(self->itemList)
  {
    u32 listSize;
    err = MP4GetListEntryCount(self->itemList, &listSize);
    if(err) goto bail;
    for(i = 0; i < listSize; i++)
    {
      MetaItemLocationPtr a;
      err = MP4GetListEntry(self->itemList, i, (char **)&a);
      if(err) goto bail;
      if(a->extentList)
      {
        u32 list2Size;
        err = MP4GetListEntryCount(a->extentList, &list2Size);
        if(err) goto bail;
        for(j = 0; j < list2Size; j++)
        {
          MetaExtentLocationPtr b;
          err = MP4GetListEntry(a->extentList, j, (char **)&b);
          if(err) goto bail;
          if(b) free(b);
        }
        err = MP4DeleteLinkedList(a->extentList);
        if(err) goto bail;
      }
      if(a) free(a);
    }
    err = MP4DeleteLinkedList(self->itemList);
    if(err) goto bail;
  }

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i, j;
  u32 item_total;
  u8 x;

  ISOItemLocationAtomPtr self = (ISOItemLocationAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  item_total = 0;

  x = (self->offset_size << 4) | self->length_size;
  PUT8_V(x);
  x = (self->base_offset_size << 4) | self->index_size;
  PUT8_V(x);

  if(self->itemList)
  {
    err = MP4GetListEntryCount(self->itemList, &item_total);
    if(err) goto bail;
    PUT16_V(item_total);

    for(i = 0; i < item_total; i++)
    {
      u32 list2Size;
      MetaItemLocationPtr a;
      err = MP4GetListEntry(self->itemList, i, (char **)&a);
      if(err) goto bail;

      PUT16_V((a->item_ID));

      if(self->version == 1)
      {
        x = 0;
        PUT8_V(x);
        x = a->construction_method;
        PUT8_V(x);
      }

      PUT16_V((a->dref_index));
      if(self->base_offset_size == 8)
      {
        PUT64_V((a->base_offset));
      }
      else if(self->base_offset_size == 4)
      {
        u32 x1;
        x1 = (u32)a->base_offset;
        PUT32_V(x1);
      }

      list2Size = 0;

      if(a->extentList)
      {
        err = MP4GetListEntryCount(a->extentList, &list2Size);
        if(err) goto bail;
        PUT16_V(list2Size);

        for(j = 0; j < list2Size; j++)
        {
          MetaExtentLocationPtr b;
          err = MP4GetListEntry(a->extentList, j, (char **)&b);
          if(err) goto bail;

          if(self->version == 1)
          {
            if(self->index_size == 8)
            {
              PUT64_V((b->extent_index));
            }
            else if(self->index_size == 4)
            {
              u32 x2;
              x2 = (u32)b->extent_index;
              PUT32_V(x2);
            }
          }

          if(self->offset_size == 8)
          {
            PUT64_V((b->extent_offset));
          }
          else if(self->offset_size == 4)
          {
            u32 x3;
            x3 = (u32)b->extent_offset;
            PUT32_V(x3);
          }

          if(self->length_size == 8)
          {
            PUT64_V((b->extent_length));
          }
          else if(self->length_size == 4)
          {
            u32 x4;
            x4 = (u32)b->extent_length;
            PUT32_V(x4);
          }
        }
      }
      else
      {
        PUT16_V(list2Size);
      }
    }
  }
  else
  {
    PUT16_V(item_total);
  };

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOItemLocationAtomPtr self = (ISOItemLocationAtomPtr)s;
  u32 i, j;
  u32 item_total, extent_total;
  err = MP4NoErr;

  self->offset_size      = 0;
  self->length_size      = 0;
  self->base_offset_size = 0;
  self->index_size       = 0;

  item_total   = 0;
  extent_total = 0;

  if(self->itemList)
  {
    err = MP4GetListEntryCount(self->itemList, &item_total);
    if(err) goto bail;

    for(i = 0; i < item_total; i++)
    {
      MetaItemLocationPtr a;
      err = MP4GetListEntry(self->itemList, i, (char **)&a);
      if(err) goto bail;
      if(((a->base_offset) >> 32) > 0) self->base_offset_size = 8;
      else if((self->base_offset_size < 4) && ((a->base_offset) > 0))
        self->base_offset_size = 4;

      if((a->dref_index == 0) && (a->extentList))
      {
        if(self->offset_size < 4) self->offset_size = 4;
        if(self->base_offset_size < 4) self->base_offset_size = 4;
      }
      /* we have to allow space for the mdat to move up;  if this item has one extent, and it's at
         offset 0 in the mdat, we might conclude we need only have 0 bytes for offsets, but by the
         time we serialize, the offset (in the file now) is not zero */
      if(a->extentList)
      {
        u32 list2Size;
        err = MP4GetListEntryCount(a->extentList, &list2Size);
        if(err) goto bail;
        extent_total += list2Size;

        for(j = 0; j < list2Size; j++)
        {
          MetaExtentLocationPtr b;
          err = MP4GetListEntry(a->extentList, j, (char **)&b);
          if(err) goto bail;

          if(((b->extent_offset) >> 32) > 0) self->offset_size = 8;
          else if((self->offset_size < 4) && ((b->extent_offset) > 0))
            self->offset_size = 4;

          if(((b->extent_length) >> 32) > 0) self->length_size = 8;
          else if((self->length_size < 4) && ((b->extent_length) > 0))
            self->length_size = 4;

          if(((b->extent_index) >> 32) > 0) self->index_size = 8;
          else if((self->index_size < 4) && ((b->extent_index) > 0))
            self->index_size = 4;
        }
      }
    }
  }

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += 4 + item_total * (self->base_offset_size + 6) +
                extent_total * (self->offset_size + self->length_size);

  if(self->version == 1) self->size += item_total * 2 + extent_total * self->index_size;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mdatMoved(ISOItemLocationAtomPtr self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset)
{
  MP4Err err;
  u32 i, j, item_total;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  if(self->itemList)
  {
    err = MP4GetListEntryCount(self->itemList, &item_total);
    if(err) goto bail;

    for(i = 0; i < item_total; i++)
    {
      MetaItemLocationPtr a;
      err = MP4GetListEntry(self->itemList, i, (char **)&a);
      if(err) goto bail;

      if((a->construction_method == 0) && (a->dref_index == 0) && (a->base_offset >= mdatBase) &&
         (a->base_offset < mdatEnd))
        a->base_offset += mdatOffset;

      if((a->construction_method == 0) && (a->dref_index == 0) && (a->extentList))
      {
        u32 list2Size;
        err = MP4GetListEntryCount(a->extentList, &list2Size);
        if(err) goto bail;

        for(j = 0; j < list2Size; j++)
        {
          MetaExtentLocationPtr b;
          err = MP4GetListEntry(a->extentList, j, (char **)&b);
          if(err) goto bail;

          if((b->extent_offset >= mdatBase) && (b->extent_offset < mdatEnd))
            b->extent_offset += mdatOffset;
        }
      }
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setItemsMeta(ISOItemLocationAtomPtr self, MP4AtomPtr meta)
{
  MP4Err err;
  u32 i;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  if(self->itemList)
  {
    for(i = 0; i < self->itemList->entryCount; i++)
    {
      MetaItemLocationPtr a;
      err = MP4GetListEntry(self->itemList, i, (char **)&a);
      if(err) goto bail;
      a->meta = meta;
    }
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 x;
  u32 item_count;
  u32 i;

  ISOItemLocationAtomPtr self = (ISOItemLocationAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET8_V(x);
  self->offset_size = (u8)(x >> 4);
  self->length_size = (x & 0xF);
  GET8_V_MSG(x, "base offset size");
  self->base_offset_size = (u8)(x >> 4);
  self->index_size       = (x & 0xF);

  GET16_V(item_count);
  err = MP4MakeLinkedList(&(self->itemList));
  if(err) goto bail;

  for(i = 0; i < item_count; i++)
  {
    MetaItemLocationPtr a;
    u32 j, tmp;
    u32 extent_count;

    a   = calloc(1, sizeof(MetaItemLocation));
    err = MP4AddListEntry((void *)a, self->itemList);
    if(err) goto bail;

    GET16_V_MSG(tmp, "item_ID");
    a->item_ID = (u16)tmp;

    if(self->version == 1)
    {
      GET16_V_MSG(tmp, "construction_method");
      a->construction_method = (u8)tmp;
    }

    GET16_V_MSG(tmp, "dref_index");
    a->dref_index = (u16)tmp;
    switch(self->base_offset_size)
    {
    case 8:
      GET64_V_MSG((a->base_offset), "base_offset");
      break;
    case 4:
      GET32_V_MSG(tmp, "base_offset");
      a->base_offset = tmp;
      break;
    case 0:
      a->base_offset = 0;
      break;
    default:
      BAILWITHERROR(MP4BadDataErr);
    }
    GET16_V(extent_count);
    err = MP4MakeLinkedList(&(a->extentList));
    if(err) goto bail;

    for(j = 0; j < extent_count; j++)
    {
      MetaExtentLocationPtr b;
      b   = calloc(1, sizeof(MetaExtentLocation));
      err = MP4AddListEntry((void *)b, a->extentList);
      if(err) goto bail;

      if(self->version == 1)
      {
        switch(self->index_size)
        {
        case 8:
          GET64_V_MSG((b->extent_index), "extent_index");
          break;
        case 4:
          GET32_V_MSG(tmp, "extent_index");
          b->extent_index = tmp;
          break;
        case 0:
          b->extent_index = 0;
          break;
        default:
          BAILWITHERROR(MP4BadDataErr);
        }
      }

      switch(self->offset_size)
      {
      case 8:
        GET64_V_MSG((b->extent_offset), "extent_offset");
        break;
      case 4:
        GET32_V_MSG(tmp, "extent_offset");
        b->extent_offset = tmp;
        break;
      case 0:
        b->extent_offset = 0;
        break;
      default:
        BAILWITHERROR(MP4BadDataErr);
      }
      switch(self->length_size)
      {
      case 8:
        GET64_V_MSG((b->extent_length), "extent_length");
        break;
      case 4:
        GET32_V_MSG(tmp, "extent_length");
        b->extent_length = tmp;
        break;
      case 0:
        b->extent_length = 0;
        break;
      default:
        BAILWITHERROR(MP4BadDataErr);
      }
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err ISOCreateItemLocationAtom(ISOItemLocationAtomPtr *outAtom)
{
  MP4Err err;
  ISOItemLocationAtomPtr self;

  self = (ISOItemLocationAtomPtr)calloc(1, sizeof(ISOItemLocationAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOItemLocationAtomType;
  self->name                  = "item location";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->mdatMoved             = mdatMoved;
  self->setItemsMeta          = setItemsMeta;

  err = MP4MakeLinkedList(&(self->itemList));
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
