/* This software module was originally developed by Apple Computer, Inc. in the course of
development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module or
modifications thereof for use in hardware or software products claiming conformance to MPEG-4. Those
intending to use this software module in hardware or software products are advised that its use may
infringe existing patents. The original developer of this software module and his/her company, the
subsequent editors and their companies, and ISO/IEC have no liability for use of this software
module or modifications thereof in an implementation. Copyright is not released for non MPEG-4
conforming products. Apple Computer, Inc. retains full right to use the code for its own purpose,
assign or donate the code to a third party and to inhibit third parties from using the code for non
MPEG-4 conforming products.

This copyright notice must be included in all copies or derivative works. Copyright (c) 2016.
*/

#include <fstream>
#include "avc_tool.h"
#include "hevc_tool.h"

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"
#include "TLibDecoder/TDecEntropy.h"
#include "TLibDecoder/TDecCAVLC.h"

#define AVC_NALU_TYPE_SPS 7
#define AVC_NALU_TYPE_SPS_EXT 13
#define AVC_NALU_TYPE_PPS 8

extern "C"
{
#include "StringUtils.h"
#include "Logger.h"
}

using namespace std;

MP4Err processAVC_SPS(ISOVCConfigAtomPtr avcC, MP4Handle nalData);

MP4Err createAVC_ImageCollection(ISOIFF_ImageCollection *collection)
{
  MP4Err err;

  logMsg(LOGLEVEL_INFO, "Creating AVC image collection.");
  err = ISOIFF_CreateImageCollection(collection, ISOIFF_4CC_mif1, 0);
  if(err) goto bail;
  logMsg(LOGLEVEL_INFO, "Creating AVC image collection finished.");
bail:
  return err;
}
MP4Err processAVC_NALUnits(ISOVCConfigAtomPtr avcC, ISOIFF_HEVCItemData itemData, Options *options)
{
  MP4Err err;
  u8 lengthSizeMinusOne;

  err = MP4NoErr;

  logMsg(LOGLEVEL_INFO, "Processing AVC NAL Units.");

  ifstream bitstreamFile(options->inputFile, ifstream::in | ifstream::binary);
  InputByteStream bytestream(bitstreamFile);

  while(!!bitstreamFile)
  {
    AnnexBStats stats = AnnexBStats();
    vector<uint8_t> nalUnit;
    InputNALUnit nalu;
    MP4Handle nalData;
    u8 *buffer;

    byteStreamNALUnit(bytestream, nalu.getBitstream().getFifo(), stats);

    if(!nalu.getBitstream().getFifo().empty())
    {
      err = MP4NewHandle((u32)nalu.getBitstream().getFifo().size(), &nalData);
      if(err) goto bail;
      buffer = (u8 *)*nalData;

      for(u32 i = 0; i < nalu.getBitstream().getFifo().size(); i++)
      {
        buffer[i] = (u8)nalu.getBitstream().getFifo().at(i);
      }

      read(nalu);

      u8 naluType = buffer[0] & 0x1F;

      logMsg(LOGLEVEL_INFO, "%d NALU found", naluType);

      if(naluType == AVC_NALU_TYPE_SPS)
      {
        err = processAVC_SPS(avcC, nalData);
        err = avcC->addParameterSet(avcC, nalData, AVCsps);
        if(err) goto bail;
      }
      else if(naluType == AVC_NALU_TYPE_PPS)
      {
        err = avcC->addParameterSet(avcC, nalData, AVCpps);
        if(err) goto bail;
      }
      else if(naluType == AVC_NALU_TYPE_SPS_EXT)
      {
        err = avcC->addParameterSet(avcC, nalData, AVCspsext);
        if(err) goto bail;
      }
      else
      {
        err = ISOIFF_AddNALUnitToHEVCItemData(itemData, nalData);
        if(err) goto bail;
      }

      err = MP4DisposeHandle(nalData);
      if(err) goto bail;
    }
  }

  err = ISOIFF_GetHEVCItemDataLengthSizeMinusOne(itemData, &lengthSizeMinusOne);
  if(err) goto bail;
  avcC->length_size = lengthSizeMinusOne + 1;

  logMsg(LOGLEVEL_INFO, "Processing AVC NAL Units finished.");
bail:
  return err;
}
MP4Err addAVCImageToCollection(ISOIFF_ImageCollection collection, ISOVCConfigAtomPtr avcC,
                               ISOIFF_HEVCItemData itemData, u32 width, u32 height)
{
  MP4Err err;
  MP4Handle imageData;
  ISOIFF_Image image;
  ISOIFF_ImageSpatialExtentsPropertyAtomPtr ispe;

  logMsg(LOGLEVEL_INFO, "Adding AVC image to collection.");

  err = MP4NoErr;
  err = MP4NewHandle(0, &imageData);
  if(err) goto bail;

  err = ISOIFF_PutHEVCItemDataIntoHandle(itemData, imageData);
  if(err) goto bail;
  err = ISOIFF_NewImage(collection, &image, ISOIFF_4CC_avc1, imageData);
  if(err) goto bail;

  err = ISOIFF_AddImageProperty(image, (MP4AtomPtr)avcC, 1);
  if(err) goto bail;

  err = ISOIFF_CreateImageSpatialExtentsPropertyAtom(&ispe);
  if(err) goto bail;
  ispe->image_width  = width;
  ispe->image_height = height;
  err                = ISOIFF_AddImageProperty(image, (MP4AtomPtr)ispe, 0);
  if(err) goto bail;

  err = MP4DisposeHandle(imageData);
  if(err) goto bail;
  err = ISOIFF_FreeImage(image);
  if(err) goto bail;

  logMsg(LOGLEVEL_INFO, "Adding AVC image to collection finished.");
bail:
  return err;
}
MP4Err getAVCImages(ISOIFF_ImageCollection collection, ISOIFF_Image **images,
                    ISOVCConfigAtomPtr **avcCAtoms, u32 *numberOfImagesFound)
{
  MP4Err err;
  u32 i, j;

  logMsg(LOGLEVEL_INFO, "Requesting AVC images from collection.");

  err = MP4NoErr;
  err = ISOIFF_GetAllImagesWithType(collection, ISOIFF_4CC_avc1, images, numberOfImagesFound);
  if(err) goto bail;

  logMsg(LOGLEVEL_INFO, "Number of images found: %d", *numberOfImagesFound);

  *avcCAtoms = (ISOVCConfigAtomPtr *)calloc(*numberOfImagesFound, sizeof(ISOVCConfigAtomPtr));

  for(i = 0; i < *numberOfImagesFound; i++)
  {
    MP4GenericAtom *properties;
    u32 numberOfPropertiesFound;

    properties = NULL;
    err        = ISOIFF_GetImageProperties((*images)[i], &properties, &numberOfPropertiesFound);
    if(err) goto bail;

    if(numberOfPropertiesFound == 0) BAILWITHERROR(MP4BadDataErr);

    for(j = 0; j < numberOfPropertiesFound; j++)
    {
      MP4AtomPtr property;
      property = (MP4AtomPtr)properties[j];
      if(property->type == ISOIFF_4CC_avcC)
      {
        ISOVCConfigAtomPtr avcC;
        (*avcCAtoms)[i] = (ISOVCConfigAtomPtr)property;
      }
      else if(property->type == ISOIFF_4CC_ispe)
      {
        err = ISOIFF_ParseImageSpatialExtends(property);
        if(err) goto bail;
      }
    }

    free(properties);
  }

bail:
  return err;
}
MP4Err getAVCBitstreamFromImage(ISOIFF_Image image, ISOVCConfigAtomPtr avcC, MP4Handle bitstreamH)
{
  MP4Err err;
  u8 *buffer;
  MP4Handle startCodePrefixH;
  MP4Handle zeroByteH;
  MP4Handle naluH;
  MP4Handle imgDatH;
  ISOIFF_HEVCItemData hevcItemData;
  MP4LinkedList imageNALUnits;
  u32 i;

  logMsg(LOGLEVEL_INFO, "Creating AVC bitstream from image.");

  err = MP4MakeLinkedList(&imageNALUnits);
  if(err) goto bail;

  err = MP4NewHandle(3, &startCodePrefixH);
  if(err) goto bail;
  buffer    = (u8 *)*startCodePrefixH;
  buffer[0] = 0;
  buffer[1] = 0;
  buffer[2] = 1;

  err = MP4NewHandle(1, &zeroByteH);
  if(err) goto bail;
  buffer    = (u8 *)*zeroByteH;
  buffer[0] = 0;

  if(avcC->spsList->entryCount > 0)
  {
    err = MP4GetListEntry(avcC->spsList, 0, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, zeroByteH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  if(avcC->ppsList->entryCount > 0)
  {
    err = MP4GetListEntry(avcC->ppsList, 0, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, zeroByteH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  if(avcC->spsextList->entryCount > 0)
  {
    err = MP4GetListEntry(avcC->spsextList, 0, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, zeroByteH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  err = MP4NewHandle(0, &imgDatH);
  if(err) goto bail;
  err = ISOIFF_GetImageData(image, imgDatH);
  if(err) goto bail;

  err = ISOIFF_CreateHEVCItemDataFromHandle(imgDatH, &hevcItemData, avcC->length_size - 1);
  if(err) goto bail;
  err = ISOIFF_GetNALUDataHandlesFromHEVCItemData(hevcItemData, imageNALUnits);
  if(err) goto bail;

  for(i = 0; i < imageNALUnits->entryCount; i++)
  {
    err = MP4GetListEntry(imageNALUnits, i, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  err = MP4DisposeHandle(startCodePrefixH);
  if(err) goto bail;
  err = MP4DisposeHandle(imgDatH);
  if(err) goto bail;
  err = MP4DisposeHandle(zeroByteH);
  if(err) goto bail;

  err = MP4DeleteLinkedList(imageNALUnits);
  if(err) goto bail;

  err = ISOIFF_FreeHEVCItemData(hevcItemData);
  if(err) goto bail;

bail:
  return err;
}

int32_t getBitByPos(unsigned char *buffer, int32_t pos)
{
  return (buffer[pos / 8] >> (8 - pos % 8) & 0x01);
}

uint32_t decodeGolomb(unsigned char *byteStream, uint32_t *index)
{
  uint32_t leadingZeroBits = -1;
  uint32_t codeNum         = 0;
  uint32_t pos             = *index;

  for(int32_t b = 0; !b; leadingZeroBits++)
    b = getBitByPos(byteStream, pos++);

  for(int32_t b = leadingZeroBits; b > 0; b--)
    codeNum = codeNum | (getBitByPos(byteStream, pos++) << (b - 1));

  *index = pos;
  return ((1 << leadingZeroBits) - 1 + codeNum);
}

MP4Err processAVC_SPS(ISOVCConfigAtomPtr avcC, MP4Handle nalData)
{
  MP4Err err;
  u8 *buffer;
  err    = MP4NoErr;
  buffer = (u8 *)*nalData;

  avcC->profile               = buffer[1];
  avcC->profile_compatibility = buffer[2];
  avcC->level                 = buffer[3];

  avcC->chroma_format           = 1;
  avcC->bit_depth_chroma_minus8 = 0;
  avcC->bit_depth_luma_minus8   = 0;

  if(avcC->profile == 100 || avcC->profile == 110 || avcC->profile == 122 || avcC->profile == 144)
  {
    uint32_t index = 33;
    decodeGolomb(buffer, &index);
    avcC->chroma_format           = decodeGolomb(buffer, &index);
    avcC->bit_depth_chroma_minus8 = decodeGolomb(buffer, &index);
    avcC->bit_depth_luma_minus8   = decodeGolomb(buffer, &index);
  }

bail:
  return err;
}
