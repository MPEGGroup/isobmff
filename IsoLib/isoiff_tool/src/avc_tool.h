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

/*!
 @header avc_tool
 Provides ISO Image File Format functions, that will work with AVC bitstream
 @copyright Apple
 @updated 2016-12-05
 @author Armin Trattnig
 @version 1.0
 */

#ifndef avc_tool_h
#define avc_tool_h

#include "TLibDecoder/TDecEntropy.h"

extern "C"
{
    #include "MP4Movies.h"
    #include "MP4Atoms.h"
    #include "Options.h"
    #include "isoiff.h"
	#include "isoiff_hevc.h"
}

/*!
 * @discussion Creates an ISOIFF_ImageCollection with handler type 'avc'
 * @param collection Pointer to an ISOIFF_ImageCollection
 */
MP4Err     createAVC_ImageCollection    (ISOIFF_ImageCollection *collection);

/*!
 * @discussion Will read bytes from the option's inputFile and will parse the NAL Units and creating contents
 of the ISOIFF_AVCDecoderConfigRecord and the ISOIFF_AVCItemData for a single AVC still image.
 * @param avcC ISOVCConfigAtomPtr must be created before calling this function
 * @param itemData ISOIFF_AVCItemData must be created before calling this function
 * @param options Pointer to Options for the input file path
 */
MP4Err     processAVC_NALUnits          (ISOVCConfigAtomPtr	avcC, ISOIFF_HEVCItemData itemData, Options  *options);

/*!
 * @discussion Will add an AVC image item to the provided collection. The decoder configuration record will be linked to the image.
 NOTE: Before calling this function, create the image collection and generate the record and itemData using processAVC_NALUnits
 * @param collection The image collection the provided AVC image will be added to
 * @param record The decoder configuration record (use processAVC_NALUnits to fill with data)
 * @param itemData The AVC Item (image) data (use processAVC_NALUnits to fill with data)
 * @param width The image width
 * @param height The image height
 */
MP4Err     addAVCImageToCollection      (ISOIFF_ImageCollection collection, ISOVCConfigAtomPtr avcC, ISOIFF_HEVCItemData itemData, u32 width, u32 height);

/*!
 * @discussion Collects all AVC Images from a image collection and provides the results in form of arrays. 
 The corresponding decoder configuration record of an image can be found in the decoderConfigs array with the same index.
 * @param collection The image collection that will be searched for AVC images
 * @param images Returning array with all AVC images found
 * @param decoderConfigs Returning array of decoder configuration records, that belong to the image of the same array index
 * @param numberOfImagesFound The number of images found
 */
MP4Err     getAVCImages                 (ISOIFF_ImageCollection collection, ISOIFF_Image **images, ISOVCConfigAtomPtr **avcCAtoms, u32 *numberOfImagesFound);

/*!
 * @discussion Restores the AVC bitstream from an AVC image.
 * @param image AVC Image
 * @param decoderConfig Decoder Configuration Record (Contains VPS, SPS, PPS)
 * @param bitstreamH Handle that will contain the bitstream data
 */
MP4Err     getAVCBitstreamFromImage     (ISOIFF_Image image, ISOVCConfigAtomPtr avcC, MP4Handle bitstreamH);

#endif