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
 @header isoiff_hevc
 Provides structs for HEVC Images in ISO Image File Format
 @copyright Apple
 @updated 2014-11-21
 @author Armin Trattnig
 @version 1.0
 */

#ifndef isoiff_hevc_h
#define isoiff_hevc_h

#include "MP4Movies.h"
#include "MP4Atoms.h"

enum
{
    ISOIFF_4CC_hevc  = MP4_FOUR_CHAR_CODE('h', 'e', 'v', 'c'),
	ISOIFF_4CC_heic  = MP4_FOUR_CHAR_CODE('h', 'e', 'i', 'c'),
    ISOIFF_4CC_hvc1  = MP4_FOUR_CHAR_CODE('h', 'v', 'c', '1'),
    ISOIFF_4CC_hvcC  = MP4_FOUR_CHAR_CODE('h', 'v', 'c', 'C'),
    ISOIFF_4CC_init  = MP4_FOUR_CHAR_CODE('i', 'n', 'i', 't'),
	ISOIFF_4CC_ispe  = MP4_FOUR_CHAR_CODE('i', 's', 'p', 'e')
};

/*!
 * @typedef ISOIFF_HEVCItemData
 * @brief Struct that holds HEVC NAL units for usage as an ISOIFF_Image's data
 */
typedef struct  ISOIFF_HEVCItemDataS                       *ISOIFF_HEVCItemData;

/*!
 * @typedef ISOIFF_HEVCDecoderConfigRecord
 * @brief HEVC decoder configuration record
 */
typedef struct  ISOIFF_HEVCDecoderConfigRecordS
{
    u8              configurationVersion;
    u8              general_profile_space;
    u8              general_tier_flag;
    u8              general_profile_idc;
    u32             general_profile_compatibility_flags;
    u64             general_constraint_indicator_flags;
    u8              general_level_idc;
    u16             min_spatial_segmentation_idc;
    u8              parallelismType;
    u8              chromaFormat;
    u8              bitDepthLumaMinus8;
    u8              bitDepthChromaMinus8;
    u16             avgFrameRate;
    u8              constantFrameRate;
    u8              numTemporalLayers;
    u8              temporalIdNested;
    u8              lengthSizeMinusOne;
    u8              numOfArrays;
    MP4LinkedList   arrays;
    
}   *ISOIFF_HEVCDecoderConfigRecord;

/*!
* @typedef ISOIFF_HEVCConfigurationAtom
* @brief HEVCConfigurationBox
*/
typedef struct ISOIFF_HEVCConfigurationAtom
{
	MP4_BASE_ATOM

	ISOIFF_HEVCDecoderConfigRecord	configRecord;
	MP4Handle						configRecordHandle;
} ISOIFF_HEVCConfigurationAtom, *ISOIFF_HEVCConfigurationAtomPtr;

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
* @discussion Creates a HEVCConfigurationAtom (Allocates memory and initializes fields)
* @param outAtom Pointer that will hold a reference to the created HEVCConfigurationAtom
* @param configRecord HEVCDecoderConfigRecord  that will be put into the atom
*/
MP4Err ISOIFF_CreateHEVCConfigurationAtom					(ISOIFF_HEVCConfigurationAtomPtr *outAtom, ISOIFF_HEVCDecoderConfigRecord configRecord);

/*!
* @discussion Creates a ImageSpatialExtentsPropertyAtom (Allocates memory and initializes fields)
* @param outAtom Pointer that will hold a reference to the created ImageSpatialExtentsPropertyAtom
*/
MP4Err ISOIFF_CreateImageSpatialExtentsPropertyAtom			(ISOIFF_ImageSpatialExtentsPropertyAtomPtr *outAtom);


/*!
 * @discussion Creates a HEVC item data (Allocates memory and initializes fields)
 * @param hevcItemData This ISOIFF_HEVCItemData will be allocated and initialized
 */
MP4Err          ISOIFF_CreateHEVCItemData                  (ISOIFF_HEVCItemData *hevcItemData);

/*!
 * @discussion Adds a NAL unit to a HEVC item data. (This will update the lengthSizeMinusOne field)
 * @param hevcItemData The HEVC item data, the NAL unit will be added to
 * @param nalUnitData A MP4Handle that contents will be copied and added
 */
MP4Err          ISOIFF_AddNALUnitToHEVCItemData            (ISOIFF_HEVCItemData hevcItemData, MP4Handle nalUnitData);

/*!
 * @discussion Gets the size of an HEVC item data.
 * @param hevcItemData The HEVC item data
 * @param outSize Will contain the size, must be allocated before calling the function
 */
MP4Err          ISOIFF_GetHEVCItemDataSize                 (ISOIFF_HEVCItemData hevcItemData, u32 *outSize);

/*!
 * @discussion Puts the HEVC item data into a MP4Handle
 * @param hevcItemData The HEVC item data
 * @param itemDataHandle Will contain the HEVC item data in bytestream form (Must be created with MP4NewHandle before calling this function)
 */
MP4Err          ISOIFF_PutHEVCItemDataIntoHandle           (ISOIFF_HEVCItemData hevcItemData, MP4Handle itemDataHandle);

/*!
 * @discussion Creates a HEVC item data from a MP4Handle (This is the counterpart to ISOIFF_PutHEVCItemDataIntoHandle)
 * @param itemDataHandle MP4Handle that contains the HEVC imgaes' item data
 * @param outHevcItemData Will be allocated and filled with contents parsed from the itemDataHandle
 * @param lengthSizeMinusOne This is needed to determine the size of the lenght field. (Can be found in the HEVC decoder configuration record)
 */
MP4Err          ISOIFF_CreateHEVCItemDataFromHandle        (MP4Handle itemDataHandle, ISOIFF_HEVCItemData *outHevcItemData, u8 lengthSizeMinusOne);

/*!
 * @discussion Collects all HEVC NAL units from a HEVC item data
 * @param hevcItemData The HEVC item data, that contains the NAL units
 * @param naluDataHandles This is a MP4LinkedList containing MP4Handles of the HEVC NAL units
 */
MP4Err          ISOIFF_GetNALUDataHandlesFromHEVCItemData  (ISOIFF_HEVCItemData hevcItemData, MP4LinkedList naluDataHandles);

/*!
 * @discussion Gets the lengthSizeMinusOne (This is used for setting the  HEVC decoder configuration record's field to the correct value)
 * @param hevcItemData The HEVC item data, that contains the lengthSizeMinusOne field
 * @param lengthSizeMinusOne Will contain the value
 */
MP4Err          ISOIFF_GetHEVCItemDataLengthSizeMinusOne   (ISOIFF_HEVCItemData hevcItemData, u8 *lengthSizeMinusOne);

/*!
 * @discussion Creates a HEVC decoder configuration record (Allocates memory and initializes fields)
 * @param hevcDecConfRec This ISOIFF_HEVCDecoderConfigRecord will be allocated and initialized
 */
MP4Err          ISOIFF_CreateHEVCDecoderConfigRecord       (ISOIFF_HEVCDecoderConfigRecord *hevcDecConfRec);

/*!
 * @discussion Adds a NAL unit to a HEVC decoder configuration record (VPS, SPS, PPS or SEI NAL unit)
 * @param hevcDecConfRec The HEVC decoder configuration record, the NAL unit will be added to
 * @param type The type of the NAL Unit (VPS, SPS, PPS or SEI)
 * @param nalUnitData A MP4Handle that contents will be copied and added
 */
MP4Err          ISOIFF_AddNALUnitToHEVCDecConfRec          (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, u8 type, MP4Handle nalUnitData);

/*!
 * @discussion Gets the size of an HEVC decoder configuration record.
 * @param hevcDecConfRec The HEVC decoder configuration record
 * @param outSize Will contain the size, must be allocated before calling the function
 */
MP4Err          ISOIFF_GetHEVCDecConfRecordSize            (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, u32 *outSize);

/*!
 * @discussion Puts the HEVC decoder configuration record into a MP4Handle
 * @param hevcDecConfRec The HEVC item data
 * @param recordDataHandle Will contain the HEVC decoder configuration record in bytestream form (Must be created with MP4NewHandle before calling this function)
 */
MP4Err          ISOIFF_PutHEVCDecConfRecordIntoHandle      (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, MP4Handle recordDataHandle);

/*!
 * @discussion Creates a HEVC decoder configuration record from a MP4Handle (This is the counterpart to ISOIFF_PutHEVCItemDataIntoHandle)
 * @param recordDataHandle MP4Handle that contains the HEVC decoder configuration record's item data
 * @param outRecord Will be allocated and filled with contents parsed from the recordDataHandle
 */
MP4Err          ISOIFF_CreateHEVCDecConfRecFromHandle      (MP4Handle recordDataHandle, ISOIFF_HEVCDecoderConfigRecord *outRecord);

/*!
 * @discussion Collects all HEVC NAL units of a certain type from a HEVC decoder configuration record (VPS, SPS, PPS or SEI NAL unit)
 * @param hevcDecConfRec The HEVC decoder configuration record, that contains the NAL units
 * @param type This is a MP4LinkedList containing MP4Handles of the HEVC NAL units
 * @param nalUnits This is a MP4LinkedList containing MP4Handles of the HEVC NAL units
 */
MP4Err          ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf  (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec, u8 type, MP4LinkedList nalUnits);

/*!
 * @discussion Deallocates the memory used by an HEVC item data
 * @param hevcItemData HEVC item data that will be deallocated
 */
MP4Err          ISOIFF_FreeHEVCItemData                    (ISOIFF_HEVCItemData hevcItemData);

/*!
 * @discussion Deallocates the memory used by HEVC decoder configuration record
 * @param hevcDecConfRec HEVC decoder configuration record that will be deallocated
 */
MP4Err          ISOIFF_FreeHEVCDecoderConfigRecord         (ISOIFF_HEVCDecoderConfigRecord hevcDecConfRec);

/*!
* @discussion Parses a HEVCDecoderConfigRecord from a property atom
* @param property Property atom that is of type hvcC
* @param decoderConfig HEVC decoder configuration record, which will be parsed
*/
MP4Err			ISOIFF_GetHEVCDecoderConfigRecordFromProperty(MP4AtomPtr property, ISOIFF_HEVCDecoderConfigRecord *decoderConfig);

/*!
* @discussion Parses an ImageSpatialExtentsProperty from a property atom and prints width and height
* @param property Property atom that is of type ispe
*/
MP4Err			ISOIFF_ParseImageSpatialExtends				(MP4AtomPtr property);

#endif