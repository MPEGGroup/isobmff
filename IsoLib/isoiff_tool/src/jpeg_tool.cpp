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
derivative works. Copyright (c) 2016.
*/

#include "jpeg_tool.h"

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <fstream>


extern "C"
{
    #include "StringUtils.h"
    #include "Logger.h"
}

using namespace std;


MP4Err     createJPEG_ImageCollection(ISOIFF_ImageCollection *collection)
{
	MP4Err err;

	logMsg(LOGLEVEL_INFO, "Creating JPEG image collection.");
	err = ISOIFF_CreateImageCollection(collection, ISOIFF_4CC_mif1, 0);         if (err) goto bail;
	logMsg(LOGLEVEL_INFO, "Creating JPEG image collection finished.");
bail:
	return err;
}
MP4Err     addJPEGImageToCollection(ISOIFF_ImageCollection collection, ISOIFF_JPEGConfigurationAtomPtr jpgC, MP4Handle itemData, u32 width, u32 height)
{
	MP4Err											err;
	MP4Handle										imageData;
	ISOIFF_Image									image;
	ISOIFF_ImageSpatialExtentsPropertyAtomPtr		ispe;

	logMsg(LOGLEVEL_INFO, "Adding JPEG image to collection.");

	err = MP4NoErr;
	err = MP4NewHandle(0, &imageData);                                          if (err) goto bail;

	err = ISOIFF_NewImage(collection, &image, ISOIFF_4CC_jpeg, itemData);       if (err) goto bail;

	if (jpgC)
	{
		err = ISOIFF_AddImageProperty(image, (MP4AtomPtr)jpgC, 1);				if (err) goto bail;
	}

	err = ISOIFF_CreateImageSpatialExtentsPropertyAtom(&ispe);					if (err) goto bail;
	ispe->image_width = width;
	ispe->image_height = height;
	err = ISOIFF_AddImageProperty(image, (MP4AtomPtr)ispe, 0);					if (err) goto bail;

	err = MP4DisposeHandle(imageData);                                          if (err) goto bail;
	err = ISOIFF_FreeImage(image);                                              if (err) goto bail;

	logMsg(LOGLEVEL_INFO, "Adding JPEG image to collection finished.");
bail:
	return err;
}
MP4Err     getJPEGImages(ISOIFF_ImageCollection collection, ISOIFF_Image **images, ISOIFF_JPEGConfigurationAtomPtr **jpgCAtoms, u32 *numberOfImagesFound)
{
	MP4Err                  err;
	u32                     i, j;

	logMsg(LOGLEVEL_INFO, "Requesting AVC images from collection.");

	err = MP4NoErr;
	err = ISOIFF_GetAllImagesWithType(collection, ISOIFF_4CC_jpeg, images, numberOfImagesFound);            if (err) goto bail;

	logMsg(LOGLEVEL_INFO, "Number of images found: %d", *numberOfImagesFound);

	*jpgCAtoms = (ISOIFF_JPEGConfigurationAtomPtr *)calloc(*numberOfImagesFound, sizeof(ISOIFF_JPEGConfigurationAtomPtr));

	for (i = 0; i < *numberOfImagesFound; i++)
	{
		MP4GenericAtom					*properties;
		u32							   numberOfPropertiesFound;

		(*jpgCAtoms)[i] = NULL;
		properties = NULL;
		err = ISOIFF_GetImageProperties((*images)[i], &properties, &numberOfPropertiesFound);					if (err) goto bail;

		if (numberOfPropertiesFound == 0)
			BAILWITHERROR(MP4BadDataErr);

		for (j = 0; j < numberOfPropertiesFound; j++)
		{
			MP4AtomPtr property;
			property = (MP4AtomPtr)properties[j];
			if (property->type == ISOIFF_4CC_jpgC)
			{
				MP4Handle							jpegPrefix;
				MP4UnknownAtomPtr					unkownAtom;
				ISOIFF_JPEGConfigurationAtomPtr		jpgC;
				u8									*buffer;
				unkownAtom = (MP4UnknownAtomPtr) property;
				char *data = unkownAtom->data;
				err = MP4NewHandle(unkownAtom->dataSize, &jpegPrefix); if (err) goto bail;
				buffer = (u8 *)*jpegPrefix;
				memcpy(buffer, data, unkownAtom->dataSize);
				err = ISOIFF_CreateJPEGConfigurationAtom(&jpgC, jpegPrefix); if (err) goto bail;
				(*jpgCAtoms)[i] = (ISOIFF_JPEGConfigurationAtomPtr) jpgC;
			}
			else if (property->type == ISOIFF_4CC_ispe)
			{
				err = ISOIFF_ParseImageSpatialExtends(property); if (err) goto bail;
			}
		}

		free(properties);
	}

bail:
	return err;
}
MP4Err     getJPEGBitstreamFromImage(ISOIFF_Image image, ISOIFF_JPEGConfigurationAtomPtr jpgC, MP4Handle bitstreamH)
{
	MP4Err					err;
	MP4Handle               imgDatH;

	err = MP4NewHandle(0, &imgDatH);            if (err) goto bail;
	err = ISOIFF_GetImageData(image, imgDatH);  if (err) goto bail;

	if (jpgC != NULL)
	{
		err = MP4HandleCat(bitstreamH, jpgC->jpegPrefix);    if (err) goto bail;
	}

	err = MP4HandleCat(bitstreamH, imgDatH);    if (err) goto bail;
bail:
	return err;
}

MP4Err     processJPEGBitstream(MP4Handle prefixData, MP4Handle itemData, Options  *options)
{
	MP4Err					err;
	MP4Handle				inputH;
	u8						*buffer;
	u32						prefixLength;

	FILE *f = fopen(options->inputFile, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET); 

	err = MP4NewHandle(fsize, &inputH);   if (err) goto bail;
	buffer = (u8 *)*inputH;
	fread(buffer, fsize, 1, f);
	fclose(f);

	prefixLength = 0;

	while ((buffer[prefixLength] != 0xFF) && (buffer[prefixLength] != 0xD8))
	{
		prefixLength++;
	}

	if (prefixLength == 0)
	{
		err = MP4HandleCat(itemData, inputH);
		goto bail;
	}

	err = MP4SetHandleSize(prefixData, prefixLength); if (err) goto bail;
	u8						*prefixHbuffer;
	prefixHbuffer = (u8 *)*prefixData;

	for (u32 i = 0; i < prefixLength; i++)
	{
		prefixHbuffer[i] = buffer[i];
	}

	err = MP4SetHandleOffset(inputH, prefixLength); if (err) goto bail;
	err = MP4HandleCat(itemData, inputH); if (err) goto bail;

bail:
	return err;
}