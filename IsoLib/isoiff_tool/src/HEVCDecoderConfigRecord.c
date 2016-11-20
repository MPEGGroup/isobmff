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

typedef struct ISOIFF_HEVCDecoderConfigurationRecordArrayS
{
    u8              array_completeness;
    u8              NAL_unit_type;
    u16             numNalus;
    MP4LinkedList   nalUnits;
    
} *ISOIFF_HEVCDecoderConfigurationRecordArray;

MP4Err          ISOIFF_CreateHEVCDecoderConfigRecord       (ISOIFF_HEVCDecoderConfigRecord *hevcDecConfRec)
{
    MP4Err  err;
    
    logMsg(LOGLEVEL_DEBUG, "Creating HEVC Decoder Configuration Record");
    
    *hevcDecConfRec     = calloc(1, sizeof(struct ISOIFF_HEVCDecoderConfigRecordS));
    
    (*hevcDecConfRec)->configurationVersion                     = 1;
    (*hevcDecConfRec)->general_profile_space                    = 0;
    (*hevcDecConfRec)->general_tier_flag                        = 0;
    (*hevcDecConfRec)->general_profile_idc                      = 3;
    (*hevcDecConfRec)->general_profile_compatibility_flags      = 0;
    (*hevcDecConfRec)->general_constraint_indicator_flags       = 0;
    (*hevcDecConfRec)->general_level_idc                        = 0;
    (*hevcDecConfRec)->min_spatial_segmentation_idc             = 0;
    (*hevcDecConfRec)->parallelismType                          = 0;
    (*hevcDecConfRec)->chromaFormat                             = 0;
    (*hevcDecConfRec)->bitDepthLumaMinus8                       = 0;
    (*hevcDecConfRec)->bitDepthChromaMinus8                     = 0;
    (*hevcDecConfRec)->avgFrameRate                             = 0;
    (*hevcDecConfRec)->constantFrameRate                        = 0;
    (*hevcDecConfRec)->numTemporalLayers                        = 0;
    (*hevcDecConfRec)->temporalIdNested                         = 0;
    (*hevcDecConfRec)->lengthSizeMinusOne                       = 0;
    (*hevcDecConfRec)->numOfArrays                              = 0;
    
    err                 = MP4MakeLinkedList(&(*hevcDecConfRec)->arrays);    if (err) goto bail;
bail:
    return err;
}

MP4Err          ISOIFF_AddNALUnitToHEVCDecConfRec          (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, u8 type, MP4Handle nalUnitData)
{
    MP4Err                                          err;
    u32                                             i;
    ISOIFF_HEVCDecoderConfigurationRecordArray      array;
    ISOIFF_HEVCItemDataNalUnit                      hevcNalUnit;
    
    logMsg(LOGLEVEL_DEBUG, "Adding NAL unit with type %d to HEVC decoder configuration record", type);
    array               = NULL;
    
    for (i = 0; i < hevcDecConfRec->arrays->entryCount; i++)
    {
        ISOIFF_HEVCDecoderConfigurationRecordArray  a;
        err     = MP4GetListEntry(hevcDecConfRec->arrays, i, (char **) &a); if (err) goto bail;
        
        if (a->NAL_unit_type == type)
        {
            array = a;
            break;
        }
    }
    
    if (array == NULL)
    {
        array   = calloc(1, sizeof(struct ISOIFF_HEVCDecoderConfigurationRecordArrayS));
        
        logMsg(LOGLEVEL_TRACE, "Creating new array of NAL units of type %d", type);
        
        array->array_completeness = 1; /* For the still image case, this is always 1 */
        array->numNalus           = 0;
        array->NAL_unit_type      = type;
        
        err     = MP4MakeLinkedList(&array->nalUnits); if (err) goto bail;
        err     = MP4AddListEntry(array, hevcDecConfRec->arrays); if (err) goto bail;
        hevcDecConfRec->numOfArrays++;
        logMsg(LOGLEVEL_TRACE, "Number of arrays: %d", hevcDecConfRec->numOfArrays);
    }
    
    array->numNalus++;
    hevcNalUnit     = calloc(1, sizeof(struct ISOIFF_HEVCItemDataNalUnitS));
    
    logMsg(LOGLEVEL_TRACE, "Adding NAL unit to array. Number of NAL units in array now: %d", array->numNalus);
    
    err             = MP4NewHandle(0, &hevcNalUnit->nalUnitH);                      if (err) goto bail;
    err             = MP4GetHandleSize(nalUnitData, &hevcNalUnit->nalUnitLength);   if (err) goto bail;
    err             = MP4HandleCat(hevcNalUnit->nalUnitH, nalUnitData);             if (err) goto bail;
    err             = MP4AddListEntry(hevcNalUnit, array->nalUnits);                if (err) goto bail;
    
    logMsg(LOGLEVEL_TRACE, "HEVC NAL unit size: %d", hevcNalUnit->nalUnitLength);
    
bail:
    return err;
}

MP4Err          ISOIFF_GetHEVCDecConfRecordSize            (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, u32 *outSize)
{
    MP4Err  err;
    u32     size;
    u32     i;
    u32     j;
    
    err = MP4NoErr;
    
    size = 23;
    
    for (i = 0; i < hevcDecConfRec->arrays->entryCount; i++)
    {
        ISOIFF_HEVCDecoderConfigurationRecordArray  array;
        err     = MP4GetListEntry(hevcDecConfRec->arrays, i, (char **) &array); if (err) goto bail;
        
        size += 3;
        
        for (j = 0; j < array->nalUnits->entryCount; j++)
        {
            ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
            err     = MP4GetListEntry(array->nalUnits, j, (char **) &hevcNalUnit); if (err) goto bail;
            
            size += 2 + hevcNalUnit->nalUnitLength;
        }
    }
    
    *outSize = size;
bail:
    return err;
}

MP4Err          ISOIFF_PutHEVCDecConfRecordIntoHandle      (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, MP4Handle recordDataHandle)
{
    MP4Err  err;
    u32     size;
    u8      *buffer;
    u8      tmp8;
    u16     tmp16;
    u64     tmp64;
    u32     i;
    u32     j;

    err     = ISOIFF_GetHEVCDecConfRecordSize(hevcDecConfRec, &size);   if (err) goto bail;
    err     = MP4SetHandleSize(recordDataHandle, size);                 if (err) goto bail;
    tmp8    = 0;
    tmp16   = 0;
    tmp64   = 0;
    
    
    buffer  = (u8 *) *recordDataHandle;
    
    memcpy(buffer, &hevcDecConfRec->configurationVersion, 1);
    buffer += 1;
    
    tmp8 =  hevcDecConfRec->general_profile_space << 6;
    tmp8 += hevcDecConfRec->general_tier_flag << 5;
    tmp8 += hevcDecConfRec->general_profile_idc;
    memcpy(buffer, &tmp8, 1);
    buffer += 1;
    
	*(u8*)buffer = (u8)((hevcDecConfRec->general_profile_compatibility_flags >> 24) & 0xff);
	buffer += 1; 
	*(u8*)buffer = (u8)((hevcDecConfRec->general_profile_compatibility_flags >> 16) & 0xff);
	buffer += 1; 
	*(u8*)buffer = (u8)((hevcDecConfRec->general_profile_compatibility_flags >> 8) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->general_profile_compatibility_flags) & 0xff);
	buffer += 1;

	*(u8*)buffer = (u8)((hevcDecConfRec->general_constraint_indicator_flags >> 40) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->general_constraint_indicator_flags >> 32) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->general_constraint_indicator_flags >> 24) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->general_constraint_indicator_flags >> 16) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->general_constraint_indicator_flags >> 8) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->general_constraint_indicator_flags) & 0xff);
	buffer += 1;
    
    memcpy(buffer, &hevcDecConfRec->general_level_idc, 1);
    buffer += 1;
    
	*(u8*)buffer = (u8)((hevcDecConfRec->min_spatial_segmentation_idc >> 8) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->min_spatial_segmentation_idc) & 0xff);
	buffer += 1;
    
    tmp8 = 0xFC | hevcDecConfRec->parallelismType;
    memcpy(buffer, &tmp8, 1);
    buffer += 1;
    
    tmp8 = 0xFC | hevcDecConfRec->chromaFormat;
    memcpy(buffer, &tmp8, 1);
    buffer += 1;
    
    tmp8 = 0xF8 | hevcDecConfRec->bitDepthChromaMinus8;
    memcpy(buffer, &tmp8, 1);
    buffer += 1;
    
    tmp8 = 0xF8 | hevcDecConfRec->bitDepthLumaMinus8;
    memcpy(buffer, &tmp8, 1);
    buffer += 1;
    
	*(u8*)buffer = (u8)((hevcDecConfRec->avgFrameRate >> 8) & 0xff);
	buffer += 1;
	*(u8*)buffer = (u8)((hevcDecConfRec->avgFrameRate) & 0xff);
	buffer += 1;
    
    tmp8 =  hevcDecConfRec->constantFrameRate << 6;
    tmp8 += hevcDecConfRec->numTemporalLayers << 3;
    tmp8 += hevcDecConfRec->temporalIdNested << 2;
    tmp8 += hevcDecConfRec->lengthSizeMinusOne;
    memcpy(buffer, &tmp8, 1);
    buffer += 1;
    
    memcpy(buffer, &hevcDecConfRec->numOfArrays, 1);
    buffer += 1;
    
    for (i = 0; i < hevcDecConfRec->arrays->entryCount; i++)
    {
        ISOIFF_HEVCDecoderConfigurationRecordArray  array;
        err     = MP4GetListEntry(hevcDecConfRec->arrays, i, (char **) &array); if (err) goto bail;
        
        tmp8 =  array->array_completeness << 7;
        tmp8 += array->NAL_unit_type;
        memcpy(buffer, &tmp8, 1);
        buffer += 1;
        
		*(u8*)buffer = (u8)((array->numNalus >> 8) & 0xff);
		buffer += 1;
		*(u8*)buffer = (u8)((array->numNalus) & 0xff);
		buffer += 1;
        
        for (j = 0; j < array->nalUnits->entryCount; j++)
        {
            ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
            err     = MP4GetListEntry(array->nalUnits, j, (char **) &hevcNalUnit); if (err) goto bail;
            
			*(u8*)buffer = (u8)((hevcNalUnit->nalUnitLength >> 8) & 0xff);
			buffer += 1;
			*(u8*)buffer = (u8)((hevcNalUnit->nalUnitLength) & 0xff);
			buffer += 1;
            
            memcpy(buffer, (char *) *hevcNalUnit->nalUnitH, hevcNalUnit->nalUnitLength);
            buffer += hevcNalUnit->nalUnitLength;
        }
    }
    
bail:
    return err;
}

MP4Err          ISOIFF_CreateHEVCDecConfRecFromHandle(MP4Handle recordDataHandle, ISOIFF_HEVCDecoderConfigRecord *outRecord)
{
	MP4Err  err;
	u32     size;
	u8      *buffer;
	u8      tmp8;
	u16     tmp16;
	u64     tmp64;
	u32     i;
	u32     j;

	ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec;

	hevcDecConfRec = calloc(1, sizeof(struct ISOIFF_HEVCDecoderConfigRecordS));
	err = MP4GetHandleSize(recordDataHandle, &size);     if (err) goto bail;
	buffer = (u8 *)*recordDataHandle;
	tmp8 = 0;
	tmp16 = 0;
	tmp64 = 0;
	err = MP4MakeLinkedList(&hevcDecConfRec->arrays);    if (err) goto bail;

	if (size < 23)
		BAILWITHERROR(MP4BadDataErr);

	memcpy(&hevcDecConfRec->configurationVersion, buffer, 1);
	buffer += 1;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;

	hevcDecConfRec->general_profile_space = tmp8 >> 6;
	tmp8 = tmp8 & 0x3F;
	hevcDecConfRec->general_tier_flag = tmp8 >> 5;
	hevcDecConfRec->general_profile_idc = tmp8 & 0x1F;

	//memcpy(&hevcDecConfRec->general_profile_compatibility_flags, buffer, 4);
	//buffer += 4;
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_profile_compatibility_flags = (tmp8 << 24);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_profile_compatibility_flags |= (tmp8 << 16);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_profile_compatibility_flags |= (tmp8 << 8);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_profile_compatibility_flags |= tmp8;


	//memcpy(&tmp64, buffer, 6);
	//buffer += 6;
	//hevcDecConfRec->general_constraint_indicator_flags = tmp64 >> 16;
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_constraint_indicator_flags = tmp8 << 40;
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_constraint_indicator_flags |= (tmp8 << 32);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_constraint_indicator_flags |= (tmp8 << 24);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_constraint_indicator_flags |= (tmp8 << 16);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_constraint_indicator_flags |= (tmp8 << 8);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->general_constraint_indicator_flags |= tmp8;

	memcpy(&hevcDecConfRec->general_level_idc, buffer, 1);
	buffer += 1;

	//memcpy(&tmp16, buffer, 2);
	//buffer += 2;
	//hevcDecConfRec->min_spatial_segmentation_idc = tmp16 & 0x0FFF;
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->min_spatial_segmentation_idc |= (tmp8 << 8);
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->min_spatial_segmentation_idc |= tmp8;
	hevcDecConfRec->min_spatial_segmentation_idc &= 0x0FFF;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->parallelismType = tmp8 & 0x03;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->chromaFormat = tmp8 & 0x03;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->bitDepthChromaMinus8 = tmp8 & 0x07;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->bitDepthLumaMinus8 = tmp8 & 0x07;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->avgFrameRate = tmp8 << 8;
	memcpy(&tmp8, buffer, 1);
	buffer += 1;
	hevcDecConfRec->avgFrameRate |= tmp8;

	memcpy(&tmp8, buffer, 1);
	buffer += 1;

	hevcDecConfRec->constantFrameRate = tmp8 >> 6;
	tmp8 = tmp8 & 0x3F;
	hevcDecConfRec->numTemporalLayers = tmp8 >> 3;
	tmp8 = tmp8 & 0x07;
	hevcDecConfRec->temporalIdNested = tmp8 >> 2;
	hevcDecConfRec->lengthSizeMinusOne = tmp8 & 0x03;

	memcpy(&hevcDecConfRec->numOfArrays, buffer, 1);
	buffer += 1;

	size -= 23;

	for (i = 0; i < hevcDecConfRec->numOfArrays; i++)
	{
		ISOIFF_HEVCDecoderConfigurationRecordArray  array;

		array = calloc(1, sizeof(struct ISOIFF_HEVCDecoderConfigurationRecordArrayS));
		err = MP4MakeLinkedList(&array->nalUnits); if (err) goto bail;

		if (size < 3)
			BAILWITHERROR(MP4BadDataErr);

		memcpy(&tmp8, buffer, 1);
		buffer += 1;

		array->array_completeness = tmp8 >> 7;
		array->NAL_unit_type = tmp8 & 0x3F;

		memcpy(&tmp8, buffer, 1);
		buffer += 1;
		array->numNalus = tmp8 << 8;
		memcpy(&tmp8, buffer, 1);
		buffer += 1;
		array->numNalus |= tmp8;

		size -= 3;

		for (j = 0; j < array->numNalus; j++)
		{
			ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;

			if (size < 2)
				BAILWITHERROR(MP4BadDataErr);

			hevcNalUnit = calloc(1, sizeof(struct ISOIFF_HEVCItemDataNalUnitS));

			memcpy(&tmp8, buffer, 1);
			buffer += 1;
			hevcNalUnit->nalUnitLength = tmp8 << 8;
			memcpy(&tmp8, buffer, 1);
			buffer += 1;
			hevcNalUnit->nalUnitLength |= tmp8;

			size -= 2;

			if (size < hevcNalUnit->nalUnitLength)
				BAILWITHERROR(MP4BadDataErr);

			err = MP4NewHandle(hevcNalUnit->nalUnitLength, &hevcNalUnit->nalUnitH); if (err) goto bail;

			memcpy((char *)*hevcNalUnit->nalUnitH, buffer, hevcNalUnit->nalUnitLength);

			buffer += hevcNalUnit->nalUnitLength;
			size -= hevcNalUnit->nalUnitLength;

			err = MP4AddListEntry(hevcNalUnit, array->nalUnits); if (err) goto bail;
		}
		err = MP4AddListEntry(array, hevcDecConfRec->arrays); if (err) goto bail;
	}

	*outRecord = hevcDecConfRec;
bail:
	return err;
}

MP4Err          ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf  (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, u8 type, MP4LinkedList nalUnits)
{
    MP4Err  err;
    u32     i;
    u32     j;
    
    err = MP4NoErr;

    for (i = 0; i < hevcDecConfRec->numOfArrays; i++)
    {
        ISOIFF_HEVCDecoderConfigurationRecordArray  array;
        err     = MP4GetListEntry(hevcDecConfRec->arrays, i, (char **) &array); if (err) goto bail;
        
        if (array->NAL_unit_type == type)
        {
            for (j = 0; j < array->nalUnits->entryCount; j++)
            {
                ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
                err = MP4GetListEntry(array->nalUnits, j, (char **) &hevcNalUnit); if (err) goto bail;
                
                err = MP4AddListEntry(hevcNalUnit->nalUnitH, nalUnits); if (err) goto bail;
            }
            break;
        }
        
    }
    
bail:
    return err;
}

MP4Err          ISOIFF_FreeHEVCDecoderConfigRecord         (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec)
{
    MP4Err  err;
    u32     i;
    u32     j;
    
    err = MP4NoErr;
    
    for (i = 0; i < hevcDecConfRec->numOfArrays; i++)
    {
        ISOIFF_HEVCDecoderConfigurationRecordArray  array;
        err     = MP4GetListEntry(hevcDecConfRec->arrays, i, (char **) &array); if (err) goto bail;
        
        for (j = 0; j < array->nalUnits->entryCount; j++)
        {
            ISOIFF_HEVCItemDataNalUnit  hevcNalUnit;
            err = MP4GetListEntry(array->nalUnits, j, (char **) &hevcNalUnit); if (err) goto bail;
            
            err = MP4DisposeHandle(hevcNalUnit->nalUnitH); if (err) goto bail;
            free (hevcNalUnit);
        }
        
        err = MP4DeleteLinkedList(array->nalUnits); if (err) goto bail;
        free (array);
    }
    
    err = MP4DeleteLinkedList(hevcDecConfRec->arrays); if (err) goto bail;
    free (hevcDecConfRec);
bail:
    return err;
}
