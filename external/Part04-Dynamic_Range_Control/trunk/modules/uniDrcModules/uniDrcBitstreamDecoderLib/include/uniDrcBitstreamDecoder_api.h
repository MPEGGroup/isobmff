/***********************************************************************************
 
 This software module was originally developed by
 
 Apple Inc. and Fraunhofer IIS
 
 in the course of development of the ISO/IEC 23003-4 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23003-4 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23003-4 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23003-4
 standard.
 
 Apple Inc. and Fraunhofer IIS retains full right to modify and use the code for its
 own purpose, assign or donate the code to a third party and to inhibit third parties
 from using the code for products that do not conform to MPEG-related ITU Recommenda-
 tions and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works.
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/


#ifndef _UNI_DRC_BITSTREAM_DECODER_API_H_
#define _UNI_DRC_BITSTREAM_DECODER_API_H_

#include "uniDrcCommon_api.h"
#include "uniDrcCommon.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* handles */
typedef struct T_UNI_DRC_BS_DEC_STRUCT *HANDLE_UNI_DRC_BS_DEC_STRUCT;

/* open */
int
openUniDrcBitstreamDec(HANDLE_UNI_DRC_BS_DEC_STRUCT *phUniDrcBsDecStruct,
                       HANDLE_UNI_DRC_CONFIG *phUniDrcConfig,
                       HANDLE_LOUDNESS_INFO_SET *phLoudnessInfoSet,
                       HANDLE_UNI_DRC_GAIN *phUniDrcGain);
    
/* init */
int
initUniDrcBitstreamDec(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                       const int audioSampleRate,
                       const int audioFrameSize,
                       const int delayMode,
                       const int lfeChannelMapCount, /* pass -1 if lfeChannelMap is not present for downmixCoefficient decoding */
                       const int* lfeChannelMap);
   
/* process uniDrc (without uniDrcGain() payload) */
int
processUniDrcBitstreamDec_uniDrc(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                 HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                 HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                 unsigned char* bitstreamConfig,
                                 const int nBytes,
                                 const int nBitsOffset,
                                 int* nBitsRead);
    
/* process uniDrcConfig */
int
processUniDrcBitstreamDec_uniDrcConfig(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                       HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                       HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                       unsigned char* bitstreamConfig,
                                       const int nBytes,
                                       const int nBitsOffset,
                                       int* nBitsRead);
#if ISOBMFF_SYNTAX
#if AMD1_SYNTAX
#if !MPEG_H_SYNTAX
/* process isobmff */
int processUniDrcBitstreamDec_isobmff(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                      HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                      HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                      int baseChannelCount,
                                      unsigned char* bitstreamIsobmff,
                                      const int nBytesIsobmff,
                                      const int nBitsOffsetIsobmff,
                                      int* nBitsReadIsobmff);
#endif
#endif
#endif
/* process uniDrcGain */
int
processUniDrcBitstreamDec_uniDrcGain(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                     HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                     HANDLE_UNI_DRC_GAIN hUniDrcGain,
                                     unsigned char* bitstreamGain,
                                     const int nBytes,
                                     const int nBitsOffset,
                                     int* nBitsRead);

#if MPEG_H_SYNTAX
/* set targetChannelCount for each downmixId provided by downmixMatrixSet() structure */
/* note that this function needs to be called before calling processUniDrcBitstreamDec_uniDrcConfig() */
int setMpeghDownmixMatrixSetParamsUniDrcSelectionProcess(HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                         int downmixIdCount,
                                                         int* downmixId,
                                                         int* targetChannelCountForDownmixId);
#endif
    
/* process loudnessInfoSet */
int processUniDrcBitstreamDec_loudnessInfoSet(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                              HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                              HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                              unsigned char* bitstreamLoudness,
                                              const int nBytesLoudness,
                                              const int nBitsOffsetLoudness,
                                              int* nBitsReadLoudness);

/* close */
int closeUniDrcBitstreamDec(HANDLE_UNI_DRC_BS_DEC_STRUCT *phUniDrcBsDecStruct,
                            HANDLE_UNI_DRC_CONFIG *phUniDrcConfig,
                            HANDLE_LOUDNESS_INFO_SET *phLoudnessInfoSet,
                            HANDLE_UNI_DRC_GAIN *phUniDrcGain);
 
/* plot info */
void plotInfoBS( HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                 HANDLE_UNI_DRC_GAIN hUniDrcGain,
                 HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                 int frameNo, int nBytes, int plotInfo);

#endif /* _UNI_DRC_BITSTREAM_DECODER_H_ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

