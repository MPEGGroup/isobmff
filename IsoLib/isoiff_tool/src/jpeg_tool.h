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
 * the code for non MPEG-4 conforming products. This copyright notice must be included in all copies
 * or derivative works. Copyright (c) 2016.
 */

/*!
 @header JPEG_tool
 Provides ISO Image File Format functions, that will work with JPEG bitstream
 @copyright Apple
 @updated 2016-12-05
 @author Armin Trattnig
 @version 1.0
 */

#ifndef JPEG_tool_h
#define JPEG_tool_h

#include "TLibDecoder/TDecEntropy.h"

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "Options.h"
#include "isoiff.h"
#include "isoiff_jpeg.h"
}

/*!
 * @discussion Creates an ISOIFF_ImageCollection with handler type 'JPEG'
 * @param collection Pointer to an ISOIFF_ImageCollection
 */
MP4Err createJPEG_ImageCollection(ISOIFF_ImageCollection *collection);

/*!
 * @discussion Will add an JPEG image item to the provided collection. The decoder configuration
 record will be linked to the image. NOTE: Before calling this function, create the image collection
 and generate the record and itemData using processJPEG_NALUnits
 * @param collection The image collection the provided JPEG image will be added to
 * @param record The decoder configuration record (use processJPEG_NALUnits to fill with data)
 * @param itemData The JPEG Item (image) data (use processJPEG_NALUnits to fill with data)
 * @param width The image width
 * @param height The image height
 */
MP4Err addJPEGImageToCollection(ISOIFF_ImageCollection collection,
                                ISOIFF_JPEGConfigurationAtomPtr jpgC, MP4Handle itemData, u32 width,
                                u32 height);

/*!
 * @discussion Collects all JPEG Images from a image collection and provides the results in form of
 arrays. The corresponding decoder configuration record of an image can be found in the
 decoderConfigs array with the same index.
 * @param collection The image collection that will be searched for JPEG images
 * @param images Returning array with all JPEG images found
 * @param decoderConfigs Returning array of decoder configuration records, that belong to the image
 of the same array index
 * @param numberOfImagesFound The number of images found
 */
MP4Err getJPEGImages(ISOIFF_ImageCollection collection, ISOIFF_Image **images,
                     ISOIFF_JPEGConfigurationAtomPtr **jpgCAtoms, u32 *numberOfImagesFound);

/*!
 * @discussion Restores the JPEG bitstream from an JPEG image.
 * @param image JPEG Image
 * @param decoderConfig Decoder Configuration Record (Contains VPS, SPS, PPS)
 * @param bitstreamH Handle that will contain the bitstream data
 */
MP4Err getJPEGBitstreamFromImage(ISOIFF_Image image, ISOIFF_JPEGConfigurationAtomPtr jpgC,
                                 MP4Handle bitstreamH);

/*!
* @discussion Will read bytes from the option's inputFile and will parse the JPEG bytestream and
creating contents of the ISOIFF_JPEGConfigurationAtomPtr and the MP4Handle for a single JPEG still
image.
* @param record ISOIFF_JPEGConfigurationAtomPtr will be created and overwritten, will be NULL if not
needed
* @param itemData MP4Handle must be created before calling this function
* @param options Pointer to Options for the input file path
*/
MP4Err processJPEGBitstream(MP4Handle prefixData, MP4Handle itemData, Options *options);

#endif
