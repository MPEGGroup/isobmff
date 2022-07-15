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
 * the code for non MPEG-4 conforming products.
 *
 * This copyright notice must be included in all copies or derivative works. Copyright (c) 2014.
 */

#include "hevc_tool.h"
#include "avc_tool.h"
#include "jpeg_tool.h"
#include <vector>

MP4Err test();

MP4Err processWriteMode(Options *options);
MP4Err processWriteModeHEVC(Options *options);
MP4Err processWriteModeAVC(Options *options);
MP4Err processWriteModeJPEG(Options *options);
MP4Err processReadMode(Options *options);
MP4Err readHEVCImages(ISOIFF_ImageCollection collection, MP4Handle bitstreamH, u32 *imgCount);
MP4Err readAVCImages(ISOIFF_ImageCollection collection, MP4Handle bitstreamH, u32 *imgCount);
MP4Err readJPEGImages(ISOIFF_ImageCollection collection, MP4Handle bitstreamH, u32 *imgCount);
MP4Err writeHandleToFile(const char *filename, MP4Handle handle);

int main(int argc, char **argv)
{
  MP4Err err;
  Options options;

  logOutput = stdout;
  err       = MP4NoErr;

  setDefaultValues(&options);

  if(!parseArguments(argc, argv, &options))
  {
    logMsg(LOGLEVEL_ERROR, "Parsing options failed!");
    BAILWITHERROR(MP4BadParamErr);
  }

  if(options.isJustAskingForHelp) goto bail;
  logLevel = options.debugLevel;

  logMsg(LOGLEVEL_INFO, "ISO Image File Format Tool started.\n");
  printOptions(&options);

  if(options.mode == ISOIFF_ToolMode_Write)
  {
    err = processWriteMode(&options);
    if(err) goto bail;
  }
  else if(options.mode == ISOIFF_ToolMode_Read)
  {
    err = processReadMode(&options);
    if(err) goto bail;
  }

  logMsg(LOGLEVEL_INFO, "ISO Image File Format Tool finished.");
  freeOptions(&options);
bail:
  fflush(stdout);
  return err;
}

MP4Err test()
{
  MP4Err err;
  err = MP4NoErr;

bail:
  return err;
}

MP4Err processWriteMode(Options *options)
{
  MP4Err err;
  err = MP4NoErr;

  if(strcmp(options->inputType, "hevc") == 0)
  {
    logMsg(LOGLEVEL_INFO, "Processing HEVC Write Mode..");
    err = processWriteModeHEVC(options);
  }
  else if(strcmp(options->inputType, "avc") == 0)
  {
    logMsg(LOGLEVEL_INFO, "Processing AVC Write Mode..");
    err = processWriteModeAVC(options);
  }
  else if(strcmp(options->inputType, "jpeg") == 0)
  {
    logMsg(LOGLEVEL_INFO, "Processing JPEG Write Mode..");
    err = processWriteModeJPEG(options);
  }

bail:
  return err;
}

MP4Err processWriteModeHEVC(Options *options)
{
  MP4Err err;
  ISOIFF_HEVCDecoderConfigRecord decoderConfigRecord;
  ISOIFF_HEVCItemData hevcImageItemData;
  ISOIFF_ImageCollection collection;

  err = MP4NoErr;

  err = ISOIFF_CreateHEVCDecoderConfigRecord(&decoderConfigRecord);
  if(err) goto bail;
  err = ISOIFF_CreateHEVCItemData(&hevcImageItemData);
  if(err) goto bail;

  err = processHEVC_NALUnits(decoderConfigRecord, hevcImageItemData, options);
  if(err) goto bail;

  err = createHEVC_ImageCollection(&collection);
  if(err) goto bail;
  err = addHEVCImageToCollection(collection, decoderConfigRecord, hevcImageItemData,
                                 (u32)options->width, (u32)options->height);
  if(err) goto bail;

  err = ISOIFF_WriteCollectionToFile(collection, options->outputFile);
  if(err) goto bail;

  if(options->debugLevel >= 4)
  {
    err = ISOIFF_ReadCollectionFromFile(&collection, options->outputFile);
    if(err) goto bail;
  }
  err = ISOIFF_FreeHEVCItemData(hevcImageItemData);
  if(err) goto bail;
  err = ISOIFF_FreeHEVCDecoderConfigRecord(decoderConfigRecord);
  if(err) goto bail;
  err = ISOIFF_FreeImageCollection(collection);
  if(err) goto bail;
bail:
  return err;
}

MP4Err processWriteModeAVC(Options *options)
{
  MP4Err err;
  ISOVCConfigAtomPtr avcC;
  ISOIFF_HEVCItemData hevcImageItemData;
  ISOIFF_ImageCollection collection;

  err = MP4NoErr;

  err = MP4CreateVCConfigAtom(&avcC);
  if(err) goto bail;
  err = ISOIFF_CreateHEVCItemData(&hevcImageItemData);
  if(err) goto bail;

  err = processAVC_NALUnits(avcC, hevcImageItemData, options);
  if(err) goto bail;

  err = createAVC_ImageCollection(&collection);
  if(err) goto bail;
  err = addAVCImageToCollection(collection, avcC, hevcImageItemData, (u32)options->width,
                                (u32)options->height);
  if(err) goto bail;

  err = ISOIFF_WriteCollectionToFile(collection, options->outputFile);
  if(err) goto bail;

  if(options->debugLevel >= 4)
  {
    err = ISOIFF_ReadCollectionFromFile(&collection, options->outputFile);
    if(err) goto bail;
  }
  err = ISOIFF_FreeHEVCItemData(hevcImageItemData);
  if(err) goto bail;
  avcC->destroy((MP4AtomPtr)avcC);
  err = ISOIFF_FreeImageCollection(collection);
  if(err) goto bail;

bail:
  return err;
}

MP4Err processWriteModeJPEG(Options *options)
{
  MP4Err err;
  ISOIFF_JPEGConfigurationAtomPtr jpgC;
  MP4Handle jpegPrefixHeaderH;
  MP4Handle jpegImageItemDataH;
  ISOIFF_ImageCollection collection;
  u32 prefixHeaderSize;

  err = MP4NoErr;

  jpgC = NULL;
  err  = MP4NewHandle(0, &jpegImageItemDataH);
  if(err) goto bail;
  err = MP4NewHandle(0, &jpegPrefixHeaderH);
  if(err) goto bail;

  err = processJPEGBitstream(jpegPrefixHeaderH, jpegImageItemDataH, options);
  if(err) goto bail;

  err = MP4GetHandleSize(jpegPrefixHeaderH, &prefixHeaderSize);
  if(err) goto bail;

  if(prefixHeaderSize != 0)
  {
    err = ISOIFF_CreateJPEGConfigurationAtom(&jpgC, jpegPrefixHeaderH);
    if(err) goto bail;
  }

  err = createJPEG_ImageCollection(&collection);
  if(err) goto bail;
  err = addJPEGImageToCollection(collection, jpgC, jpegImageItemDataH, (u32)options->width,
                                 (u32)options->height);
  if(err) goto bail;

  err = ISOIFF_WriteCollectionToFile(collection, options->outputFile);
  if(err) goto bail;

  if(options->debugLevel >= 4)
  {
    err = ISOIFF_ReadCollectionFromFile(&collection, options->outputFile);
    if(err) goto bail;
  }

  if(jpgC)
  {
    jpgC->destroy((MP4AtomPtr)jpgC);
  }
  err = ISOIFF_FreeImageCollection(collection);
  if(err) goto bail;

bail:
  return err;
}

MP4Err processReadMode(Options *options)
{
  u32 imgCount;
  u32 size;
  MP4Handle bitstreamH;
  MP4Err err;
  ISOIFF_ImageCollection collection;
  FILE *file;

  err = MP4NoErr;
  err = MP4NewHandle(0, &bitstreamH);
  if(err) goto bail;

  err = ISOIFF_ReadCollectionFromFile(&collection, options->inputFile);
  if(err) goto bail;

  err = readHEVCImages(collection, bitstreamH, &imgCount);
  if(err) goto bail;

  if(imgCount >= 1)
  {
    logMsg(LOGLEVEL_INFO, "Writing first HEVC Image to output file %s", options->outputFile);
    err = writeHandleToFile(options->outputFile, bitstreamH);
    if(err) goto bail;
  }
  else
  {
    logMsg(LOGLEVEL_INFO, "No HEVC images found. Looking for AVC images next.");
    err = readAVCImages(collection, bitstreamH, &imgCount);
    if(err) goto bail;

    if(imgCount >= 1)
    {
      logMsg(LOGLEVEL_INFO, "Writing first AVC Image to output file %s", options->outputFile);
      err = writeHandleToFile(options->outputFile, bitstreamH);
      if(err) goto bail;
    }
    else
    {
      logMsg(LOGLEVEL_INFO, "No AVC images found. Looking for JPEG images next.");
      err = readJPEGImages(collection, bitstreamH, &imgCount);
      if(err) goto bail;

      if(imgCount >= 1)
      {
        logMsg(LOGLEVEL_INFO, "Writing first JPEG Image to output file %s", options->outputFile);
        err = writeHandleToFile(options->outputFile, bitstreamH);
        if(err) goto bail;
      }
    }
  }

  err = MP4DisposeHandle(bitstreamH);
  if(err) goto bail;
  err = ISOIFF_FreeImageCollection(collection);
  if(err) goto bail;
bail:
  return err;
}

MP4Err readHEVCImages(ISOIFF_ImageCollection collection, MP4Handle bitstreamH, u32 *imgCount)
{
  u32 i;
  MP4Err err;
  ISOIFF_Image *images;
  ISOIFF_HEVCDecoderConfigRecord *records;
  FILE *file;

  logMsg(LOGLEVEL_INFO, "Looking for HEVC Images..\n");

  err = getHEVCImages(collection, &images, &records, imgCount);
  if(err) goto bail;

  logMsg(LOGLEVEL_INFO, "Found %d HEVC Images.", *imgCount);

  if(*imgCount >= 1) err = getHEVCBitstreamFromImage(images[0], records[0], bitstreamH);
  if(err) goto bail;

  for(i = 0; i < *imgCount; i++)
  {
    err = ISOIFF_FreeImage(images[i]);
    if(err) goto bail;
    err = ISOIFF_FreeHEVCDecoderConfigRecord(records[i]);
    if(err) goto bail;
  }
  free(images);
  free(records);

bail:
  return err;
}

MP4Err readAVCImages(ISOIFF_ImageCollection collection, MP4Handle bitstreamH, u32 *imgCount)
{
  u32 i;
  MP4Err err;
  ISOIFF_Image *images;
  ISOVCConfigAtomPtr *avcCAtoms;
  FILE *file;

  logMsg(LOGLEVEL_INFO, "Looking for AVC Images..\n");

  err = getAVCImages(collection, &images, &avcCAtoms, imgCount);
  if(err) goto bail;

  logMsg(LOGLEVEL_INFO, "Found %d AVC Images.", *imgCount);

  if(*imgCount >= 1) err = getAVCBitstreamFromImage(images[0], avcCAtoms[0], bitstreamH);
  if(err) goto bail;
  /*
  for (i = 0; i < *imgCount; i++)
  {
    err = ISOIFF_FreeImage(images[i]);                                                      if (err)
  goto bail;
  }
  */

  free(images);
  free(avcCAtoms);

bail:
  return err;
}

MP4Err readJPEGImages(ISOIFF_ImageCollection collection, MP4Handle bitstreamH, u32 *imgCount)
{
  u32 i;
  MP4Err err;
  ISOIFF_Image *images;
  ISOIFF_JPEGConfigurationAtomPtr *jpgCAtoms;
  FILE *file;

  logMsg(LOGLEVEL_INFO, "Looking for JPEG Images..\n");

  err = getJPEGImages(collection, &images, &jpgCAtoms, imgCount);
  if(err) goto bail;

  logMsg(LOGLEVEL_INFO, "Found %d JPEG Images.", *imgCount);

  if(*imgCount >= 1) err = getJPEGBitstreamFromImage(images[0], jpgCAtoms[0], bitstreamH);
  if(err) goto bail;

  for(i = 0; i < *imgCount; i++)
  {
    err = ISOIFF_FreeImage(images[i]);
    if(err) goto bail;
  }

  free(images);
  free(jpgCAtoms);

bail:
  return err;
}

MP4Err writeHandleToFile(const char *filename, MP4Handle handle)
{
  MP4Err err;
  u32 size;
  FILE *file;

  err = MP4GetHandleSize(handle, &size);
  if(err) goto bail;

  file = fopen(filename, "wb");
  fwrite(*handle, size, 1, file);
  fclose(file);
bail:
  return err;
}
