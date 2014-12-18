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

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <fstream>

#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"
#include "TLibDecoder/TDecEntropy.h"
#include "TLibDecoder/TDecCAVLC.h"

extern "C"
{
    #include "StringUtils.h"
    #include "Logger.h"
}

using namespace std;

MP4Err     processHEVC_SPS            (ISOIFF_HEVCDecoderConfigRecord record, InputNALUnit *nalu);

MP4Err     processHEVC_NALUnits       (ISOIFF_HEVCDecoderConfigRecord record, ISOIFF_HEVCItemData itemData, Options  *options)
{
    MP4Err  err;
    u8      lengthSizeMinusOne;
    
    err = MP4NoErr;
    
    logMsg(LOGLEVEL_INFO, "Processing HEVC NAL Units.");
    
    ifstream        bitstreamFile   (options->inputFile, ifstream::in | ifstream::binary);
    InputByteStream bytestream      (bitstreamFile);
    
    while (!!bitstreamFile)
    {
        AnnexBStats         stats = AnnexBStats();
        vector<uint8_t>     nalUnit;
        InputNALUnit        nalu;
        MP4Handle           nalData;
        u8                  *buffer;
        
        byteStreamNALUnit(bytestream, nalUnit, stats);
        
        if (!nalUnit.empty())
        {
            err     = MP4NewHandle((u32) nalUnit.size(), &nalData);                                     if (err) goto bail;
            buffer  = (u8*) *nalData;
            
            for (u32 i = 0; i < nalUnit.size(); i++)
            {
                buffer[i] = (u8) nalUnit.at(i);
            }
            
            read(nalu, nalUnit);
            
            switch (nalu.m_nalUnitType)
            {
                case NAL_UNIT_VPS:
                {
                    logMsg(LOGLEVEL_INFO, "VPS NAL Unit found!");
                    
                    err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);       if (err) goto bail;
                    break;
                }
                case NAL_UNIT_PPS:
                {
                    logMsg(LOGLEVEL_INFO, "PPS NAL Unit found!");
                    
                    err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);       if (err) goto bail;
                    break;
                }
                case NAL_UNIT_SPS:
                {
                    logMsg(LOGLEVEL_INFO, "SPS NAL Unit found!");
                    
                    err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);       if (err) goto bail;
                    
                    err = processHEVC_SPS(record, &nalu); if (err) goto bail;
                    break;
                }
                case NAL_UNIT_PREFIX_SEI:
                case NAL_UNIT_SUFFIX_SEI:
                {
                    logMsg(LOGLEVEL_INFO, "SEI NAL Unit found!");
                    
                    err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);       if (err) goto bail;
                    break;
                }
                default:
                {
                    logMsg(LOGLEVEL_INFO, "Non parameter set NAL Unit found!", nalu.m_nalUnitType);
                    
                    err = ISOIFF_AddNALUnitToHEVCItemData(itemData, nalData);                           if (err) goto bail;
                    break;
                }
            }
            
            err = MP4DisposeHandle(nalData);                                                            if (err) goto bail;
        }
    }
    
    err                         = ISOIFF_GetHEVCItemDataLengthSizeMinusOne(itemData, &lengthSizeMinusOne); if (err) goto bail;
    record->lengthSizeMinusOne  = lengthSizeMinusOne;
    
    logMsg(LOGLEVEL_INFO, "Processing HEVC NAL Units finished.");
bail:
    return err;
}

MP4Err     processHEVC_SPS            (ISOIFF_HEVCDecoderConfigRecord record, InputNALUnit *nalu)
{
    MP4Err              err;
    TComSPS             *sps;
    ProfileTierLevel    *ptl;
    TDecEntropy         m_cEntropyDecoder;
    TDecCavlc           m_cCavlcDecoder;
    
    err = MP4NoErr;
    sps = new TComSPS();
    
    logMsg(LOGLEVEL_INFO, "Processing HEVC SPS NAL Unit.");
    
    m_cEntropyDecoder.setEntropyDecoder(&m_cCavlcDecoder);
    m_cEntropyDecoder.setBitstream(nalu->m_Bitstream);
    m_cEntropyDecoder.decodeSPS(sps);
    
    ptl = sps->getPTL()->getGeneralPTL();
    
    if (sps->getVuiParametersPresentFlag())
    {
        TComVUI *vui = sps->getVuiParameters();
        record->min_spatial_segmentation_idc = vui->getMinSpatialSegmentationIdc();
    }
    
    record->general_tier_flag       = ptl->getTierFlag();
    record->general_level_idc       = ptl->getLevelIdc();
    record->chromaFormat            = sps->getChromaFormatIdc();
    record->bitDepthLumaMinus8      = sps->getBitDepth(CHANNEL_TYPE_LUMA)      - 8;
    record->bitDepthChromaMinus8    = sps->getBitDepth(CHANNEL_TYPE_CHROMA)    - 8;
    
    logMsg(LOGLEVEL_DEBUG, "General tier flag: %d",     record->general_tier_flag);
    logMsg(LOGLEVEL_DEBUG, "General levle idc: %d",     record->general_level_idc);
    logMsg(LOGLEVEL_DEBUG, "Chroma format: %d",         record->chromaFormat);
    logMsg(LOGLEVEL_DEBUG, "Bit depth luma - 8: %d",    record->bitDepthLumaMinus8);
    logMsg(LOGLEVEL_DEBUG, "Bit depth chroma - 8: %d",  record->bitDepthChromaMinus8);
    
    if (ptl->getProgressiveSourceFlag())
    {
        logMsg(LOGLEVEL_DEBUG, "Progressive source flag is on");
        record->general_constraint_indicator_flags |= 0x800000000000;
    }
    else
    {
        logMsg(LOGLEVEL_DEBUG, "Progressive source flag is off");
    }
    
    if (ptl->getInterlacedSourceFlag())
    {
        logMsg(LOGLEVEL_DEBUG, "Interlaced source flag is on");
        record->general_constraint_indicator_flags |= 0x400000000000;
    }
    else
    {
        logMsg(LOGLEVEL_DEBUG, "Interlaced source flag is off");
    }
    
    if (ptl->getNonPackedConstraintFlag())
    {
        logMsg(LOGLEVEL_DEBUG, "Non packed constraint flag is on");
        record->general_constraint_indicator_flags |= 0x200000000000;
    }
    else
    {
        logMsg(LOGLEVEL_DEBUG, "Non packed constraint flag is off");
    }
    
    if (ptl->getFrameOnlyConstraintFlag())
    {
        logMsg(LOGLEVEL_DEBUG, "Frame only constraint flag is on");
        record->general_constraint_indicator_flags |= 0x100000000000;
    }
    else
    {
        logMsg(LOGLEVEL_DEBUG, "Frame only constraint is off");
    }
    delete sps;
bail:
    return err;
}

MP4Err     createHEVC_ImageCollection    (ISOIFF_ImageCollection *collection)
{
    MP4Err err;

    logMsg(LOGLEVEL_INFO, "Creating HEVC image collection.");
    err = ISOIFF_CreateImageCollection(collection, ISOIFF_4CC_hevc, 0);         if (err) goto bail;
    logMsg(LOGLEVEL_INFO, "Creating HEVC image collection finished.");
bail:
    return err;
}

MP4Err     addHEVCImageToCollection    (ISOIFF_ImageCollection collection, ISOIFF_HEVCDecoderConfigRecord record, ISOIFF_HEVCItemData itemData)
{
    MP4Err                  err;
    MP4Handle               metaData;
    MP4Handle               imageData;
    ISOIFF_Image            image;
    ISOIFF_Meta             meta;
    
    logMsg(LOGLEVEL_INFO, "Adding HEVC image to collection.");
    
    err = MP4NoErr;
    err = MP4NewHandle(0, &metaData);                                           if (err) goto bail;
    err = MP4NewHandle(0, &imageData);                                          if (err) goto bail;
    
    err = ISOIFF_PutHEVCDecConfRecordIntoHandle(record, metaData);              if (err) goto bail;
    err = ISOIFF_PutHEVCItemDataIntoHandle(itemData, imageData);                if (err) goto bail;
    
    err = ISOIFF_NewImage(collection, &image, ISOIFF_4CC_hvc1, imageData);      if (err) goto bail;
    err = ISOIFF_NewMeta(collection, &meta, ISOIFF_4CC_hvcC, metaData);         if (err) goto bail;
    
    err = ISOIFF_AddMetaToImage(image, meta, ISOIFF_4CC_init);                  if (err) goto bail;
    
    err = MP4DisposeHandle(metaData);                                           if (err) goto bail;
    err = MP4DisposeHandle(imageData);                                          if (err) goto bail;
    err = ISOIFF_FreeImage(image);                                              if (err) goto bail;
    err = ISOIFF_FreeMeta(meta);                                                if (err) goto bail;
    
    logMsg(LOGLEVEL_INFO, "Adding HEVC image to collection finished.");
bail:
    return err;
}

MP4Err     getHEVCImages                 (ISOIFF_ImageCollection collection, ISOIFF_Image **images, ISOIFF_HEVCDecoderConfigRecord **decoderConfigs, u32 *numberOfImagesFound)
{
    MP4Err                  err;
    u32                     i;
    
    logMsg(LOGLEVEL_INFO, "Requesting HEVC images from collection.");
    
    err = MP4NoErr;
    err = ISOIFF_GetAllImagesWithType(collection, ISOIFF_4CC_hvc1, images, numberOfImagesFound);            if (err) goto bail;
    
    logMsg(LOGLEVEL_INFO, "Number of images found: %d", *numberOfImagesFound);
    
    *decoderConfigs = (ISOIFF_HEVCDecoderConfigRecord *) calloc(*numberOfImagesFound, sizeof(struct ISOIFF_HEVCDecoderConfigRecordS));
    
    for (i = 0; i < *numberOfImagesFound; i++)
    {
        ISOIFF_Meta                    *metas;
        MP4Handle                      metaDataH;
        u32                            numberOfMetasFound;
        
        err = MP4NewHandle(0, &metaDataH);                                                                  if (err) goto bail;
        err = ISOIFF_GetMetasOfImageWithType(*images[i], ISOIFF_4CC_init, &metas, &numberOfMetasFound);     if (err) goto bail;
        
        if (numberOfMetasFound == 0)
            BAILWITHERROR(MP4BadDataErr);
        
        err = ISOIFF_GetMetaData(metas[0], metaDataH);                                                      if (err) goto bail;
        err = ISOIFF_CreateHEVCDecConfRecFromHandle(metaDataH, &(*decoderConfigs[i]));                      if (err) goto bail;
        
        err = ISOIFF_FreeMeta(metas[0]);                                                                    if (err) goto bail;
        err = MP4DisposeHandle(metaDataH);                                                                  if (err) goto bail;
        free (metas);
    }
    
bail:
    return err;
}

MP4Err     getHEVCBitstreamFromImage    (ISOIFF_Image image, ISOIFF_HEVCDecoderConfigRecord decoderConfig, MP4Handle bitstreamH)
{
    u32                     i;
    u8                      *buffer;
    MP4Err                  err;
    MP4Handle               naluH;
    MP4Handle               imgDatH;
    MP4Handle               startCodePrefixH;
    MP4Handle               zeroByteH;
    MP4LinkedList           vpsNALUnits;
    MP4LinkedList           spsNALUnits;
    MP4LinkedList           ppsNALUnits;
    MP4LinkedList           prefixSEI_NALUnits;
    MP4LinkedList           suffixSEI_NALUnits;
    MP4LinkedList           imageNALUnits;
    ISOIFF_HEVCItemData     hevcItemData;
    
    logMsg(LOGLEVEL_INFO, "Creating HEVC bitstream from image.");
    
    err = MP4MakeLinkedList(&vpsNALUnits);          if (err) goto bail;
    err = MP4MakeLinkedList(&spsNALUnits);          if (err) goto bail;
    err = MP4MakeLinkedList(&ppsNALUnits);          if (err) goto bail;
    err = MP4MakeLinkedList(&imageNALUnits);        if (err) goto bail;
    err = MP4MakeLinkedList(&prefixSEI_NALUnits);   if (err) goto bail;
    err = MP4MakeLinkedList(&suffixSEI_NALUnits);   if (err) goto bail;
    
    err = MP4NewHandle(3, &startCodePrefixH);   if (err) goto bail;
    buffer = (u8 *) *startCodePrefixH;
    buffer[0] = 0;
    buffer[1] = 0;
    buffer[2] = 1;
    
    err = MP4NewHandle(1, &zeroByteH);   if (err) goto bail;
    buffer = (u8 *) *zeroByteH;
    buffer[0] = 0;
    
    err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_VPS, vpsNALUnits); if (err) goto bail;
    err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_SPS, spsNALUnits); if (err) goto bail;
    err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_PPS, ppsNALUnits); if (err) goto bail;
    err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_PREFIX_SEI, prefixSEI_NALUnits); if (err) goto bail;
    err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_SUFFIX_SEI, suffixSEI_NALUnits); if (err) goto bail;
    
    if (vpsNALUnits->entryCount > 0)
    {
        err = MP4GetListEntry(vpsNALUnits, 0, (char **) &naluH);    if (err) goto bail;
        err = MP4HandleCat(bitstreamH, zeroByteH);                  if (err) goto bail;
        err = MP4HandleCat(bitstreamH, startCodePrefixH);           if (err) goto bail;
        err = MP4HandleCat(bitstreamH, naluH);                      if (err) goto bail;
    }
    
    if (spsNALUnits->entryCount > 0)
    {
        err = MP4GetListEntry(spsNALUnits, 0, (char **) &naluH);    if (err) goto bail;
        err = MP4HandleCat(bitstreamH, zeroByteH);                  if (err) goto bail;
        err = MP4HandleCat(bitstreamH, startCodePrefixH);           if (err) goto bail;
        err = MP4HandleCat(bitstreamH, naluH);                      if (err) goto bail;
    }
    
    if (ppsNALUnits->entryCount > 0)
    {
        err = MP4GetListEntry(ppsNALUnits, 0, (char **) &naluH);    if (err) goto bail;
        err = MP4HandleCat(bitstreamH, zeroByteH);                  if (err) goto bail;
        err = MP4HandleCat(bitstreamH, startCodePrefixH);           if (err) goto bail;
        err = MP4HandleCat(bitstreamH, naluH);                      if (err) goto bail;
    }
    
    for (i = 0; i < prefixSEI_NALUnits->entryCount; i++)
    {
        err = MP4GetListEntry(prefixSEI_NALUnits, i, (char **) &naluH);  if (err) goto bail;
        err = MP4HandleCat(bitstreamH, startCodePrefixH);                if (err) goto bail;
        err = MP4HandleCat(bitstreamH, naluH);                           if (err) goto bail;
    }
    
    err = MP4NewHandle(0, &imgDatH);            if (err) goto bail;
    err = ISOIFF_GetImageData(image, imgDatH);  if (err) goto bail;
    
    err = ISOIFF_CreateHEVCItemDataFromHandle(imgDatH, &hevcItemData, decoderConfig->lengthSizeMinusOne);   if (err) goto bail;
    err = ISOIFF_GetNALUDataHandlesFromHEVCItemData(hevcItemData, imageNALUnits);                           if (err) goto bail;
    
    for (i = 0; i < imageNALUnits->entryCount; i++)
    {
        err = MP4GetListEntry(imageNALUnits, i, (char **) &naluH);  if (err) goto bail;
        err = MP4HandleCat(bitstreamH, startCodePrefixH);           if (err) goto bail;
        err = MP4HandleCat(bitstreamH, naluH);                      if (err) goto bail;
    }
    
    for (i = 0; i < suffixSEI_NALUnits->entryCount; i++)
    {
        err = MP4GetListEntry(suffixSEI_NALUnits, i, (char **) &naluH);  if (err) goto bail;
        err = MP4HandleCat(bitstreamH, startCodePrefixH);                if (err) goto bail;
        err = MP4HandleCat(bitstreamH, naluH);                           if (err) goto bail;
    }
    
    err = MP4DisposeHandle(startCodePrefixH);           if (err) goto bail;
    err = MP4DisposeHandle(imgDatH);                    if (err) goto bail;
    err = MP4DisposeHandle(zeroByteH);                  if (err) goto bail;
    
    err = MP4DeleteLinkedList(vpsNALUnits);             if (err) goto bail;
    err = MP4DeleteLinkedList(spsNALUnits);             if (err) goto bail;
    err = MP4DeleteLinkedList(ppsNALUnits);             if (err) goto bail;
    err = MP4DeleteLinkedList(imageNALUnits);           if (err) goto bail;
    err = MP4DeleteLinkedList(prefixSEI_NALUnits);      if (err) goto bail;
    err = MP4DeleteLinkedList(suffixSEI_NALUnits);      if (err) goto bail;
    
    err = ISOIFF_FreeHEVCItemData(hevcItemData);    if (err) goto bail;
bail:
    return err;
}