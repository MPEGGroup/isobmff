/***********************************************************************************
 
 This software module was originally developed by
 
 Apple Inc.
 
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
 
 Apple Inc. retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works.
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#ifndef _UNI_DRC_H_
#define _UNI_DRC_H_

#include "uniDrcTables.h"
#include "uniDrcCommon.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Defines for bitstream payload */
#define METHOD_DEFINITION_UNKNOWN_OTHER             0
#define METHOD_DEFINITION_PROGRAM_LOUDNESS          1
#define METHOD_DEFINITION_ANCHOR_LOUDNESS           2
#define METHOD_DEFINITION_MAX_OF_LOUDNESS_RANGE     3
#define METHOD_DEFINITION_MOMENTARY_LOUDNESS_MAX    4
#define METHOD_DEFINITION_SHORT_TERM_LOUDNESS_MAX   5
#define METHOD_DEFINITION_LOUDNESS_RANGE            6
#define METHOD_DEFINITION_MIXING_LEVEL              7
#define METHOD_DEFINITION_ROOM_TYPE                 8
#define METHOD_DEFINITION_SHORT_TERM_LOUDNESS       9

#define MEASUREMENT_SYSTEM_UNKNOWN_OTHER            0
#define MEASUREMENT_SYSTEM_EBU_R_128                1
#define MEASUREMENT_SYSTEM_BS_1770_4                2
#define MEASUREMENT_SYSTEM_BS_1770_3                MEASUREMENT_SYSTEM_BS_1770_4
#define MEASUREMENT_SYSTEM_BS_1770_4_PRE_PROCESSING 3
#define MEASUREMENT_SYSTEM_BS_1770_3_PRE_PROCESSING MEASUREMENT_SYSTEM_BS_1770_4_PRE_PROCESSING
#define MEASUREMENT_SYSTEM_USER                     4
#define MEASUREMENT_SYSTEM_EXPERT_PANEL             5
#define MEASUREMENT_SYSTEM_BS_1771_1                6
#define MEASUREMENT_SYSTEM_RESERVED_A               7
#define MEASUREMENT_SYSTEM_RESERVED_B               8
#define MEASUREMENT_SYSTEM_RESERVED_C               9
#define MEASUREMENT_SYSTEM_RESERVED_D               10
#define MEASUREMENT_SYSTEM_RESERVED_E               11

#define RELIABILITY_UKNOWN                          0
#define RELIABILITY_UNVERIFIED                      1
#define RELIABILITY_CEILING                         2
#define RELIABILITY_ACCURATE                        3

#define EFFECT_BIT_COUNT                            12

#define EFFECT_BIT_NONE                             (-1)  /* this effect bit is virtual */
#define EFFECT_BIT_NIGHT                            0x0001
#define EFFECT_BIT_NOISY                            0x0002
#define EFFECT_BIT_LIMITED                          0x0004
#define EFFECT_BIT_LOWLEVEL                         0x0008
#define EFFECT_BIT_DIALOG                           0x0010
#define EFFECT_BIT_GENERAL_COMPR                    0x0020
#define EFFECT_BIT_EXPAND                           0x0040
#define EFFECT_BIT_ARTISTIC                         0x0080
#define EFFECT_BIT_CLIPPING                         0x0100
#define EFFECT_BIT_FADE                             0x0200
#define EFFECT_BIT_DUCK_OTHER                       0x0400
#define EFFECT_BIT_DUCK_SELF                        0x0800

#define GAIN_CODING_PROFILE_REGULAR                 0
#define GAIN_CODING_PROFILE_FADING                  1
#define GAIN_CODING_PROFILE_CLIPPING                2
#define GAIN_CODING_PROFILE_CONSTANT                3
#define GAIN_CODING_PROFILE_DUCKING                 GAIN_CODING_PROFILE_CLIPPING

#define GAIN_INTERPOLATION_TYPE_SPLINE              0
#define GAIN_INTERPOLATION_TYPE_LINEAR              1

/* Defines for user requests related to target loudness */
#define USER_METHOD_DEFINITION_DEFAULT              0
#define USER_METHOD_DEFINITION_PROGRAM_LOUDNESS     1
#define USER_METHOD_DEFINITION_ANCHOR_LOUDNESS      2

#define USER_MEASUREMENT_SYSTEM_DEFAULT             0
#define USER_MEASUREMENT_SYSTEM_BS_1770_4           1
#define USER_MEASUREMENT_SYSTEM_BS_1770_3           USER_MEASUREMENT_SYSTEM_BS_1770_4
#define USER_MEASUREMENT_SYSTEM_USER                2
#define USER_MEASUREMENT_SYSTEM_EXPERT_PANEL        3
#define USER_MEASUREMENT_SYSTEM_RESERVED_A          4
#define USER_MEASUREMENT_SYSTEM_RESERVED_B          5
#define USER_MEASUREMENT_SYSTEM_RESERVED_C          6
#define USER_MEASUREMENT_SYSTEM_RESERVED_D          7
#define USER_MEASUREMENT_SYSTEM_RESERVED_E          8

#define USER_LOUDNESS_PREPROCESSING_DEFAULT         0
#define USER_LOUDNESS_PREPROCESSING_OFF             1
#define USER_LOUDNESS_PREPROCESSING_HIGHPASS        2
    
#define LOUDNESS_DEVIATION_MAX_DEFAULT              63
#define LOUDNESS_NORMALIZATION_GAIN_MAX_DEFAULT     1000 /* infinity as default */

/* Requestable dynamic range measurement values */
#define SHORT_TERM_LOUDNESS_TO_AVG                  0
#define MOMENTARY_LOUDNESS_TO_AVG                   1
#define TOP_OF_LOUDNESS_RANGE_TO_AVG                2

/* AMD1_SYNTAX */
#if MPEG_D_DRC_EXTENSION_V1
#define DRC_COMPLEXITY_LEVEL_MAX                    0xF
#define EQ_COMPLEXITY_LEVEL_MAX                     0xF
#define COMPLEXITY_LEVEL_SUPPORTED_TOTAL            20.0f    /* Must be set to actual capability of decoder */

#define COMPLEXITY_W_SUBBAND_EQ                     2.5f
#define COMPLEXITY_W_FIR                            0.4f
#define COMPLEXITY_W_IIR                            5.0f
#define COMPLEXITY_W_MOD_TIME                       1.0f
#define COMPLEXITY_W_MOD_SUBBAND                    2.0f
#define COMPLEXITY_W_LAP                            2.0f
#define COMPLEXITY_W_SHAPE                          6.0f
#define COMPLEXITY_W_SPLINE                         5.0f
#define COMPLEXITY_W_LINEAR                         2.5f
#define COMPLEXITY_W_PARAM_DRC_FILT                 5.0f
#define COMPLEXITY_W_PARAM_DRC_SUBBAND              5.0f
#define COMPLEXITY_W_PARAM_LIM_FILT                 4.5f
#define COMPLEXITY_W_PARAM_DRC_ATTACK               136.0f
    
#define LEFT_SIDE                                   0
#define RIGHT_SIDE                                  1
    
#define CHARACTERISTIC_SIGMOID                      0
#define CHARACTERISTIC_NODES                        1
#define CHARACTERISTIC_PASS_THRU                    2
    
#define GAINFORMAT_QMF32                            0x1
#define GAINFORMAT_QMFHYBRID39                      0x2
#define GAINFORMAT_QMF64                            0x3
#define GAINFORMAT_QMFHYBRID71                      0x4
#define GAINFORMAT_QMF128                           0x5
#define GAINFORMAT_QMFHYBRID135                     0x6
#define GAINFORMAT_UNIFORM                          0x7
    
#define DRC_INPUT_LOUDNESS_TARGET                   (-31.0f)  /* dB */
    
#define SHAPE_FILTER_TYPE_OFF                       0
#define SHAPE_FILTER_TYPE_LF_CUT                    1
#define SHAPE_FILTER_TYPE_LF_BOOST                  2
#define SHAPE_FILTER_TYPE_HF_CUT                    3
#define SHAPE_FILTER_TYPE_HF_BOOST                  4
    
#define SHAPE_FILTER_DRC_GAIN_MAX_MINUS_ONE         1583.8931924611f  /* 10^3.2 - 1 */
    
typedef struct {
    int   type;
    float gainOffset;
    float y1Bound;
    float warpedGainMax;
    float factor;
    float coeffSum;
    float partialCoeffSum;
    float gNorm;
    float a1;
    float a2;
    float b1;
    float b2;
    float audioInState1[CHANNEL_COUNT_MAX];
    float audioInState2[CHANNEL_COUNT_MAX];
    float audioOutState1[CHANNEL_COUNT_MAX];
    float audioOutState2[CHANNEL_COUNT_MAX];
} ShapeFilter;

typedef struct {
    int shapeFilterBlockPresent;
    float drcGainLast;
    ShapeFilter shapeFilter[4];
} ShapeFilterBlock;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    
/* ======================================================================
 Structures to reflect the information conveyed by the bitstream syntax
 ====================================================================== */

#if AMD1_SYNTAX
typedef struct {
    int levelEstimKWeightingType;
    int levelEstimIntegrationTimePresent;
    int levelEstimIntegrationTime;
    int drcCurveDefinitionType;
    int drcCharacteristic;
    int nodeCount;
    int nodeLevel[PARAM_DRC_TYPE_FF_NODE_COUNT_MAX];
    int nodeGain[PARAM_DRC_TYPE_FF_NODE_COUNT_MAX];
    int drcGainSmoothParametersPresent;
    int gainSmoothAttackTimeSlow;
    int gainSmoothReleaseTimeSlow;
    int gainSmoothTimeFastPresent;
    int gainSmoothAttackTimeFast;
    int gainSmoothReleaseTimeFast;
    int gainSmoothThresholdPresent;
    int gainSmoothAttackThreshold;
    int gainSmoothReleaseThreshold;
    int gainSmoothHoldOffCountPresent;
    int gainSmoothHoldOff;
    
    /* derived data */
    int disableParamtricDrc;
} ParametricDrcTypeFeedForward;
    
#ifdef AMD1_PARAMETRIC_LIMITER
typedef struct {
    int parametricLimThresholdPresent;
    float parametricLimThreshold;
    int parametricLimAttack;
    int parametricLimReleasePresent;
    int parametricLimRelease;
    int drcCharacteristic;
    
    /* derived data */
    int disableParamtricDrc;
} ParametricDrcTypeLim;
#endif

typedef struct {
    int parametricDrcId;
    int parametricDrcLookAheadPresent;
    int parametricDrcLookAhead;
    int parametricDrcPresetIdPresent;
    int parametricDrcPresetId;
    int parametricDrcType;
    int lenBitSize;
    ParametricDrcTypeFeedForward parametricDrcTypeFeedForward;
#ifdef AMD1_PARAMETRIC_LIMITER
    ParametricDrcTypeLim parametricDrcTypeLim;
#endif
    
    /* derived data */
    int drcCharacteristic;
    int disableParamtricDrc;
} ParametricDrcInstructions;

typedef struct {
    int parametricDrcId;
    int sideChainConfigType;
    int downmixId;
    int levelEstimChannelWeightFormat;
    float levelEstimChannelWeight[CHANNEL_COUNT_MAX];
    int drcInputLoudnessPresent;
    float drcInputLoudness;
    
    /* derived data */
    int channelCountFromDownmixId;
} ParametricDrcGainSetParams;

typedef struct {
    int drcLocation;
    int parametricDrcFrameSizeFormat;
    int parametricDrcFrameSize;
    int parametricDrcDelayMaxPresent;
    int parametricDrcDelayMax;
    int resetParametricDrc;
    int parametricDrcGainSetCount;
    ParametricDrcGainSetParams parametricDrcGainSetParams[SEQUENCE_COUNT_MAX];
} DrcCoefficientsParametricDrc;
#endif /* AMD1_SYNTAX */
    
typedef struct {
    int baseChannelCount;
    int layoutSignalingPresent;
    int definedLayout;
    int speakerPosition[SPEAKER_POS_COUNT_MAX];
} ChannelLayout;

typedef struct {
    int downmixId;
    int targetChannelCount;
    int targetLayout;
    int downmixCoefficientsPresent;
    float downmixCoefficient[DOWNMIX_COEFF_COUNT_MAX];
} DownmixInstructions;

typedef struct {
    int gainSequenceIndex;
    int drcCharacteristic;
#if MPEG_D_DRC_EXTENSION_V1
    int drcCharacteristicPresent;
    int drcCharacteristicFormatIsCICP;
    int drcCharacteristicLeftIndex;
    int drcCharacteristicRightIndex;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    int crossoverFreqIndex;
    int startSubBandIndex;
} GainParams;

typedef struct {
    int duckingScalingPresent;
    float duckingScaling;
    float duckingScalingQuantized;
} DuckingModifiers;

typedef struct {
#if MPEG_D_DRC_EXTENSION_V1
    int targetCharacteristicLeftPresent [DRC_BAND_COUNT_MAX];
    int targetCharacteristicLeftIndex   [DRC_BAND_COUNT_MAX];
    int targetCharacteristicRightPresent[DRC_BAND_COUNT_MAX];
    int targetCharacteristicRightIndex  [DRC_BAND_COUNT_MAX];
    int shapeFilterPresent;
    int shapeFilterIndex;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    int     gainScalingPresent  [BAND_COUNT_MAX];
    float   attenuationScaling  [BAND_COUNT_MAX];
    float   amplificationScaling[BAND_COUNT_MAX];
    int     gainOffsetPresent   [BAND_COUNT_MAX];
    float   gainOffset          [BAND_COUNT_MAX];
} GainModifiers;

typedef struct {
    int gainCodingProfile;
    int gainInterpolationType;
    int fullFrame;
    int timeAlignment;
    int timeDeltaMinPresent;
    int timeDeltaMin;
    int bandCount;
    int drcBandType;
    GainParams gainParams[BAND_COUNT_MAX];
    
    /* derived data */
    int nGainValuesMax;
    Tables tables;
} GainSetParams;

#if MPEG_D_DRC_EXTENSION_V1
#define SPLIT_CHARACTERISTIC_NODE_COUNT_MAX 4  /* one side of characteristic */
typedef struct {
    int characteristicFormat;
    float ioRatio;
    float gain;
    float exp;
    int flipSign;
    int characteristicNodeCount;
    float nodeLevel[SPLIT_CHARACTERISTIC_NODE_COUNT_MAX+1];
    float nodeGain [SPLIT_CHARACTERISTIC_NODE_COUNT_MAX+1];
} SplitDrcCharacteristic;

typedef struct {
    int cornerFreqIndex;
    int filterStrengthIndex;
} ShapeFilterParams;

typedef struct {
    int lfCutFilterPresent;
    ShapeFilterParams lfCutParams;
    int lfBoostFilterPresent;
    ShapeFilterParams lfBoostParams;
    int hfCutFilterPresent;
    ShapeFilterParams hfCutParams;
    int hfBoostFilterPresent;
    ShapeFilterParams hfBoostParams;
} ShapeFilterBlockParams;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    
typedef struct {
    int drcLocation;
    int drcCharacteristic;
} DrcCoefficientsBasic;

typedef struct {
    int version;
    int drcLocation;
    int drcFrameSizePresent;
    int drcFrameSize;
    int gainSetCount;
    GainSetParams gainSetParams[GAIN_SET_COUNT_MAX];
#if MPEG_D_DRC_EXTENSION_V1
    int drcCharacteristicLeftPresent;
    int characteristicLeftCount;
    SplitDrcCharacteristic splitCharacteristicLeft [SPLIT_CHARACTERISTIC_COUNT_MAX];
    int drcCharacteristicRightPresent;
    int characteristicRightCount;
    SplitDrcCharacteristic splitCharacteristicRight[SPLIT_CHARACTERISTIC_COUNT_MAX];
    int shapeFiltersPresent;
    int shapeFilterCount;
    ShapeFilterBlockParams shapeFilterBlockParams[SHAPE_FILTER_COUNT_MAX+1];
    int gainSequenceCount;
    /* --- derived data --- */
    int gainSetParamsIndexForGainSequence[SEQUENCE_COUNT_MAX];
#endif /* MPEG_D_DRC_EXTENSION_V1 */
#if AMD1_SYNTAX
    int gainSetCountPlus; /* includes information from drcCoefficientsParametricDrc */
    /* -------------------- */
#endif /* AMD1_SYNTAX */
} DrcCoefficientsUniDrc;

typedef struct {
    int drcSetId;
    int drcLocation;
    int downmixIdCount;
    int downmixId[DOWNMIX_ID_COUNT_MAX];
    int drcSetEffect;
    int limiterPeakTargetPresent;
    float limiterPeakTarget;
    int drcSetTargetLoudnessPresent;
    int drcSetTargetLoudnessValueUpper;
    int drcSetTargetLoudnessValueLowerPresent;
    int drcSetTargetLoudnessValueLower;
} DrcInstructionsBasic;

typedef struct {
    int drcSetId;
#if MPEG_D_DRC_EXTENSION_V1
    int drcSetComplexityLevel;
    int requiresEq;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    int drcApplyToDownmix;
    int drcLocation;
    int downmixIdCount;
    int downmixId[DOWNMIX_ID_COUNT_MAX];
    int dependsOnDrcSetPresent;
    int dependsOnDrcSet;
    int noIndependentUse;
    int drcSetEffect;
    int gainSetIndex[CHANNEL_COUNT_MAX];
    GainModifiers gainModifiersForChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    DuckingModifiers duckingModifiersForChannel[CHANNEL_COUNT_MAX];
    int limiterPeakTargetPresent;
    float limiterPeakTarget;
    int drcSetTargetLoudnessPresent;
    int drcSetTargetLoudnessValueUpper;
    int drcSetTargetLoudnessValueLowerPresent;
    int drcSetTargetLoudnessValueLower;
#if MPEG_H_SYNTAX
    int drcInstructionsType;
    int mae_groupID;
    int mae_groupPresetID;
#endif /* MPEG_H_SYNTAX */
    
    /* derived data */
    int audioChannelCount; /* could be different from drcChannelCount, e.g. for DRC sets with downmixId=0x7F */
    int nDrcChannelGroups;
    int gainSetIndexForChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    int bandCountForChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    int gainInterpolationTypeForChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    int timeDeltaMinForChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    int timeAlignmentForChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    DuckingModifiers duckingModifiersForChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    int channelGroupForChannel[CHANNEL_COUNT_MAX];
    int nChannelsPerChannelGroup[CHANNEL_GROUP_COUNT_MAX];
    int gainElementCount;               /* number of different DRC gains inluding all DRC bands*/
    int multibandAudioSignalCount;      /* number of audio signals including all channels with all subbands*/    
#if AMD1_SYNTAX
    int channelGroupIsParametricDrc[CHANNEL_GROUP_COUNT_MAX];
    int gainSetIndexForChannelGroupParametricDrc[CHANNEL_GROUP_COUNT_MAX];
    int parametricDrcLookAheadSamples[CHANNEL_GROUP_COUNT_MAX];
    int parametricDrcLookAheadSamplesMax;
#endif /* AMD1_SYNTAX */
} DrcInstructionsUniDrc;

typedef struct {
    int methodDefinition;
    float methodValue;
    int measurementSystem;
    int reliability;
} LoudnessMeasure;

typedef struct {
    int drcSetId;
#if MPEG_D_DRC_EXTENSION_V1
    int eqSetId;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    int downmixId;
    int samplePeakLevelPresent;
    float samplePeakLevel;
    int truePeakLevelPresent;
    float truePeakLevel;
    int truePeakLevelMeasurementSystem;
    int truePeakLevelReliability;
    int measurementCount;
    LoudnessMeasure loudnessMeasure[MEASUREMENT_COUNT_MAX];
#if MPEG_H_SYNTAX
    int loudnessInfoType;
    int mae_groupID;
    int mae_groupPresetID;
#endif /* MPEG_H_SYNTAX */
} LoudnessInfo;
    
#if MPEG_D_DRC_EXTENSION_V1
typedef struct {
    int loudEqSetId;
    int drcLocation;
    int downmixIdCount;
    int downmixId [DOWNMIX_ID_COUNT_MAX];
    int drcSetIdCount;
    int drcSetId [DRC_SET_ID_COUNT_MAX];
    int eqSetIdCount;
    int eqSetId [EQ_SET_ID_COUNT_MAX];
    int loudnessAfterDrc;
    int loudnessAfterEq;
    int loudEqGainSequenceCount;
    int gainSequenceIndex               [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
    int drcCharacteristicFormatIsCICP   [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
    int drcCharacteristic               [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
    int drcCharacteristicLeftIndex      [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
    int drcCharacteristicRightIndex     [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
    int frequencyRangeIndex             [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
    float loudEqScaling                 [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
    float loudEqOffset                  [LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX];
} LoudEqInstructions;

typedef struct {
    int filterElementIndex;
    int filterElementGainPresent;
    float filterElementGain;
} FilterElement;

typedef struct {
    int filterElementCount;
    FilterElement filterElement[FILTER_ELEMENT_COUNT_MAX];
} FilterBlock;

typedef struct {
    int eqFilterFormat;
    int realZeroRadiusOneCount;
    int realZeroCount;
    int genericZeroCount;
    int realPoleCount;
    int complexPoleCount;
    int zeroSign            [REAL_ZERO_RADIUS_ONE_COUNT_MAX];
    float realZeroRadius    [REAL_ZERO_COUNT_MAX];
    float genericZeroRadius [COMPLEX_ZERO_COUNT_MAX];
    float genericZeroAngle  [COMPLEX_ZERO_COUNT_MAX];
    float realPoleRadius    [REAL_POLE_COUNT_MAX];
    float complexPoleRadius [COMPLEX_POLE_COUNT_MAX];
    float complexPoleAngle  [COMPLEX_POLE_COUNT_MAX];
    int firFilterOrder;
    int firSymmetry;
    float firCoefficient [FIR_ORDER_MAX/2];
} UniqueTdFilterElement;

typedef struct {
    int nEqNodes;
    float eqSlope       [EQ_NODE_COUNT_MAX];
    int eqFreqDelta     [EQ_NODE_COUNT_MAX];
    float eqGainInitial;
    float eqGainDelta   [EQ_NODE_COUNT_MAX];
} EqSubbandGainSpline;

typedef struct {
    float eqSubbandGain [EQ_SUBBAND_GAIN_COUNT_MAX];
} EqSubbandGainVector;

typedef struct {
    int eqDelayMaxPresent;
    int eqDelayMax;
    int uniqueFilterBlockCount;
    FilterBlock filterBlock [FILTER_BLOCK_COUNT_MAX];
    int uniqueTdFilterElementCount;
    UniqueTdFilterElement uniqueTdFilterElement [FILTER_ELEMENT_COUNT_MAX];
    int uniqueEqSubbandGainsCount;
    int eqSubbandGainRepresentation;
    int eqSubbandGainFormat;
    int eqSubbandGainCount;
    EqSubbandGainSpline eqSubbandGainSpline   [UNIQUE_SUBBAND_GAIN_COUNT_MAX];
    EqSubbandGainVector eqSubbandGainVector [UNIQUE_SUBBAND_GAIN_COUNT_MAX];
} EqCoefficients;

typedef struct {
    int filterBlockCount;
    int filterBlockIndex[EQ_FILTER_BLOCK_COUNT_MAX];
} FilterBlockRefs;

typedef struct {
    int eqCascadeGainPresent        [EQ_CHANNEL_GROUP_COUNT_MAX];
    float eqCascadeGain             [EQ_CHANNEL_GROUP_COUNT_MAX];
    FilterBlockRefs filterBlockRefs [EQ_CHANNEL_GROUP_COUNT_MAX];
    int eqPhaseAlignmentPresent;
    int eqPhaseAlignment [EQ_CHANNEL_GROUP_COUNT_MAX][EQ_CHANNEL_GROUP_COUNT_MAX];
} TdFilterCascade;

typedef struct {
    int eqSetId;
    int eqSetComplexityLevel;
    int downmixIdCount;
    int downmixId[DOWNMIX_ID_COUNT_MAX];
    int eqApplyToDownmix;
    int drcSetIdCount;
    int drcSetId[DRC_SET_ID_COUNT_MAX];
    int eqSetPurpose;
    int dependsOnEqSetPresent;
    int dependsOnEqSet;
    int noIndependentEqUse;
    int eqChannelCount;
    int eqChannelGroupCount;
    int eqChannelGroupForChannel [CHANNEL_COUNT_MAX];
    int tdFilterCascadePresent;
    TdFilterCascade tdFilterCascade;
    int subbandGainsPresent;
    int subbandGainsIndex [EQ_CHANNEL_GROUP_COUNT_MAX];
    int eqTransitionDurationPresent;
#if AMD2_COR2
    float eqTransitionDuration;
#else
    int eqTransitionDuration;
#endif
} EqInstructions;
#endif /* MPEG_D_DRC_EXTENSION_V1 */

typedef struct {
    int uniDrcConfigExtType[EXT_COUNT_MAX];
    int extBitSize[EXT_COUNT_MAX-1];
    
#if AMD1_SYNTAX
    int parametricDrcPresent;
    DrcCoefficientsParametricDrc drcCoefficientsParametricDrc;
    int parametricDrcInstructionsCount;
    ParametricDrcInstructions parametricDrcInstructions[PARAM_DRC_INSTRUCTIONS_COUNT_MAX];
#endif /* AMD1_SYNTAX */
#if MPEG_D_DRC_EXTENSION_V1
    int drcExtensionV1Present;
    int loudEqInstructionsPresent;
    int loudEqInstructionsCount;
    LoudEqInstructions loudEqInstructions [LOUD_EQ_INSTRUCTIONS_COUNT_MAX];
    int eqPresent;
    EqCoefficients eqCoefficients;
    int eqInstructionsCount;
    EqInstructions eqInstructions [EQ_INSTRUCTIONS_COUNT_MAX];
#endif /* MPEG_D_DRC_EXTENSION_V1 */
} UniDrcConfigExt;

typedef struct T_UNI_DRC_CONFIG_STRUCT{
    int sampleRatePresent;
    int sampleRate;
    int downmixInstructionsCount;
    int drcCoefficientsUniDrcCount;
    int drcInstructionsUniDrcCount;
    int drcInstructionsCountPlus; /* includes (pseudo) DRC instructions for all configurations with DRC off */
    int drcDescriptionBasicPresent;
    int drcCoefficientsBasicCount;
    int drcInstructionsBasicCount;
    int uniDrcConfigExtPresent;
    int applyDrc;
    UniDrcConfigExt uniDrcConfigExt;
    DrcCoefficientsBasic drcCoefficientsBasic[DRC_COEFF_COUNT_MAX];
    DrcInstructionsBasic drcInstructionsBasic[DRC_INSTRUCTIONS_COUNT_MAX];
    DrcCoefficientsUniDrc drcCoefficientsUniDrc[DRC_COEFF_COUNT_MAX];
    DrcInstructionsUniDrc drcInstructionsUniDrc[DRC_INSTRUCTIONS_COUNT_MAX];
    ChannelLayout channelLayout;
    DownmixInstructions downmixInstructions[DOWNMIX_INSTRUCTION_COUNT_MAX];
#if MPEG_H_SYNTAX
    int mpegh3daDownmixInstructionsCount;
    DownmixInstructions mpegh3daDownmixInstructions[DOWNMIX_INSTRUCTION_COUNT_MAX];
    int loudnessInfoSetPresent;
#endif /* MPEG_H_SYNTAX */
} UniDrcConfig;

typedef struct {
    int loudnessInfoSetExtType[EXT_COUNT_MAX];
    int extBitSize[EXT_COUNT_MAX-1];
} LoudnessInfoSetExt;

typedef struct T_LOUDNESS_INFO_SET_STRUCT{
    int loudnessInfoAlbumCount;
    int loudnessInfoCount;
    int loudnessInfoSetExtPresent;
    LoudnessInfo loudnessInfoAlbum[LOUDNESS_INFO_COUNT_MAX];
    LoudnessInfo loudnessInfo[LOUDNESS_INFO_COUNT_MAX];
    LoudnessInfoSetExt loudnessInfoSetExt;
#if MPEG_H_SYNTAX
    int loudnessInfoAlbumPresent;
#endif /* MPEG_H_SYNTAX */
} LoudnessInfoSet;

typedef struct {
    float gainDb;
    float slope;
    int time;
} Node;

typedef struct {
    int drcGainCodingMode;
    int nNodes;
    Node node[NODE_COUNT_MAX];
} SplineNodes;

typedef struct {
#if MPEG_D_DRC_EXTENSION_V1
    SplineNodes splineNodes [1];
#else /* !MPEG_D_DRC_EXTENSION_V1 */
    int bandCount;
    SplineNodes splineNodes[BAND_COUNT_MAX];
#endif /* MPEG_D_DRC_EXTENSION_V1 */
} DrcGainSequence;

typedef struct {
    int uniDrcGainExtType[EXT_COUNT_MAX];
    int extBitSize[EXT_COUNT_MAX-1];
} UniDrcGainExt;

typedef struct T_UNI_DRC_GAIN_STRUCT{
    int nDrcGainSequences;
    DrcGainSequence drcGainSequence[SEQUENCE_COUNT_MAX];
    int uniDrcGainExtPresent;
    UniDrcGainExt uniDrcGainExt;
} UniDrcGain;

#ifdef __cplusplus
}
#endif
#endif
