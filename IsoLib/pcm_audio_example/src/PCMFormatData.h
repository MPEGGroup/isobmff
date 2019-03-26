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

#ifndef PCMFormatData_h
#define PCMFormatData_h

#include "WAVData.h"

/*!
 * @brief Contains the information about the PCM data format.
 * @field channelCount The number of audio channels in the PCM data
 * @field sampleRate The samplerate of the audio
 * @field pcmSampleSize The bit count per sample (wavIO converts 16 bit PCM to 32 bit float PCM!)
 * @field isInteger Is nonzero for integer sample format, otherwise the format is floating point
 * @field isLittleEndian Is nonzero for little endian byte order, otherwise the order is big endian
 */
typedef struct PCMFormatDataStruct
{
    unsigned int    channelCount;
    unsigned int    sampleRate;
    unsigned int    pcmSampleSize;
    unsigned int    isInteger;
    unsigned int    isLittleEndian;
} PCMFormatData;

/*!
 * @discussion Initialize the PCMFormatData structure.
 * @param pcmFormatData Pointer PCMFormatData structure
 */
void initPCMFormatData (PCMFormatData* pcmFormatData);

/*!
 * @discussion Set the PCM format data based on the WAV header information.
 * @param wavData Pointer to an initialized WAVData structure for the input data
 * @param pcmFormatData Pointer to an initialized PCMFormatData for the output data
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
int setPCMFormatDataFromWAV (WAVData* wavData, PCMFormatData* pcmFormatData);

#endif
