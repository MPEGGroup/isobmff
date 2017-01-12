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

/*!
 @header isoiff
 Provides tools to process images through iso media file format
 @copyright Apple
 @updated 2014-11-17
 @author Armin Trattnig
 @version 1.0
 */

#ifndef isoiff_h
#define isoiff_h

#include "MP4Movies.h"
#include "MP4Atoms.h"

enum
{
	ISOIFF_4CC_init = MP4_FOUR_CHAR_CODE('i', 'n', 'i', 't'),
	ISOIFF_4CC_ispe = MP4_FOUR_CHAR_CODE('i', 's', 'p', 'e'),
	ISOIFF_4CC_mif1 = MP4_FOUR_CHAR_CODE('m', 'i', 'f', '1'),
	ISOIFF_4CC_avcC = MP4_FOUR_CHAR_CODE('a', 'v', 'c', 'C'),
	ISOIFF_4CC_avc1 = MP4_FOUR_CHAR_CODE('a', 'v', 'c', '1')
};

/*!
 * @typedef ISOIFF_ImageCollection
 * @brief Represents a collection of images (Stored conforming to ISO Media Based Image File Format)
 */
typedef struct  ISOIFF_ImageCollectionS              *ISOIFF_ImageCollection;

/*!
 * @typedef ISOIFF_Image
 * @brief Represents an image contained in an ISO Media Based Image File Format image collection
 */
typedef struct  ISOIFF_ImageS                        *ISOIFF_Image;

/*!
 * @typedef ISOIFF_Meta
 * @brief Represents an meta item contained in an ISO Media Based Image File Format image collection
 */
typedef struct  ISOIFF_MetaS                         *ISOIFF_Meta;

/*!
* @typedef ISOIFF_ImageSpatialExtentsPropertyAtom
* @brief ImageSpatialExtentsPropertyAtom
*/
typedef struct ISOIFF_ImageSpatialExtentsPropertyAtom
{
	MP4_FULL_ATOM
	u32 image_width;
	u32 image_height;
} ISOIFF_ImageSpatialExtentsPropertyAtom, *ISOIFF_ImageSpatialExtentsPropertyAtomPtr;

/*!
* @discussion Creates a ImageSpatialExtentsPropertyAtom (Allocates memory and initializes fields)
* @param outAtom Pointer that will hold a reference to the created ImageSpatialExtentsPropertyAtom
*/
MP4Err			ISOIFF_CreateImageSpatialExtentsPropertyAtom	(ISOIFF_ImageSpatialExtentsPropertyAtomPtr *outAtom);

/*!
* @discussion Parses an ImageSpatialExtentsProperty from a property atom and prints width and height
* @param property Property atom that is of type ispe
*/
MP4Err			ISOIFF_ParseImageSpatialExtends					(MP4AtomPtr property);

/*!
 * @discussion Creates an image collection conforming to ISO Base Media Image File Format.
 * @param collection This ISOIFF_ImageCollection will be allocated and initialized
 * @param brand The brand of the image collection
 * @param minorVersion The minorversion of the image collection
 */
MP4Err          ISOIFF_CreateImageCollection        (ISOIFF_ImageCollection *collection, u32 brand, u32 minorVersion);

/*!
 * @discussion Creates and adds an image with a given type and data to an image collection
 * @param collection The collection to which the image will be added to
 * @param image The image that will be allocated and created
 * @param type The type of the image in 4CC
 */
MP4Err          ISOIFF_NewImage                     (ISOIFF_ImageCollection collection, ISOIFF_Image *image, u32 type, MP4Handle data);

/*!
 * @discussion Creates and adds a meta item with a given type and data to an image collection
 * @param collection The collection to which the meta item will be added to
 * @param meta The meta item that will be allocated and created
 * @param type The type of the meta item in 4CC
 */
MP4Err          ISOIFF_NewMeta                      (ISOIFF_ImageCollection collection, ISOIFF_Meta *meta, u32 type, MP4Handle data);

/*!
 * @discussion Writes an image collection to a file
 * @param collection The image collection
 * @param filename Path ot the file
 */
MP4Err          ISOIFF_WriteCollectionToFile        (ISOIFF_ImageCollection collection, const char *filename);

/*!
 * @discussion Reads an image collection from a file
 * @param collection The image collection
 * @param filename Path ot the file
 */
MP4Err          ISOIFF_ReadCollectionFromFile       (ISOIFF_ImageCollection *collection, const char *filename);

/*!
 * @discussion Collects all images of a given type from an image collections and presents the result as an array
 * @param collection The image collection
 * @param type The 4cc type of the images, that will be collected
 * @param images Pointer to an ISOIFF_Image array. Will be allocated and filled with images.
 * @param numberOfImagesFound Number of images in the images array
 */
MP4Err          ISOIFF_GetAllImagesWithType         (ISOIFF_ImageCollection collection, u32 type, ISOIFF_Image **images, u32 *numberOfImagesFound);

/*!
 * @discussion Collects all meta items of a given type from an image collections and presents the result as an array
 * @param collection The image collection
 * @param type The 4cc type of the meta items, that will be collected
 * @param metas Pointer to an ISOIFF_Meta array. Will be allocated and filled with meta items.
 * @param numberOfMetasFound Number of metas in the metas array
 */
MP4Err          ISOIFF_GetAllMetasWithType          (ISOIFF_ImageCollection collection, u32 type, ISOIFF_Meta **metas, u32 *numberOfMetasFound);

/*!
 * @discussion Gets the 4cc type of an image
 * @param image Image with the type requested
 * @param outType Will contain the 4cc type of the image
 */
MP4Err          ISOIFF_GetImageType                 (ISOIFF_Image image, u32 *outType);

/*!
 * @discussion Gets the 4cc type of an meta item
 * @param meta Meta item with the type requested
 * @param outType Will contain the 4cc type of the meta item
 */
MP4Err          ISOIFF_GetMetaType                  (ISOIFF_Meta meta, u32 *outType);

/*!
 * @discussion Gets the data of an image in form of an MP4Handle
 * @param image Image with the data requested
 * @param data Will contain the data of the image
 */
MP4Err          ISOIFF_GetImageData                 (ISOIFF_Image image, MP4Handle data);

/*!
 * @discussion Gets the data of an meta item in form of an MP4Handle
 * @param meta Meta itme with the data requested
 * @param data Will contain the data of the meta item
 */
MP4Err          ISOIFF_GetMetaData                  (ISOIFF_Meta meta, MP4Handle data);

/*!
 * @discussion Sets an image as the cover image of its collection. (This means it will become the primary item)
 * @param image Image, which will be set as cover image
 */
MP4Err          ISOIFF_SetImageAsCover              (ISOIFF_Image image);

/*!
 * @discussion Adds a relation between two images to their collection.
 * @param fromImage The relation points from this image to the toImage
 * @param toImage The relation points to this image from the fromImage
 * @param relationType The 4cc relation type
 */
MP4Err          ISOIFF_AddImageRelation             (ISOIFF_Image fromImage, ISOIFF_Image toImage, u32 relationType);

/*!
 * @discussion Adds a meta item to an image (This means there will be a relation from the image to the meta item)
 * @param fromImage The relation points from this image to the toMeta meta item
 * @param toMeta The relation points to this meta item from the fromImage
 * @param relationType The 4cc relation type
 */
MP4Err          ISOIFF_AddMetaToImage               (ISOIFF_Image fromImage, ISOIFF_Meta toMeta, u32 relationType);

/*!
 * @discussion Adds a image to an meta item (This means there will be a relation from the meta item to the image)
 * @param fromMeta The relation points from this meta item to the toImage
 * @param toImage The relation points to this image from the fromMeta
 * @param relationType The 4cc relation type
 */
MP4Err          ISOIFF_AddImageToMeta               (ISOIFF_Meta fromMeta, ISOIFF_Image toImage, u32 relationType);

/*!
 * @discussion Collects all images that an image points to, with a certain relation type.
 * @param fromImage The image from which the relations of the type relationType point to the toImages
 * @param relationType The 4cc relation type
 * @param toImages This array will be allocated and filled with images found for the fromImage's relations
 * @param numberOfImagesFound The number of images in toImages
 */
MP4Err          ISOIFF_GetImagesOfImageWithType     (ISOIFF_Image fromImage, u32 relationType, ISOIFF_Image **toImages, u32 *numberOfImagesFound);

/*!
 * @discussion Collects all meta itmes that an image points to, with a certain relation type.
 * @param fromImage The image from which the relations of the type relationType point to the toMetas
 * @param relationType The 4cc relation type
 * @param toMetas This array will be allocated and filled with meta items found for the fromImage's relations
 * @param numberOfMetasFound The number of meta items in toMetas
 */
MP4Err          ISOIFF_GetMetasOfImageWithType      (ISOIFF_Image fromImage, u32 relationType, ISOIFF_Meta **toMetas, u32 *numberOfMetasFound);

/*!
 * @discussion Collects all images that an meta item points to, with a certain relation type.
 * @param fromMeta The meta item from which the relations of the type relationType point to the toImages
 * @param relationType The 4cc relation type
  * @param toImages This array will be allocated and filled with images found for the fromMeta's relations
 * @param numberOfImagesFound The number of images in toImages
 */
MP4Err          ISOIFF_GetImagesOfMetaWithType      (ISOIFF_Meta fromMeta, u32 relationType, ISOIFF_Image **toImages, u32 *numberOfImagesFound);

/*!
 * @discussion Deallocates the memory used by an image collection
 * @param collection Collection that will be deallocated
 */
MP4Err          ISOIFF_FreeImageCollection          (ISOIFF_ImageCollection collection);

/*!
 * @discussion Deallocates the memory used by an ISOIFF_Meta struct
 * @param meta ISOIFF_Meta that will be deallocated
 */
MP4Err          ISOIFF_FreeMeta                     (ISOIFF_Meta meta);

/*!
 * @discussion Deallocates the memory used by an ISOIFF_Image struct
 * @param image ISOIFF_Image that will be deallocated
 */
MP4Err          ISOIFF_FreeImage                    (ISOIFF_Image image);

/*!
* @discussion Adds a property to the image
* @param image ISOIFF_Image image to which the property gets added
* @param property MP4AtomPtr the property, which is added to the image
* @param essential u8 marks the property as essential for the image
*/
MP4Err          ISOIFF_AddImageProperty				(ISOIFF_Image image, MP4AtomPtr property, u8 essential);

/*!
* @discussion Obtains an array of properties from an image
* @param image ISOIFF_Image image that has properties
* @param properties MP4AtomPtr* array of properties of the image
* @param propertiesFound u32* the amount of properties found for the image
*/
MP4Err          ISOIFF_GetImageProperties			(ISOIFF_Image image, MP4GenericAtom **properties, u32 *propertiesFound);

#endif