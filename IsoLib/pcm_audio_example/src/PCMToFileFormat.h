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
derivative works. Copyright (c) 2019.
*/

/*!
 @header PCMToFileFormat
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef PCMToFileFormat_h
#define PCMToFileFormat_h

#include "PCMAtoms.h"
#include "PCMFormatData.h"

/*!
 * @discussion Adds PCM atoms to a audio sample entry. All the PCM atoms will be created and filled with
 data found in the WAV file.
 * @param pcmFormatData Pointer to an initialized PCMFormatData structure
 * @param audioSampleEntry Pointer to an initialized MP4AudioSampleEntryAtom
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  addPCMDataToAudioSampleEntry    (PCMFormatData *pcmFormatData, MP4AudioSampleEntryAtomPtr audioSampleEntry);

#endif
