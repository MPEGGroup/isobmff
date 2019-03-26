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
 @header FileFormatToPCM
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef FileFormatToPCM_h
#define FileFormatToPCM_h

#include "PCMAtoms.h"

/*!
 * @brief Represents the PCM format data in MPEG-4 boxes.
 * @field sampleEntryAtom Audio Sample Entry
 * @field channelLayoutAtom ChannelLayoutBox
 * @field pcmConfigAtom PCM configuration
 */
typedef struct PCMAtoms
{
    MP4AudioSampleEntryAtomPtr      sampleEntryAtom;
    MP4ChannelLayoutAtomPtr         channelLayoutAtom;
    MP4PCMConfigAtomPtr             pcmConfigAtom;
} PCMAtoms;

/*!
 * @discussion Collects the PCM format parameters contained inside an audio track
 * @param trak The audio track
 * @param pcmFormatData Pointer to an allocated PCMFormatData structure
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  createPCMAtomsFromAudioTrack   (MP4Track trak, PCMFormatData *pcmFormatData, PCMAtoms *pcmAtoms);

/*!
 * @discussion Frees memory used by a PCMAtoms struct
 * @param pcmAtoms Pointer to a pcmAtoms
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  freePCMAtoms                   (PCMAtoms *pcmAtoms);

#endif
