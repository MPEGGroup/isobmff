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
 * or derivative works. Copyright (c) 1999, 2000.
 */
/*
  $Id: MP4Movies.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "FileMappingObject.h"
#include "MP4Descriptors.h"
#include "MdatDataHandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
#include <strings.h>
#endif

static ISOErr newMeta(ISOMetaAtomPtr *outMeta, u32 metaType)
{
  ISOMetaAtomPtr meta;
  MP4HandlerAtomPtr hdlr;
  char name[8];

  MP4Err ISOCreateMetaAtom(ISOMetaAtomPtr * outAtom);
  MP4Err MP4CreateHandlerAtom(MP4HandlerAtomPtr * outAtom);

  MP4Err err;
  err = MP4NoErr;

  err = ISOCreateMetaAtom(&meta);
  if(err) goto bail;
  err = MP4CreateHandlerAtom(&hdlr);
  if(err) goto bail;

  hdlr->handlerType = metaType;
  MP4TypeToString(metaType, name);
  hdlr->setName((MP4AtomPtr)hdlr, name, 0);

  err = meta->addAtom(meta, (MP4AtomPtr)hdlr);
  if(err) goto bail;

  *outMeta = meta;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISONewFileMeta(ISOMovie theMovie, u32 metaType, ISOMeta *outMeta)
{
  ISOMetaAtomPtr myMeta;
  ISOAdditionalMetaDataContainerAtomPtr mecoPtr;
  GETMOOV(theMovie);
  err = newMeta(&myMeta, metaType);
  if(err) goto bail;

  if(moov->meta != NULL)
  {
    if(moov->meta->type == metaType) BAILWITHERROR(MP4BadParamErr);

    printf("WARNING: adding additional meta box to 'meco' container is deprecated!\n");
    if(moov->meco == NULL)
    {
      err = ISOCreateAdditionalMetaDataContainerAtom(
        (ISOAdditionalMetaDataContainerAtomPtr *)&moov->meco);
      if(err) goto bail;
    }

    mecoPtr = (ISOAdditionalMetaDataContainerAtomPtr)moov->meco;
    err     = mecoPtr->addMeta(mecoPtr, (MP4AtomPtr)myMeta);
    if(err) goto bail;
    myMeta->relatedMeco = (MP4AtomPtr)mecoPtr;
  }
  else
  {
    moov->meta = (MP4AtomPtr)myMeta;
  }

  err = myMeta->setMdat(myMeta, moov->mdat);
  if(err) goto bail;
  *outMeta = (ISOMeta)myMeta;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISONewMovieMeta(ISOMovie theMovie, u32 metaType, ISOMeta *outMeta)
{
  ISOMetaAtomPtr myMeta;
  ISOAdditionalMetaDataContainerAtomPtr mecoPtr;
  GETMOVIEATOM(theMovie);
  err = newMeta(&myMeta, metaType);
  if(err) goto bail;

  mecoPtr = NULL;

  if(movieAtom->meta != NULL)
  {
    if(movieAtom->meta->type == metaType) BAILWITHERROR(MP4BadParamErr);

    if(movieAtom->meco == NULL)
    {
      MP4AtomPtr tempMeco;
      err = ISOCreateAdditionalMetaDataContainerAtom(
        (ISOAdditionalMetaDataContainerAtomPtr *)&tempMeco);
      if(err) goto bail;
      err = movieAtom->addAtom(movieAtom, tempMeco);
      if(err) goto bail;
      myMeta->relatedMeco = (MP4AtomPtr)mecoPtr;
    }

    mecoPtr = (ISOAdditionalMetaDataContainerAtomPtr)movieAtom->meco;
    err     = mecoPtr->addMeta(mecoPtr, (MP4AtomPtr)myMeta);
    if(err) goto bail;
  }
  else
  {
    err = movieAtom->addAtom(movieAtom, (MP4AtomPtr)myMeta);
    if(err) goto bail;
  }

  err = myMeta->setMdat(myMeta, moov->mdat);
  if(err) goto bail;

  *outMeta = (ISOMeta)myMeta;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISONewTrackMeta(ISOTrack theTrack, u32 metaType, ISOMeta *outMeta)
{
  ISOMetaAtomPtr myMeta;
  MP4TrackAtomPtr trak;
  ISOAdditionalMetaDataContainerAtomPtr mecoPtr;
  MP4Err err;

  err = newMeta(&myMeta, metaType);
  if(err) goto bail;
  trak = (MP4TrackAtomPtr)theTrack;

  mecoPtr = NULL;

  if(trak->meta != NULL)
  {
    if(trak->meta->type == metaType) BAILWITHERROR(MP4BadParamErr);

    if(trak->meco == NULL)
    {
      MP4AtomPtr tempMeco;
      err = ISOCreateAdditionalMetaDataContainerAtom(
        (ISOAdditionalMetaDataContainerAtomPtr *)&tempMeco);
      if(err) goto bail;
      err = trak->addAtom(trak, tempMeco);
      if(err) goto bail;
      myMeta->relatedMeco = (MP4AtomPtr)mecoPtr;
    }

    mecoPtr = (ISOAdditionalMetaDataContainerAtomPtr)trak->meco;
    err     = mecoPtr->addMeta(mecoPtr, (MP4AtomPtr)myMeta);
    if(err) goto bail;
  }
  else
  {
    err = trak->addAtom(trak, (MP4AtomPtr)myMeta);
    if(err) goto bail;
  }

  err = myMeta->setMdat(myMeta, trak->mdat);
  if(err) goto bail;

  *outMeta = (ISOMeta)myMeta;

bail:
  TEST_RETURN(err);
  return err;
}

/* Deprecated */
ISO_EXTERN(ISOErr) ISOAddMetaBoxRelation(ISOMeta first_meta, ISOMeta second_meta, u8 relation_type)
{
  MP4Err err;
  ISOMetaAtomPtr firstMeta;
  ISOMetaAtomPtr secondMeta;
  ISOAdditionalMetaDataContainerAtomPtr meco;
  ISOMetaboxRelationAtomPtr mere;
  MP4HandlerAtomPtr hdlr;

  err        = MP4NoErr;
  firstMeta  = (ISOMetaAtomPtr)first_meta;
  secondMeta = (ISOMetaAtomPtr)second_meta;
  meco       = NULL;

  if(firstMeta->relatedMeco != NULL)
    meco = (ISOAdditionalMetaDataContainerAtomPtr)firstMeta->relatedMeco;

  if(secondMeta->relatedMeco != NULL)
    meco = (ISOAdditionalMetaDataContainerAtomPtr)secondMeta->relatedMeco;

  if(meco == NULL) BAILWITHERROR(MP4BadParamErr);

  err = ISOCreateMetaboxRelationAtom(&mere);
  if(err) goto bail;

  hdlr                              = (MP4HandlerAtomPtr)firstMeta->hdlr;
  mere->first_metabox_handler_type  = hdlr->handlerType;
  hdlr                              = (MP4HandlerAtomPtr)secondMeta->hdlr;
  mere->second_metabox_handler_type = hdlr->handlerType;
  mere->metabox_relation            = relation_type;

  err = MP4AddListEntry(mere, meco->relationList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

/* Deprecated */
ISO_EXTERN(ISOErr) ISOGetMetaBoxRelation(ISOMeta first_meta, ISOMeta second_meta, u8 *relation_type)
{
  MP4Err err;
  ISOMetaAtomPtr firstMeta;
  ISOMetaAtomPtr secondMeta;
  ISOAdditionalMetaDataContainerAtomPtr meco;
  ISOMetaboxRelationAtomPtr mere;
  u32 i;

  err            = MP4NoErr;
  firstMeta      = (ISOMetaAtomPtr)first_meta;
  secondMeta     = (ISOMetaAtomPtr)second_meta;
  meco           = NULL;
  *relation_type = 0;

  if(firstMeta->relatedMeco != NULL)
    meco = (ISOAdditionalMetaDataContainerAtomPtr)firstMeta->relatedMeco;

  if(secondMeta->relatedMeco != NULL)
    meco = (ISOAdditionalMetaDataContainerAtomPtr)secondMeta->relatedMeco;

  if(meco == NULL) BAILWITHERROR(MP4BadParamErr);

  for(i = 0; i < meco->relationList->entryCount; i++)
  {
    err = MP4GetListEntry(meco->relationList, i, (char **)&mere);
    if(err) goto bail;
    if((mere->first_metabox_handler_type == firstMeta->type) &&
       (mere->second_metabox_handler_type == secondMeta->type))
    {
      *relation_type = mere->metabox_relation;
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOAddMetaDataReference(ISOMeta meta, u16 *out_ref, ISOHandle urlHandle, ISOHandle urnHandle)
{
  MP4Err MP4CreateDataInformationAtom(MP4DataInformationAtomPtr * outAtom);
  MP4Err MP4CreateDataReferenceAtom(MP4DataReferenceAtomPtr * outAtom);
  MP4Err MP4CreateDataEntryURLAtom(MP4DataEntryURLAtomPtr * outAtom);
  ISOMetaAtomPtr myMeta;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;

  MP4Err err;

  myMeta = (ISOMetaAtomPtr)meta;

  dinf = (MP4DataInformationAtomPtr)myMeta->dinf;
  if(dinf)
  {
    dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
    if(dref == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
  }
  else
  {
    err = MP4CreateDataInformationAtom(&dinf);
    if(err) goto bail;
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)dinf);
    if(err) goto bail;

    err = MP4CreateDataReferenceAtom(&dref);
    if(err) goto bail;
    err = dinf->addAtom(dinf, (MP4AtomPtr)dref);
    if(err) goto bail;
  }
  if(urnHandle == NULL)
  {
    u32 sz;
    MP4DataEntryURLAtomPtr url;
    err = MP4CreateDataEntryURLAtom(&url);
    if(err) goto bail;
    err = MP4GetHandleSize(urlHandle, &sz);
    if(err) goto bail;
    url->locationLength = (u32)sz;
    url->location       = (char *)calloc(1, sz);
    memcpy(url->location, *urlHandle, sz);
    err = dref->addDataEntry(dref, (MP4AtomPtr)url);
    if(err) goto bail;
  }
  else
  {
    u32 sz;
    MP4DataEntryURNAtomPtr urn;
    err = MP4CreateDataEntryURNAtom(&urn);
    if(err) goto bail;
    err = MP4GetHandleSize(urlHandle, &sz);
    if(err) goto bail;
    urn->locationLength = (u32)sz;
    urn->location       = (char *)calloc(1, sz);
    memcpy(urn->location, *urlHandle, sz);

    err = MP4GetHandleSize(urnHandle, &sz);
    if(err) goto bail;
    urn->nameLength = (u32)sz;
    urn->name       = (char *)calloc(1, sz);
    memcpy(urn->name, *urnHandle, sz);
    err = dref->addDataEntry(dref, (MP4AtomPtr)urn);
    if(err) goto bail;
  }
  *out_ref = (u16)dref->getEntryCount(dref);

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err requestItemID(ISOMetaAtomPtr myMeta, u16 itemID)
{
  MP4Err err;

  err = MP4NoErr;
  if(itemID == 0) BAILWITHERROR(MP4BadParamErr);

  if(myMeta == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  if(itemID >= myMeta->next_item_ID)
  {
    myMeta->next_item_ID = itemID + 1;
  }
  else
  {
    u32 i;
    u32 itemCount = 0;
    ISOItemLocationAtomPtr iloc;
    iloc = (ISOItemLocationAtomPtr)myMeta->iloc;

    err = MP4BadParamErr;

    if((iloc == NULL) || (!(iloc->itemList))) BAILWITHERROR(MP4NotFoundErr)

    err = MP4GetListEntryCount(iloc->itemList, &itemCount);
    if(err) goto bail;
    for(i = 0; i < itemCount; i++)
    {
      MetaItemLocationPtr a;
      err = MP4GetListEntry(iloc->itemList, i, (char **)&a);
      if(err) goto bail;
      if(a == NULL) BAILWITHERROR(MP4InvalidMediaErr);
      if(a->item_ID == itemID) goto bail;
    }
    err = MP4NoErr;
  }
bail:
  return err;
}

ISO_EXTERN(ISOErr)
ISOAddMetaItem(ISOMeta meta, ISOMetaItem *outItem, u64 base_offset, u16 data_ref_index)
{
  ISOMetaAtomPtr myMeta;

  myMeta = (ISOMetaAtomPtr)meta;
  return ISOAddMetaItemWithID(meta, outItem, base_offset, data_ref_index, myMeta->next_item_ID);
}

ISO_EXTERN(ISOErr)
ISOAddMetaItemWithID(ISOMeta meta, ISOMetaItem *outItem, u64 base_offset, u16 data_ref_index,
                     u16 item_ID)
{
  MP4Err ISOCreateItemLocationAtom(ISOItemLocationAtomPtr * outAtom);
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr item;
  ISOItemLocationAtomPtr iloc;

  MP4Err err;

  myMeta = (ISOMetaAtomPtr)meta;

  err = requestItemID(myMeta, item_ID);
  if(err) goto bail;

  item = (MetaItemLocationPtr)calloc(1, sizeof(MetaItemLocation));
  TESTMALLOC(item);

  item->item_ID             = item_ID;
  item->base_offset         = base_offset;
  item->dref_index          = data_ref_index;
  item->construction_method = 0;
  item->meta                = (MP4AtomPtr)myMeta;

  err = MP4MakeLinkedList(&(item->extentList));
  if(err) goto bail;

  iloc = (ISOItemLocationAtomPtr)myMeta->iloc;
  if(!iloc)
  {
    err = ISOCreateItemLocationAtom(&iloc);
    if(err) goto bail;
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)iloc);
  }
  err = MP4AddListEntry((void *)item, iloc->itemList);

  *outItem = (ISOMetaItem)item;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOAddItemExtent(ISOMetaItem item, MP4Handle data)
{
  MetaExtentLocationPtr extent;
  ISOMetaAtomPtr myMeta;
  MP4DataInformationAtomPtr dinf;
  MetaItemLocationPtr myItem;
  u32 size, extents;
  MP4Err err;

  extent = (MetaExtentLocationPtr)calloc(1, sizeof(MetaExtentLocation));
  TESTMALLOC(extent);

  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;

  err = MP4GetHandleSize(data, &size);
  if(err) goto bail;
  extent->extent_length = size;
  extent->extent_index  = 0;

  if(myItem->dref_index)
  {
    dinf = (MP4DataInformationAtomPtr)myMeta->dinf;
    if(!dinf)
    {
      BAILWITHERROR(ISOBadParamErr);
    }

    err = dinf->getOffset(dinf, myItem->dref_index, &(extent->extent_offset));
    if(err) goto bail;
    err = dinf->addSamples(dinf, 1, myItem->dref_index, data);
    if(err) goto bail;
  }
  else
  {
    MP4MediaDataAtomPtr mdat;
    mdat                  = (MP4MediaDataAtomPtr)myMeta->mdat;
    extent->extent_offset = mdat->dataSize;
    err                   = mdat->addData(mdat, data);
    if(err) goto bail;
  }
  err = MP4GetListEntryCount(myItem->extentList, &extents);
  if(err) goto bail;
  if(extents == 0) myItem->base_offset += extent->extent_offset;

  err = MP4AddListEntry((void *)extent, myItem->extentList);

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOAddItemExtentReference(ISOMetaItem item, u64 offset, u64 length)
{
  MetaExtentLocationPtr extent;
  ISOMetaAtomPtr myMeta;
  MP4DataInformationAtomPtr dinf;
  MetaItemLocationPtr myItem;
  MP4Err err;
  u32 extents;

  extent = (MetaExtentLocationPtr)calloc(1, sizeof(MetaExtentLocation));
  TESTMALLOC(extent);

  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;

  extent->extent_length = length;
  extent->extent_offset = offset;
  extent->extent_index  = 0;

  if(myItem->dref_index)
  {
    dinf = (MP4DataInformationAtomPtr)myMeta->dinf;
    if(!dinf)
    {
      BAILWITHERROR(ISOBadParamErr);
    }
  }
  else
  {
    BAILWITHERROR(ISOBadParamErr);
  }

  err = MP4GetListEntryCount(myItem->extentList, &extents);
  if(err) goto bail;
  if(extents == 0) myItem->base_offset += extent->extent_offset;

  err = MP4AddListEntry((void *)extent, myItem->extentList);

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOAddItemExtentUsingItemData(ISOMetaItem item, MP4Handle data)
{
  MetaExtentLocationPtr extent;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemLocationAtomPtr iloc;
  ISOItemDataAtomPtr idat;
  u32 size;
  u32 offset;
  u32 extents;
  MP4Err err;

  extent = (MetaExtentLocationPtr)calloc(1, sizeof(MetaExtentLocation));
  TESTMALLOC(extent);

  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;
  iloc   = (ISOItemLocationAtomPtr)myMeta->iloc;
  idat   = (ISOItemDataAtomPtr)myMeta->idat;

  if(idat == NULL)
  {
    err = ISOCreateItemDataAtom(&idat);
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)idat);
    if(err) goto bail;
  }

  err = MP4GetHandleSize(data, &size);
  if(err) goto bail;
  extent->extent_length = size;

  err = MP4GetHandleSize(idat->data, &offset);
  if(err) goto bail;
  extent->extent_offset = offset;

  extent->extent_index = 0;

  err = MP4GetListEntryCount(myItem->extentList, &extents);
  if(err) goto bail;
  if(extents == 0)
  {
    iloc->version               = 1;
    myItem->construction_method = 1;
  }
  else
  {
    if(myItem->construction_method != 1) BAILWITHERROR(MP4BadParamErr);
  }

  err = MP4HandleCat(idat->data, data);
  if(err) goto bail;

  if(extents == 0) myItem->base_offset += extent->extent_offset;

  err = MP4AddListEntry((void *)extent, myItem->extentList);

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOAddItemExtentItem(ISOMetaItem item, ISOMetaItem extent_item, u32 offset, u32 length)
{
  MetaExtentLocationPtr extent;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  MetaItemLocationPtr extentItem;
  ISOItemLocationAtomPtr iloc;
  u32 extents;
  MP4Err err;
  u32 index;

  extent = (MetaExtentLocationPtr)calloc(1, sizeof(MetaExtentLocation));
  TESTMALLOC(extent);

  myItem     = (MetaItemLocationPtr)item;
  extentItem = (MetaItemLocationPtr)extent_item;
  myMeta     = (ISOMetaAtomPtr)myItem->meta;
  iloc       = (ISOItemLocationAtomPtr)myMeta->iloc;

  err = MP4GetListEntryCount(myItem->extentList, &extents);
  if(err) goto bail;
  if(extents == 0)
  {
    iloc->version               = 1;
    myItem->construction_method = 2;
  }
  else
  {
    if(myItem->construction_method != 2) BAILWITHERROR(MP4BadParamErr);
  }

  extent->extent_length = length;
  extent->extent_offset = offset;

  err =
    ISOAddItemReference(item, MP4_FOUR_CHAR_CODE('i', 'l', 'o', 'c'), extentItem->item_ID, &index);
  if(err) goto bail;

  extent->extent_index = index;

  if(extents == 0) myItem->base_offset += extent->extent_offset;
  err = MP4AddListEntry((void *)extent, myItem->extentList);

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOAddPrimaryData(ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom)
{
  ISOMetaAtomPtr myMeta;
  MP4UnknownAtomPtr myAtom;
  char *dataPtr;
  u32 len;

  MP4Err err;
  err = MP4NoErr;

  myMeta = (ISOMetaAtomPtr)meta;

  err = MP4CreateUnknownAtom(&myAtom);
  if(err) goto bail;

  err = MP4GetHandleSize(data, &len);
  if(err) goto bail;
  if(is_full_atom) len += 4;

  dataPtr = (char *)calloc(1, len);
  TESTMALLOC(dataPtr);
  myAtom->data     = dataPtr;
  myAtom->dataSize = len;
  myAtom->type     = box_type;

  if(is_full_atom)
  {
    dataPtr[0] = dataPtr[1] = dataPtr[2] = dataPtr[3] = 0;
    dataPtr += 4;
    len -= 4;
  }
  memcpy(dataPtr, *data, len);

  err = myMeta->addAtom(myMeta, (MP4AtomPtr)myAtom);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetItemReferences(ISOMetaItem item, u32 reference_type, u16 *reference_count,
                     MP4Handle to_item_IDs)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemReferenceAtomPtr irefPtr;
  ISOSingleItemTypeReferenceAtomPtr singleIrefPtr;
  u32 i;

  err              = MP4NoErr;
  myItem           = (MetaItemLocationPtr)item;
  myMeta           = (ISOMetaAtomPtr)myItem->meta;
  *reference_count = 0;

  if(myMeta->iref == NULL) BAILWITHERROR(MP4BadParamErr);

  irefPtr = (ISOItemReferenceAtomPtr)myMeta->iref;

  for(i = 0; i < irefPtr->atomList->entryCount; i++)
  {
    err = MP4GetListEntry(irefPtr->atomList, i, (char **)&singleIrefPtr);
    if(err) goto bail;
    if((singleIrefPtr->from_item_ID == myItem->item_ID) && (singleIrefPtr->type == reference_type))
    {
      *reference_count = singleIrefPtr->reference_count;
      err = MP4SetHandleSize(to_item_IDs, singleIrefPtr->reference_count * sizeof(u32));
      memcpy((char *)*to_item_IDs, singleIrefPtr->to_item_IDs,
             singleIrefPtr->reference_count * sizeof(u32));
      break;
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetItemReference(ISOMetaItem item, u32 reference_type, u16 reference_index, ISOMetaItem *outItem)
{
  MP4Err err;
  MetaItemLocationPtr myItem;
  MP4Handle toItemIDS;
  u16 existingCount;
  u32 *toItemIDsArray;
  u32 referencedItemID;
  ISOMetaAtomPtr myMeta;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;

  err = MP4NewHandle(0, &toItemIDS);
  if(err) goto bail;
  err = ISOGetItemReferences(item, reference_type, &existingCount, toItemIDS);
  if(err) goto bail;

  if(existingCount < reference_index) BAILWITHERROR(MP4BadParamErr);

  toItemIDsArray = (u32 *)*toItemIDS;

  referencedItemID = toItemIDsArray[reference_index - 1];

  err = ISOFindItemByID((ISOMeta)myMeta, outItem, (u16)referencedItemID);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOAddItemReference(ISOMetaItem item, u32 reference_type, u32 to_item_ID, u32 *outIndex)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemReferenceAtomPtr irefPtr;
  ISOSingleItemTypeReferenceAtomPtr singleIrefPtr;
  u16 existingCount;
  MP4Handle toItemIDS;
  u32 i;

  err       = MP4NoErr;
  myItem    = (MetaItemLocationPtr)item;
  myMeta    = (ISOMetaAtomPtr)myItem->meta;
  *outIndex = 1;

  if(myMeta->iref == NULL)
  {
    err = ISOCreateItemReferenceAtom(&irefPtr);
    if(err) goto bail;
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)irefPtr);
    if(err) goto bail;
  }

  irefPtr = (ISOItemReferenceAtomPtr)myMeta->iref;

  err = MP4NewHandle(0, &toItemIDS);
  if(err) goto bail;
  err = ISOGetItemReferences(item, reference_type, &existingCount, toItemIDS);
  if(err) goto bail;

  if(existingCount == 0)
  {
    err = MP4SetHandleSize(toItemIDS, sizeof(u32));
    if(err) goto bail;
    *((u32 *)*toItemIDS) = to_item_ID;
    err                  = ISOAddItemReferences(item, reference_type, 1, toItemIDS);
    if(err) goto bail;
  }
  else
  {
    for(i = 0; i < irefPtr->atomList->entryCount; i++)
    {
      err = MP4GetListEntry(irefPtr->atomList, i, (char **)&singleIrefPtr);
      if(err) goto bail;
      if((singleIrefPtr->from_item_ID == myItem->item_ID) &&
         (singleIrefPtr->type == reference_type))
      {
        singleIrefPtr->reference_count++;
        singleIrefPtr->to_item_IDs =
          (u32 *)realloc(singleIrefPtr->to_item_IDs, singleIrefPtr->reference_count * sizeof(u32));
        singleIrefPtr->to_item_IDs[singleIrefPtr->reference_count - 1] = to_item_ID;
        *outIndex = singleIrefPtr->reference_count;
        break;
      }
    }
  }

  err = MP4DisposeHandle(toItemIDS);
  if(err) goto bail;
bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOAddItemReferences(ISOMetaItem item, u32 reference_type, u16 reference_count,
                     MP4Handle to_item_IDs)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemReferenceAtomPtr irefPtr;
  ISOSingleItemTypeReferenceAtomPtr singleIrefPtr;
  u32 *toItemIdsArray;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;

  if(myMeta->iref == NULL)
  {
    err = ISOCreateItemReferenceAtom(&irefPtr);
    if(err) goto bail;
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)irefPtr);
    if(err) goto bail;
  }

  irefPtr = (ISOItemReferenceAtomPtr)myMeta->iref;

  err = ISOCreateSingleItemTypeReferenceAtom(&singleIrefPtr, reference_type, (u8)irefPtr->version);
  if(err) goto bail;

  singleIrefPtr->from_item_ID    = myItem->item_ID;
  singleIrefPtr->reference_count = reference_count;
  toItemIdsArray                 = (u32 *)*to_item_IDs;

  singleIrefPtr->to_item_IDs = (u32 *)calloc(reference_count, sizeof(u32));
  memcpy(singleIrefPtr->to_item_IDs, toItemIdsArray, reference_count * sizeof(u32));

  err = MP4AddListEntry(singleIrefPtr, irefPtr->atomList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOAddItemRelation(ISOMetaItem fromItem, ISOMetaItem toItem, u32 relationType)
{
  MP4Err err;
  u32 outIndex;
  u16 toItemID;

  err = ISOGetItemID(toItem, &toItemID);
  if(err) goto bail;

  err = ISOAddItemReference(fromItem, relationType, toItemID, &outIndex);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOGetPrimaryData(ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom)
{
  ISOMetaAtomPtr myMeta;

  MP4Err err;
  err = MP4NoErr;

  myMeta = (ISOMetaAtomPtr)meta;

  err = myMeta->getData(myMeta, box_type, data, is_full_atom);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetPrimaryItemData(ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom)
{
  MP4Err err;

  err = ISOGetPrimaryData(meta, box_type, data, is_full_atom);
  if(err == MP4NotFoundErr)
  {
    u64 boff;
    u16 ID;
    ISOMetaItem item;
    err = ISOGetPrimaryItemID(meta, &ID);
    if(err) goto bail;
    err = ISOFindItemByID(meta, &item, ID);
    if(err) goto bail;
    err = ISOGetItemData(item, data, &boff);
    if(err) goto bail;
  }

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOSetPrimaryItem(ISOMeta meta, ISOMetaItem item)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  ISOPrimaryItemAtomPtr pitm;
  MetaItemLocationPtr myItem;

  err = MP4NoErr;

  myMeta = (ISOMetaAtomPtr)meta;
  myItem = (MetaItemLocationPtr)item;

  if(myMeta->pitm == NULL)
  {
    err = ISOCreatePrimaryItemAtom(&pitm);
    if(err) goto bail;
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)pitm);
    if(err) goto bail;
  }

  pitm          = (ISOPrimaryItemAtomPtr)myMeta->pitm;
  pitm->item_ID = myItem->item_ID;
bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOGetPrimaryItemID(ISOMeta meta, u16 *ID)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  ISOPrimaryItemAtomPtr pitm;

  err = MP4NoErr;

  myMeta = (ISOMetaAtomPtr)meta;
  pitm   = (ISOPrimaryItemAtomPtr)myMeta->pitm;
  if(pitm)
  {
    *ID = (u16)pitm->item_ID;
  }
  else
    BAILWITHERROR(MP4NotFoundErr);

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOGetItemID(ISOMetaItem item, u16 *ID)
{
  MP4Err err;
  MetaItemLocationPtr myItem;

  err = MP4NoErr;

  myItem = (MetaItemLocationPtr)item;
  if(item) *ID = myItem->item_ID;
  else
  {
    BAILWITHERROR(MP4BadParamErr);
  }

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOSetItemInfo(ISOMetaItem item, u16 protection_index, char *name, char *content_type,
               char *content_encoding)
{
  MP4Err ISOCreateItemInfoAtom(ISOItemInfoAtomPtr * outAtom);
  MP4Err ISOCreateItemInfoEntryAtom(ISOItemInfoEntryAtomPtr * outAtom);
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemInfoAtomPtr iinf;
  ISOItemInfoEntryAtomPtr infe;
  u32 sz;

  MP4Err err;
  err = MP4NoErr;

  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;

  iinf = (ISOItemInfoAtomPtr)myMeta->iinf;
  if(!iinf)
  {
    err = ISOCreateItemInfoAtom(&iinf);
    if(err) goto bail;
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)iinf);
    if(err) goto bail;
  }
  err = ISOCreateItemInfoEntryAtom(&infe);
  if(err) goto bail;
  err = iinf->addAtom(iinf, (MP4AtomPtr)infe);
  if(err) goto bail;

  if(name)
  {
    sz              = (u32)strlen(name);
    infe->item_name = (char *)calloc(1, sz + 1);
    memcpy(infe->item_name, name, sz);
    (infe->item_name)[sz] = '\0';
  }
  else
  {
    infe->item_name      = (char *)calloc(1, 1);
    (infe->item_name)[0] = '\0';
  }

  if(content_encoding)
  {
    sz                     = (u32)strlen(content_encoding);
    infe->content_encoding = (char *)calloc(1, sz);
    memcpy(infe->content_encoding, content_encoding, sz);
  }
  else
  {
    infe->content_encoding      = (char *)calloc(1, 1);
    (infe->content_encoding)[0] = '\0';
  }

  if(content_type)
  {
    sz                 = (u32)strlen(content_type);
    infe->content_type = (char *)calloc(1, sz);
    memcpy(infe->content_type, content_type, sz);
  }
  else
  {
    infe->content_type      = (char *)calloc(1, 1);
    (infe->content_type)[0] = '\0';
  }

  infe->item_ID          = myItem->item_ID;
  infe->protection_index = protection_index;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOSetItemInfoExtension(ISOMetaItem item, MP4Handle extension, u32 extension_type)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemInfoAtomPtr iinf;
  ISOItemInfoEntryAtomPtr infe;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;
  iinf   = (ISOItemInfoAtomPtr)myMeta->iinf;

  if(!iinf) BAILWITHERROR(MP4BadParamErr);

  infe = NULL;
  err  = iinf->getEntry(iinf, myItem->item_ID, &infe);
  if(err) goto bail;

  if(infe == NULL) BAILWITHERROR(MP4BadParamErr);

  err = MP4NewHandle(0, &infe->item_info_extension);
  if(err) goto bail;
  err = MP4HandleCat(infe->item_info_extension, extension);
  if(err) goto bail;

  infe->extension_type = extension_type;
  infe->version        = 1;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetItemInfoExtension(ISOMetaItem item, MP4Handle extension, u32 *extension_type)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemInfoAtomPtr iinf;
  ISOItemInfoEntryAtomPtr infe;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;
  iinf   = (ISOItemInfoAtomPtr)myMeta->iinf;

  if(!iinf) BAILWITHERROR(MP4BadParamErr);

  infe = NULL;
  err  = iinf->getEntry(iinf, myItem->item_ID, &infe);
  if(err) goto bail;

  if(infe == NULL) BAILWITHERROR(MP4BadParamErr);

  if(infe->version != 1) BAILWITHERROR(MP4InvalidMediaErr);

  err = MP4HandleCat(extension, infe->item_info_extension);
  if(err) goto bail;
  *extension_type = infe->extension_type;
bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOSetItemInfoItemType(ISOMetaItem item, u32 item_type, char *item_uri_type)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemInfoAtomPtr iinf;
  ISOItemInfoEntryAtomPtr infe;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;
  iinf   = (ISOItemInfoAtomPtr)myMeta->iinf;

  if(!iinf) BAILWITHERROR(MP4BadParamErr);

  infe = NULL;
  err  = iinf->getEntry(iinf, myItem->item_ID, &infe);
  if(err) goto bail;

  if(infe == NULL) BAILWITHERROR(MP4BadParamErr);

  infe->version   = 2;
  infe->item_type = item_type;

  if(item_uri_type)
  {
    u32 sz              = (u32)strlen(item_uri_type);
    infe->item_uri_type = (char *)calloc(1, sz + 1);
    memcpy(infe->item_uri_type, item_uri_type, sz);
  }
  else
  {
    infe->item_uri_type      = (char *)calloc(1, 1);
    (infe->item_uri_type)[0] = '\0';
  }
bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOHideItem(ISOMetaItem item)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemInfoAtomPtr iinf;
  ISOPrimaryItemAtomPtr pitm;
  ISOItemInfoEntryAtomPtr infe;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;
  iinf   = (ISOItemInfoAtomPtr)myMeta->iinf;
  pitm   = (ISOPrimaryItemAtomPtr)myMeta->pitm;

  if(!iinf) BAILWITHERROR(MP4InvalidMediaErr);

  if(pitm->item_ID == myItem->item_ID) BAILWITHERROR(MP4InvalidMediaErr);

  infe = NULL;
  err  = iinf->getEntry(iinf, myItem->item_ID, &infe);
  if(err) goto bail;

  if(infe == NULL) BAILWITHERROR(MP4InvalidMediaErr);

  infe->flags |= 1U;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOIsItemHidden(ISOMetaItem item)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemInfoAtomPtr iinf;
  ISOItemInfoEntryAtomPtr infe;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;
  iinf   = (ISOItemInfoAtomPtr)myMeta->iinf;

  if(!iinf) BAILWITHERROR(MP4InvalidMediaErr);

  infe = NULL;
  err  = iinf->getEntry(iinf, myItem->item_ID, &infe);
  if(err) goto bail;

  if(infe == NULL) BAILWITHERROR(MP4InvalidMediaErr);

  err = MP4NotFoundErr;
  if(infe->flags & 1U) err = MP4NoErr;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOGetItemInfoItemType(ISOMetaItem item, u32 *item_type, char **item_uri_type)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  ISOItemInfoAtomPtr iinf;
  ISOItemInfoEntryAtomPtr infe;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  myMeta = (ISOMetaAtomPtr)myItem->meta;
  iinf   = (ISOItemInfoAtomPtr)myMeta->iinf;

  if(!iinf) BAILWITHERROR(MP4BadParamErr);

  infe = NULL;
  err  = iinf->getEntry(iinf, myItem->item_ID, &infe);
  if(err) goto bail;

  if(infe == NULL) BAILWITHERROR(MP4BadParamErr);

  if(infe->version != 2) BAILWITHERROR(MP4InvalidMediaErr);

  *item_type = infe->item_type;

  if(item_uri_type)
  {
    if(infe->item_uri_type)
    {
      u32 sz         = (u32)strlen(infe->item_uri_type);
      *item_uri_type = (char *)calloc(1, sz + 1);
      memcpy(*item_uri_type, infe->item_uri_type, sz);
    }
  }
bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetFileMeta(ISOMovie theMovie, ISOMeta *meta, u32 inMetaType, u32 *outMetaType)
{
  ISOMetaAtomPtr myMeta;
  MP4HandlerAtomPtr hdlr;
  ISOAdditionalMetaDataContainerAtomPtr meco;

  GETMOOV(theMovie);
  myMeta = (ISOMetaAtomPtr)moov->meta;

  if(!myMeta)
  {
    BAILWITHERROR(MP4BadParamErr);
  }
  hdlr = (MP4HandlerAtomPtr)myMeta->hdlr;
  if(!hdlr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  if((inMetaType != 0) && (hdlr->handlerType != inMetaType))
  {
    meco = (ISOAdditionalMetaDataContainerAtomPtr)moov->meco;
    if(!meco) BAILWITHERROR(MP4BadParamErr);

    err = meco->getMeta(meco, inMetaType, &myMeta);
    if(err) goto bail;
    hdlr = (MP4HandlerAtomPtr)myMeta->hdlr;
    if(!hdlr)
    {
      BAILWITHERROR(MP4BadDataErr);
    }
  }
  if(outMetaType) *outMetaType = hdlr->handlerType;
  if(meta) *meta = (ISOMeta)myMeta;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetMovieMeta(ISOMovie theMovie, ISOMeta *meta, u32 inMetaType, u32 *outMetaType)
{
  ISOMetaAtomPtr myMeta;
  MP4HandlerAtomPtr hdlr;
  ISOAdditionalMetaDataContainerAtomPtr meco;

  GETMOVIEATOM(theMovie);
  myMeta = (ISOMetaAtomPtr)movieAtom->meta;

  if(!myMeta)
  {
    BAILWITHERROR(MP4BadParamErr);
  }
  hdlr = (MP4HandlerAtomPtr)myMeta->hdlr;
  if(!hdlr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  if((inMetaType != 0) && (hdlr->handlerType != inMetaType))
  {
    meco = (ISOAdditionalMetaDataContainerAtomPtr)movieAtom->meco;
    if(!meco) BAILWITHERROR(MP4BadParamErr);

    err = meco->getMeta(meco, inMetaType, &myMeta);
    if(err) goto bail;
    hdlr = (MP4HandlerAtomPtr)myMeta->hdlr;
    if(!hdlr)
    {
      BAILWITHERROR(MP4BadDataErr);
    }
  }
  if(outMetaType) *outMetaType = hdlr->handlerType;
  if(meta) *meta = (ISOMeta)myMeta;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr)
ISOGetTrackMeta(ISOTrack theTrack, ISOMeta *meta, u32 inMetaType, u32 *outMetaType)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MP4HandlerAtomPtr hdlr;
  MP4TrackAtomPtr trak;
  ISOAdditionalMetaDataContainerAtomPtr meco;

  err = MP4NoErr;

  trak   = (MP4TrackAtomPtr)theTrack;
  myMeta = (ISOMetaAtomPtr)trak->meta;

  if(!myMeta)
  {
    BAILWITHERROR(MP4BadParamErr);
  }
  hdlr = (MP4HandlerAtomPtr)myMeta->hdlr;
  if(!hdlr)
  {
    BAILWITHERROR(MP4BadDataErr);
  }

  if((inMetaType != 0) && (hdlr->handlerType != inMetaType))
  {
    meco = (ISOAdditionalMetaDataContainerAtomPtr)trak->meco;
    if(!meco) BAILWITHERROR(MP4BadParamErr);

    err = meco->getMeta(meco, inMetaType, &myMeta);
    if(err) goto bail;
    hdlr = (MP4HandlerAtomPtr)myMeta->hdlr;
    if(!hdlr)
    {
      BAILWITHERROR(MP4BadDataErr);
    }
  }
  if(outMetaType) *outMetaType = hdlr->handlerType;
  if(meta) *meta = (ISOMeta)myMeta;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOFindItemByID(ISOMeta meta, ISOMetaItem *item, u16 ID)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  ISOItemLocationAtomPtr iloc;
  u32 i, item_total;

  err    = MP4NoErr;
  myMeta = (ISOMetaAtomPtr)meta;

  if(myMeta == NULL) BAILWITHERROR(MP4BadParamErr)
  iloc  = (ISOItemLocationAtomPtr)myMeta->iloc;
  *item = NULL;

  if(iloc == NULL) BAILWITHERROR(MP4NotFoundErr)

  *item = NULL;

  if(iloc->itemList)
  {
    err = MP4GetListEntryCount(iloc->itemList, &item_total);
    if(err) goto bail;

    for(i = 0; i < item_total; i++)
    {
      MetaItemLocationPtr a;
      err = MP4GetListEntry(iloc->itemList, i, (char **)&a);
      if(err) goto bail;

      if(a->item_ID == ID)
      {
        a->meta = (MP4AtomPtr)myMeta;
        *item   = (ISOMetaItem)a;
        err     = MP4NoErr;
        goto bail;
      }
    }
  }
  else
    BAILWITHERROR(MP4BadDataErr);
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(ISOErr)
ISOGetAllItemsWithType(ISOMeta meta, u32 type, ISOMetaItem **items, u32 *numberOfItemsFound)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  ISOItemLocationAtomPtr iloc;
  u32 i, item_total;

  err    = MP4NoErr;
  myMeta = (ISOMetaAtomPtr)meta;

  if(myMeta == NULL) BAILWITHERROR(MP4BadParamErr);

  iloc = (ISOItemLocationAtomPtr)myMeta->iloc;

  if(iloc == NULL) BAILWITHERROR(MP4NotFoundErr);

  *numberOfItemsFound = 0;
  *items              = NULL;
  if(iloc->itemList)
  {
    err = MP4GetListEntryCount(iloc->itemList, &item_total);
    if(err) goto bail;

    for(i = 0; i < item_total; i++)
    {
      char *tmp;
      u32 outType;
      MetaItemLocationPtr a;

      err = MP4GetListEntry(iloc->itemList, i, (char **)&a);
      if(err) goto bail;
      err = ISOGetItemInfoItemType((ISOMetaItem)a, &outType, &tmp);
      if(err) goto bail;

      if(type == outType)
      {
        *numberOfItemsFound = *numberOfItemsFound + 1;
        *items = (ISOMetaItem *)realloc(*items, *numberOfItemsFound * sizeof(ISOMetaItem));
        (*items)[*numberOfItemsFound - 1] = (ISOMetaItem)a;
      }
    }
  }
  else
    BAILWITHERROR(MP4BadDataErr);

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(ISOErr) ISOFindItemByName(ISOMeta meta, ISOMetaItem *item, char *name, u8 exact_case)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  ISOItemInfoAtomPtr iinf;
  u32 i, item_total;

  err    = MP4NoErr;
  myMeta = (ISOMetaAtomPtr)meta;

  if(myMeta == NULL) BAILWITHERROR(MP4BadParamErr)
  iinf  = (ISOItemInfoAtomPtr)myMeta->iinf;
  *item = NULL;

  if(iinf == NULL) BAILWITHERROR(MP4NotFoundErr)

  if(iinf->atomList)
  {
    err = MP4GetListEntryCount(iinf->atomList, &item_total);
    if(err) goto bail;

    for(i = 0; i < item_total; i++)
    {
      ISOItemInfoEntryAtomPtr infe;
      err = MP4GetListEntry(iinf->atomList, i, (char **)&infe);
      if(err) goto bail;

      if((exact_case && (strcmp(name, infe->item_name) == 0)) ||
         (!exact_case && (strcasecmp(name, infe->item_name) == 0)))
        return ISOFindItemByID(meta, item, (u16)infe->item_ID);
    }
  }
  else
    BAILWITHERROR(MP4BadDataErr)
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(ISOErr) ISOGetItemData(ISOMetaItem item, MP4Handle data, u64 *base_offset)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  MP4DataHandlerPtr dhlr;
  ISOItemDataAtomPtr idat;

  err    = MP4NoErr;
  dhlr   = NULL;
  idat   = NULL;
  myItem = (MetaItemLocationPtr)item;
  if(myItem == NULL) BAILWITHERROR(MP4BadParamErr)

  myMeta = (ISOMetaAtomPtr)myItem->meta;
  if(myMeta == NULL) BAILWITHERROR(MP4BadDataErr)

  if(myItem->construction_method == 0)
  {
    if(myMeta->dataEntryIndex != myItem->dref_index)
    {
      if(myMeta->dataHandler)
      {
        err = myMeta->closeDataHandler(myMeta);
        if(err) goto bail;
      }

      err = myMeta->openDataHandler(myMeta, myItem->dref_index);
      if(err) goto bail;
    }

    dhlr = (MP4DataHandlerPtr)myMeta->dataHandler;
    if(dhlr == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
  }
  else if(myItem->construction_method == 1)
  {
    idat = (ISOItemDataAtomPtr)myMeta->idat;
    if(idat == NULL) BAILWITHERROR(MP4BadDataErr);
  }

  err = MP4SetHandleSize(data, 0);
  if(err) goto bail;

  *base_offset = myItem->base_offset;

  if(myItem->extentList)
  {
    u32 list2Size;
    u32 datasize, j;

    err = MP4GetListEntryCount(myItem->extentList, &list2Size);
    if(err) goto bail;

    datasize = 0;

    for(j = 0; j < list2Size; j++)
    {
      MetaExtentLocationPtr b;
      ISOMetaItem referenceItem;
      MP4Handle referenceItemData;
      u32 length   = 0;
      u64 fileSize = 0;

      err = MP4GetListEntry(myItem->extentList, j, (char **)&b);
      if(err) goto bail;

      if(b->extent_length == 0)
      {
        if(myItem->construction_method == 0)
        {
          err = dhlr->getDataSize(dhlr, &fileSize);
          if(err) goto bail;
          if(fileSize >> 32) BAILWITHERROR(MP4NoLargeAtomSupportErr);
          length = (u32)fileSize;
        }
        else if(myItem->construction_method == 1)
        {
          err = MP4GetHandleSize(idat->data, &length);
          if(err) goto bail;
        }
        else if(myItem->construction_method == 2)
        {
          u64 k;
          err = ISOGetItemReference(item, MP4_FOUR_CHAR_CODE('i', 'l', 'o', 'c'),
                                    (u16)b->extent_index, &referenceItem);
          if(err) goto bail;
          err = MP4NewHandle(0, &referenceItemData);
          if(err) goto bail;
          err = ISOGetItemData(referenceItem, referenceItemData, &k);
          if(err) goto bail;
          err = MP4GetHandleSize(referenceItemData, &length);
          if(err) goto bail;
        }

        if(b->extent_offset) BAILWITHERROR(MP4BadDataErr);
      }
      else
      {
        if(b->extent_length >> 32) BAILWITHERROR(MP4NoLargeAtomSupportErr);
        length = (u32)b->extent_length;
      }

      err = MP4SetHandleSize(data, datasize + length);
      if(err) goto bail;

      if(myItem->construction_method == 0)
      {
        err = dhlr->copyData(dhlr, b->extent_offset + *base_offset, (*data) + datasize, length);
        if(err) goto bail;
      }
      else if(myItem->construction_method == 1)
      {
        err = idat->getData((MP4AtomPtr)idat, (*data) + datasize, (u32)b->extent_offset, length);
        if(err) goto bail;
      }
      else if(myItem->construction_method == 2)
      {
        u64 k;
        u32 referenceDataLength;

        err = ISOGetItemReference(item, MP4_FOUR_CHAR_CODE('i', 'l', 'o', 'c'),
                                  (u16)b->extent_index, &referenceItem);
        if(err) goto bail;
        err = MP4NewHandle(0, &referenceItemData);
        if(err) goto bail;
        err = ISOGetItemData(referenceItem, referenceItemData, &k);
        if(err) goto bail;
        err = MP4GetHandleSize(referenceItemData, &referenceDataLength);
        if(err) goto bail;

        if((referenceDataLength - b->extent_offset) < length) BAILWITHERROR(MP4BadDataErr);

        memcpy((*data) + datasize, (*referenceItemData) + b->extent_offset, length);
      }
      if(j == 0) *base_offset -= b->extent_offset;

      datasize += length;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(ISOErr)
ISOGetItemInfo(ISOMetaItem item, u16 *protection_index, char *name, char *content_type,
               char *content_encoding)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  ISOItemInfoAtomPtr iinf;
  u32 i, item_total;
  MetaItemLocationPtr myItem;

  err    = MP4NoErr;
  myItem = (MetaItemLocationPtr)item;
  if(myItem == NULL) BAILWITHERROR(MP4BadParamErr)

  myMeta = (ISOMetaAtomPtr)myItem->meta;
  if(myMeta == NULL) BAILWITHERROR(MP4BadDataErr)

  iinf = (ISOItemInfoAtomPtr)myMeta->iinf;

  if(iinf == NULL) BAILWITHERROR(MP4NotFoundErr)

  if(iinf->atomList)
  {
    err = MP4GetListEntryCount(iinf->atomList, &item_total);
    if(err) goto bail;

    for(i = 0; i < item_total; i++)
    {
      ISOItemInfoEntryAtomPtr infe;
      err = MP4GetListEntry(iinf->atomList, i, (char **)&infe);
      if(err) goto bail;
      if(infe->item_ID == myItem->item_ID)
      {
        u32 sz;
        if(name && infe->item_name)
        {
          sz = (u32)strlen(infe->item_name);
          memcpy(name, infe->item_name, sz + 1);
        }
        else if(name)
          name[0] = 0;

        if(content_encoding && infe->content_encoding)
        {
          sz = (u32)strlen(infe->content_encoding);
          memcpy(content_encoding, infe->content_encoding, sz + 1);
        }
        else if(content_encoding)
          content_encoding[0] = 0;

        if(content_type && infe->content_type)
        {
          sz = (u32)strlen(infe->content_type);
          memcpy(content_type, infe->content_type, sz + 1);
        }
        else if(content_type)
          name[0] = 0;

        if(protection_index) *protection_index = (u16)infe->protection_index;
        err = MP4NoErr;
        goto bail;
      }
    }
  }
  else
    BAILWITHERROR(MP4BadDataErr)
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOCheckMetaDataReferences(ISOMeta meta)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryAtomPtr dataEntryAtom;
  u32 count, dataEntryIndex;

  err    = MP4NoErr;
  myMeta = (ISOMetaAtomPtr)meta;

  dinf = (MP4DataInformationAtomPtr)myMeta->dinf;
  if(!dinf) goto bail;

  dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
  if(dref == NULL)
  {
    BAILWITHERROR(MP4InvalidMediaErr);
  }

  count = dref->getEntryCount(dref);
  for(dataEntryIndex = 1; dataEntryIndex <= count; dataEntryIndex++)
  {
    err = dref->getEntry(dref, dataEntryIndex, &dataEntryAtom);
    if(err) goto bail;
    if(dataEntryAtom == NULL) BAILWITHERROR(MP4InvalidMediaErr)
    err = MP4PreflightDataHandler(myMeta->inputStream, dataEntryAtom);
    if(err) goto bail;
  }

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISONewMetaProtection(ISOMeta meta, u32 sch_type, u32 sch_version, char *sch_url,
                     u16 *protection_index)
{
  MP4Err err;

  ISOMetaAtomPtr myMeta;
  MP4SecurityInfoAtomPtr sinf;
  MP4SchemeTypeAtomPtr schm;
  MP4SchemeInfoAtomPtr schi;
  ISOItemProtectionAtomPtr ipro;
  u32 j;

  char *sch_url_copy = NULL;
  schm               = NULL;

  err    = MP4NoErr;
  myMeta = (ISOMetaAtomPtr)meta;

  err = MP4CreateSchemeInfoAtom(&schi);
  if(err) goto bail;

  err = MP4CreateSchemeTypeAtom(&schm);
  if(err) goto bail;
  schm->scheme_type    = sch_type;
  schm->scheme_version = sch_version;

  if(sch_url)
  {
    sch_url_copy = (char *)calloc(1, strlen(sch_url) + 1);
    TESTMALLOC(sch_url_copy);
    memcpy(sch_url_copy, sch_url, strlen(sch_url) + 1);
    schm->scheme_url = sch_url_copy;
    sch_url_copy     = NULL;
  }
  else
    schm->scheme_url = NULL;

  err = MP4CreateSecurityInfoAtom(&sinf);
  if(err) goto bail;
  sinf->MP4SchemeType = (MP4AtomPtr)schm;
  schm                = NULL;
  sinf->MP4SchemeInfo = (MP4AtomPtr)schi;
  schi                = NULL;

  ipro = (ISOItemProtectionAtomPtr)myMeta->ipro;
  if(!ipro)
  {
    err = ISOCreateItemProtectionAtom(&ipro);
    if(err) goto bail;
    err = myMeta->addAtom(myMeta, (MP4AtomPtr)ipro);
    if(err) goto bail;
  }
  err = MP4GetListEntryCount(ipro->atomList, &j);
  if(err) goto bail;
  *protection_index = (u16)(j + 1);

  err = ipro->addAtom(ipro, (MP4AtomPtr)sinf);
  if(err) goto bail;

bail:
  if(schm) schm->destroy((MP4AtomPtr)schm);
  if(schi) schi->destroy((MP4AtomPtr)schi);
  if(sch_url_copy) free(sch_url_copy);

  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOAddMetaProtectionInfo(ISOMeta meta, u16 protection_index, MP4GenericAtom schi_atom)
{
  MP4Err err;

  ISOMetaAtomPtr myMeta;
  MP4SecurityInfoAtomPtr sinf;
  MP4SchemeInfoAtomPtr schi;
  ISOItemProtectionAtomPtr ipro;

  err    = MP4NoErr;
  myMeta = (ISOMetaAtomPtr)meta;
  ipro   = (ISOItemProtectionAtomPtr)myMeta->ipro;
  if(!ipro) BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntry(ipro->atomList, protection_index - 1, (char **)&sinf);
  if(err) goto bail;
  schi = (MP4SchemeInfoAtomPtr)sinf->MP4SchemeInfo;
  if(!schi) BAILWITHERROR(MP4BadParamErr);
  err = schi->addAtom(schi, (MP4AtomPtr)schi_atom);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOGetMetaProtection(ISOMeta meta, u16 protection_index, u32 *sch_type, u32 *sch_version,
                     char *sch_url)
{
  MP4Err err;

  ISOMetaAtomPtr myMeta;
  MP4SecurityInfoAtomPtr sinf;
  MP4SchemeTypeAtomPtr schm;
  ISOItemProtectionAtomPtr ipro;

  err    = MP4NoErr;
  myMeta = (ISOMetaAtomPtr)meta;

  ipro = (ISOItemProtectionAtomPtr)myMeta->ipro;
  if(!ipro) BAILWITHERROR(MP4BadParamErr);
  err = MP4GetListEntry(ipro->atomList, protection_index - 1, (char **)&sinf);
  if(err) goto bail;

  schm = (MP4SchemeTypeAtomPtr)sinf->MP4SchemeType;

  *sch_type    = schm->scheme_type;
  *sch_version = schm->scheme_version;

  if(sch_url && schm->scheme_url)
  {
    memcpy(sch_url, schm->scheme_url, strlen(schm->scheme_url) + 1);
  }

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOGetMetaProtectionInfo(ISOMeta meta, u16 protection_index, u32 atom_type,
                         MP4GenericAtom *schi_atom)
{
  MP4Err err;

  ISOMetaAtomPtr myMeta;
  MP4SecurityInfoAtomPtr sinf;
  MP4SchemeInfoAtomPtr schi;
  ISOItemProtectionAtomPtr ipro;
  u32 item_total, i;

  err        = MP4NoErr;
  *schi_atom = NULL;
  myMeta     = (ISOMetaAtomPtr)meta;
  ipro       = (ISOItemProtectionAtomPtr)myMeta->ipro;
  if(!ipro) BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntry(ipro->atomList, protection_index - 1, (char **)&sinf);
  if(err) goto bail;
  schi = (MP4SchemeInfoAtomPtr)sinf->MP4SchemeInfo;
  if(!schi) BAILWITHERROR(MP4BadParamErr);

  if(schi->atomList)
  {
    err = MP4GetListEntryCount(schi->atomList, &item_total);
    if(err) goto bail;

    for(i = 0; i < item_total; i++)
    {
      MP4AtomPtr a;
      err = MP4GetListEntry(schi->atomList, i, (char **)&a);
      if(err) goto bail;

      if(a->type == atom_type)
      {
        *schi_atom = (MP4GenericAtom)a;
        err        = MP4NoErr;
        goto bail;
      }
    }
  }
  else
    BAILWITHERROR(MP4BadDataErr)
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOAddMetaItemProperty(ISOMetaItem item, MP4GenericAtom *itemProperty, u8 essential)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  MP4ItemPropertiesAtomPtr iprp;
  u16 itemId;
  err = MP4NoErr;

  myItem = (MetaItemLocationPtr)item;
  if(myItem == NULL) BAILWITHERROR(MP4BadParamErr)

  myMeta = (ISOMetaAtomPtr)myItem->meta;
  if(myMeta == NULL) BAILWITHERROR(MP4BadDataErr)

  if(myMeta->iprp == NULL)
  {
    err = MP4CreateItemPropertiesAtom((MP4ItemPropertiesAtomPtr *)&myMeta->iprp);
    if(err) goto bail;
    err = MP4AddListEntry(myMeta->iprp, myMeta->atomList);
    if(err) goto bail;
  }

  iprp = (MP4ItemPropertiesAtomPtr)myMeta->iprp;

  err = ISOGetItemID(item, &itemId);
  err = iprp->addItemProperty(iprp, (MP4AtomPtr)itemProperty, itemId, essential);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOGetProperitesOfMetaItem(ISOMetaItem item, MP4GenericAtom **properties, u32 *propertiesFound)
{
  MP4Err err;
  ISOMetaAtomPtr myMeta;
  MetaItemLocationPtr myItem;
  MP4ItemPropertiesAtomPtr iprp;
  u16 itemId;
  MP4LinkedList properitiesList;
  u32 listSize;
  u32 i;
  MP4GenericAtom property;
  MP4GenericAtom *outProperties;
  err = MP4NoErr;

  myItem = (MetaItemLocationPtr)item;
  if(myItem == NULL) BAILWITHERROR(MP4BadParamErr)

  myMeta = (ISOMetaAtomPtr)myItem->meta;
  if(myMeta == NULL) BAILWITHERROR(MP4BadDataErr)

  if(myMeta->iprp == NULL)
  {
    err = MP4CreateItemPropertiesAtom((MP4ItemPropertiesAtomPtr *)&myMeta->iprp);
    if(err) goto bail;
    err = MP4AddListEntry(myMeta->iprp, myMeta->atomList);
    if(err) goto bail;
  }

  iprp = (MP4ItemPropertiesAtomPtr)myMeta->iprp;

  err = ISOGetItemID(item, &itemId);
  if(err) goto bail;
  err = iprp->getPropertiesOfItem(iprp, itemId, &properitiesList);
  if(err) goto bail;

  err = MP4GetListEntryCount(properitiesList, &listSize);
  if(err) goto bail;

  *propertiesFound = listSize;
  outProperties    = (MP4GenericAtom *)malloc(listSize * sizeof(MP4GenericAtom));

  for(i = 0; i < listSize; i++)
  {
    err = MP4GetListEntry(properitiesList, i, (char **)&property);
    if(err) goto bail;
    outProperties[i] = (MP4GenericAtom)property;
  }

  *properties = outProperties;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISONewEntityGroup(ISOMeta meta, u32 grouping_type, u32 group_id)
{
  u32 listSize, i;
  GroupListBoxPtr grpl;
  EntityToGroupBoxPtr entityGroupAtom;
  MP4Err err = MP4NoErr;

  ISOMetaAtomPtr metaAtom = (ISOMetaAtomPtr)meta;
  if(metaAtom == NULL) BAILWITHERROR(MP4BadParamErr);

  /* create grpl if not present */
  if(metaAtom->grpl == NULL)
  {
    err = MP4CreateGroupListBox((GroupListBoxPtr *)&metaAtom->grpl);
    if(err) goto bail;
    err = MP4AddListEntry(metaAtom->grpl, metaAtom->atomList);
    if(err) goto bail;
  }

  grpl = (GroupListBoxPtr)metaAtom->grpl;
  err  = MP4GetListEntryCount(grpl->atomList, &listSize);
  if(err) goto bail;
  for(i = 0; i < listSize; i++)
  {
    MP4AtomPtr anAtom;
    err = MP4GetListEntry(grpl->atomList, i, (char **)&anAtom);
    if(err) goto bail;
    entityGroupAtom = (EntityToGroupBoxPtr)anAtom;
    if(entityGroupAtom->group_id == group_id) BAILWITHERROR(MP4BadParamErr);
  }

  err = MP4CreateEntityToGroupBox(&entityGroupAtom, grouping_type);
  if(err) goto bail;
  entityGroupAtom->group_id = group_id;
  err                       = grpl->addAtom(grpl, (MP4AtomPtr)entityGroupAtom);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOAddEntityIDToGroup(ISOMeta meta, u32 group_id, u32 entity_id)
{
  u32 listSize, i;
  GroupListBoxPtr grpl;
  EntityToGroupBoxPtr entityGroupAtom;
  MP4Err err = MP4NoErr;

  ISOMetaAtomPtr metaAtom = (ISOMetaAtomPtr)meta;
  if(metaAtom == NULL) BAILWITHERROR(MP4BadParamErr);

  if(metaAtom->grpl == NULL) BAILWITHERROR(MP4NotFoundErr);
  grpl = (GroupListBoxPtr)metaAtom->grpl;
  err  = MP4GetListEntryCount(grpl->atomList, &listSize);
  if(err) goto bail;
  for(i = 0; i < listSize; i++)
  {
    MP4AtomPtr anAtom;
    err = MP4GetListEntry(grpl->atomList, i, (char **)&anAtom);
    if(err) goto bail;
    entityGroupAtom = (EntityToGroupBoxPtr)anAtom;
    if(entityGroupAtom->group_id == group_id)
    {
      err = entityGroupAtom->addEntityId(entityGroupAtom, entity_id);
      if(err) goto bail;
      return MP4NoErr;
    }
  }
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOGetEntityIDCnt(ISOMeta meta, u32 group_id, u32 *num_entities_in_group)
{
  u32 listSize, i;
  GroupListBoxPtr grpl;
  EntityToGroupBoxPtr entityGroupAtom;
  MP4Err err = MP4NoErr;

  ISOMetaAtomPtr metaAtom = (ISOMetaAtomPtr)meta;
  if(metaAtom == NULL) BAILWITHERROR(MP4BadParamErr);

  if(metaAtom->grpl == NULL) BAILWITHERROR(MP4NotFoundErr);
  grpl = (GroupListBoxPtr)metaAtom->grpl;
  err  = MP4GetListEntryCount(grpl->atomList, &listSize);
  if(err) goto bail;
  for(i = 0; i < listSize; i++)
  {
    MP4AtomPtr anAtom;
    err = MP4GetListEntry(grpl->atomList, i, (char **)&anAtom);
    if(err) goto bail;
    entityGroupAtom = (EntityToGroupBoxPtr)anAtom;
    if(entityGroupAtom->group_id == group_id)
    {
      *num_entities_in_group = entityGroupAtom->num_entities_in_group;
      return MP4NoErr;
    }
  }
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);
  return err;
}

ISO_EXTERN(ISOErr) ISOGetEntityGroupEntries(ISOMeta meta, EntityGroupEntryPtr *pEntries, u32 *cnt)
{
  u32 listSize, i, j;
  GroupListBoxPtr grpl;
  EntityToGroupBoxPtr entityGroupAtom;
  EntityGroupEntryPtr pGroupEntries = NULL;
  MP4Err err                        = MP4NoErr;

  ISOMetaAtomPtr metaAtom = (ISOMetaAtomPtr)meta;
  if(metaAtom == NULL) BAILWITHERROR(MP4BadParamErr);

  if(metaAtom->grpl == NULL) BAILWITHERROR(MP4NotFoundErr);
  grpl = (GroupListBoxPtr)metaAtom->grpl;
  err  = MP4GetListEntryCount(grpl->atomList, &listSize);
  if(err) goto bail;
  *cnt = listSize;

  pGroupEntries = (EntityGroupEntryPtr)calloc(listSize, sizeof(EntityGroupEntry));
  TESTMALLOC(pGroupEntries);

  for(i = 0; i < listSize; i++)
  {
    MP4AtomPtr anAtom;
    err = MP4GetListEntry(grpl->atomList, i, (char **)&anAtom);
    if(err) goto bail;
    entityGroupAtom = (EntityToGroupBoxPtr)anAtom;

    (pGroupEntries + i)->grouping_type         = entityGroupAtom->type;
    (pGroupEntries + i)->group_id              = entityGroupAtom->group_id;
    (pGroupEntries + i)->num_entities_in_group = entityGroupAtom->num_entities_in_group;

    if(entityGroupAtom->num_entities_in_group > 0)
    {
      (pGroupEntries + i)->entity_ids = calloc(entityGroupAtom->num_entities_in_group, sizeof(u32));
      for(j = 0; j < entityGroupAtom->num_entities_in_group; j++)
      {
        u32 *temp;
        err = MP4GetListEntry(entityGroupAtom->entity_ids, j, (char **)&temp);
        if(err)
        {
          free((pGroupEntries + i)->entity_ids);
          goto bail;
        }
        *((pGroupEntries + i)->entity_ids + j) = *temp;
      }
    }
  }

  *pEntries = pGroupEntries;

bail:
  if(err != MP4NoErr && pGroupEntries != NULL)
  {
    free(pGroupEntries);
  }
  TEST_RETURN(err);
  return err;
}
