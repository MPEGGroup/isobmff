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

#include "hevc_tool.h"

MP4Err  processWriteMode    (Options *options);
MP4Err  processReadMode     (Options *options);

int     main                ( int argc, char **argv )
{
    MP4Err                          err;
    Options                         options;
    
    logOutput   = stdout;
    err         = MP4NoErr;
    
    setDefaultValues(&options);
    
    if (!parseArguments(argc, argv, &options))
    {
        logMsg(LOGLEVEL_ERROR, "Parsing options failed!");
        BAILWITHERROR(MP4BadParamErr);
    }
    
    if (options.isJustAskingForHelp) goto bail;
    logLevel    = options.debugLevel;
    
    logMsg(LOGLEVEL_INFO, "ISO Image File Format Tool started.\n");
    printOptions(&options);
    
    if        (options.mode == ISOIFF_ToolMode_Write)
    {
        err = processWriteMode(&options);   if (err) goto bail;
    }
    else if   (options.mode == ISOIFF_ToolMode_Read)
    {
        err = processReadMode(&options);    if (err) goto bail;
    }
    
    
    logMsg(LOGLEVEL_INFO, "ISO Image File Format Tool finished.");
    freeOptions(&options);
bail:
    fflush(stdout);
	return err;
}

MP4Err  processWriteMode    (Options *options)
{
    MP4Err                              err;
    ISOIFF_HEVCDecoderConfigRecord      decoderConfigRecord;
    ISOIFF_HEVCItemData                 hevcImageItemData;
    ISOIFF_ImageCollection              collection;
    
    err = MP4NoErr;
    
    err = ISOIFF_CreateHEVCDecoderConfigRecord(&decoderConfigRecord);                           if (err) goto bail;
    err = ISOIFF_CreateHEVCItemData(&hevcImageItemData);                                        if (err) goto bail;
    
    err = processHEVC_NALUnits(decoderConfigRecord, hevcImageItemData, options);                if (err) goto bail;
    
    err = createHEVC_ImageCollection(&collection);                                              if (err) goto bail;
    err = addHEVCImageToCollection(collection, decoderConfigRecord, hevcImageItemData);         if (err) goto bail;
    
    err = ISOIFF_WriteCollectionToFile(collection, options->outputFile);                        if (err) goto bail;
    //err = ISOIFF_ReadCollectionFromFile(&collection, options->outputFile);                      if (err) goto bail;
    
    err = ISOIFF_FreeHEVCItemData(hevcImageItemData);                                           if (err) goto bail;
    err = ISOIFF_FreeHEVCDecoderConfigRecord(decoderConfigRecord);                              if (err) goto bail;
    err = ISOIFF_FreeImageCollection(collection);                                               if (err) goto bail;
bail:
    return err;
}

MP4Err  processReadMode     (Options *options)
{
    u32                                 imgCount;
    u32                                 size;
    u32                                 i;
    MP4Handle                           bitstreamH;
    MP4Err                              err;
    ISOIFF_ImageCollection              collection;
    ISOIFF_Image                        *images;
    ISOIFF_HEVCDecoderConfigRecord      *records;
    FILE                                *file;
    
    err = MP4NoErr;
    err = MP4NewHandle(0, &bitstreamH);                                                         if (err) goto bail;
    
    err = ISOIFF_ReadCollectionFromFile(&collection, options->inputFile);                       if (err) goto bail;
    
    err = getHEVCImages(collection, &images, &records, &imgCount);                              if (err) goto bail;
    err = getHEVCBitstreamFromImage(images[0], records[0], bitstreamH);                         if (err) goto bail;
    
    err = MP4GetHandleSize(bitstreamH, &size);                                                  if (err) goto bail;
    
    file = fopen(options->outputFile, "wb");
    fwrite(*bitstreamH, size, 1, file);
    fclose(file);
    
    for (i = 0; i < imgCount; i++)
    {
        err = ISOIFF_FreeImage(images[i]);                                                      if (err) goto bail;
        err = ISOIFF_FreeHEVCDecoderConfigRecord(records[i]);                                   if (err) goto bail;
        free (images);
        free (records);
    }
    
    err = MP4DisposeHandle(bitstreamH);                                                         if (err) goto bail;
    err = ISOIFF_FreeImageCollection(collection);                                               if (err) goto bail;
bail:
    return err;
}

