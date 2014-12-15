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
 @header DRCData
 @copyright Apple
 @updated 2014-09-10
 @author Armin Trattnig
 @version 1.0
 */

#ifndef DRCData_h
#define DRCData_h

#include "DRCAtoms.h"

#include <stdio.h>
#include <stdlib.h>
#include <uniDrcBitstreamDecoder_api.h>
#include <readonlybitbuf.h>

/*!
 * @brief Contains instream DRC Data and handles and variables to read a DRC bitstream using 
 MPEG-D reference software's uniDrcBitstreamDecoder.
 * @field hUniDrcBsDecStruct Handle for uniDrcBitstreamDecoder
 * @field hUniDrcConfig Contains instream data of a uniDrcConfig
 * @field hLoudnessInfoSet Contains instream data of a LoudnessInfoSet
 * @field hUniDrcGain Contains instream data of a UniDrcGain
 * @field bufferOffset Number of bits, which need to be skipped to get to the correct position on bitlevel
 * @field bufferIndex Current position in the bitstreamBuffer
 * @field bufferBytesRemaining Bytes not yet processed from the DRC bitstream
 * @field bitstreamBuffer Buffer, which will contain the whole DRC bitstream
 * @field gainCount Number of uniDrcGains read from the DRC bitstream
 * @field bitstream Buffer, that can operate on bit level
 */
typedef struct DRCData
{
    HANDLE_UNI_DRC_BS_DEC_STRUCT    hUniDrcBsDecStruct;
    HANDLE_UNI_DRC_CONFIG           hUniDrcConfig;
    HANDLE_LOUDNESS_INFO_SET        hLoudnessInfoSet;
    HANDLE_UNI_DRC_GAIN             hUniDrcGain;
    int                             bufferOffset;
    int                             bufferIndex;
    int                             bufferBytesRemaining;
    unsigned char                   *bitstreamBuffer;
    int                             gainCount;
    robitbufHandle                  bitstream;
} DRCData;

/*!
 * @discussion Initializes DRC Data from a file, containing a DRC bitstream.
 The first LoudnessInfoSet and UniDrcConfig will be read.
 * @param drcData Pointer to a allocated DRCdata structure
 * @param inputFileStr Path to the drc bitstream file
 * @param sampleRate Samplerate of the audio data
 * @param channels Number of channels in audio data
 * @param framesize Number of samples in an audio frame
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  initDRCData                     (DRCData *drcData, char *inputFileStr, int sampleRate, int channels, int framesize);

/*!
 * @discussion Will read the next UniDrcGain from the drc bitstream. 
 The data will be byte aligned in order to put it into a MP4 track as a sample.
 * @param drcData Pointer to a initialized DRCdata structure
 * @param dataH MP4 handle, which will hold the byte aligned UniDrcGain
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  nextUniDRCGain                  (DRCData *drcData, MP4Handle dataH);

/*!
 * @discussion Will free memory of DRCData struct
 * @param drcData Pointer to a initialized DRCdata structure
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  freeDRCData                  (DRCData *drcData);

#endif