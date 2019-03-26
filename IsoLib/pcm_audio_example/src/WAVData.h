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
 @header WAVData
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef WAVData_h
#define WAVData_h

#include <stdio.h>
#include <stdlib.h>
#include "wavIO.h"

/*!
 @definedblock
 @define CHUNKSIZE The block size used for WAV file I/O
 */
#define CHUNKSIZE (16*1024)
/*! @/definedblock */

/*!
 @definedblock
 @define WAVE_FORMAT_TAG_INTEGER Format tag for integer format in WAV
 */
#define WAVE_FORMAT_TAG_INTEGER     1
/*! @/definedblock */

/*!
 @definedblock
 @define WAVE_FORMAT_TAG_IEEE_FLOAT Format tag for floating point format in WAV
 */
#define WAVE_FORMAT_TAG_IEEE_FLOAT  3
/*! @/definedblock */

/*!
 * @brief Contains the information about a wav audio file and the handles to
 read the wav audio file.
 * @field wavIOHandle Handle for wavIO library
 * @field inputFile The wav input file
 * @field channels The number of audio channels in the wav audio file
 * @field sampleRate The samplerate of the audio
 * @field byteDepth The bytes per sample (wavIO converts 16 bit PCM to 32 bit float PCM!)
 * @field totalSamplesPerChannel Number of samples per channel in total
 * @field samplesPerChannelFilled Number of samples per channels filled
 */
typedef struct WAVData
{
    WAVIO_HANDLE    wavIOHandle;
    FILE            *file;
    unsigned int    channels;
    unsigned int    sampleRate;
    unsigned int    byteDepth;
    unsigned long   totalSamplesPerChannel;
    unsigned int    formatTag;
    int             samplesPerChannelFilled;
    unsigned int    totalBytesWritten;
} WAVData;

/*!
 * @discussion Initializes WAV data from a file for reading
 * @param wavData Pointer to a allocated WAVData structure
 * @param inputFileStr Path to a wav file
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
int  initWAVDataForReading   (WAVData *wavData, char *inputFileStr);

/*!
 * @discussion Initializes WAV data from a file for writing
 * @param wavData Pointer to a allocated WAVData structure
 * @param outputFileStr File, to which the audio data will be written
 * @param channelCount The amount of channels in the audio data
 * @param sampleRate The samplerate of the audio data
 * @param bytesPerSample Byte depth of an audio sample
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
int  initWAVDataForWriting   (WAVData *wavData, char *outputFileStr, int channelCount, int sampleRate, int bytesPerSample);

/*!
 * @discussion Updates the size fields in the wav header and closes the wav file
 * @param wavData Pointer to an initialized WAVData structure
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
int  closeWAVDataFile        (WAVData *wavData);

/*!
 * @discussion Frees WAVData's memory
 * @param wavData Pointer to an initialized WAVData structure
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
int  freeWAVData             (WAVData *wavData);

#endif
