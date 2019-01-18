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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "uniDrcBitstreamDecoder.h"
#include "uniDrcParser.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
    
/* initDrcParams */
int
initDrcParamsBsDec(const int audioFrameSize,
                   const int audioSampleRate,
                   const int delayMode,
                   const int lfeChannelMapCount,
                   const int* lfeChannelMap,
                   DrcParamsBsDec* drcParams)
{
    int i;

    if (audioFrameSize < 1)
    {
        fprintf(stderr, "ERROR: the audio frame size must be positive.\n");
        return(PARAM_ERROR);
    }
    
    if (audioSampleRate < 1000)
    {
        fprintf(stderr, "ERROR: audio sample rates below 1000 Hz are not supported.\n");
        return(PARAM_ERROR);
    }
#if AMD2_COR2
    drcParams->sampleRateDefault = audioSampleRate;
#endif
    
    if (audioFrameSize > AUDIO_CODEC_FRAME_SIZE_MAX)
    {
        fprintf(stderr, "ERROR: the audio frame size is too large for this implementation: %d.\n", audioFrameSize);
        fprintf(stderr, "Increase the value of AUDIO_CODEC_FRAME_SIZE_MAX.\n");
        return(PARAM_ERROR);
    }

    drcParams->drcFrameSize = audioFrameSize;
    
    if (drcParams->drcFrameSize < 0.001f * audioSampleRate)
    {
        fprintf(stderr, "ERROR: DRC frame size must be at least 1ms.\n");
        return(PARAM_ERROR);
    }

    drcParams->deltaTminDefault = getDeltaTmin(audioSampleRate);
    if (drcParams->deltaTminDefault > drcParams->drcFrameSize)
    {
        fprintf(stderr, "ERROR: DRC time interval (deltaTmin) cannot exceed audio frame size. %d %d\n", drcParams->deltaTminDefault, drcParams->drcFrameSize);
        return(PARAM_ERROR);
    }

    drcParams->nGainValuesMaxDefault = drcParams->drcFrameSize / drcParams->deltaTminDefault;
    drcParams->delayMode = delayMode;
    
    if (lfeChannelMapCount >= 0) {
        if ((lfeChannelMap == NULL) || (lfeChannelMapCount > CHANNEL_COUNT_MAX))
            return(PARAM_ERROR);
        drcParams->lfeChannelMapCount = lfeChannelMapCount;
        for (i=0; i<lfeChannelMapCount; i++) {
            drcParams->lfeChannelMap[i] = lfeChannelMap[i];
        }
    } else {
        drcParams->lfeChannelMapCount = -1;
        for (i=0; i<CHANNEL_COUNT_MAX; i++) {
            drcParams->lfeChannelMap[i] = 0;
        }
    }
    return (0);
}
    
/* open */
int
openUniDrcBitstreamDec(HANDLE_UNI_DRC_BS_DEC_STRUCT *phUniDrcBsDecStruct,
                       HANDLE_UNI_DRC_CONFIG *phUniDrcConfig,
                       HANDLE_LOUDNESS_INFO_SET *phLoudnessInfoSet,
                       HANDLE_UNI_DRC_GAIN *phUniDrcGain)
{
    int err = 0;
    UNI_DRC_BS_DEC_STRUCT *hUniDrcBsDrcStruct = NULL;
    hUniDrcBsDrcStruct = (HANDLE_UNI_DRC_BS_DEC_STRUCT)calloc(1,sizeof(struct T_UNI_DRC_BS_DEC_STRUCT));
    
    memset(&hUniDrcBsDrcStruct->bitstream, 0, sizeof(robitbuf));
    memset(&hUniDrcBsDrcStruct->drcParams, 0, sizeof(DrcParamsBsDec));
    memset(&hUniDrcBsDrcStruct->tablesDefault, 0, sizeof(Tables));
    
    *phUniDrcBsDecStruct = hUniDrcBsDrcStruct;
    
    if ( *phUniDrcConfig == NULL)
    {
        UniDrcConfig *hUniDrcConfig = NULL;
        hUniDrcConfig = (HANDLE_UNI_DRC_CONFIG)calloc(1,sizeof(struct T_UNI_DRC_CONFIG_STRUCT));
        
        *phUniDrcConfig = hUniDrcConfig;
    }
    
    if ( *phLoudnessInfoSet == NULL)
    {
        LoudnessInfoSet *hLoudnessInfoSet = NULL;
        hLoudnessInfoSet = (HANDLE_LOUDNESS_INFO_SET)calloc(1,sizeof(struct T_LOUDNESS_INFO_SET_STRUCT));
        
        *phLoudnessInfoSet = hLoudnessInfoSet;
    }
    if ( *phUniDrcGain == NULL)
    {
        UniDrcGain *hUniDrcGain = NULL;
        hUniDrcGain = (HANDLE_UNI_DRC_GAIN)calloc(1,sizeof(struct T_UNI_DRC_GAIN_STRUCT));
        
        *phUniDrcGain = hUniDrcGain;
    }

    return err;
}
    
/* init */
int
initUniDrcBitstreamDec(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                       const int audioSampleRate,
                       const int audioFrameSize,
                       const int delayMode,
                       const int lfeChannelMapCount,
                       const int* lfeChannelMap)
{
    int err = 0;

    err = initDrcParamsBsDec(audioFrameSize,
                             audioSampleRate,
                             delayMode,
                             lfeChannelMapCount,
                             lfeChannelMap,
                             &hUniDrcBsDecStruct->drcParams);
    if (err) return (err);
    err = initTables((hUniDrcBsDecStruct->drcParams).nGainValuesMaxDefault, &hUniDrcBsDecStruct->tablesDefault);
    if (err) return (err);
    
    return 0;
}

/* process uniDrc (without uniDrcGain() payload) */
int
processUniDrcBitstreamDec_uniDrc(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                 HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                 HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                 unsigned char* bitstreamConfig,
                                 const int nBytes,
                                 const int nBitsOffset,
                                 int* nBitsRead)
{
    int err = 0;
    int loudnessInfoSetPresent, uniDrcConfigPresent;
    
    robitbufHandle bitstream = &hUniDrcBsDecStruct->bitstream;
    
    if (bitstreamConfig == NULL)
    {
        *nBitsRead = 0;
    }
    else
    {
        /* robitbuf */
        if (bitstream != NULL && nBytes) {
            robitbuf_Init(bitstream, bitstreamConfig, nBytes * 8, 0);
        } else {
            return -1;
        }
        
        /* skip offset */
        err = getBits(bitstream, nBitsOffset, NULL);
        if (err) return(err);
        
        err = getBits(bitstream, 1, &loudnessInfoSetPresent);
        if (err == PROC_COMPLETE)
        {
            loudnessInfoSetPresent = 0;     /* keep going until end of audio input*/
        }
        else
        {
            if (err) return(err);
        }
        if (loudnessInfoSetPresent)
        {
            err = getBits(bitstream, 1, &uniDrcConfigPresent);
            if (err == PROC_COMPLETE)
            {
                uniDrcConfigPresent = 0;     /* keep going until end of audio input*/
            }
            else
            {
                if (err) return(err);
            }
            if (uniDrcConfigPresent)
            {
#if MPEG_H_SYNTAX
                /* Read in-stream configuration */
                err = parseUniDrcConfig(bitstream, &hUniDrcBsDecStruct->drcParams, hUniDrcConfig, hLoudnessInfoSet);
                if (err) return (err);
#else
                /* Read in-stream configuration */
                err = parseUniDrcConfig(bitstream, &hUniDrcBsDecStruct->drcParams, hUniDrcConfig);
                if (err) return (err);
#endif
#if AMD2_COR2
            } else {
                
                err = generateDrcInstructionsForDrcOff(hUniDrcConfig, &hUniDrcBsDecStruct->drcParams);
                if (err) return(err);
                hUniDrcConfig->channelLayout.baseChannelCount = -1;
#endif
            }
            /* Read in-stream loudnessInfoSet */
            err = parseLoudnessInfoSet(bitstream, &hUniDrcBsDecStruct->drcParams, hLoudnessInfoSet);
            if (err) return (err);
        }
        
        *nBitsRead =robitbuf_GetBitsRead(bitstream);
    }
    
    return err;
    
}

/* process uniDrcConfig */
int
processUniDrcBitstreamDec_uniDrcConfig(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                       HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                       HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                       unsigned char* bitstreamConfig,
                                       const int nBytes,
                                       const int nBitsOffset,
                                       int* nBitsRead)
{
    int err = 0;
    
    robitbufHandle bitstream = &hUniDrcBsDecStruct->bitstream;
    
    if (bitstreamConfig == NULL)
    {
        *nBitsRead = 0;
#if AMD2_COR2
        err = generateDrcInstructionsForDrcOff(hUniDrcConfig, &hUniDrcBsDecStruct->drcParams);
        if (err) return(err);
        hUniDrcConfig->channelLayout.baseChannelCount = -1;
#endif
    }
    else
    {
        /* robitbuf */
        if (bitstream != NULL && nBytes) {
            robitbuf_Init(bitstream, bitstreamConfig, nBytes * 8, 0);
        } else {
            return -1;
        }
        
        /* skip offset */
        err = getBits(bitstream, nBitsOffset, NULL);
        if (err) return(err);
        
#if MPEG_H_SYNTAX
        /* read config */
        err = parseUniDrcConfig(bitstream, &hUniDrcBsDecStruct->drcParams, hUniDrcConfig, hLoudnessInfoSet);
        if (err) return (err);
#else
        /* read config */
        err = parseUniDrcConfig(bitstream, &hUniDrcBsDecStruct->drcParams, hUniDrcConfig);
        if (err) return (err);
#endif

        *nBitsRead =robitbuf_GetBitsRead(bitstream);
    }

    return err;
    
}
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
                                      int* nBitsReadIsobmff)
{
    int err = 0;
    
    robitbufHandle bitstream = &hUniDrcBsDecStruct->bitstream;
    /* robitbuf */
    if (bitstreamIsobmff != NULL && nBytesIsobmff) {
        robitbuf_Init(bitstream, bitstreamIsobmff, nBytesIsobmff * 8, 0);
    } else {
        return -1;
    }
    
    /* skip offset */
    err = getBits(bitstream, nBitsOffsetIsobmff, NULL);
    if (err) return(err);

    err = parseIsobmff(bitstream, hUniDrcBsDecStruct, hUniDrcConfig, hLoudnessInfoSet, baseChannelCount);
    if (err) return(err);
    
    *nBitsReadIsobmff = robitbuf_GetBitsRead(bitstream);
    
    return 0;
}
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
                                     int* nBitsRead)
{
    int err = 0;
    int procComplete = FALSE;
    robitbufHandle bitstream = &hUniDrcBsDecStruct->bitstream;

    /* robitbuf */
    if (bitstream != NULL && nBytes) {
        robitbuf_Init(bitstream, bitstreamGain, nBytes * 8, 0);
    } else {
        return -1;
    }

    /* skip offset */
    err = getBits(bitstream, nBitsOffset, NULL);
    if (err) return(err);
    
    /* read gain sequences from bitstream and decode nodes */
    err = parseUniDrcGain(bitstream,
                          hUniDrcBsDecStruct,
                          hUniDrcConfig,
                          hUniDrcGain);
    if (err > PROC_COMPLETE) return (err);
    if (err == PROC_COMPLETE)
    {
        procComplete = TRUE;
    }
    
    *nBitsRead =robitbuf_GetBitsRead(bitstream);
    
    if (procComplete==TRUE) return (PROC_COMPLETE);
    return 0;
}

#if MPEG_H_SYNTAX
int
setMpeghDownmixMatrixSetParamsUniDrcSelectionProcess(HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                                     int downmixIdCount,
                                                     int* downmixId,
                                                     int* targetChannelCountForDownmixId)
{
    
    int i;
    
    hUniDrcConfig->mpegh3daDownmixInstructionsCount = downmixIdCount;
    for (i=0; i<downmixIdCount; i++) {
        hUniDrcConfig->mpegh3daDownmixInstructions[i].downmixId                  = downmixId[i];
        hUniDrcConfig->mpegh3daDownmixInstructions[i].targetLayout               = 0;
        hUniDrcConfig->mpegh3daDownmixInstructions[i].targetChannelCount         = targetChannelCountForDownmixId[i];
        hUniDrcConfig->mpegh3daDownmixInstructions[i].downmixCoefficientsPresent = 0;
    }
    
    return 0;
}
#endif

/* process loudnessInfoSet */
int processUniDrcBitstreamDec_loudnessInfoSet(HANDLE_UNI_DRC_BS_DEC_STRUCT hUniDrcBsDecStruct,
                                              HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                                              HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                                              unsigned char* bitstreamLoudness,
                                              const int nBytesLoudness,
                                              const int nBitsOffsetLoudness,
                                              int* nBitsReadLoudness)
{
    int err = 0;

#if MPEG_H_SYNTAX
    if (hUniDrcConfig->loudnessInfoSetPresent == 0)
    {
        robitbufHandle bitstream = &hUniDrcBsDecStruct->bitstream;
        /* robitbuf */
        if (bitstreamLoudness != NULL && nBytesLoudness) {
            robitbuf_Init(bitstream, bitstreamLoudness, nBytesLoudness * 8, 0);
        } else {
            return -1;
        }
        
        /* skip offset */
        err = getBits(bitstream, nBitsOffsetLoudness, NULL);
        if (err) return(err);

        err = parseMpegh3daLoudnessInfoSet(bitstream, hLoudnessInfoSet);
        if (err) return(err);

        *nBitsReadLoudness =robitbuf_GetBitsRead(bitstream);
    }
#else
    robitbufHandle bitstream = &hUniDrcBsDecStruct->bitstream;
    /* robitbuf */
    if (bitstreamLoudness != NULL && nBytesLoudness) {
        robitbuf_Init(bitstream, bitstreamLoudness, nBytesLoudness * 8, 0);
    } else {
        return -1;
    }
        
    /* skip offset */
    err = getBits(bitstream, nBitsOffsetLoudness, NULL);
    if (err) return(err);
    
    err = parseLoudnessInfoSet(bitstream, &hUniDrcBsDecStruct->drcParams, hLoudnessInfoSet);
    if (err) return(err);
        
    *nBitsReadLoudness =robitbuf_GetBitsRead(bitstream);
#endif
    return 0;
}

/* close */
int closeUniDrcBitstreamDec(HANDLE_UNI_DRC_BS_DEC_STRUCT *phUniDrcBsDecStruct,
                            HANDLE_UNI_DRC_CONFIG *phUniDrcConfig,
                            HANDLE_LOUDNESS_INFO_SET *phLoudnessInfoSet,
                            HANDLE_UNI_DRC_GAIN *phUniDrcGain)
{
    if ( *phUniDrcBsDecStruct != NULL )
    {
        free( *phUniDrcBsDecStruct );
        *phUniDrcBsDecStruct = NULL;
    }
    if ( *phUniDrcConfig != NULL )
    {
        free( *phUniDrcConfig );
        *phUniDrcConfig = NULL;
    }
    if ( *phLoudnessInfoSet != NULL )
    {
        free( *phLoudnessInfoSet );
        *phLoudnessInfoSet = NULL;
    }
    if ( *phUniDrcGain != NULL )
    {
        free( *phUniDrcGain );
        *phUniDrcGain = NULL;
    }
    return 0;
}

/* plot info */
void plotInfoBS( HANDLE_UNI_DRC_CONFIG hUniDrcConfig,
                HANDLE_UNI_DRC_GAIN hUniDrcGain,
                HANDLE_LOUDNESS_INFO_SET hLoudnessInfoSet,
                int frameNo, int nBytes, int plotInfo)
{
    int i, j, k;
#if MPEG_D_DRC_EXTENSION_V1 || AMD1_SYNTAX
    int n;
#endif
    printf("frame #%d decoding ... (nBytes = %d)\n",frameNo, nBytes);
    if (plotInfo==1 || plotInfo == 3 ) {
        printf("uniDrcConfig():\n");
        printf("     sampleRate = %d\n",hUniDrcConfig->sampleRate);
        printf("     channelCount = %d\n",hUniDrcConfig->channelLayout.baseChannelCount);
        printf("     downmixInstructionsCount = %d\n",hUniDrcConfig->downmixInstructionsCount);
        printf("     drcCoefficientsUniDrcCount = %d\n",hUniDrcConfig->drcCoefficientsUniDrcCount);
        printf("     drcInstructionsUniDrcCount = %d\n",hUniDrcConfig->drcInstructionsUniDrcCount);
        printf("     loudnessInfoAlbumCount = %d\n",hLoudnessInfoSet->loudnessInfoAlbumCount);
        printf("     loudnessInfoCount = %d\n",hLoudnessInfoSet->loudnessInfoCount);
    }
    if (plotInfo == 2 || plotInfo == 3) {
        printf("uniDrcGain():\n");
#if MPEG_D_DRC_EXTENSION_V1 || AMD1_SYNTAX
        n = 0;
        for (i=0; i<hUniDrcConfig->drcCoefficientsUniDrc[0].gainSetCount; i++) {
            if (hUniDrcConfig->drcCoefficientsUniDrc[0].gainSetParams[i].gainCodingProfile == 3) {
                printf("     seq #%d --> codingProfile = %d\n", i, hUniDrcConfig->drcCoefficientsUniDrc[i].gainSetParams[i].gainCodingProfile);
            } else {
                for (j=0; j<hUniDrcConfig->drcCoefficientsUniDrc[0].gainSetParams[i].bandCount; j++) {
                    printf("     seq #%d, band #%d --> codingProfile = %d, codingMode = %d, nNodes = %d: \n", i,j, hUniDrcConfig->drcCoefficientsUniDrc[0].gainSetParams[i].gainCodingProfile, hUniDrcGain->drcGainSequence[n].splineNodes[j].drcGainCodingMode, hUniDrcGain->drcGainSequence[n].splineNodes[j].nNodes);
                    for (k=0; k<hUniDrcGain->drcGainSequence[n].splineNodes[j].nNodes; k++) {
                        printf("          node #%d: gainDb = %3.2f, slope = %3.2f, time = %d\n",k,hUniDrcGain->drcGainSequence[n].splineNodes[j].node[k].gainDb,hUniDrcGain->drcGainSequence[n].splineNodes[j].node[k].slope,hUniDrcGain->drcGainSequence[n].splineNodes[j].node[k].time);
                    }
                    n++;
                }
            }
#else
            for (i=0; i<hUniDrcConfig->drcCoefficientsUniDrc[0].gainSetCount; i++) {
                if (hUniDrcConfig->drcCoefficientsUniDrc[i].gainSetParams[i].gainCodingProfile == 3) {
                    printf("     seq #%d --> codingProfile = %d\n", i, hUniDrcConfig->drcCoefficientsUniDrc[i].gainSetParams[i].gainCodingProfile);
                } else {
                    for (j=0; j<hUniDrcConfig->drcCoefficientsUniDrc[0].gainSetParams[i].bandCount; j++) {
                        printf("     seq #%d, band #%d --> codingProfile = %d, codingMode = %d, nNodes = %d: \n", i,j, hUniDrcConfig->drcCoefficientsUniDrc[i].gainSetParams[i].gainCodingProfile, hUniDrcGain->drcGainSequence[i].splineNodes[j].drcGainCodingMode, hUniDrcGain->drcGainSequence[i].splineNodes[j].nNodes);
                        for (k=0; k<hUniDrcGain->drcGainSequence[i].splineNodes[j].nNodes; k++) {
                            printf("          node #%d: gainDb = %3.2f, slope = %3.2f, time = %d\n",k,hUniDrcGain->drcGainSequence[i].splineNodes[j].node[k].gainDb,hUniDrcGain->drcGainSequence[i].splineNodes[j].node[k].slope,hUniDrcGain->drcGainSequence[i].splineNodes[j].node[k].time);
                        }
                    }
                }
#endif
            }
        }
    }


#ifdef __cplusplus
}
#endif /* __cplusplus */

