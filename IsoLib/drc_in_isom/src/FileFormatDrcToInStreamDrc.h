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
 @header InStreamDrcToFileFormatDrc
 @copyright Apple
 @updated 2014-10-03
 @author Armin Trattnig
 @version 1.0
 */

#ifndef InStreamDrcToFileFormatDrc_h
#define InStreamDrcToFileFormatDrc_h

#include "DRCAtoms.h"

#include <uniDrc.h>
#include <writeonlybitbuf.h>
#include <uniDrcBitstreamDecoder_api.h>


/*!
 * @brief Represents the static drc data in file format syntax. Contains
 all drc related atoms and fields.
 * @field channelCount  Number of channels
 * @field sampleRate The sample rate of the base audio signal
 * @field bytesPerSample Byte depth of a single audio sample
 * @field drcLocation Location of the drc data, will be used for writing in stream drc
 * @field loudnessAtom LoudnessBox, according to ISO/IEC 14496-12 Amd.4
 * @field channelLayoutAtom ChannelLayoutBox
 * @field downMixInstructionsAtoms LinkedList of DownMixInstructionBoxes
 * @field drcCoefficientsBasicAtoms LinkedList of CoefficientsBasicBoxes
 * @field drcInstructionsBasicAtoms LinkedList of InstructionsBasicBoxes
 * @field drcCoefficientsUniDrcAtoms LinkedList of CoefficientUniDrcBoxes
 * @field drcInstructionsUniDrcAtoms LinkedList of InstructionsUniDrcBoxes
 */
typedef struct StaticDrcData
{
    int                             channelCount;
    int                             sampleRate;
    int                             bytesPerSample;
    int                             drcLocation;
    MP4AudioSampleEntryAtomPtr      sampleEntryAtom;
    MP4LoudnessAtomPtr              loudnessAtom;
    MP4ChannelLayoutAtomPtr         channelLayoutAtom;
    MP4LinkedList                   downMixInstructionsAtoms;
    MP4LinkedList                   drcCoefficientsBasicAtoms;
    MP4LinkedList                   drcInstructionsBasicAtoms;
    MP4LinkedList                   drcCoefficientsUniDrcAtoms;
    MP4LinkedList                   drcInstructionsUniDrcAtoms;
} StaticDrcData;

/*!
 * @brief Contains information to control a writeonlybitbuf, which operates on
 bit level.
 * @field totalBufferSizeInBytes Total number of bytes allocated for the bistreamBuffer
 * @field currentBytePosition The current byte position in the bitstreamBuffer
 * @field offsetInBits The offset in bits from the current byte position
 * @field bitstreamBuffer The actual buffer for the drc data
 * @field bitstream writeonlybitbuf handle
 */
typedef struct DrcBitstreamHandle
{
    int                 totalBufferSizeInBytes;
    int                 currentBytePosition;
    int                 offsetInBits;
    unsigned char       *bitstreamBuffer;
    wobitbufHandle      bitstream;
} DrcBitstreamHandle;

/*!
 * @brief Contains handles for the DrcBitstreamDecoderLib. Will be used for checking written 
 drc data and to obtain the exact size in bits for specific drc parts.
 * @field hUniDrcBsDecStruct Handle for uniDrcBitstreamDecoder
 * @field hUniDrcConfig Contains instream data of a uniDrcConfig
 * @field hLoudnessInfoSet Contains instream data of a LoudnessInfoSet
 * @field hUniDrcGain Contains instream data of a UniDrcGain
 * @field gainCount Number of gains processed
 */
typedef struct DrcBitStreamHelper
{
    HANDLE_UNI_DRC_BS_DEC_STRUCT    hUniDrcBsDecStruct;
    HANDLE_UNI_DRC_CONFIG           hUniDrcConfig;
    HANDLE_LOUDNESS_INFO_SET        hLoudnessInfoSet;
    HANDLE_UNI_DRC_GAIN             hUniDrcGain;
    int                             gainCount;
} DrcBitStreamHelper;

/*!
 * @discussion Collects the static drc data contained inside an audio track
 * @param trak The audio track
 * @param staticDrcData Pointer to an allocated StaticDrcData structure
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  createStaticDrcDataFromAudioTrack   (MP4Track trak, StaticDrcData *staticDrcData);

/*!
 * @discussion Initializes the buffer for the drc output file
 * @param staticDrcData Data for static drc. Used to estimate buffer size
 * @param drcBitStreamHandle Pointer to a drcBitStreamHandle, must be allocated
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  initDrcBitstream                    (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);

/*!
 * @discussion Converts the file format syntax to in stream syntax for static drc data
 and writes the in stream data in the drc buffer.
 * @param staticDrcData Data for static drc
 * @param drcBitStreamHandle Pointer to a drcBitStreamHandle, must be allocated
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  writeStaticDrcDataToBitstream       (StaticDrcData *staticDrcData, DrcBitstreamHandle *drcBitStreamHandle);

/*!
 * @discussion Prepares a DrcBitStreamHelper by checking static drc data and set buffer position
 and offset correctly
 * @param drcBitStreamHelper Pointer to DrcBitStreamHelper, must be allocated
 * @param drcBitStreamHandle Pointer to a drcBitStreamHandle, must already contain static drc data
 * @param staticDrcData Data for static drc
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  prepareDrcBitStreamHelper           (DrcBitStreamHelper *drcBitStreamHelper, DrcBitstreamHandle *drcBitStreamHandle, StaticDrcData *staticDrcData);

/*!
 * @discussion Writes 0 for uniDrcLoudnessInfoSetPresent flag and checks the uniDrcGain() contained
 in the packetH. It then writes the bits of the gain to the bitstreamBuffer. Additional buffersize will be 
 allocated and the padding bits will be dropped.
 * @param packetH MP4Handle containing a dynamic drc sample from a drc metadata track
 * @param drcBitStreamHandle Pointer to a drcBitStreamHandle, must already contain static drc data
 * @param drcBitStreamHelper Pointer to a DrcBitStreamHelper, must be prepared with prepareDrcBitStreamHelper
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  writeDrcGainToBitstream             (MP4Handle packetH, DrcBitstreamHandle *drcBitStreamHandle, DrcBitStreamHelper *drcBitStreamHelper);

/*!
 * @discussion Frees memory used by a DrcBitstreamHandle struct
 * @param drcBitStreamHandle Pointer to a drcBitStreamHandle
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  freeDrcBitstreamHandle              (DrcBitstreamHandle *drcBitStreamHandle);

/*!
 * @discussion Frees memory used by a DrcBitStreamHelper struct
 * @param drcBitStreamHelper Pointer to a drcBitStreamHelper
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  freeDrcBitStreamHelper              (DrcBitStreamHelper *drcBitStreamHelper);

/*!
 * @discussion Frees memory used by a StaticDrcData struct
 * @param staticDrcData Pointer to a staticDrcData
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  freeStaticDrcData                   (StaticDrcData *staticDrcData);

#endif