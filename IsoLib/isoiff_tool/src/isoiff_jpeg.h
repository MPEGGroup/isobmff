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
 @header isoiff_jpeg
 Provides structs for JPEG Images in ISO Image File Format
 @copyright Apple
 @updated 2016-12-21
 @author Armin Trattnig
 @version 1.0
 */

#ifndef isoiff_jpeg_h
#define isoiff_jpeg_h

#include "MP4Movies.h"
#include "MP4Atoms.h"

enum
{
  ISOIFF_4CC_jpeg = MP4_FOUR_CHAR_CODE('j', 'p', 'e', 'g'),
  ISOIFF_4CC_jpgC = MP4_FOUR_CHAR_CODE('j', 'p', 'g', 'C'),
};

/*!
 * @typedef ISOIFF_JPEGConfigurationAtom
 * @brief JPEGConfigurationBox
 */
typedef struct ISOIFF_JPEGConfigurationAtom
{
  MP4_BASE_ATOM

  MP4Handle jpegPrefix;
} ISOIFF_JPEGConfigurationAtom, *ISOIFF_JPEGConfigurationAtomPtr;

/*!
 * @discussion Creates a JPEGConfigurationAtom (Allocates memory and initializes fields)
 * @param outAtom Pointer that will hold a reference to the created JPEGConfigurationAtom
 * @param jpegPrefix MP4Handle  that will be put into the atom
 */
MP4Err ISOIFF_CreateJPEGConfigurationAtom(ISOIFF_JPEGConfigurationAtomPtr *outAtom,
                                          MP4Handle jpegPrefix);

#endif
