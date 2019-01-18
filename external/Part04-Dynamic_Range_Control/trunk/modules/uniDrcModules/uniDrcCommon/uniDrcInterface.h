/***********************************************************************************
 
 This software module was originally developed by
 
 Fraunhofer IIS
 
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
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works.
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#ifndef _UNI_DRC_INTERFACE_H_
#define _UNI_DRC_INTERFACE_H_

/* ======================================================================
 Structures to reflect the information conveyed by the bitstream syntax
 ====================================================================== */

#include "uniDrcCommon.h"

#if MPEG_D_DRC_EXTENSION_V1
typedef struct {
    int     loudnessEqRequestPresent;
    int     loudnessEqRequest;
    int     sensitivityPresent;
    float   sensitivity;
    int     playbackGainPresent;
    float   playbackGain;
} LoudnessEqParameterInterface;

typedef struct {
    int     eqSetPurposeRequest;
} EqualizationControlInterface;
#endif /* MPEG_D_DRC_EXTENSION_V1 */

typedef struct {
    int   extSizeBits;
    int   extBitSize;
    int   uniDrcInterfaceExtType;
} SpecificInterfaceExtension;

typedef struct {
    int interfaceExtensionCount;
    SpecificInterfaceExtension specificInterfaceExtension[EXT_COUNT_MAX];
#if MPEG_D_DRC_EXTENSION_V1
    int loudnessEqParameterInterfacePresent;
    int equalizationControlInterfacePresent;
    LoudnessEqParameterInterface            loudnessEqParameterInterface;
    EqualizationControlInterface            equalizationControlInterface;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
} UniDrcInterfaceExtension;

typedef struct {
    int   changeCompress;
    int   changeBoost;
    float compress;
    float boost;
    int   changeDrcCharacteristicTarget;
    int   drcCharacteristicTarget;
} DynamicRangeControlParameterInterface;

typedef struct {
    int   dynamicRangeControlOn;
    int   numDrcFeatureRequests;
    int   drcFeatureRequestType[MAX_NUM_DRC_FEATURE_REQUESTS];
    int   numDrcEffectTypeRequests[MAX_NUM_DRC_FEATURE_REQUESTS];
    int   numDrcEffectTypeRequestsDesired[MAX_NUM_DRC_FEATURE_REQUESTS];
    int   drcEffectTypeRequest[MAX_NUM_DRC_FEATURE_REQUESTS][MAX_NUM_DRC_EFFECT_TYPE_REQUESTS];
    int   dynRangeMeasurementRequestType[MAX_NUM_DRC_FEATURE_REQUESTS];
    int   dynRangeRequestedIsRange[MAX_NUM_DRC_FEATURE_REQUESTS];
    float dynamicRangeRequestValue[MAX_NUM_DRC_FEATURE_REQUESTS];
    float dynamicRangeRequestValueMin[MAX_NUM_DRC_FEATURE_REQUESTS];
    float dynamicRangeRequestValueMax[MAX_NUM_DRC_FEATURE_REQUESTS];
    int   drcCharacteristicRequest[MAX_NUM_DRC_FEATURE_REQUESTS];
} DynamicRangeControlInterface;

typedef struct {
    int   albumMode;
    int   peakLimiterPresent;
    int   changeLoudnessDeviationMax;
    int   loudnessDeviationMax; /* only 1 dB steps */
    int   changeLoudnessMeasurementMethod;
    int   loudnessMeasurementMethod;
    int   changeLoudnessMeasurementSystem;
    int   loudnessMeasurementSystem;
    int   changeLoudnessMeasurementPreProc;
    int   loudnessMeasurementPreProc;
    int   changeDeviceCutOffFrequency;
    int   deviceCutOffFrequency;
    int   changeLoudnessNormalizationGainDbMax;
    float loudnessNormalizationGainDbMax;
    int   changeLoudnessNormalizationGainModificationDb;
    float loudnessNormalizationGainModificationDb;
    int   changeOutputPeakLevelMax;
    float outputPeakLevelMax;
} LoudnessNormalizationParameterInterface;

typedef struct {
    int   loudnessNormalizationOn;
    float targetLoudness;
} LoudnessNormalizationControlInterface;

typedef struct {
    int   targetConfigRequestType;
    int   numDownmixIdRequests;
    int   downmixIdRequested[MAX_NUM_DOWNMIX_ID_REQUESTS];
    int   targetLayoutRequested;
    int   targetChannelCountRequested;
} SystemInterface;

typedef struct {
    int   uniDrcInterfaceSignatureType;
    int   uniDrcInterfaceSignatureDataLength;
    unsigned int uniDrcInterfaceSignatureData[MAX_SIGNATURE_DATA_LENGTH_PLUS_ONE*8];
} UniDrcInterfaceSignature;

typedef struct T_UNI_DRC_INTERFACE_STRUCT{
    int uniDrcInterfaceSignaturePresent;
    int systemInterfacePresent;
    int loudnessNormalizationControlInterfacePresent;
    int loudnessNormalizationParameterInterfacePresent;
    int dynamicRangeControlInterfacePresent;
    int dynamicRangeControlParameterInterfacePresent;
    int uniDrcInterfaceExtensionPresent;
    UniDrcInterfaceSignature                uniDrcInterfaceSignature;
    SystemInterface                         systemInterface;
    LoudnessNormalizationControlInterface   loudnessNormalizationControlInterface;
    LoudnessNormalizationParameterInterface loudnessNormalizationParameterInterface;
    DynamicRangeControlInterface            dynamicRangeControlInterface;
    DynamicRangeControlParameterInterface   dynamicRangeControlParameterInterface;
    UniDrcInterfaceExtension                uniDrcInterfaceExtension;
} UniDrcInterface;

#endif /* _UNI_DRC_INTERFACE_H_ */
