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

#ifndef _UNI_DRC_BITSTREAM_DECODER_H_
#define _UNI_DRC_BITSTREAM_DECODER_H_

#include "uniDrcBitstreamDecoder_api.h"
#include "uniDrcTables.h"
#include "readonlybitbuf.h"
#include "uniDrcCommon.h"
#include "uniDrc.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Major DRC tool parameters */
typedef struct {
    int deltaTminDefault;
#if AMD2_COR2
    int sampleRateDefault;
#endif
    int drcFrameSize;
    int nGainValuesMaxDefault;
    int delayMode;
    int lfeChannelMapCount;             /* number of entries of lfeChannelMap, either -1 (no entries) or baseChannelCount */
    int lfeChannelMap[CHANNEL_COUNT_MAX];
} DrcParamsBsDec;

typedef struct T_UNI_DRC_BS_DEC_STRUCT
{
    robitbuf bitstream;

    Tables tablesDefault;
    DrcParamsBsDec drcParams;
    
} UNI_DRC_BS_DEC_STRUCT;

int
initDrcParamsBsDec(const int audioFrameSize,
                   const int audioSampleRate,
                   const int delayMode,
                   const int lfeChannelMapCount,
                   const int* lfeChannelMap,
                   DrcParamsBsDec* drcParams);

#ifdef __cplusplus
}
#endif
#endif
