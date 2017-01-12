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

#include "isoiff.h"
#include "Logger.h"
#include "ISOMovies.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

enum
{
    ISOIFF_IMAGE_META_HANDLER_TYPE  = MP4_FOUR_CHAR_CODE('p', 'i', 'c', 't'),
    ISOIFF_IMAGE_COLLECTION_BRAND   = MP4_FOUR_CHAR_CODE('m', 'i', 'f', '1')
};

struct ISOIFF_ImageCollectionS
{
    MP4Movie        moov;
    ISOMeta         meta;
    u8              isPrimaryImageSet;
};

struct ISOIFF_ImageS
{
    ISOMetaItem                 item;
    ISOIFF_ImageCollection      collection;
};

struct ISOIFF_MetaS
{
    ISOMetaItem                 item;
    ISOIFF_ImageCollection      collection;
};

static void     typeToString                        (u32 inType, char* ioStr)
{
    u32     i;
    int     ch;
    
    for (i = 0; i < 4; i++, ioStr++)
    {
        ch = inType >> (8*(3-i)) & 0xff;
        if ( isprint(ch) )
            *ioStr = ch;
        else
            *ioStr = '.';
    }
    *ioStr = 0;
}

static void     logType                             (int debugLevel, const char *text, u32 type)
{
    char    typeString[8];
    typeToString( type, typeString );
    logMsg(debugLevel, "%s :'%s'", text, typeString);
}

MP4Err          ISOIFF_CreateImageCollection         (ISOIFF_ImageCollection *collection, u32 brand, u32 minorVersion)
{
    MP4Err  err;
    u32     tmp;

    logMsg(LOGLEVEL_DEBUG, "Creating image collection");
    logType(LOGLEVEL_DEBUG, "Brand", brand);
    logMsg(LOGLEVEL_DEBUG, "Minorversion: %d", minorVersion);
    
    *collection = calloc(1, sizeof(struct ISOIFF_ImageCollectionS));
    
    err         = ISONewMetaMovie(&(*collection)->moov, ISOIFF_IMAGE_META_HANDLER_TYPE, brand, minorVersion);           if (err) goto bail;
    err         = ISOGetFileMeta((*collection)->moov, &(*collection)->meta, ISOIFF_IMAGE_META_HANDLER_TYPE, &tmp);      if (err) goto bail;
    err         = ISOSetMovieCompatibleBrand((*collection)->moov, ISOIFF_IMAGE_COLLECTION_BRAND);                       if (err) goto bail;
    
    (*collection)->isPrimaryImageSet = 0;
bail:
    return err;
}

MP4Err          ISOIFF_NewImage                     (ISOIFF_ImageCollection collection, ISOIFF_Image *image, u32 type, MP4Handle data)
{
    MP4Err  err;
    
    logType(LOGLEVEL_DEBUG, "Adding image with type", type);
    
    *image  = calloc(1, sizeof(struct ISOIFF_ImageS));
    err     = ISOAddMetaItem(collection->meta, &(*image)->item, 0, 0);  if (err) goto bail;
    err     = ISOAddItemExtentUsingItemData((*image)->item, data );     if (err) goto bail;
    err     = ISOSetItemInfo((*image)->item, 0, "image", NULL, NULL);   if (err) goto bail;
    err     = ISOSetItemInfoItemType ( (*image)->item, type, NULL);     if (err) goto bail;
    
    (*image)->collection = collection;
    if (collection->isPrimaryImageSet == 0)
    {
        collection->isPrimaryImageSet = 1;
        err = ISOIFF_SetImageAsCover(*image); if (err) goto bail;
    }
    
bail:
    return err;
}

MP4Err          ISOIFF_NewMeta                      (ISOIFF_ImageCollection collection, ISOIFF_Meta *meta, u32 type, MP4Handle data)
{
    MP4Err err;

    logType(LOGLEVEL_DEBUG, "Adding meta with type", type);

    *meta   = calloc(1, sizeof(struct ISOIFF_MetaS));
    err     = ISOAddMetaItem(collection->meta, &(*meta)->item, 0, 0);       if (err) goto bail;
    err     = ISOAddItemExtentUsingItemData((*meta)->item, data );          if (err) goto bail;
    err     = ISOSetItemInfo((*meta)->item, 0, "image meta", NULL, NULL);   if (err) goto bail;
    err     = ISOSetItemInfoItemType ( (*meta)->item, type, NULL);          if (err) goto bail;
    
bail:
    return err;
}

MP4Err          ISOIFF_WriteCollectionToFile        (ISOIFF_ImageCollection collection, const char *filename)
{
    MP4Err err;
    
    logMsg(LOGLEVEL_DEBUG, "Writing Image Collection to file: '%s'", filename);

    err = MP4WriteMovieToFile( collection->moov, filename );                if (err) goto bail;
bail:
    return err;
}

MP4Err          ISOIFF_ReadCollectionFromFile       (ISOIFF_ImageCollection *collection, const char *filename)
{
    MP4Err  err;
    u32     outHandleType;
    
    logMsg(LOGLEVEL_DEBUG, "Read Image Collection from file: '%s'", filename);
    
    *collection     = calloc(1, sizeof(struct ISOIFF_ImageCollectionS));
    err             = MP4OpenMovieFile( &(*collection)->moov, filename, MP4OpenMovieDebug );                                        if (err) goto bail;
    err             = ISOGetFileMeta((*collection)->moov, &(*collection)->meta, ISOIFF_IMAGE_META_HANDLER_TYPE, &outHandleType);    if (err) goto bail;
    
    if (outHandleType != ISOIFF_IMAGE_META_HANDLER_TYPE)
        err = MP4BadDataErr;
    
bail:
    return err;
}

MP4Err          ISOIFF_GetAllImagesWithType         (ISOIFF_ImageCollection collection, u32 type, ISOIFF_Image **images, u32 *numberOfImagesFound)
{
    MP4Err          err;
    ISOMetaItem     *items;
    u32             i;
    
    logType(LOGLEVEL_DEBUG, "Collecting images with type", type);
    
    err = ISOGetAllItemsWithType( collection->meta, type, &items, numberOfImagesFound ); if (err) goto bail;
    
    *images = calloc(*numberOfImagesFound, sizeof(ISOIFF_Image));
    for (i = 0; i < *numberOfImagesFound; i++)
    {
        ISOIFF_Image image = (*images)[i];
        
        image                   = calloc(1, sizeof(struct ISOIFF_ImageS));
        image->item             = (ISOMetaItem) items[i];
        image->collection       = collection;
        (*images)[i]            = image;
    }
    
    free (items);
    logMsg(LOGLEVEL_DEBUG, "Images found: %d", *numberOfImagesFound);
bail:
    return err;
}

MP4Err          ISOIFF_GetAllMetasWithType          (ISOIFF_ImageCollection collection, u32 type, ISOIFF_Meta **metas, u32 *numberOfMetasFound)
{
    MP4Err          err;
    ISOMetaItem     *items;
    u32             i;
    
    logType(LOGLEVEL_DEBUG, "Collecting image metas with type", type);
    
    err = ISOGetAllItemsWithType( collection->meta, type, &items, numberOfMetasFound ); if (err) goto bail;
    
    *metas = calloc(*numberOfMetasFound, sizeof(ISOIFF_Meta));
    for (i = 0; i < *numberOfMetasFound; i++)
    {
        ISOIFF_Meta meta;
        
        meta                   = calloc(1, sizeof(struct ISOIFF_MetaS));
        meta->item             = (ISOMetaItem) items[i];
        meta->collection       = collection;
        (*metas)[i]            = meta;
    }
    
    logMsg(LOGLEVEL_DEBUG, "Metas found: %d", *numberOfMetasFound);
bail:
    return err;
}

MP4Err          ISOIFF_SetImageAsCover              (ISOIFF_Image image)
{
    MP4Err  err;
    u32     type;
    
    err = ISOIFF_GetImageType(image, &type);             if (err) goto bail;
    
    logType(LOGLEVEL_DEBUG, "Setting Image Collection Cover Image. Type", type);
    
    err = ISOSetPrimaryItem(image->collection->meta, image->item);      if (err) goto bail;
bail:
    return err;
}

MP4Err          ISOIFF_AddImageRelation             (ISOIFF_Image fromImage, ISOIFF_Image toImage, u32 relationType)
{
    MP4Err  err;
    u32     outIndex;
    u16     toItemID;
    u32     fromType;
    u32     toType;
    
    logType(LOGLEVEL_DEBUG, "Adding image relation with relation type", relationType);
    
    err = ISOGetItemID(toImage->item, &toItemID);                                           if (err) goto bail;
    err = ISOAddItemReference(fromImage->item, relationType, toItemID, &outIndex);          if (err) goto bail;
    
    err = ISOIFF_GetImageType(fromImage, &fromType);                         if (err) goto bail;
    err = ISOIFF_GetImageType(toImage, &toType);                             if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Added image relation details:");
    logType(LOGLEVEL_DEBUG, "From", fromType);
    logType(LOGLEVEL_DEBUG, "To", toType);
bail:
    return err;
}

MP4Err          ISOIFF_AddMetaToImage               (ISOIFF_Image fromImage, ISOIFF_Meta toMeta, u32 relationType)
{
    MP4Err err;
    u32     outIndex;
    u16     toItemID;
    u32     fromType;
    u32     toType;
    
    logType(LOGLEVEL_DEBUG, "Adding meta relation with relation type", relationType);
    
    err = ISOGetItemID( toMeta->item, &toItemID );                                          if (err) goto bail;
    err = ISOAddItemReference(fromImage->item, relationType, toItemID, &outIndex);          if (err) goto bail;
    
    err = ISOIFF_GetImageType(fromImage, &fromType);                        if (err) goto bail;
    err = ISOIFF_GetMetaType(toMeta, &toType);                              if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Added meta relation details:");
    logType(LOGLEVEL_DEBUG, "From", fromType);
    logType(LOGLEVEL_DEBUG, "To", toType);
    
bail:
    return err;
}

MP4Err          ISOIFF_AddImageToMeta               (ISOIFF_Meta fromMeta, ISOIFF_Image toImage, u32 relationType)
{
    MP4Err  err;
    u32     outIndex;
    u16     toItemID;
    u32     fromType;
    u32     toType;
    
    logType(LOGLEVEL_DEBUG, "Adding meta relation with relation type", relationType);
    
    err = ISOGetItemID( toImage->item, &toItemID );                                         if (err) goto bail;
    err = ISOAddItemReference(fromMeta->item, relationType, toItemID, &outIndex);           if (err) goto bail;
    
    err = ISOIFF_GetMetaType(fromMeta, &fromType);                              if (err) goto bail;
    err = ISOIFF_GetImageType(toImage, &toType);                                if (err) goto bail;
    
    logMsg(LOGLEVEL_DEBUG, "Added meta relation details:");
    logType(LOGLEVEL_DEBUG, "From", fromType);
    logType(LOGLEVEL_DEBUG, "To", toType);
    
bail:
    return err;
}

MP4Err          ISOIFF_GetImageType                 (ISOIFF_Image image, u32 *outType)
{
    MP4Err  err;
    char    *tmp;
    
    tmp = NULL;
    err = ISOGetItemInfoItemType(image->item, outType, &tmp);   if (err) goto bail;
    if (tmp)
        free (tmp);
bail:
    return err;
}

MP4Err          ISOIFF_GetMetaType                  (ISOIFF_Meta meta, u32 *outType)
{
    MP4Err  err;
    char    *tmp;
    
    tmp = NULL;
    err = ISOGetItemInfoItemType(meta->item, outType, &tmp);   if (err) goto bail;
    if (tmp)
        free (tmp);
bail:
    return err;
}

MP4Err          ISOIFF_GetImageData                 (ISOIFF_Image image, MP4Handle data)
{
    MP4Err  err;
    u64     baseOffset;
    
    err = ISOGetItemData(image->item, data, &baseOffset);
    
bail:
    return err;
}

MP4Err          ISOIFF_GetMetaData                  (ISOIFF_Meta meta, MP4Handle data)
{
    MP4Err  err;
    u64     baseOffset;
    
    err = ISOGetItemData(meta->item, data, &baseOffset);
    
bail:
    return err;
}

MP4Err          ISOIFF_GetImagesOfImageWithType     (ISOIFF_Image fromImage, u32 relationType, ISOIFF_Image **toImages, u32 *numberOfImagesFound)
{
    MP4Err      err;
    MP4Handle   toItemIDs;
    u16         referenceCount;
    u32         *toItemIDsArray;
    u16         i;
    
    logType(LOGLEVEL_DEBUG, "Collecting images of image with relation type", relationType);
    
    err                     = MP4NewHandle(0, &toItemIDs);                                                          if (err) goto bail;
    err                     = ISOGetItemReferences(fromImage->item, relationType, &referenceCount, toItemIDs );     if (err) goto bail;
    
    *numberOfImagesFound    = referenceCount;
    toItemIDsArray          = (u32*) *toItemIDs;
    
    *toImages = calloc(referenceCount, sizeof(ISOIFF_Image));
    for (i = 0; i < referenceCount; i++)
    {
        ISOIFF_Image        refImage;
        ISOMetaItem         item;
        
        refImage               = calloc(1, sizeof(struct ISOIFF_ImageS));
        err                    = ISOFindItemByID(fromImage->collection->meta, &item, (u16) toItemIDsArray[i]);      if (err) goto bail;
        refImage->item         = item;
        refImage->collection   = fromImage->collection;
        (*toImages)[i]         = refImage;
    }
    
    logMsg(LOGLEVEL_DEBUG, "Images found: %d", *numberOfImagesFound);
bail:
    return err;
}

MP4Err          ISOIFF_GetMetasOfImageWithType      (ISOIFF_Image fromImage, u32 relationType, ISOIFF_Meta **toMetas, u32 *numberOfMetasFound)
{
    MP4Err      err;
    MP4Handle   toItemIDs;
    u16         referenceCount;
    u32         *toItemIDsArray;
    u16         i;
    
    logType(LOGLEVEL_DEBUG, "Collecting metas of image with relation type", relationType);

    err                     = MP4NewHandle(0, &toItemIDs);                                                          if (err) goto bail;
    err                     = ISOGetItemReferences(fromImage->item, relationType, &referenceCount, toItemIDs );     if (err) goto bail;
    
    *numberOfMetasFound     = referenceCount;
    toItemIDsArray          = (u32*) *toItemIDs;
    
    *toMetas = calloc(referenceCount, sizeof(ISOIFF_Meta));
    for (i = 0; i < referenceCount; i++)
    {
        ISOIFF_Meta     meta;
        ISOMetaItem     item;
        
        meta                   = calloc(1, sizeof(struct ISOIFF_MetaS));
        err                    = ISOFindItemByID(fromImage->collection->meta, &item, (u16) toItemIDsArray[i]);      if (err) goto bail;
        meta->item             = item;
        meta->collection       = fromImage->collection;
        (*toMetas)[i]          = meta;
    }
    
    err = MP4DisposeHandle(toItemIDs);  if (err) goto bail;
    logMsg(LOGLEVEL_DEBUG, "Metas found: %d", *numberOfMetasFound);
bail:
    return err;
}

MP4Err          ISOIFF_GetImagesOfMetaWithType      (ISOIFF_Meta fromMeta, u32 relationType, ISOIFF_Image **toImages, u32 *numberOfImagesFound)
{
    MP4Err      err;
    MP4Handle   toItemIDs;
    u16         referenceCount;
    u32         *toItemIDsArray;
    u16         i;
    
    logType(LOGLEVEL_DEBUG, "Collecting images of meta with relation type", relationType);
    
    err                     = MP4NewHandle(0, &toItemIDs);                                                          if (err) goto bail;
    err                     = ISOGetItemReferences(fromMeta->item, relationType, &referenceCount, toItemIDs );      if (err) goto bail;
    
    *numberOfImagesFound    = referenceCount;
    toItemIDsArray          = (u32*) *toItemIDs;
    
    *toImages = calloc(referenceCount, sizeof(ISOIFF_Image));
    for (i = 0; i < referenceCount; i++)
    {
        ISOIFF_Image        image;
        ISOMetaItem         item;
        
        image                  = calloc(1, sizeof(struct ISOIFF_ImageS));
        err                    = ISOFindItemByID(fromMeta->collection->meta, &item, (u16) toItemIDsArray[i]);      if (err) goto bail;
        image->item            = item;
        image->collection      = fromMeta->collection;
        (*toImages)[i]         = image;
    }
    
    logMsg(LOGLEVEL_DEBUG, "Images found: %d", *numberOfImagesFound);
bail:
    return err;
}

MP4Err          ISOIFF_FreeImageCollection          (ISOIFF_ImageCollection collection)
{
    MP4Err      err;
    
    err = MP4DisposeMovie(collection->moov); if (err) goto bail;
    free (collection);
bail:
    return err;
}

MP4Err          ISOIFF_FreeMeta                     (ISOIFF_Meta meta)
{
    MP4Err      err;
    
    err     =   MP4NoErr;
    free (meta);
bail:
    return err;
}

MP4Err          ISOIFF_FreeImage                    (ISOIFF_Image image)
{
    MP4Err      err;
    
    err     =   MP4NoErr;
    free (image);
bail:
    return err;
}

MP4Err          ISOIFF_AddImageProperty				(ISOIFF_Image image, MP4AtomPtr property, u8 essential)
{
	MP4Err  err;
	
	err = ISOAddMetaItemProperty(image->item, (MP4GenericAtom *) property, essential);
bail:
	return err;
}
MP4Err          ISOIFF_GetImageProperties			(ISOIFF_Image image, MP4GenericAtom **properties, u32 *propertiesFound)
{
	MP4Err  err;

	err = ISOGetProperitesOfMetaItem(image->item, properties, propertiesFound);
bail:
	return err;
}

MP4Err			ISOIFF_ParseImageSpatialExtends		(MP4AtomPtr property)
{
	MP4Err			  err;
	MP4UnknownAtomPtr ispe;
	MP4Handle         metaDataH;
	char  		      *buffer;
	u8				  tmp8;
	u32				  width;
	u32				  height;
	err = MP4NoErr;


	ispe = (MP4UnknownAtomPtr)property;
	buffer = ispe->data;

	buffer += 4;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	width = tmp8 << 24;
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	width |= (tmp8 << 16);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	width |= (tmp8 << 8);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	width |= tmp8;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	height = tmp8 << 24;
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	height |= (tmp8 << 16);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	height |= (tmp8 << 8);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	height |= tmp8;

	logMsg(LOGLEVEL_INFO, "Parsed image spatial extends. Width: %d, Height: %d", width, height);
bail:
	return err;
}