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
 @header WAVData
 @copyright Apple
 @updated 2014-09-10
 @author Armin Trattnig
 @version 1.0
 */

#ifndef WAVData_h
#define WAVData_h

#include "DRCAtoms.h"

#include <stdio.h>
#include <stdlib.h>
#include <wavIO.h>



/*!
 @definedblock
 @define BLOCKLENGTH The number of samples contained in one audio frame
 */
#define BLOCKLENGTH 1024

/*! @/definedblock */


/*!
 @enum DRC Atoms
 @abstract Extends the list of atom types from MP4Atoms.h with atoms used with drc data
 @constant DRCUniDrcSampleEntryAtomType Type for DRCUniDrcSampleEntry. "unid"
 @constant DRCCoefficientBasicAtomType Type for DRCCoefficientBasic. "udc1"
 @constant DRCInstructionsBasicAtomType Type for DRCInstructionsBasic. "udi1"
 @constant DRCCoefficientUniDRCAtomType Type for DRCCoefficientUniDRC. "udc2"
 @constant DRCInstructionsUniDRCAtomType Type for DRCInstructionsUniDRC. "udi2"
 */
enum
{
    AudioUnsigned16BitLittleEndianSampleEntryType            = MP4_FOUR_CHAR_CODE( 'r', 'a', 'w', ' ' ),
    AudioSigned16BitBigEndianSampleEntryType                 = MP4_FOUR_CHAR_CODE( 't', 'w', 'o', 's' ),
    AudioSigned16BitLittleEndianSampleEntryType              = MP4_FOUR_CHAR_CODE( 's', 'o', 'w', 't' )
};

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
    int             samplesPerChannelFilled;
    u32             totalBytesWritten;
} WAVData;

/*!
 * @discussion Initializes WAV data from a file for reading
 * @param wavData Pointer to a allocated WAVData structure
 * @param inputFileStr Path to a wav file
 * @return An MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  initWAVDataForReading            (WAVData *wavData, char *inputFileStr);

/*!
 * @discussion Reads an audio frame form the wav input file
 * @param wavData Pointer to an initialized WAVData structure
 * @param sampleH MP4 handle, which will contain the audio frame
 * @param duration The number of samples contained in the audio frame, which is equal to the duration
 * @return An MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  readFrame                       (WAVData *wavData, MP4Handle sampleH, u32 *duration);

/*!
 * @discussion Initializes WAV data from a file for writing
 * @param wavData Pointer to a allocated WAVData structure
 * @param outputFileStr File, to which the audio data will be written
 * @param channelCount The amount of channels in the audio data
 * @param sampleRate The samplerate of the audio data
 * @param bytesPerSample Byte depth of an audio sample
 * @return An MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  initWAVDataForWriting            (WAVData *wavData, char *outputFileStr, int channelCount, int sampleRate, int bytesPerSample);

/*!
 * @discussion Writes an audio frame to the wav output file
 * @param wavData Pointer to a for writing initialized WAVData structure
 * @param sampleH MP4 handle, which contains the audio frame
 * @param size Size in bytes of the input mp4handle
 * @return An MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  writeFrame                       (WAVData *wavData, MP4Handle sampleH, u32 size);

/*!
 * @discussion Updates the size fields in the wav header and closes the wav file
 * @param wavData Pointer to an initialized WAVData structure
 * @return An MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  closeWAVDataFile                 (WAVData *wavData);
#endif