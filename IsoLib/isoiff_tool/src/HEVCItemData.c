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

#include "isoiff_hevc.h"
#include "Logger.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

typedef struct ISOIFF_HEVCItemDataNalUnitS
{
    u32         nalUnitLength;
    MP4Handle   nalUnitH;
    
} *ISOIFF_HEVCItemDataNalUnit;

struct ISOIFF_HEVCItemDataS
{
    u8              lengthInSizeMinusOne;
    MP4LinkedList   nalUnits;
};

MP4Err          ISOIFF_CreateHEVCItemData                  (ISOIFF_HEVCItemData *hevcItemData)
{
    MP4Err  err;
    
    logMsg(LOGLEVEL_DEBUG, "Creating HEVC item data");
    
    *hevcItemData   = calloc(1, sizeof(struct ISOIFF_HEVCItemDataS));
    err             = MP4MakeLinkedList(&(*hevcItemData)->nalUnits);    if (err) goto bail;
    
    (*hevcItemData)->lengthInSizeMinusOne = 0;
bail:
    return err;
}

MP4Err          ISOIFF_AddNALUnitToHEVCItemData            (ISOIFF_HEVCItemData hevcItemData, MP4Handle nalUnitData)
{
    MP4Err                      err;
    ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
    
    logMsg(LOGLEVEL_DEBUG, "Adding HEVC NAL unit to item data");
    
    hevcNalUnit     = calloc(1, sizeof(struct ISOIFF_HEVCItemDataNalUnitS));
    
    
    err             = MP4NewHandle(0, &hevcNalUnit->nalUnitH);                      if (err) goto bail;
    err             = MP4GetHandleSize(nalUnitData, &hevcNalUnit->nalUnitLength);   if (err) goto bail;
    err             = MP4HandleCat(hevcNalUnit->nalUnitH, nalUnitData);             if (err) goto bail;
    err             = MP4AddListEntry(hevcNalUnit, hevcItemData->nalUnits);         if (err) goto bail;
    
    logMsg(LOGLEVEL_TRACE, "HEVC NAL unit size: %d", hevcNalUnit->nalUnitLength);
    
    if (hevcItemData->lengthInSizeMinusOne == 0)
        if ((hevcNalUnit->nalUnitLength >> 8) > 0)
            hevcItemData->lengthInSizeMinusOne = 1;
    
    if (hevcItemData->lengthInSizeMinusOne == 1)
        if ((hevcNalUnit->nalUnitLength >> 16) > 0)
            hevcItemData->lengthInSizeMinusOne = 2;
    
    if (hevcItemData->lengthInSizeMinusOne == 2)
        if ((hevcNalUnit->nalUnitLength >> 24) > 0)
            hevcItemData->lengthInSizeMinusOne = 3;

bail:
    return err;
}

MP4Err          ISOIFF_GetHEVCItemDataSize                 (ISOIFF_HEVCItemData hevcItemData, u32 *outSize)
{
    MP4Err  err;
    u32     size;
    u32     i;
    
    err                 = MP4NoErr;
    size                = 0;
    
    for (i = 0; i < hevcItemData->nalUnits->entryCount; i++)
    {
        ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
        err = MP4GetListEntry(hevcItemData->nalUnits, i, (char **) &hevcNalUnit); if (err) goto bail;
        size += hevcItemData->lengthInSizeMinusOne + 1;
        size += hevcNalUnit->nalUnitLength;
    }
    
    logMsg(LOGLEVEL_TRACE, "Requested HEVC item data size: %d", size);
    
    *outSize = size;
bail:
    return err;
}

MP4Err          ISOIFF_PutHEVCItemDataIntoHandle           (ISOIFF_HEVCItemData hevcItemData, MP4Handle itemDataHandle)
{
    MP4Err  err;
    u32     i;
    u32     size;
    u32     bytePosition;
    u8      *buffer;
    
    logMsg(LOGLEVEL_TRACE, "Putting HEVC item data into handle");
    
    bytePosition    = 0;
    err             = ISOIFF_GetHEVCItemDataSize(hevcItemData, &size);      if (err) goto bail;
    err             = MP4SetHandleSize(itemDataHandle, size);               if (err) goto bail;
    
    buffer = (u8*) *itemDataHandle;
    
    for (i = 0; i < hevcItemData->nalUnits->entryCount; i++)
    {
        ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
        err = MP4GetListEntry(hevcItemData->nalUnits, i, (char **) &hevcNalUnit); if (err) goto bail;

        if (hevcItemData->lengthInSizeMinusOne == 0)
        {
            buffer[bytePosition] = (u8) hevcNalUnit->nalUnitLength;
        }
        if (hevcItemData->lengthInSizeMinusOne == 1)
        {
            buffer[bytePosition]        = (u8) ((hevcNalUnit->nalUnitLength  >> 8) & 0xff);
            buffer[bytePosition + 1]    = (u8) (hevcNalUnit->nalUnitLength & 0xff);
        }
        
        if (hevcItemData->lengthInSizeMinusOne == 2)
        {
            buffer[bytePosition]        = (u8) ((hevcNalUnit->nalUnitLength  >> 16) & 0xff);
            buffer[bytePosition + 1]    = (u8) ((hevcNalUnit->nalUnitLength  >> 8) & 0xff);
            buffer[bytePosition + 2]    = (u8) (hevcNalUnit->nalUnitLength & 0xff);
        }
        if (hevcItemData->lengthInSizeMinusOne == 3)
        {
            buffer[bytePosition]        = (u8) ((hevcNalUnit->nalUnitLength  >> 24) & 0xff);
            buffer[bytePosition + 1]    = (u8) ((hevcNalUnit->nalUnitLength  >> 16) & 0xff);
            buffer[bytePosition + 2]    = (u8) ((hevcNalUnit->nalUnitLength  >> 8) & 0xff);
            buffer[bytePosition + 3]    = (u8) (hevcNalUnit->nalUnitLength & 0xff);
        }
        
        bytePosition += hevcItemData->lengthInSizeMinusOne + 1;
        
        memcpy(&buffer[bytePosition], (char *) *hevcNalUnit->nalUnitH, hevcNalUnit->nalUnitLength);
        
        bytePosition += hevcNalUnit->nalUnitLength;
    }
    
bail:
    return err;
}

MP4Err          ISOIFF_GetHEVCItemDataLengthSizeMinusOne   (ISOIFF_HEVCItemData hevcItemData, u8 *lengthSizeMinusOne)
{
    MP4Err  err;
    
    err                 = MP4NoErr;
    *lengthSizeMinusOne = hevcItemData->lengthInSizeMinusOne;
bail:
    return err;
}

MP4Err          ISOIFF_CreateHEVCItemDataFromHandle        (MP4Handle itemDataHandle, ISOIFF_HEVCItemData *outHevcItemData, u8 lengthSizeMinusOne)
{
    MP4Err  err;
    u8      lengthSize;
    u32     bufferSize;
    u32     bufferPosition;
    u8      *buffer;
    
    lengthSize          = lengthSizeMinusOne + 1;
    bufferPosition      = 0;
    buffer              = (u8 *) *itemDataHandle;
    err                 = MP4GetHandleSize(itemDataHandle, &bufferSize); if (err) goto bail;
    err                 = ISOIFF_CreateHEVCItemData(outHevcItemData); if (err) goto bail;
    
    (*outHevcItemData)->lengthInSizeMinusOne = lengthSizeMinusOne;
    
    while (bufferPosition < bufferSize)
    {
        ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
        
        hevcNalUnit     = calloc(1, sizeof(struct ISOIFF_HEVCItemDataNalUnitS));
        
        if (lengthSize == 1)
        {
            u8 b1;
            b1 = (u8) buffer[bufferPosition];
            hevcNalUnit->nalUnitLength = b1;
        }
        if (lengthSize == 2)
        {
            u8 b1, b2;
            b1 = (u8) buffer[bufferPosition];
            b2 = (u8) buffer[bufferPosition + 1];
            hevcNalUnit->nalUnitLength = (u16) ((b1 << 8) + b2);
        }
        if (lengthSize == 3)
        {
            u8 b1, b2, b3;
            b1 = (u8) buffer[bufferPosition];
            b2 = (u8) buffer[bufferPosition + 1];
            b3 = (u8) buffer[bufferPosition + 2];
            hevcNalUnit->nalUnitLength = (u32) ((b1 << 16) + (b2 << 8) + b3);
        }
        if (lengthSize == 4)
        {
            u8 b1, b2, b3, b4;
            b1 = (u8) buffer[bufferPosition];
            b2 = (u8) buffer[bufferPosition + 1];
            b3 = (u8) buffer[bufferPosition + 2];
            b4 = (u8) buffer[bufferPosition + 3];
            hevcNalUnit->nalUnitLength = (u32) ((b1 << 24) + (b2 << 16) + (b3 << 8) + b4);
        }
        bufferPosition += lengthSize;
        
        err             = MP4NewHandle(hevcNalUnit->nalUnitLength, &hevcNalUnit->nalUnitH); if (err) goto bail;
        
        memcpy(*hevcNalUnit->nalUnitH, &buffer[bufferPosition], hevcNalUnit->nalUnitLength);
        
        bufferPosition += hevcNalUnit->nalUnitLength;
        err             = MP4AddListEntry(hevcNalUnit, (*outHevcItemData)->nalUnits);       if (err) goto bail;
    }
bail:
    return err;
}

MP4Err          ISOIFF_GetNALUDataHandlesFromHEVCItemData  (ISOIFF_HEVCItemData hevcItemData, MP4LinkedList naluDataHandles)
{
    MP4Err  err;
    u32     i;
    
    err     = MP4NoErr;
    
    for (i = 0; i < hevcItemData->nalUnits->entryCount; i++)
    {
        ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
        err = MP4GetListEntry(hevcItemData->nalUnits, i, (char **) &hevcNalUnit); if (err) goto bail;
        err = MP4AddListEntry(hevcNalUnit->nalUnitH, naluDataHandles); if (err) goto bail;
    }
    
bail:
    return err;
}

MP4Err          ISOIFF_FreeHEVCItemData                    (ISOIFF_HEVCItemData hevcItemData)
{
    MP4Err  err;
    u32     i;
    
    err     = MP4NoErr;
    
    for (i = 0; i < hevcItemData->nalUnits->entryCount; i++)
    {
        ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
        err = MP4GetListEntry(hevcItemData->nalUnits, i, (char **) &hevcNalUnit); if (err) goto bail;
        if (hevcNalUnit->nalUnitH != NULL)
        {
            err = MP4DisposeHandle(hevcNalUnit->nalUnitH);  if (err) goto bail;
        }
        free (hevcNalUnit);
    }
    err = MP4DeleteLinkedList(hevcItemData->nalUnits);  if (err) goto bail;
    
    free (hevcItemData);
bail:
    return err;
}