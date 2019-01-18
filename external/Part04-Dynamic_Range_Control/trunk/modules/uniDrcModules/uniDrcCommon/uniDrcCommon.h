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

#ifndef _UNI_DRC_COMMON_H_
#define _UNI_DRC_COMMON_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* ISOBMFF */
#define ISOBMFF_SYNTAX                  1

/* Corrigendum on Reference Software */
#define AMD2_COR1                       1
#define AMD2_COR2                       1
#define AMD2_COR3                       1

/* MPEG-D DRC AMD 1 */
#ifdef AMD1
#define AMD1_SYNTAX                     1
#define MPEG_D_DRC_EXTENSION_V1         1
#define AMD1_PARAMETRIC_LIMITER         1
#else
#define AMD1_SYNTAX                     0
#define MPEG_D_DRC_EXTENSION_V1         0
#define AMD1_PARAMETRIC_LIMITER         0
#endif
    
/* MPEG-H 3DA */
#ifdef MPEG_H
#define MPEG_H_SYNTAX                   1
#else
#define MPEG_H_SYNTAX                   0
#endif
    
#define EQ_IS_SUPPORTED                 1   /* Indicates if the decoder supports EQ processing (AMD1) */
    
#define MEASURE_AVERAGE_BITRATE         0
#define DEBUG_NODES                     0

#define DRC_GAIN_DEBUG_FILE             0   /*create a file with the DRC gains*/
#define DEBUG_BITSTREAM                 0   /*print bitstream bits written and read*/
#define DEBUG_DRC_SELECTION             0   /* display information about selected DRC sets */
#define DEBUG_WARNINGS                  0   /* display warnings */
#define ENABLE_ADDITIONAL_TESTS         1

#define SPEAKER_POS_COUNT_MAX           128
#define DOWNMIX_COEFF_COUNT_MAX         32*32  /*128*128 reduced to save memory */
#define CHANNEL_COUNT_MAX               128
#define BAND_COUNT_MAX                  8   /*16    reduced to save memory*/
#define SEQUENCE_COUNT_MAX              24  /*64    reduced to save memory*/
#define GAIN_SET_COUNT_MAX              SEQUENCE_COUNT_MAX
#define MEASUREMENT_COUNT_MAX           16
#define DOWNMIX_INSTRUCTION_COUNT_MAX   16
#define DRC_COEFF_COUNT_MAX             8
#define DRC_INSTRUCTIONS_COUNT_MAX      (DOWNMIX_INSTRUCTION_COUNT_MAX + 20)  /*64    reduced to save memory*/
#define LOUDNESS_INFO_COUNT_MAX         (DOWNMIX_INSTRUCTION_COUNT_MAX + 20)  /*64    reduced to save memory*/
#define AUDIO_CODEC_FRAME_SIZE_MAX      4096 /*covers AAC and HE-AAC and USAC */
#define DRC_CODEC_FRAME_SIZE_MAX        (AUDIO_CODEC_FRAME_SIZE_MAX/8)  /*assuming min sample rate of 8kHz*/
#define NODE_COUNT_MAX                  DRC_CODEC_FRAME_SIZE_MAX
#define CHANNEL_GROUP_COUNT_MAX         SEQUENCE_COUNT_MAX
#define SUB_DRC_COUNT                   4
#define SEL_DRC_COUNT                   3
#define DOWNMIX_ID_COUNT_MAX            8
#define MAX_SIGNAL_DELAY                4500    /* maximum signal delay in samples */
    
#if MPEG_D_DRC_EXTENSION_V1
#define DRC_BAND_COUNT_MAX              BAND_COUNT_MAX
#define SPLIT_CHARACTERISTIC_COUNT_MAX  8 /* reduced size */
#define GAIN_SET_COUNT_MAX              SEQUENCE_COUNT_MAX
#define SHAPE_FILTER_COUNT_MAX          8 /* reduced size */
#define DRC_SET_ID_COUNT_MAX            16
#define EQ_SET_ID_COUNT_MAX             8
#define LOUD_EQ_GAIN_SEQUENCE_COUNT_MAX 4
#define FILTER_ELEMENT_COUNT_MAX        16  /* reduced size */
#define REAL_ZERO_RADIUS_ONE_COUNT_MAX  14
#define REAL_ZERO_COUNT_MAX             64
#define COMPLEX_ZERO_COUNT_MAX          64
#define REAL_POLE_COUNT_MAX             16
#define COMPLEX_POLE_COUNT_MAX          16
#define FIR_ORDER_MAX                   128
#define EQ_NODE_COUNT_MAX               33
#define EQ_SUBBAND_GAIN_COUNT_MAX       135
#define UNIQUE_SUBBAND_GAIN_COUNT_MAX   16  /* reduced size */
#define FILTER_BLOCK_COUNT_MAX          16
#define FILTER_ELEMENT_COUNT_MAX        16  /* reduced size */
#define UNIQUE_SUBBAND_GAINS_COUNT_MAX  8   /* reduced size */
#define EQ_CHANNEL_GROUP_COUNT_MAX      4   /* reduced size */
#define EQ_FILTER_BLOCK_COUNT_MAX       4   /* reduced size */
#define LOUD_EQ_INSTRUCTIONS_COUNT_MAX  8   /* reduced size */
#define EQ_INSTRUCTIONS_COUNT_MAX       8
#define SELECTED_EQ_SET_COUNT_MAX       2
#define SUB_EQ_COUNT                    2
#endif /* MPEG_D_DRC_EXTENSION_V1 */

#define DELAY_MODE_REGULAR_DELAY        0
#define DELAY_MODE_LOW_DELAY            1
#define DELAY_MODE_DEFAULT              DELAY_MODE_REGULAR_DELAY

#define FEATURE_REQUEST_COUNT_MAX       10
#define EFFECT_TYPE_REQUEST_COUNT_MAX   10
    
#define SELECTION_CANDIDATE_COUNT_MAX   32

#define PROC_COMPLETE                   1
#define UNEXPECTED_ERROR                2
#define PARAM_ERROR                     3
#define EXTERNAL_ERROR                  4
#define ERRORHANDLING                   5
#define BITSTREAM_ERROR                 6

#define UNDEFINED_LOUDNESS_VALUE        1000.0f
#define ID_FOR_BASE_LAYOUT              0x0
#define ID_FOR_ANY_DOWNMIX              0x7F
#define ID_FOR_NO_DRC                   0x0
#define ID_FOR_ANY_DRC                  0x3F
#define ID_FOR_ANY_EQ                   0x3F
    
#define LOCATION_MP4_INSTREAM_UNIDRC    0x1
#define LOCATION_MP4_DYN_RANGE_INFO     0x2
#define LOCATION_MP4_COMPRESSION_VALUE  0x3
#define LOCATION_SELECTED               LOCATION_MP4_INSTREAM_UNIDRC  /* set to location selected by system */

/* SUBBAND DOMAIN */
#define AUDIO_CODEC_SUBBAND_COUNT_MAX   256

#define SUBBAND_DOMAIN_MODE_OFF         0
#define SUBBAND_DOMAIN_MODE_QMF64       1
#define SUBBAND_DOMAIN_MODE_QMF71       2
#define SUBBAND_DOMAIN_MODE_STFT256     3
    
/* QMF64 */
#define AUDIO_CODEC_SUBBAND_COUNT_QMF64                     64
#define AUDIO_CODEC_SUBBAND_DOWNSAMPLING_FACTOR_QMF64       64
#define AUDIO_CODEC_SUBBAND_ANALYSE_DELAY_QMF64             320
    
/* QMF71 (according to ISO/IEC 23003-1:2007) */
#define AUDIO_CODEC_SUBBAND_COUNT_QMF71                     71
#define AUDIO_CODEC_SUBBAND_DOWNSAMPLING_FACTOR_QMF71       64
#define AUDIO_CODEC_SUBBAND_ANALYSE_DELAY_QMF71             320+384
    
/* STFT256 (according to ISO/IEC 23008-3:2015/AMD3) */
#define AUDIO_CODEC_SUBBAND_COUNT_STFT256                   256
#define AUDIO_CODEC_SUBBAND_DOWNSAMPLING_FACTOR_STFT256     256
#define AUDIO_CODEC_SUBBAND_ANALYSE_DELAY_STFT256           256
    
/* uniDrcInterface */
#define MAX_NUM_DOWNMIX_ID_REQUESTS        15
#define MAX_NUM_DRC_FEATURE_REQUESTS       7
#define MAX_NUM_DRC_EFFECT_TYPE_REQUESTS   15
#define MAX_SIGNATURE_DATA_LENGTH_PLUS_ONE 256

/* extensions */
#define EXT_COUNT_MAX                   2
#define UNIDRCGAINEXT_TERM              0x0
#define UNIDRCLOUDEXT_TERM              0x0
#define UNIDRCCONFEXT_TERM              0x0
#define UNIDRCINTERFACEEXT_TERM         0x0

/* MPEG-D DRC AMD 1 */
#if AMD1_SYNTAX

#define UNIDRCCONFEXT_PARAM_DRC          0x1

#define PARAM_DRC_DEBUG                  0
#define PARAM_DRC_INSTRUCTIONS_COUNT_MAX 8
#define PARAM_DRC_INSTANCE_COUNT_MAX     8
    
#define PARAM_DRC_TYPE_FF                0x0
#define PARAM_DRC_TYPE_FF_NODE_COUNT_MAX 9
#define PARAM_DRC_TYPE_FF_LEVEL_ESTIM_FRAME_COUNT_MAX 64
    
#ifdef AMD1_PARAMETRIC_LIMITER
#define PARAM_DRC_TYPE_LIM                    0x1
#define PARAM_DRC_TYPE_LIM_THRESHOLD_DEFAULT  (-1.f)
#define PARAM_DRC_TYPE_LIM_ATTACK_DEFAULT     5
#define PARAM_DRC_TYPE_LIM_RELEASE_DEFAULT    50
#endif
    
#endif /* AMD1_SYNTAX */    
    
#if MPEG_D_DRC_EXTENSION_V1
#define UNIDRCCONFEXT_V1                0x2
#define UNIDRCLOUDEXT_EQ                0x1
#define UNIDRCINTERFACEEXT_EQ           0x1
    
#define LOUD_EQ_REQUEST_OFF             0x0
#define LOUD_EQ_REQUEST_LIGHT           0x1
#define LOUD_EQ_REQUEST_REGULAR         0x2
#define LOUD_EQ_REQUEST_HEAVY           0x3

#define EQ_PURPOSE_EQ_OFF               0
#define EQ_PURPOSE_DEFAULT              (1<<0)
#define EQ_PURPOSE_LARGE_ROOM           (1<<1)
#define EQ_PURPOSE_SMALL_SPACE          (1<<2)
#define EQ_PURPOSE_AVERAGE_ROOM         (1<<3)
#define EQ_PURPOSE_CAR_CABIN            (1<<4)
#define EQ_PURPOSE_HEADPHONES           (1<<5)
#define EQ_PURPOSE_LATE_NIGHT           (1<<6)
    
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    
/* MPEG-H */
#if MPEG_H_SYNTAX
#define MAX_NUM_GROUP_ID_REQUESTS        15
#define MAX_NUM_GROUP_PRESET_ID_REQUESTS 15
#define MAX_NUM_MEMBERS_GROUP_PRESET     15
#endif
    
#define MAXPACKETLOSSTIME               2.5f

#define SLOPE_FACTOR_DB_TO_LINEAR   0.1151f               /* ln(10) / 20 */

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef bool
#define bool int
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif
    
typedef struct T_UNI_DRC_SEL_PROC_OUTPUT{   
    float outputPeakLevelDb;
    float loudnessNormalizationGainDb;
    float outputLoudness;
        
    int selectedDrcSetIds[SUB_DRC_COUNT];
    int selectedDownmixIds[SUB_DRC_COUNT];
    int numSelectedDrcSets;
    
    int activeDownmixId;
    int baseChannelCount;
    int targetChannelCount;
    int targetLayout;
    int downmixMatrixPresent;
    float downmixMatrix[CHANNEL_COUNT_MAX][CHANNEL_COUNT_MAX];

    float boost;
    float compress;
    int drcCharacteristicTarget;
    
#if MPEG_H_SYNTAX
    int groupIdLoudnessCount;
    int groupId[LOUDNESS_INFO_COUNT_MAX];
    float groupIdLoudness[LOUDNESS_INFO_COUNT_MAX];
#endif

#if MPEG_D_DRC_EXTENSION_V1
    float mixingLevel;
    int selectedEqSetIds[SELECTED_EQ_SET_COUNT_MAX];
    int selectedLoudEqId;
#endif
} UniDrcSelProcOutput;

#ifdef __cplusplus
}
#endif
#endif
