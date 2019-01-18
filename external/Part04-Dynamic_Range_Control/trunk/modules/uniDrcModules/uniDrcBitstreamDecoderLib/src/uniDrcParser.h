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

#ifndef _UNI_DRC_PARSER_H_
#define _UNI_DRC_PARSER_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* ====================================================================================
 Get bits from robitbuf
 ====================================================================================*/

int
getBits (robitbufHandle bitstream,
         const int nBitsRequested,
         int* result);

/* ====================================================================================
 Parsing of in-stream DRC configuration
 ====================================================================================*/
int
decDuckingScaling(robitbufHandle bitstream,
                  int* duckingScalingPresent,
                  float* duckingScaling);

int
decGainModifiers(robitbufHandle bitstream,
                 const int version,
                 const int bandCount,
                 GainModifiers* gainModifiers);

int
decMethodValue(robitbufHandle bitstream,
               const int methodDefinition,
               float* methodValue);

int
parseLoudnessMeasure(robitbufHandle bitstream,
                     LoudnessMeasure* loudnessMeasure);

int
parseLoudnessInfo(robitbufHandle bitstream,
                  const int version,
                  LoudnessInfo* loudnessInfo);


int
parseLoudnessInfoSetExtension(robitbufHandle bitstream,
                              LoudnessInfoSet* loudnessInfoSet);
    
int
selectDrcCoefficients(UniDrcConfig* uniDrcConfig,
                      const int location,
                      DrcCoefficientsUniDrc** drcCoefficientsUniDrc);

int
generateDrcInstructionsDerivedData(UniDrcConfig* uniDrcConfig,
                                   DrcParamsBsDec* drcParams,
                                   DrcInstructionsUniDrc* drcInstructionsUniDrc);

int
generateDrcInstructionsForDrcOff(UniDrcConfig* uniDrcConfig, DrcParamsBsDec* drcParams);

int
parseDrcInstructionsBasic(robitbufHandle bitstream,
                          UniDrcConfig* uniDrcConfig,
                          DrcInstructionsBasic* drcInstructionsBasic);

int
parseDrcInstructionsUniDrc( robitbufHandle bitstream,
                            const int version,
                            UniDrcConfig* uniDrcConfig,
                            ChannelLayout* channelLayout,
                            DrcParamsBsDec* drcParams,
                            DrcInstructionsUniDrc* drcInstructionsUniDrc);

int
parseGainSetParamsCharacteristics(robitbufHandle bitstream,
                                  const int version,
                                  GainParams* gainParams);

int
parseGainSetParamsCrossoverFreqIndex(robitbufHandle bitstream,
                                     GainParams* gainParams,
                                     int drcBandType);

int
parseGainSetParams(robitbufHandle bitstream,
                   const int version,
                   int* gainSequenceIndex,
                   GainSetParams* gainSetParams);

int
parseDrcCoefficientsBasic(robitbufHandle bitstream,
                          DrcCoefficientsBasic* drcCoefficientsBasic);

int
parseDrcCoefficientsUniDrc(robitbufHandle bitstream,
                           const int version,
                           DrcParamsBsDec* drcParams,
                           DrcCoefficientsUniDrc* drcCoefficientsUniDrc);

int
parseDownmixInstructions(robitbufHandle bitstream,
                         const int version,
                         DrcParamsBsDec* drcParams,
                         ChannelLayout* channelLayout,
                         DownmixInstructions* downmixInstructions);

#if MPEG_D_DRC_EXTENSION_V1
int
parseDrcExtensionV1(robitbufHandle bitstream,
                    DrcParamsBsDec* drcParams,
                    UniDrcConfig* uniDrcConfig,
                    UniDrcConfigExt* uniDrcConfigExt);
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    
int
parseChannelLayout(robitbufHandle bitstream,
                   DrcParamsBsDec* drcParams,
                   ChannelLayout* channelLayout);

#if MPEG_H_SYNTAX
int
parseMpegh3daLoudnessInfoSet(robitbufHandle bitstream,
                             LoudnessInfoSet* loudnessInfoSet);

int
parseMpegh3daUniDrcChannelLayout(robitbufHandle bitstream,
                                 DrcParamsBsDec* drcParams,
                                 ChannelLayout* channelLayout);
#endif /* MPEG_H_SYNTAX */

#if AMD1_SYNTAX
int
parametricDrcTypeFeedForwardInitializeDrcCurveParameters(int drcCharacteristic,
                                                         ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward);
    
int
parametricDrcTypeFeedForwardInitializeDrcGainSmoothParameters(int drcCharacteristic,
                                                              ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward);
    
int
parametricDrcTypeFeedForwardInitializeParameters(int drcCharacteristic,
                                                 int drcFrameSizeParametricDrc,
                                                 ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward);
    
int
parseParametricDrcTypeFeedForward(robitbufHandle bitstream,
                                  int drcFrameSizeParametricDrc,
                                  ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward);

#ifdef AMD1_PARAMETRIC_LIMITER
int
parseParametricDrcTypeLim(robitbufHandle bitstream,
                          ParametricDrcTypeLim* parametricDrcTypeLim);
#endif
    
int
parseParametricDrcInstructions(robitbufHandle bitstream,
                               int drcFrameSizeParametricDrc,
                               DrcParamsBsDec* drcParams,
                               ParametricDrcInstructions* parametricDrcInstructions);
    
int
parseParametricDrcGainSetParams(robitbufHandle bitstream,
                                UniDrcConfig* uniDrcConfig,
                                ParametricDrcGainSetParams* parametricDrcGainSetParams);
    
int
parseDrcCoefficientsParametricDrc(robitbufHandle bitstream,
                                  UniDrcConfig* uniDrcConfig,
                                  DrcCoefficientsParametricDrc* drcCoefficientsParametricDrc);
    
int
generateVirtualGainSetsForParametricDrc(UniDrcConfig* uniDrcConfig);
#endif /* AMD1_SYNTAX */
    
#if MPEG_D_DRC_EXTENSION_V1
int
parseLoudEqInstructions(robitbufHandle bitstream,
                        LoudEqInstructions* loudEqInstructions);

int
parseEqCoefficients(robitbufHandle bitstream,
                    EqCoefficients* eqCoefficients);

int
parseEqInstructions(robitbufHandle bitstream,
                    UniDrcConfig* uniDrcConfig,
                    EqInstructions* eqInstructions);
    
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    
int
parseUniDrcConfigExtension(robitbufHandle bitstream,
                           DrcParamsBsDec* drcParams,
                           UniDrcConfig* uniDrcConfig,
                           UniDrcConfigExt* uniDrcConfigExt);

int
parseUniDrcConfig(robitbufHandle bitstream,
                  DrcParamsBsDec* drcParams,
                  UniDrcConfig* uniDrcConfig
#if MPEG_H_SYNTAX
                  ,LoudnessInfoSet* loudnessInfoSet
#endif
                  );

int
parseLoudnessInfoSet( robitbufHandle bitstream,
                      DrcParamsBsDec* drcParams,
                      LoudnessInfoSet* loudnessInfoSet);

/* ====================================================================================
Parsing of ISOBMFF configuration
====================================================================================*/
#if ISOBMFF_SYNTAX    
#if AMD1_SYNTAX
#if !MPEG_H_SYNTAX
int
parseIsobmff( robitbufHandle bitstream,
              UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
              UniDrcConfig* uniDrcConfig,
              LoudnessInfoSet* loudnessInfoSet,
              int baseChannelCount);
#endif
#endif
#endif
/* ====================================================================================
 Parsing of DRC gain sequences
 ====================================================================================*/

int
decInitialGain(robitbufHandle bitstream,
               const int gainCodingProfile,
               float* gainInitial);

int
decGains(robitbufHandle bitstream,
         Tables* tables,
         const int nNodes,
         const int deltaTmin,
         const int gainCodingProfile,
         Node* node);

int
decSlopes(robitbufHandle bitstream,
          int* nNodes,
          const int gainInterpolationType,
          Node* node);

int
decTimes(robitbufHandle bitstream,
         Tables* tables,
         const int nNodes,
         const int deltaTmin,
         const int drcFrameSize,
         const int fullFrame,
         const int timeOffset,
         Node* node);

int
parseSplineNodes(robitbufHandle bitstream,
                 UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                 GainSetParams* gainSetParams,
                 SplineNodes* splineNodes);

int
parseDrcGainSequence(robitbufHandle bitstream,
                     UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                     GainSetParams* gainSetParams,
                     DrcGainSequence* drcGainSequence);

int
parseUniDrcGainExtension(robitbufHandle bitstream,
                         UniDrcGainExt* uniDrcGainExt);

int
parseUniDrcGain(robitbufHandle bitstream,
                UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                UniDrcConfig* uniDrcConfig,
                UniDrcGain* uniDrcGain);

#ifdef __cplusplus
}
#endif
#endif
