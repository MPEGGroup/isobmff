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
#include <string.h>
#include <math.h>
#include "uniDrcBitstreamDecoder.h"
#include "uniDrcParser.h"

extern const float downmixCoeff[];
extern const float downmixCoeffLfe[];

#if AMD1_SYNTAX
extern const float channelWeight[];
#endif /* AMD1_SYNTAX */

#if MPEG_D_DRC_EXTENSION_V1
extern const float downmixCoeffV1[];
extern const float eqSlopeTable[];
extern const float eqGainDeltaTable[];
extern const float zeroPoleRadiusTable[];
extern const float zeroPoleAngleTable[];
#endif /* MPEG_D_DRC_EXTENSION_V1 */

/* ====================================================================================
                                Get bits from robitbuf
 ====================================================================================*/

int
getBits (robitbufHandle bitstream,
         const int nBitsRequested,
         int* result)
{
    int nBitsRemaining = robitbuf_GetBitsAvail(bitstream);

    if (nBitsRemaining<=0)
    {
        printf("Processing complete. Reached end of bitstream.\n");
        return (PROC_COMPLETE);
    }
    
    if (result != NULL) {
        *result = (int)robitbuf_ReadBits(bitstream, nBitsRequested);
#if DEBUG_BITSTREAM
        printf("nBitsRequested = %2d Value = 0x%02X\n", nBitsRequested, *result);
#endif
    } else {
        robitbuf_ReadBits(bitstream, nBitsRequested);
    }
  
    return (0);
}

/* ====================================================================================
                        Parsing of in-stream DRC configuration
 ====================================================================================*/

int
decDuckingScaling(robitbufHandle bitstream,
                  int* duckingScalingPresent,
                  float* duckingScaling)
{
    int err, duckingScalingPresentTmp, bsDuckingScaling, sigma, mu;
    
    err = getBits(bitstream, 1, &duckingScalingPresentTmp);
    if (err) return(err);
    
    if (duckingScalingPresentTmp == 0)
    {
        *duckingScalingPresent = FALSE;
        *duckingScaling = 1.0f;
    }
    else
    {
        *duckingScalingPresent = TRUE;
        err = getBits(bitstream, 4, &bsDuckingScaling);
        if (err) return(err);
        
        sigma = bsDuckingScaling >> 3;
        mu = bsDuckingScaling & 0x7;
        
        if (sigma == 0)
        {
            *duckingScaling = 1.0f + 0.125f * (1.0f + mu);
        }
        else
        {
            *duckingScaling = 1.0f - 0.125f * (1.0f + mu);
        }
    }
    return (0);
}

int
decGainModifiers(robitbufHandle bitstream,
                 const int version,
                 const int bandCount,
                 GainModifiers* gainModifiers)
{
    
    int err = 0, sign, bsGainOffset, bsAttenuationScaling, bsAmplificationScaling;
    
#if MPEG_D_DRC_EXTENSION_V1
    if (version > 0) {
        int b;
        for (b=0; b<bandCount; b++) {
            err = getBits(bitstream, 1, &(gainModifiers->targetCharacteristicLeftPresent[b]));
            if (err) return(err);
            if (gainModifiers->targetCharacteristicLeftPresent[b]) {
                err = getBits(bitstream, 4, &(gainModifiers->targetCharacteristicLeftIndex[b]));
                if (err) return(err);
            }
            err = getBits(bitstream, 1, &(gainModifiers->targetCharacteristicRightPresent[b]));
            if (err) return(err);
            if (gainModifiers->targetCharacteristicRightPresent[b]) {
                err = getBits(bitstream, 4, &(gainModifiers->targetCharacteristicRightIndex[b]));
                if (err) return(err);
            }
            err = getBits(bitstream, 1, &(gainModifiers->gainScalingPresent[b]));
            if (err) return(err);
            if (gainModifiers->gainScalingPresent[b]) {
                err = getBits(bitstream, 4, &(bsAttenuationScaling));
                if (err) return(err);
                gainModifiers->attenuationScaling[b] = bsAttenuationScaling * 0.125f;
                err = getBits(bitstream, 4, &(bsAmplificationScaling));
                if (err) return(err);
                gainModifiers->amplificationScaling[b] = bsAmplificationScaling * 0.125f;
            }
            err = getBits(bitstream, 1, &gainModifiers->gainOffsetPresent[b]);
            if (err) return(err);
            if (gainModifiers->gainOffsetPresent[b])
            {
                float gainOffset;
                err = getBits(bitstream, 1, &sign);
                if (err) return(err);
                err = getBits(bitstream, 5, &bsGainOffset);
                if (err) return(err);
                gainOffset = (1+bsGainOffset) * 0.25f;
                if (sign)
                {
                    gainOffset = - gainOffset;
                }
                gainModifiers->gainOffset[b] = gainOffset;
            }
        }
        if (bandCount == 1) {
            err = getBits(bitstream, 1, &(gainModifiers->shapeFilterPresent));
            if (err) return(err);
            if (gainModifiers->shapeFilterPresent) {
                err = getBits(bitstream, 4, &(gainModifiers->shapeFilterIndex));
                if (err) return(err);
            }
        }
    }
    else if (version == 0)
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    {
        int b, gainScalingPresent, gainOffsetPresent;
        float attenuationScaling = 1.0f, amplificationScaling = 1.0f, gainOffset = 0.0f;
        
        err = getBits(bitstream, 1, &gainScalingPresent);
        if (err) return(err);
        if(gainScalingPresent)
        {
            err = getBits(bitstream, 4, &bsAttenuationScaling);
            if (err) return(err);
            attenuationScaling = bsAttenuationScaling * 0.125f;
            err = getBits(bitstream, 4, &bsAmplificationScaling);
            if (err) return(err);
            amplificationScaling = bsAmplificationScaling * 0.125f;
        }
        err = getBits(bitstream, 1, &gainOffsetPresent);
        if (err) return(err);
        if(gainOffsetPresent)
        {
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            err = getBits(bitstream, 5, &bsGainOffset);
            if (err) return(err);
            gainOffset = (1+bsGainOffset) * 0.25f;
            if (sign)
            {
                gainOffset = - gainOffset;
            }
        }
        for (b=0; b<bandCount; b++) {
#if MPEG_D_DRC_EXTENSION_V1
            gainModifiers->targetCharacteristicLeftPresent[b] = 0;
            gainModifiers->targetCharacteristicRightPresent[b] = 0;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
            gainModifiers->gainScalingPresent[b] = gainScalingPresent;
            gainModifiers->attenuationScaling[b] = attenuationScaling;
            gainModifiers->amplificationScaling[b] = amplificationScaling;
            gainModifiers->gainOffsetPresent[b] = gainOffsetPresent;
            gainModifiers->gainOffset[b] = gainOffset;
        }
#if MPEG_D_DRC_EXTENSION_V1
        gainModifiers->shapeFilterPresent = 0;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    }
    return (0);
}

int
decMethodValue(robitbufHandle bitstream,
               const int methodDefinition,
               float* methodValue)
{
    int err = 0, tmp;
    float val;
    switch (methodDefinition) {
        case METHOD_DEFINITION_UNKNOWN_OTHER:
        case METHOD_DEFINITION_PROGRAM_LOUDNESS:
        case METHOD_DEFINITION_ANCHOR_LOUDNESS:
        case METHOD_DEFINITION_MAX_OF_LOUDNESS_RANGE:
        case METHOD_DEFINITION_MOMENTARY_LOUDNESS_MAX:
        case METHOD_DEFINITION_SHORT_TERM_LOUDNESS_MAX:
            err = getBits(bitstream, 8, &tmp);
            if (err) return (err);
            val = -57.75f + tmp * 0.25f;
            break;
        case METHOD_DEFINITION_LOUDNESS_RANGE:
            err = getBits(bitstream, 8, &tmp);
            if (err) return (err);
            if (tmp == 0)
                val = 0.0f;
            else if(tmp <= 128)
                val = tmp * 0.25f;
            else if(tmp <= 204)
                val = 0.5f * tmp - 32.0f;
            else
                val = tmp - 134.0f;
            break;
        case METHOD_DEFINITION_MIXING_LEVEL:
            err = getBits(bitstream, 5, &tmp);
            if (err) return (err);
            val = tmp + 80.0f;
            break;
        case METHOD_DEFINITION_ROOM_TYPE:
            err = getBits(bitstream, 2, &tmp);
            if (err) return (err);
            val = (float)tmp;
            break;
        case METHOD_DEFINITION_SHORT_TERM_LOUDNESS:
            err = getBits(bitstream, 8, &tmp);
            if (err) return (err);
            val = -116.f + tmp * 0.5f;
            break;
        default:
            fprintf(stderr, "ERROR: unknown methodDefinition value: %d\n", methodDefinition);
            return (UNEXPECTED_ERROR);
            break;
    }
    *methodValue = val;
    return (0);
}

int
parseLoudnessMeasure(robitbufHandle bitstream,
                     LoudnessMeasure* loudnessMeasure)
{
    int err = 0;
    
    err = getBits(bitstream, 4, &(loudnessMeasure->methodDefinition));
    if (err) return(err);
    err = decMethodValue(bitstream, loudnessMeasure->methodDefinition, &(loudnessMeasure->methodValue));
    if (err) return(err);
    err = getBits(bitstream, 4, &(loudnessMeasure->measurementSystem));
    if (err) return(err);
    err = getBits(bitstream, 2, &(loudnessMeasure->reliability));
    if (err) return(err);
    return (0);
}

int
parseLoudnessInfo(robitbufHandle bitstream,
                  const int version,
                  LoudnessInfo* loudnessInfo)
{
    int err = 0, bsSamplePeakLevel, bsTruePeakLevel, i;
    
    err = getBits(bitstream, 6, &(loudnessInfo->drcSetId));
    if (err) return(err);
#if MPEG_D_DRC_EXTENSION_V1
    if (version >= 1) {
        err = getBits(bitstream, 6, &(loudnessInfo->eqSetId));
        if (err) return(err);
    }
    else
    {
        loudnessInfo->eqSetId = 0;
    }
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    
    err = getBits(bitstream, 7, &(loudnessInfo->downmixId));
    if (err) return(err);
    
    err = getBits(bitstream, 1, &(loudnessInfo->samplePeakLevelPresent));
    if (err) return(err);
    if(loudnessInfo->samplePeakLevelPresent)
    {
        err = getBits(bitstream, 12, &bsSamplePeakLevel);
        if (err) return(err);
        if (bsSamplePeakLevel == 0)
        {
            loudnessInfo->samplePeakLevelPresent = 0;
            loudnessInfo->samplePeakLevel = 0.0f;
        }
        else
        {
            loudnessInfo->samplePeakLevel = 20.0f - bsSamplePeakLevel * 0.03125f;
        }
    }
    
    err = getBits(bitstream, 1, &(loudnessInfo->truePeakLevelPresent));
    if (err) return(err);
    if(loudnessInfo->truePeakLevelPresent)
    {
        err = getBits(bitstream, 12, &bsTruePeakLevel);
        if (err) return(err);
        if (bsTruePeakLevel == 0)
        {
            loudnessInfo->truePeakLevelPresent = 0;
            loudnessInfo->truePeakLevel = 0.0f;
        }
        else
        {
            loudnessInfo->truePeakLevel = 20.0f - bsTruePeakLevel * 0.03125f;
        }
        err = getBits(bitstream, 4, &(loudnessInfo->truePeakLevelMeasurementSystem));
        if (err) return(err);
        err = getBits(bitstream, 2, &(loudnessInfo->truePeakLevelReliability));
        if (err) return(err);
    }
    
    err = getBits(bitstream, 4, &(loudnessInfo->measurementCount));
    if (err) return(err);
    
    for (i=0; i<loudnessInfo->measurementCount; i++)
    {
        err = parseLoudnessMeasure(bitstream, &(loudnessInfo->loudnessMeasure[i]));
        if (err) return(err);
    }
    
    return(0);
}

#if MPEG_D_DRC_EXTENSION_V1
int
parseLoudnessInfoSetExtEq(robitbufHandle bitstream,
                          LoudnessInfoSet* loudnessInfoSet)
{
    int err, i, offset, version = 1;
    int loudnessInfoV1AlbumCount, loudnessInfoV1Count;
    
    err = getBits(bitstream, 6, &loudnessInfoV1AlbumCount);
    if (err) return(err);
    err = getBits(bitstream, 6, &loudnessInfoV1Count);
    if (err) return(err);
    offset = loudnessInfoSet->loudnessInfoAlbumCount;
    loudnessInfoSet->loudnessInfoAlbumCount += loudnessInfoV1AlbumCount;
    for (i=0; i<loudnessInfoV1AlbumCount; i++) {
        err = parseLoudnessInfo(bitstream, version, &loudnessInfoSet->loudnessInfoAlbum[i + offset]);
        if (err) return(err);
    }
    offset = loudnessInfoSet->loudnessInfoCount;
    loudnessInfoSet->loudnessInfoCount += loudnessInfoV1Count;
    for (i=0; i<loudnessInfoV1Count; i++) {
        err = parseLoudnessInfo(bitstream, version, &loudnessInfoSet->loudnessInfo[i + offset]);
        if (err) return(err);
    }
    return (0);
}
#endif /* MPEG_D_DRC_EXTENSION_V1 */

int
parseLoudnessInfoSetExtension(robitbufHandle bitstream,
                              LoudnessInfoSet* loudnessInfoSet)
{
    int err = 0, i, k;
    int bitSizeLen, extSizeBits, bitSize, otherBit;
    
    k = 0;
    err = getBits(bitstream, 4, &(loudnessInfoSet->loudnessInfoSetExt.loudnessInfoSetExtType[k]));
    if (err) return(err);
    while(loudnessInfoSet->loudnessInfoSetExt.loudnessInfoSetExtType[k] != UNIDRCLOUDEXT_TERM)
    {
        err = getBits(bitstream, 4, &bitSizeLen );
        if (err) return(err);
        extSizeBits = bitSizeLen + 4;
        
        err = getBits(bitstream, extSizeBits, &bitSize );
        if (err) return(err);
        loudnessInfoSet->loudnessInfoSetExt.extBitSize[k] = bitSize + 1;
        
        switch(loudnessInfoSet->loudnessInfoSetExt.loudnessInfoSetExtType[k])
        {
#if MPEG_D_DRC_EXTENSION_V1
            case UNIDRCLOUDEXT_EQ:
                err = parseLoudnessInfoSetExtEq(bitstream, loudnessInfoSet);
                if (err) return(err);
                break;
#endif
                /* add future extensions here */
            default:
                for(i = 0; i<loudnessInfoSet->loudnessInfoSetExt.extBitSize[k]; i++)
                {
                    err = getBits(bitstream, 1, &otherBit );
                    if (err) return(err);
                }
                break;
        }
        k++;
        err = getBits(bitstream, 4, &(loudnessInfoSet->loudnessInfoSetExt.loudnessInfoSetExtType[k]));
        if (err) return(err);
    }
    
    return (0);
}

/* this function exists in 3 identical copies in different libraries. */
int
selectDrcCoefficients(UniDrcConfig* uniDrcConfig,
                      const int location,
                      DrcCoefficientsUniDrc** drcCoefficientsUniDrc)
{
    int n;
    int cV1 = -1;
    int cV0 = -1;
    for(n=0; n<uniDrcConfig->drcCoefficientsUniDrcCount; n++)
    {
        if (uniDrcConfig->drcCoefficientsUniDrc[n].drcLocation == location)
        {
            if (uniDrcConfig->drcCoefficientsUniDrc[n].version == 0)
            {
                cV0 = n;
            }
            else
            {
                cV1 = n;
            }
        }
    }
#if MPEG_D_DRC_EXTENSION_V1 || AMD1_SYNTAX
    if (cV1 >= 0) {
        *drcCoefficientsUniDrc = &(uniDrcConfig->drcCoefficientsUniDrc[cV1]);
    }
    else if (cV0 >= 0) {
        *drcCoefficientsUniDrc = &(uniDrcConfig->drcCoefficientsUniDrc[cV0]);
    }
    else {
        *drcCoefficientsUniDrc = NULL; /* parametric DRC only (after bitstream parsing is complete this condition is redundant --> see generateVirtualGainSetsForParametricDrc() */
    }
#else
    if (cV0 >= 0) {
        *drcCoefficientsUniDrc = &(uniDrcConfig->drcCoefficientsUniDrc[cV0]);
    }
    else
    {
        int i;
        fprintf(stderr, "ERROR: no DrcCoefficient block available for drcLocation %d.\n", location);
        fprintf(stderr, "Available DRC set drcLocation: ");
        for (i=0; i<uniDrcConfig->drcCoefficientsUniDrcCount; i++)
        {
            fprintf(stderr, "location %d version %d", uniDrcConfig->drcCoefficientsUniDrc[i].drcLocation, uniDrcConfig->drcCoefficientsUniDrc[i].version);
        }
        fprintf(stderr, "\n");
        return(EXTERNAL_ERROR);
    }
#endif /* MPEG_D_DRC_EXTENSION_V1 || AMD1_SYNTAX */
    return (0);
}

int
generateDrcInstructionsDerivedData(UniDrcConfig* uniDrcConfig,
                                   DrcParamsBsDec* drcParams,
                                   DrcInstructionsUniDrc* drcInstructionsUniDrc)
{
    int i, n, g;
    DrcCoefficientsUniDrc* drcCoefficientsUniDrc = NULL;
#if AMD1_SYNTAX
    DrcCoefficientsParametricDrc* drcCoefficientsParametricDrc = NULL;
#endif
    int gainElementCount = 0;
    
    /* find coefficients sequences based on drcLocation */
    for(n=0; n<uniDrcConfig->drcCoefficientsUniDrcCount; n++)
    {
        if (uniDrcConfig->drcCoefficientsUniDrc[n].drcLocation == drcInstructionsUniDrc->drcLocation) break;
    }
    if ((n == uniDrcConfig->drcCoefficientsUniDrcCount)
#if AMD1_SYNTAX
        && (uniDrcConfig->drcCoefficientsUniDrcCount > 0)
#endif
        )
    {
        fprintf(stderr, "ERROR: no DrcCoefficient block available for drcLocation %d.\n", drcInstructionsUniDrc->drcLocation);
        fprintf(stderr, "Available DRC set drcLocation: ");
        for (i=0; i<uniDrcConfig->drcCoefficientsUniDrcCount; i++)
        {
            fprintf(stderr, "%d ", uniDrcConfig->drcCoefficientsUniDrc[i].drcLocation);
        }
        fprintf(stderr, "\n");
        return(EXTERNAL_ERROR);
    }
    drcCoefficientsUniDrc = &(uniDrcConfig->drcCoefficientsUniDrc[n]);
    
#if AMD1_SYNTAX
    /* find parametric coefficients */
    if (uniDrcConfig->uniDrcConfigExtPresent && uniDrcConfig->uniDrcConfigExt.parametricDrcPresent &&
        uniDrcConfig->uniDrcConfigExt.drcCoefficientsParametricDrc.drcLocation == drcInstructionsUniDrc->drcLocation) {
        drcCoefficientsParametricDrc = &uniDrcConfig->uniDrcConfigExt.drcCoefficientsParametricDrc;
    }
#endif /* AMD1_SYNTAX */
    
    for (g=0; g<drcInstructionsUniDrc->nDrcChannelGroups; g++)
    {
        int seq = drcInstructionsUniDrc->gainSetIndexForChannelGroup[g];
#if AMD1_SYNTAX
        if (seq != -1 && (uniDrcConfig->drcCoefficientsUniDrcCount == 0 || seq >= drcCoefficientsUniDrc->gainSetCount)) {
            drcInstructionsUniDrc->channelGroupIsParametricDrc[g] = 1;
            if (uniDrcConfig->drcCoefficientsUniDrcCount != 0) {
                seq = seq - drcCoefficientsUniDrc->gainSetCount;
            }
            drcInstructionsUniDrc->gainSetIndexForChannelGroupParametricDrc[g] = seq;

            if (drcCoefficientsParametricDrc == NULL || seq>=drcCoefficientsParametricDrc->parametricDrcGainSetCount) {
                fprintf(stderr, "ERROR: no parametric DRC gain set found at index %d.\n", seq);
                return(EXTERNAL_ERROR);
            }
            drcInstructionsUniDrc->gainInterpolationTypeForChannelGroup[g] = 1;
            drcInstructionsUniDrc->timeDeltaMinForChannelGroup[g] = drcCoefficientsParametricDrc->parametricDrcFrameSize;
            drcInstructionsUniDrc->timeAlignmentForChannelGroup[g] = 0;
        } else {
            drcInstructionsUniDrc->channelGroupIsParametricDrc[g] = 0;
        }
        if (drcInstructionsUniDrc->channelGroupIsParametricDrc[g] == 0) {
#endif /* AMD1_SYNTAX */
            if (seq>=drcCoefficientsUniDrc->gainSetCount) {
                fprintf(stderr, "ERROR: no DRC gain set found at index %d.\n", seq);
                return(EXTERNAL_ERROR);
            }
            drcInstructionsUniDrc->gainInterpolationTypeForChannelGroup[g] = drcCoefficientsUniDrc->gainSetParams[seq].gainInterpolationType;
            if (drcCoefficientsUniDrc->gainSetParams[seq].timeDeltaMinPresent)
            {
                drcInstructionsUniDrc->timeDeltaMinForChannelGroup[g] = drcCoefficientsUniDrc->gainSetParams[seq].timeDeltaMin;
            }
            else
            {
                drcInstructionsUniDrc->timeDeltaMinForChannelGroup[g] = drcParams->deltaTminDefault;
            }
            drcInstructionsUniDrc->timeAlignmentForChannelGroup[g] = drcCoefficientsUniDrc->gainSetParams[seq].timeAlignment;
#if AMD1_SYNTAX
        }
#endif /* AMD1_SYNTAX */
    }
    
    /* gainElementCount */
    if (drcInstructionsUniDrc->drcSetEffect & (EFFECT_BIT_DUCK_OTHER | EFFECT_BIT_DUCK_SELF))
    {
        drcInstructionsUniDrc->gainElementCount = drcInstructionsUniDrc->nDrcChannelGroups; /* one gain element per channel group */
    } else {
        for (g=0; g<drcInstructionsUniDrc->nDrcChannelGroups; g++)
        {
#if AMD1_SYNTAX
            if (drcInstructionsUniDrc->channelGroupIsParametricDrc[g] == 1)
            {
                gainElementCount++;
                drcInstructionsUniDrc->bandCountForChannelGroup[g] = 1;
            }
            else
#endif
            {
                int seq, bandCount;
                seq = drcInstructionsUniDrc->gainSetIndexForChannelGroup[g];
                bandCount = drcCoefficientsUniDrc->gainSetParams[seq].bandCount;
                drcInstructionsUniDrc->bandCountForChannelGroup[g] = bandCount;
                gainElementCount += bandCount;
            }
        }
        drcInstructionsUniDrc->gainElementCount = gainElementCount;
    }
    
    return(0);
}

int
generateDrcInstructionsForDrcOff(UniDrcConfig* uniDrcConfig, DrcParamsBsDec* drcParams)
{
    int i, k, nMixes, s;
    DrcInstructionsUniDrc* drcInstructionsUniDrc;
    s = -1;

#if MPEG_H_SYNTAX
    /* create one virtual downmix instruction for MPEG-H 3DA */
    uniDrcConfig->downmixInstructionsCount = 1;
    uniDrcConfig->downmixInstructions[0].downmixId = -1;
    uniDrcConfig->downmixInstructions[0].targetLayout = 0;
    uniDrcConfig->downmixInstructions[0].targetChannelCount = 0;
    uniDrcConfig->downmixInstructions[0].downmixCoefficientsPresent = 0;
#endif
    
    k = uniDrcConfig->drcInstructionsUniDrcCount;
    nMixes = uniDrcConfig->downmixInstructionsCount + 1;
    
    drcInstructionsUniDrc = &(uniDrcConfig->drcInstructionsUniDrc[k]);
    memset(drcInstructionsUniDrc, 0, sizeof(DrcInstructionsUniDrc));
    drcInstructionsUniDrc->drcSetId = s;                     /* no DRC */
    s--;
    drcInstructionsUniDrc->downmixId[0] = ID_FOR_BASE_LAYOUT;  /* base layout */
    drcInstructionsUniDrc->downmixIdCount = 1;
    drcInstructionsUniDrc->drcApplyToDownmix = 0;
    drcInstructionsUniDrc->dependsOnDrcSetPresent = 0;
    drcInstructionsUniDrc->noIndependentUse = 0;
    drcInstructionsUniDrc->gainElementCount = 0;
    for (i=1; i<nMixes; i++)
    {
        /* TODO: only generate DRC instructions if they don't already exist for this downmix with no compression */
        drcInstructionsUniDrc = &(uniDrcConfig->drcInstructionsUniDrc[k+i]);
        memset(drcInstructionsUniDrc, 0, sizeof(DrcInstructionsUniDrc));
        drcInstructionsUniDrc->drcSetId = s;                  /* no DRC */
        s--;
#if MPEG_D_DRC_EXTENSION_V1
        drcInstructionsUniDrc->drcSetComplexityLevel = 0;
        drcInstructionsUniDrc->requiresEq = 0;
#endif
        drcInstructionsUniDrc->downmixId[0] = uniDrcConfig->downmixInstructions[i-1].downmixId;
        drcInstructionsUniDrc->downmixIdCount = 1;
        drcInstructionsUniDrc->drcApplyToDownmix = 0;
        drcInstructionsUniDrc->dependsOnDrcSetPresent = 0;
        drcInstructionsUniDrc->noIndependentUse = 0;
        drcInstructionsUniDrc->gainElementCount = 0;
    }
    uniDrcConfig->drcInstructionsCountPlus = uniDrcConfig->drcInstructionsUniDrcCount + nMixes;
    return(0);
}

int
parseDrcInstructionsBasic(robitbufHandle bitstream,
                          UniDrcConfig* uniDrcConfig,
                          DrcInstructionsBasic* drcInstructionsBasic)
{
    int err = 0, i, bsLimiterPeakTarget;
    int additionalDownmixIdPresent, additionalDownmixIdCount;
    
    err = getBits(bitstream, 6, &(drcInstructionsBasic->drcSetId));
    if (err) return(err);
    err = getBits(bitstream, 4, &(drcInstructionsBasic->drcLocation));
    if (err) return(err);
    err = getBits(bitstream, 7, &(drcInstructionsBasic->downmixId[0]));
    if (err) return(err);
    drcInstructionsBasic->downmixIdCount = 1;
    err = getBits(bitstream, 1, &additionalDownmixIdPresent);
    if (err) return(err);
    
    if (additionalDownmixIdPresent) {
        err = getBits(bitstream, 3, &additionalDownmixIdCount);
        if (err) return(err);
        for(i=0; i<additionalDownmixIdCount; i++)
        {
            err = getBits(bitstream, 7, &(drcInstructionsBasic->downmixId[i+1]));
            if (err) return(err);
        }
        drcInstructionsBasic->downmixIdCount = 1 + additionalDownmixIdCount;
    }
    
    err = getBits(bitstream, 16, &(drcInstructionsBasic->drcSetEffect));
    if (err) return(err);

    if ((drcInstructionsBasic->drcSetEffect & (EFFECT_BIT_DUCK_OTHER | EFFECT_BIT_DUCK_SELF)) == 0)
    {
        err = getBits(bitstream, 1, &(drcInstructionsBasic->limiterPeakTargetPresent));
        if (err) return(err);
        if (drcInstructionsBasic->limiterPeakTargetPresent)
        {
            err = getBits(bitstream, 8, &bsLimiterPeakTarget);
            if (err) return(err);
            drcInstructionsBasic->limiterPeakTarget = - bsLimiterPeakTarget * 0.125f;
        }
    }

    err = getBits(bitstream, 1, &(drcInstructionsBasic->drcSetTargetLoudnessPresent));
    if (err) return(err);

    /* set default values */
    drcInstructionsBasic->drcSetTargetLoudnessValueUpper = 0;
    drcInstructionsBasic->drcSetTargetLoudnessValueLower = -63;

    if (drcInstructionsBasic->drcSetTargetLoudnessPresent == 1)
    {
        int bsDrcSetTargetLoudnessValueUpper, bsDrcSetTargetLoudnessValueLower;
        err = getBits(bitstream, 6, &bsDrcSetTargetLoudnessValueUpper);
        if (err) return(err);
        drcInstructionsBasic->drcSetTargetLoudnessValueUpper = bsDrcSetTargetLoudnessValueUpper - 63;
        err = getBits(bitstream, 1, &(drcInstructionsBasic->drcSetTargetLoudnessValueLowerPresent));
        if (err) return(err);
        if (drcInstructionsBasic->drcSetTargetLoudnessValueLowerPresent == 1)
        {
            err = getBits(bitstream, 6, &bsDrcSetTargetLoudnessValueLower);
            if (err) return(err);
            drcInstructionsBasic->drcSetTargetLoudnessValueLower = bsDrcSetTargetLoudnessValueLower - 63;
        }
    }

    return(0);
}

int
parseDrcInstructionsUniDrc( robitbufHandle bitstream,
                            const int version,
                            UniDrcConfig* uniDrcConfig,
                            ChannelLayout* channelLayout,
                            DrcParamsBsDec* drcParams,
                            DrcInstructionsUniDrc* drcInstructionsUniDrc)
{
    int err = 0, i, n, k, g, c, bsLimiterPeakTarget, idx;
    int additionalDownmixIdPresent, additionalDownmixIdCount;
    DrcCoefficientsUniDrc* drcCoefficientsUniDrc = NULL;
    int drcChannelCount;
    int uniqueIndex[CHANNEL_COUNT_MAX];
    float uniqueScaling[CHANNEL_COUNT_MAX];
    int match;
    int downmixIdPresent;
    int repeatParameters, bsRepeatParametersCount;
    int duckingSequence;
    float factor;
    
    err = getBits(bitstream, 6, &(drcInstructionsUniDrc->drcSetId));
    if (err) return(err);
#if MPEG_D_DRC_EXTENSION_V1
    if (version == 0) {
        drcInstructionsUniDrc->drcSetComplexityLevel = DRC_COMPLEXITY_LEVEL_MAX;
    }
    else {
        err = getBits(bitstream, 4, &(drcInstructionsUniDrc->drcSetComplexityLevel));
        if (err) return(err);
    }
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    err = getBits(bitstream, 4, &(drcInstructionsUniDrc->drcLocation));
    if (err) return(err);
    downmixIdPresent = 1;
#if MPEG_D_DRC_EXTENSION_V1
    if (version >= 1) {
        err = getBits(bitstream, 1, &downmixIdPresent);
        if (err) return(err);
    }
    if (downmixIdPresent == 1)
#endif
    {
        err = getBits(bitstream, 7, &(drcInstructionsUniDrc->downmixId[0]));
        if (err) return(err);
#if MPEG_D_DRC_EXTENSION_V1
        if (version >= 1) {
            err = getBits(bitstream, 1, &drcInstructionsUniDrc->drcApplyToDownmix);
            if (err) return(err);
        }
#endif
        if (version == 0) {
            if (drcInstructionsUniDrc->downmixId[0] == 0) {
                drcInstructionsUniDrc->drcApplyToDownmix = 0;
            }
            else {
                drcInstructionsUniDrc->drcApplyToDownmix = 1;
            }
        }
        err = getBits(bitstream, 1, &additionalDownmixIdPresent);
        if (err) return(err);
        
        if (additionalDownmixIdPresent) {
            err = getBits(bitstream, 3, &additionalDownmixIdCount);
            if (err) return(err);
            for(i=0; i<additionalDownmixIdCount; i++)
            {
                err = getBits(bitstream, 7, &(drcInstructionsUniDrc->downmixId[i+1]));
                if (err) return(err);
            }
            drcInstructionsUniDrc->downmixIdCount = 1 + additionalDownmixIdCount;
        } else {
            drcInstructionsUniDrc->downmixIdCount = 1;
        }
    }
#if MPEG_D_DRC_EXTENSION_V1
    else {
        drcInstructionsUniDrc->downmixId[0] = 0;
        drcInstructionsUniDrc->downmixIdCount = 1;
    }
#endif

    err = getBits(bitstream, 16, &(drcInstructionsUniDrc->drcSetEffect));
    if (err) return(err);

    if ((drcInstructionsUniDrc->drcSetEffect & (EFFECT_BIT_DUCK_OTHER | EFFECT_BIT_DUCK_SELF)) == 0)
    {
        err = getBits(bitstream, 1, &(drcInstructionsUniDrc->limiterPeakTargetPresent));
        if (err) return(err);
        if (drcInstructionsUniDrc->limiterPeakTargetPresent)
        {
            err = getBits(bitstream, 8, &bsLimiterPeakTarget);
            if (err) return(err);
            drcInstructionsUniDrc->limiterPeakTarget = - bsLimiterPeakTarget * 0.125f;
        }
    }

    err = getBits(bitstream, 1, &(drcInstructionsUniDrc->drcSetTargetLoudnessPresent));
    if (err) return(err);

    /* set default values */
    drcInstructionsUniDrc->drcSetTargetLoudnessValueUpper = 0;
    drcInstructionsUniDrc->drcSetTargetLoudnessValueLower = -63;
    
    if (drcInstructionsUniDrc->drcSetTargetLoudnessPresent == 1)
    {
        int bsDrcSetTargetLoudnessValueUpper, bsDrcSetTargetLoudnessValueLower;
        err = getBits(bitstream, 6, &bsDrcSetTargetLoudnessValueUpper);
        if (err) return(err);
        drcInstructionsUniDrc->drcSetTargetLoudnessValueUpper = bsDrcSetTargetLoudnessValueUpper - 63;
        err = getBits(bitstream, 1, &(drcInstructionsUniDrc->drcSetTargetLoudnessValueLowerPresent));
        if (err) return(err);
        if (drcInstructionsUniDrc->drcSetTargetLoudnessValueLowerPresent == 1)
        {
            err = getBits(bitstream, 6, &bsDrcSetTargetLoudnessValueLower);
            if (err) return(err);
            drcInstructionsUniDrc->drcSetTargetLoudnessValueLower = bsDrcSetTargetLoudnessValueLower - 63;
        }
    }

    err = getBits(bitstream, 1, &(drcInstructionsUniDrc->dependsOnDrcSetPresent));
    if (err) return(err);
    
    drcInstructionsUniDrc->noIndependentUse = 0;
    if (drcInstructionsUniDrc->dependsOnDrcSetPresent) {
        err = getBits(bitstream, 6, &(drcInstructionsUniDrc->dependsOnDrcSet));
        if (err) return(err);
    }
    else
    {
        err = getBits(bitstream, 1, &(drcInstructionsUniDrc->noIndependentUse));
        if (err) return(err);
    }
#if MPEG_D_DRC_EXTENSION_V1
    if (version == 0) {
        drcInstructionsUniDrc->requiresEq = 0;
    }
    else {
        err = getBits(bitstream, 1, &(drcInstructionsUniDrc->requiresEq));
        if (err) return(err);
    }
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    
    err = selectDrcCoefficients(uniDrcConfig, drcInstructionsUniDrc->drcLocation, &drcCoefficientsUniDrc);
    if (err) return (err);
    
    drcChannelCount = uniDrcConfig->channelLayout.baseChannelCount;
    
    for (c=0; c<CHANNEL_COUNT_MAX; c++)
    {
        uniqueIndex[c] = -10;
        uniqueScaling[c] = -10.0f;
    }
    
    if (drcInstructionsUniDrc->drcSetEffect & (EFFECT_BIT_DUCK_OTHER | EFFECT_BIT_DUCK_SELF))
    {
        c=0;
        while (c<drcChannelCount)
        {
            int bsGainSetIndex;
            err = getBits(bitstream, 6, &bsGainSetIndex);
            if (err) return(err);
            drcInstructionsUniDrc->gainSetIndex[c] = bsGainSetIndex - 1;
            decDuckingScaling(bitstream,
                              &(drcInstructionsUniDrc->duckingModifiersForChannel[c].duckingScalingPresent),
                              &(drcInstructionsUniDrc->duckingModifiersForChannel[c].duckingScaling));
            
            c++;
            err = getBits(bitstream, 1, &repeatParameters);
            if (err) return(err);
            
            if (repeatParameters == 1)
            {
                err = getBits(bitstream, 5, &bsRepeatParametersCount);
                if (err) return(err);
                bsRepeatParametersCount += 1;
                for (k=0; k<bsRepeatParametersCount; k++)
                {
                    drcInstructionsUniDrc->gainSetIndex[c] = drcInstructionsUniDrc->gainSetIndex[c-1];
                    drcInstructionsUniDrc->duckingModifiersForChannel[c].duckingScalingPresent = drcInstructionsUniDrc->duckingModifiersForChannel[c-1].duckingScalingPresent;
                    drcInstructionsUniDrc->duckingModifiersForChannel[c].duckingScaling = drcInstructionsUniDrc->duckingModifiersForChannel[c-1].duckingScaling;
                    c++;
                }
            }
        }
        if (c>drcChannelCount)
        {
            fprintf(stderr, "ERROR: Too many repeatParameters.\n");
            return(UNEXPECTED_ERROR);
        }
        duckingSequence = -1;
        g = 0;
        if (drcInstructionsUniDrc->drcSetEffect & EFFECT_BIT_DUCK_OTHER) {
            for (c=0; c<drcChannelCount; c++) {
                match = FALSE;
                idx = drcInstructionsUniDrc->gainSetIndex[c];
                factor = drcInstructionsUniDrc->duckingModifiersForChannel[c].duckingScaling;
                if (idx < 0) {
                    for (n=0; n<g; n++) {
                        if (uniqueScaling[n] == factor) {
                            /* channel belongs to existing group */
                            match = TRUE;
                            drcInstructionsUniDrc->channelGroupForChannel[c] = n;
                            break;
                        }
                    }
                    if (match == FALSE) {
                        /* channel belongs to new group */
                        uniqueIndex[g] = idx;
                        uniqueScaling[g] = factor;
                        drcInstructionsUniDrc->channelGroupForChannel[c] = g;
                        g++;
                    }
                }
                else {
                    /* this group contains the ducking gain sequence that must be applied to all other groups */
                    if ((duckingSequence > 0) && (duckingSequence != idx)) {
                        fprintf(stderr, "ERROR: DRC for ducking can only have one ducking gain sequence.\n");
                        return(UNEXPECTED_ERROR);
                    }
                    duckingSequence = idx;
                    drcInstructionsUniDrc->channelGroupForChannel[c] = -1;
                }
            }
            drcInstructionsUniDrc->nDrcChannelGroups = g;
            if (duckingSequence == -1)
            {
                fprintf(stderr, "ERROR: Ducking sequence expected but not found.\n");
                return(UNEXPECTED_ERROR);
            }
        } else if (drcInstructionsUniDrc->drcSetEffect & EFFECT_BIT_DUCK_SELF) {
            for (c=0; c<drcChannelCount; c++) {
                match = FALSE;
                idx = drcInstructionsUniDrc->gainSetIndex[c];
                factor = drcInstructionsUniDrc->duckingModifiersForChannel[c].duckingScaling;
                if (idx >= 0) {
                    for (n=0; n<g; n++) {
                        if ((uniqueIndex[n] == idx) && (uniqueScaling[n] == factor)) {
                            /* channel belongs to existing group */
                            match = TRUE;
                            drcInstructionsUniDrc->channelGroupForChannel[c] = n;
                            break;
                        }
                    }
                    if (match == FALSE) {
                        /* channel belongs to new group */
                        uniqueIndex[g] = idx;
                        uniqueScaling[g] = factor;
                        drcInstructionsUniDrc->channelGroupForChannel[c] = g;
                        g++;
                    }
                }
                else {
                    /* this group contains the overlay content */
                    drcInstructionsUniDrc->channelGroupForChannel[c] = -1;
                }
            }
            drcInstructionsUniDrc->nDrcChannelGroups = g;
        }
        
        for (g=0; g<drcInstructionsUniDrc->nDrcChannelGroups; g++)
        {
            int set = (drcInstructionsUniDrc->drcSetEffect & EFFECT_BIT_DUCK_OTHER) ? duckingSequence : uniqueIndex[g];
            drcInstructionsUniDrc->gainSetIndexForChannelGroup[g] = set;
            drcInstructionsUniDrc->duckingModifiersForChannelGroup[g].duckingScaling = uniqueScaling[g];
            if (uniqueScaling[g] != 1.0f)
            {
                drcInstructionsUniDrc->duckingModifiersForChannelGroup[g].duckingScalingPresent = TRUE;
            }
            else
            {
                drcInstructionsUniDrc->duckingModifiersForChannelGroup[g].duckingScalingPresent = FALSE;
            }
            drcInstructionsUniDrc->bandCountForChannelGroup[g] = 1;
        }
    }
    else
    {
        if (
#if MPEG_D_DRC_EXTENSION_V1
        ((version==0) || (drcInstructionsUniDrc->drcApplyToDownmix != 0)) &&
#endif
        (drcInstructionsUniDrc->downmixId[0] != 0) && (drcInstructionsUniDrc->downmixId[0] != ID_FOR_ANY_DOWNMIX) && (drcInstructionsUniDrc->downmixIdCount==1))
        {
#if MPEG_H_SYNTAX
            for(i=0; i<uniDrcConfig->mpegh3daDownmixInstructionsCount; i++)
            {
                if (drcInstructionsUniDrc->downmixId[0] == uniDrcConfig->mpegh3daDownmixInstructions[i].downmixId) break;
            }
            if (i == uniDrcConfig->mpegh3daDownmixInstructionsCount)
            {
                fprintf(stderr, "ERROR: downmixInstructions expected but not found: %d\n", uniDrcConfig->mpegh3daDownmixInstructionsCount);
                return(UNEXPECTED_ERROR);
            }
            drcChannelCount = uniDrcConfig->mpegh3daDownmixInstructions[i].targetChannelCount;  /*  targetChannelCountFromDownmixId*/
#else
            for(i=0; i<uniDrcConfig->downmixInstructionsCount; i++)
            {
                if (drcInstructionsUniDrc->downmixId[0] == uniDrcConfig->downmixInstructions[i].downmixId) break;
            }
            if (i == uniDrcConfig->downmixInstructionsCount)
            {
                fprintf(stderr, "ERROR: downmixInstructions expected but not found: %d\n", uniDrcConfig->downmixInstructionsCount);
                return(UNEXPECTED_ERROR);
            }
            drcChannelCount = uniDrcConfig->downmixInstructions[i].targetChannelCount;  /*  targetChannelCountFromDownmixId*/
#endif
        }
        else if (
#if MPEG_D_DRC_EXTENSION_V1
        ((version==0) || (drcInstructionsUniDrc->drcApplyToDownmix != 0)) &&
#endif
        ((drcInstructionsUniDrc->downmixId[0] == ID_FOR_ANY_DOWNMIX) || (drcInstructionsUniDrc->downmixIdCount > 1)))
        {
            drcChannelCount = 1;
        }
        
        c=0;
        while (c<drcChannelCount)
        {
            int bsGainSetIndex;
            int repeatGainSetIndex, bsRepeatGainSetIndexCount;
            err = getBits(bitstream, 6, &bsGainSetIndex);
            if (err) return(err);
            drcInstructionsUniDrc->gainSetIndex[c] = bsGainSetIndex - 1;
            c++;
            err = getBits(bitstream, 1, &repeatGainSetIndex);
            if (err) return(err);
            
            if (repeatGainSetIndex == 1)
            {
                err = getBits(bitstream, 5, &bsRepeatGainSetIndexCount);
                if (err) return(err);
                bsRepeatGainSetIndexCount += 1;
                for (k=0; k<bsRepeatGainSetIndexCount; k++)
                {
                    drcInstructionsUniDrc->gainSetIndex[c] = bsGainSetIndex - 1;
                    c++;
                }
            }
        }
        if (c>drcChannelCount)
        {
            fprintf(stderr, "ERROR: Too many repeatParameters.\n");
            return(UNEXPECTED_ERROR);
        }
        
        /* get channel group for each channel; for DRC sets with downmixId = 0x7F we have to wait till physical channel count is available --> drcGainDecoder */
        g = 0;
        if ((drcInstructionsUniDrc->downmixId[0] == ID_FOR_ANY_DOWNMIX) || (drcInstructionsUniDrc->downmixIdCount > 1)) {
            int idx = drcInstructionsUniDrc->gainSetIndex[0];
            if (idx >= 0) {
                uniqueIndex[0] = idx;
                g = 1;
            }
        }
        else {
            for (c=0; c<drcChannelCount; c++) {
                int idx = drcInstructionsUniDrc->gainSetIndex[c];
                match = FALSE;
                if (idx>=0) {
                    for (n=0; n<g; n++) {
                        if (uniqueIndex[n] == idx) {
                            match = TRUE;
                            drcInstructionsUniDrc->channelGroupForChannel[c] = n;
                            break;
                        }
                    }
                    if (match == FALSE)
                    {
                        uniqueIndex[g] = idx;
                        drcInstructionsUniDrc->channelGroupForChannel[c] = g;
                        g++;
                    }
                }
                else {
                    drcInstructionsUniDrc->channelGroupForChannel[c] = -1;
                }
            }
        }
        
        /* get gainModifiers and sequenceIndex for each channel group */
        drcInstructionsUniDrc->nDrcChannelGroups = g;
        for (g=0; g<drcInstructionsUniDrc->nDrcChannelGroups; g++)
        {
            int set, bandCount;
            /* the DRC channel groups are ordered according to increasing channel indices */
            /* Whenever a new sequenceIndex appears the channel group index is incremented */
            /* The same order has to be observed for all subsequent processing */
            
            set = uniqueIndex[g];
            drcInstructionsUniDrc->gainSetIndexForChannelGroup[g] = set;
            
            /* get bandCount */
            if (drcCoefficientsUniDrc != NULL && set < drcCoefficientsUniDrc->gainSetCount) {
                bandCount = drcCoefficientsUniDrc->gainSetParams[set].bandCount;
            } else {
                bandCount = 1; /* parametric DRC (no sanity check since not parsed, yet) */
            }
            
            err = decGainModifiers(bitstream, version, bandCount, &(drcInstructionsUniDrc->gainModifiersForChannelGroup[g]));
            if (err) return (err);
        }
    }

    return(0);
}

int
parseGainSetParamsCharacteristics(robitbufHandle bitstream,
                                  const int version,
                                  GainParams* gainParams)
{
    int err = 0;
    
    if (version == 0) {
        err = getBits(bitstream, 7, &(gainParams->drcCharacteristic));
        if (err) return(err);
#if MPEG_D_DRC_EXTENSION_V1
        if (gainParams->drcCharacteristic > 0) {
            gainParams->drcCharacteristicPresent = 1;
            gainParams->drcCharacteristicFormatIsCICP = 1;
        }
        else {
            gainParams->drcCharacteristicPresent = 0;
        }
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    }
#if MPEG_D_DRC_EXTENSION_V1
    else {
        err = getBits(bitstream, 1, &(gainParams->drcCharacteristicPresent));
        if (err) return(err);
        if (gainParams->drcCharacteristicPresent) {
            err = getBits(bitstream, 1, &(gainParams->drcCharacteristicFormatIsCICP));
            if (err) return(err);
            if (gainParams->drcCharacteristicFormatIsCICP) {
                err = getBits(bitstream, 7, &(gainParams->drcCharacteristic));
                if (err) return(err);
            }
            else {
                err = getBits(bitstream, 4, &(gainParams->drcCharacteristicLeftIndex));
                if (err) return(err);
                err = getBits(bitstream, 4, &(gainParams->drcCharacteristicRightIndex));
                if (err) return(err);
            }
        }
    }
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    return(0);
}

int
parseGainSetParamsCrossoverFreqIndex(robitbufHandle bitstream,
                                     GainParams* gainParams,
                                     int drcBandType)
{
    int err = 0;
    
    if (drcBandType) {
        err = getBits(bitstream, 4, &(gainParams->crossoverFreqIndex));
        if (err) return(err);
    } else {
        err = getBits(bitstream, 10, &(gainParams->startSubBandIndex));
        if (err) return(err);
    }
    
    return(0);
}

int
parseGainSetParams(robitbufHandle bitstream,
                   const int version,
                   int* gainSequenceIndex,
                   GainSetParams* gainSetParams)
{
    int err = 0, i;
    
    err = getBits(bitstream, 2, &(gainSetParams->gainCodingProfile));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->gainInterpolationType));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->fullFrame));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->timeAlignment));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->timeDeltaMinPresent));
    if (err) return(err);
    
    if(gainSetParams->timeDeltaMinPresent)
    {
        int bsTimeDeltaMin;
        err = getBits(bitstream, 11, &bsTimeDeltaMin);
        if (err) return(err);
        gainSetParams->timeDeltaMin = bsTimeDeltaMin + 1;
    }
    
    if (gainSetParams->gainCodingProfile == GAIN_CODING_PROFILE_CONSTANT) {
        gainSetParams->bandCount = 1;
        *gainSequenceIndex = (*gainSequenceIndex) + 1;
#if AMD2_COR2
        gainSetParams->gainParams[0].gainSequenceIndex = *gainSequenceIndex;
#endif
    }
    else
    {
        err = getBits(bitstream, 4, &(gainSetParams->bandCount));
        if (err) return(err);
        
        if(gainSetParams->bandCount>1)
        {
            err = getBits(bitstream, 1, &(gainSetParams->drcBandType));
            if (err) return(err);
        }
        for(i=0; i<gainSetParams->bandCount; i++)
        {
            if (version == 0) {
                *gainSequenceIndex = (*gainSequenceIndex) + 1;
            }
            else {
                int indexPresent;
                err = getBits(bitstream, 1, &indexPresent);
                if (err) return(err);
                if (indexPresent) {
                    int bsIndex;
                    err = getBits(bitstream, 6, &bsIndex);
                    if (err) return(err);
                    *gainSequenceIndex = bsIndex;
                }
                else {
                    *gainSequenceIndex = (*gainSequenceIndex) + 1;
                }
            }
            gainSetParams->gainParams[i].gainSequenceIndex = *gainSequenceIndex;
            err = parseGainSetParamsCharacteristics(bitstream, version, &(gainSetParams->gainParams[i]));
            if (err) return(err);
        }
        for(i=1; i<gainSetParams->bandCount; i++)
        {
            err = parseGainSetParamsCrossoverFreqIndex(bitstream, &(gainSetParams->gainParams[i]), gainSetParams->drcBandType);
            if (err) return(err);
        }
    }
    
    return(0);
}

int
parseDrcCoefficientsBasic(robitbufHandle bitstream,
                          DrcCoefficientsBasic* drcCoefficientsBasic)
{
    int err = 0;
    err = getBits(bitstream, 4, &(drcCoefficientsBasic->drcLocation));
    if (err) return(err);
    err = getBits(bitstream, 7, &(drcCoefficientsBasic->drcCharacteristic));
    if (err) return(err);

    return (0);
}

#if MPEG_D_DRC_EXTENSION_V1
int
parseSplitDrcCharacteristic(robitbufHandle bitstream, const int side, SplitDrcCharacteristic* splitDrcCharacteristic) {
    int err = 0, i;
    
    err = getBits(bitstream, 1, &(splitDrcCharacteristic->characteristicFormat));
    if (err) return(err);
    if (splitDrcCharacteristic->characteristicFormat == 0) {
        int bsGain, bsIoRatio, bsExp;
        err = getBits(bitstream, 6, &bsGain);
        if (err) return(err);
        if (side == LEFT_SIDE) {
            splitDrcCharacteristic->gain = bsGain;
        }
        else {
            splitDrcCharacteristic->gain = - bsGain;
        }
        err = getBits(bitstream, 4, &bsIoRatio);
        if (err) return(err);
        splitDrcCharacteristic->ioRatio = 0.05f + 0.15f * bsIoRatio;
        err = getBits(bitstream, 4, &bsExp);
        if (err) return(err);
        if (bsExp<15) {
            splitDrcCharacteristic->exp = 1.0f + 2.0f * bsExp;
        }
        else {
            splitDrcCharacteristic->exp = 1000.0f;
        }
        err = getBits(bitstream, 1, &(splitDrcCharacteristic->flipSign));
        if (err) return(err);
    }
    else {
        int bsCharacteristicNodeCount, bsNodeLevelDelta, bsNodeGain;
        err = getBits(bitstream, 2, &(bsCharacteristicNodeCount));
        if (err) return(err);
        splitDrcCharacteristic->characteristicNodeCount = bsCharacteristicNodeCount + 1;
        splitDrcCharacteristic->nodeLevel[0] = DRC_INPUT_LOUDNESS_TARGET;
        splitDrcCharacteristic->nodeGain[0] = 0.0f;
        for (i=1; i<=splitDrcCharacteristic->characteristicNodeCount; i++) {
            err = getBits(bitstream, 5, &bsNodeLevelDelta);
            if (err) return(err);
            if (side == LEFT_SIDE) {
                splitDrcCharacteristic->nodeLevel[i] = splitDrcCharacteristic->nodeLevel[i-1] - (1.0f + bsNodeLevelDelta);
            }
            else {
                splitDrcCharacteristic->nodeLevel[i] = splitDrcCharacteristic->nodeLevel[i-1] + (1.0f + bsNodeLevelDelta);
            }
            err = getBits(bitstream, 8, &bsNodeGain);
            if (err) return(err);
            splitDrcCharacteristic->nodeGain[i] = 0.5f * bsNodeGain - 64.0f;
        }
    }
    return(0);
}

int
parseShapeFilterParams(robitbufHandle bitstream, ShapeFilterParams* shapeFilterParams)
{
    int err = 0;
    err = getBits(bitstream, 3, &(shapeFilterParams->cornerFreqIndex));
    if (err) return(err);
    err = getBits(bitstream, 2, &(shapeFilterParams->filterStrengthIndex));
    if (err) return(err);
    return (0);
}
#endif /* MPEG_D_DRC_EXTENSION_V1 */

int
parseDrcCoefficientsUniDrc(robitbufHandle bitstream,
                           const int version,
                           DrcParamsBsDec* drcParams,
                           DrcCoefficientsUniDrc* drcCoefficientsUniDrc)
{
    int err = 0, i, bsDrcFrameSize;
    int gainSequenceIndex = -1;
    
    drcCoefficientsUniDrc->version = version;
    if (version == 0) {
#if MPEG_D_DRC_EXTENSION_V1
        int gainSequenceCount = 0;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
        err = getBits(bitstream, 4, &(drcCoefficientsUniDrc->drcLocation));
        if (err) return(err);
        err = getBits(bitstream, 1, &(drcCoefficientsUniDrc->drcFrameSizePresent));
        if (err) return(err);
        
        if (drcCoefficientsUniDrc->drcFrameSizePresent == 1)
        {
            err = getBits(bitstream, 15, &bsDrcFrameSize);
            if (err) return(err);
            drcCoefficientsUniDrc->drcFrameSize = bsDrcFrameSize + 1;
        }
        
#if MPEG_D_DRC_EXTENSION_V1
        drcCoefficientsUniDrc->drcCharacteristicLeftPresent = 0;
        drcCoefficientsUniDrc->drcCharacteristicRightPresent = 0;
        drcCoefficientsUniDrc->shapeFiltersPresent = 0;
#endif
        err = getBits(bitstream, 6, &(drcCoefficientsUniDrc->gainSetCount));
        if (err) return(err);
#if AMD1_SYNTAX
        drcCoefficientsUniDrc->gainSetCountPlus = drcCoefficientsUniDrc->gainSetCount;
#endif /* AMD1_SYNTAX */
        for(i=0; i<drcCoefficientsUniDrc->gainSetCount; i++)
        {
            err = parseGainSetParams(bitstream, version, &gainSequenceIndex, &(drcCoefficientsUniDrc->gainSetParams[i]));
            if (err) return (err);
            
            if (drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMinPresent)
            {
                if (drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin > drcParams->drcFrameSize) /* assumes that provided audioFrameSize is equal to present drcFrameSize in bitstream */
                {
                    fprintf(stderr, "ERROR: DRC time interval (deltaTmin) cannot exceed audio frame size. %d %d\n", drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin, drcParams->drcFrameSize);
                    return(PARAM_ERROR);
                }
                drcCoefficientsUniDrc->gainSetParams[i].nGainValuesMax = drcParams->drcFrameSize / drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin;
                err = initTables(drcCoefficientsUniDrc->gainSetParams[i].nGainValuesMax, &(drcCoefficientsUniDrc->gainSetParams[i].tables));
                if (err) return (err);
            }
#if MPEG_D_DRC_EXTENSION_V1
            gainSequenceCount += drcCoefficientsUniDrc->gainSetParams[i].bandCount;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
        }
#if MPEG_D_DRC_EXTENSION_V1
        drcCoefficientsUniDrc->gainSequenceCount = gainSequenceCount;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    }
#if MPEG_D_DRC_EXTENSION_V1
    else {
        
        ShapeFilterBlockParams* shapeFilterBlockParams;
        for (i=0; i<SEQUENCE_COUNT_MAX; i++) {
            drcCoefficientsUniDrc->gainSetParamsIndexForGainSequence[i] = -1;
        }
        
        err = getBits(bitstream, 4, &(drcCoefficientsUniDrc->drcLocation));
        if (err) return(err);
        err = getBits(bitstream, 1, &(drcCoefficientsUniDrc->drcFrameSizePresent));
        if (err) return(err);
        
        if (drcCoefficientsUniDrc->drcFrameSizePresent == 1)
        {
            err = getBits(bitstream, 15, &bsDrcFrameSize);
            if (err) return(err);
            drcCoefficientsUniDrc->drcFrameSize = bsDrcFrameSize + 1;
        }
        err = getBits(bitstream, 1, &(drcCoefficientsUniDrc->drcCharacteristicLeftPresent));
        if (err) return(err);
        if (drcCoefficientsUniDrc->drcCharacteristicLeftPresent == 1) {
            err = getBits(bitstream, 4, &(drcCoefficientsUniDrc->characteristicLeftCount));
            if (err) return(err);
            for (i=1; i<=drcCoefficientsUniDrc->characteristicLeftCount; i++) {
                err = parseSplitDrcCharacteristic(bitstream, LEFT_SIDE, &(drcCoefficientsUniDrc->splitCharacteristicLeft[i]));
                if (err) return(err);
            }
        }
        err = getBits(bitstream, 1, &(drcCoefficientsUniDrc->drcCharacteristicRightPresent));
        if (err) return(err);
        if (drcCoefficientsUniDrc->drcCharacteristicRightPresent == 1) {
            err = getBits(bitstream, 4, &(drcCoefficientsUniDrc->characteristicRightCount));
            if (err) return(err);
            for (i=1; i<=drcCoefficientsUniDrc->characteristicRightCount; i++) {
                err = parseSplitDrcCharacteristic(bitstream, RIGHT_SIDE, &(drcCoefficientsUniDrc->splitCharacteristicRight[i]));
                if (err) return(err);
            }
        }
        err = getBits(bitstream, 1, &(drcCoefficientsUniDrc->shapeFiltersPresent));
        if (err) return(err);
        if (drcCoefficientsUniDrc->shapeFiltersPresent == 1) {
            err = getBits(bitstream, 4, &(drcCoefficientsUniDrc->shapeFilterCount));
            if (err) return(err);
            for (i=1; i<=drcCoefficientsUniDrc->shapeFilterCount; i++) {
                shapeFilterBlockParams = &(drcCoefficientsUniDrc->shapeFilterBlockParams[i]);
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->lfCutFilterPresent));
                if (err) return(err);
                if (shapeFilterBlockParams->lfCutFilterPresent == 1) {
                    err = parseShapeFilterParams(bitstream, &(shapeFilterBlockParams->lfCutParams));
                    if (err) return(err);
                }
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->lfBoostFilterPresent));
                if (err) return(err);
                if (shapeFilterBlockParams->lfBoostFilterPresent == 1) {
                    err = parseShapeFilterParams(bitstream, &(shapeFilterBlockParams->lfBoostParams));
                    if (err) return(err);
                }
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->hfCutFilterPresent));
                if (err) return(err);
                if (shapeFilterBlockParams->hfCutFilterPresent == 1) {
                    err = parseShapeFilterParams(bitstream, &(shapeFilterBlockParams->hfCutParams));
                    if (err) return(err);
                }
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->hfBoostFilterPresent));
                if (err) return(err);
                if (shapeFilterBlockParams->hfBoostFilterPresent == 1) {
                    err = parseShapeFilterParams(bitstream, &(shapeFilterBlockParams->hfBoostParams));
                    if (err) return(err);
                }
            }
        }
        err = getBits(bitstream, 6, &(drcCoefficientsUniDrc->gainSequenceCount));
        if (err) return(err);
        err = getBits(bitstream, 6, &(drcCoefficientsUniDrc->gainSetCount));
        if (err) return(err);
#if AMD1_SYNTAX
        drcCoefficientsUniDrc->gainSetCountPlus = drcCoefficientsUniDrc->gainSetCount;
#endif
        for(i=0; i<drcCoefficientsUniDrc->gainSetCount; i++)
        {
            err = parseGainSetParams(bitstream, version, &gainSequenceIndex, &(drcCoefficientsUniDrc->gainSetParams[i]));
            if (err) return (err);
            
            if (drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMinPresent)
            {
                if (drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin > drcParams->drcFrameSize) /* assumes that provided audioFrameSize is equal to present drcFrameSize in bitstream */
                {
                    fprintf(stderr, "ERROR: DRC time interval (deltaTmin) cannot exceed audio frame size. %d %d\n", drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin, drcParams->drcFrameSize);
                    return(PARAM_ERROR);
                }
                drcCoefficientsUniDrc->gainSetParams[i].nGainValuesMax = drcParams->drcFrameSize / drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin;
                err = initTables(drcCoefficientsUniDrc->gainSetParams[i].nGainValuesMax, &(drcCoefficientsUniDrc->gainSetParams[i].tables));
                if (err) return (err);
            }
        }
        
#if AMD2_COR2
    }
#endif
    for(i=0; i<drcCoefficientsUniDrc->gainSetCount; i++)
    {
        int b;
        for (b=0; b<drcCoefficientsUniDrc->gainSetParams[i].bandCount; b++) {
            drcCoefficientsUniDrc->gainSetParamsIndexForGainSequence[drcCoefficientsUniDrc->gainSetParams[i].gainParams[b].gainSequenceIndex] = i;
        }
    }
#if !AMD2_COR2
    }
#endif
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    return(0);
}

int
parseDownmixInstructions(robitbufHandle bitstream,
                         const int version,
                         DrcParamsBsDec* drcParams,
                         ChannelLayout* channelLayout,
                         DownmixInstructions* downmixInstructions)
{
    int err = 0, i, j, k;
    
    err = getBits(bitstream, 7, &(downmixInstructions->downmixId));
    if (err) return(err);
    err = getBits(bitstream, 7, &(downmixInstructions->targetChannelCount));
    if (err) return(err);
    err = getBits(bitstream, 8, &(downmixInstructions->targetLayout));
    if (err) return(err);
    err = getBits(bitstream, 1, &(downmixInstructions->downmixCoefficientsPresent));
    if (err) return(err);
    
    if (downmixInstructions->downmixCoefficientsPresent)
    {
        if (version == 0) {
            int bsDownmixCoefficient;
            k=0;
            for (i=0; i<downmixInstructions->targetChannelCount; i++)
            {
                for (j=0; j<channelLayout->baseChannelCount; j++) {
                    err = getBits(bitstream, 4, &bsDownmixCoefficient);
                    if (err) return(err);
                    if (drcParams->lfeChannelMap[j]) {
                        downmixInstructions->downmixCoefficient[k] = (float)pow(10.0f, 0.05f * downmixCoeffLfe[bsDownmixCoefficient]);
                    } else {
                        downmixInstructions->downmixCoefficient[k] = (float)pow(10.0f, 0.05f * downmixCoeff[bsDownmixCoefficient]);
                    }
                    k++;
                }
            }
        }
#if MPEG_D_DRC_EXTENSION_V1
        else {
            int bsDownmixCoefficientV1, bsDownmixOffset;
            float a, b, downmixOffset, sum;
            
            err = getBits(bitstream, 4, &bsDownmixOffset);
            if (err) return(err);
            k=0;
            for (i=0; i<downmixInstructions->targetChannelCount; i++)
            {
                for (j=0; j<channelLayout->baseChannelCount; j++) {
                    err = getBits(bitstream, 5, &bsDownmixCoefficientV1);
                    if (err) return(err);
                    downmixInstructions->downmixCoefficient[k] = downmixCoeffV1[bsDownmixCoefficientV1];
                    k++;
                }
            }
            switch (bsDownmixOffset) {
                case 0:
                    downmixOffset = 0.0f;
                    break;
                case 1:
                    a = 20.0f * log10((float) downmixInstructions->targetChannelCount / (float)channelLayout->baseChannelCount);
                    downmixOffset = 0.5f * floor(0.5f + a);
                    break;
                case 2:
                    a = 20.0f * log10((float) downmixInstructions->targetChannelCount / (float)channelLayout->baseChannelCount);
                    downmixOffset = 0.5f * floor(0.5f + 2.0f * a);
                    break;
                case 3:
                    sum = 0.0f;
                    for (k=0; k<downmixInstructions->targetChannelCount * channelLayout->baseChannelCount; k++)
                    {
                        sum += pow(10.0f, 0.1f * downmixInstructions->downmixCoefficient[k]);
                    }
                    b = 10.0f * log10(sum);
                    downmixOffset = 0.5f * floor(0.5f + 2.0f * b);
                    break;
                    
                default:
                    /* Error: this case is not allowed */
                    return (BITSTREAM_ERROR);
                    break;
            }
            for (k=0; k<downmixInstructions->targetChannelCount * channelLayout->baseChannelCount; k++)
            {
                downmixInstructions->downmixCoefficient[k] = (float)pow(10.0f, 0.05f * (downmixInstructions->downmixCoefficient[k] + downmixOffset));
            }
        }
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    }
    return(0);
}

int
parseChannelLayout(robitbufHandle bitstream,
                   DrcParamsBsDec* drcParams,
                   ChannelLayout* channelLayout)
{
    int err = 0, i;
    
    err = getBits(bitstream, 7, &(channelLayout->baseChannelCount));
    if (err) return(err);
    if (drcParams->lfeChannelMapCount != -1 && channelLayout->baseChannelCount != drcParams->lfeChannelMapCount) {
        fprintf(stderr, "ERROR: Dimension of external LFE channel map does not match baseChannelCount uniDrcConfig().\n");
        return (UNEXPECTED_ERROR);
    }
    err = getBits(bitstream, 1, &(channelLayout->layoutSignalingPresent));
    if (err) return(err);
    
    if (channelLayout->layoutSignalingPresent) {
        err = getBits(bitstream, 8, &(channelLayout->definedLayout));
        if (err) return(err);
        /* note that the channel layout index according to ISO/IEC 23001-8 (CICP) doesn't mandatorily define the channel/LFE order */
        
        if (channelLayout->definedLayout == 0)
        {
            for (i=0; i<channelLayout->baseChannelCount; i++)
            {
                err = getBits(bitstream, 7, &(channelLayout->speakerPosition[i]));
                if (err) return(err);
                if (channelLayout->speakerPosition[i] == 3 || channelLayout->speakerPosition[i] == 26) { /* 3 and 26 are an LFE according to ISO/IEC 23001-8 (CICP) */
                    drcParams->lfeChannelMap[i] = 1;
                } else {
                    drcParams->lfeChannelMap[i] = 0;
                }
            }
        }
    }

    return(0);
}

#if MPEG_H_SYNTAX
/* Parser for loundessInfoSet() */
int
parseMpegh3daLoudnessInfoSet(robitbufHandle bitstream,
                             LoudnessInfoSet* loudnessInfoSet)
{
    int i, err = 0, version = 0;

    err = getBits(bitstream, 6, &(loudnessInfoSet->loudnessInfoCount));
    if (err) return(err);
    for (i=0; i < loudnessInfoSet->loudnessInfoCount; i++)
    {
        err = getBits(bitstream, 2, &(loudnessInfoSet->loudnessInfo[i].loudnessInfoType));
        if ((loudnessInfoSet->loudnessInfo[i].loudnessInfoType == 1) || (loudnessInfoSet->loudnessInfo[i].loudnessInfoType == 2))
        {
            err = getBits(bitstream, 7, &(loudnessInfoSet->loudnessInfo[i].mae_groupID));
        }
        else if(loudnessInfoSet->loudnessInfo[i].loudnessInfoType == 3)
        {
            err = getBits(bitstream, 5, &(loudnessInfoSet->loudnessInfo[i].mae_groupPresetID));
        }
        else
        {
            loudnessInfoSet->loudnessInfo[i].mae_groupID = -1;
            loudnessInfoSet->loudnessInfo[i].mae_groupPresetID = -1;
        }
        /* TODO: check version. index offset missing? */
        err = parseLoudnessInfo(bitstream, version, &(loudnessInfoSet->loudnessInfo[i]));
        if (err) return(err);
    }

    err = getBits(bitstream, 1, &(loudnessInfoSet->loudnessInfoAlbumPresent));
    if (err) return(err);

    if (loudnessInfoSet->loudnessInfoAlbumPresent)
    {
        err = getBits(bitstream, 6, &(loudnessInfoSet->loudnessInfoAlbumCount));
        if (err) return(err);
        for (i=0; i < loudnessInfoSet->loudnessInfoAlbumCount; i++)
        {
            loudnessInfoSet->loudnessInfo[i].loudnessInfoType = 0;
            err = parseLoudnessInfo(bitstream, version, &(loudnessInfoSet->loudnessInfoAlbum[i]));
            if (err) return(err);
        }
    }
    else
    {
        loudnessInfoSet->loudnessInfoAlbumCount = 0;
    }

    err = getBits(bitstream, 1, &(loudnessInfoSet->loudnessInfoSetExtPresent));
    if (err) return(err);

    if (loudnessInfoSet->loudnessInfoSetExtPresent)
    {
        err = parseLoudnessInfoSetExtension(bitstream, loudnessInfoSet);
        if (err) return(err);
    }

    return err;
}

int
parseMpegh3daUniDrcChannelLayout(robitbufHandle bitstream,
                                 DrcParamsBsDec* drcParams,
                                 ChannelLayout* channelLayout)
{
    int err = 0;
    
    err = getBits(bitstream, 7, &(channelLayout->baseChannelCount));
    if (err) return(err);

    channelLayout->layoutSignalingPresent = 0;

    return 0;
}
#endif /* MPEG_H_SYNTAX */

#if AMD1_SYNTAX
int
parametricDrcTypeFeedForwardInitializeDrcCurveParameters(int drcCharacteristic,
                                                         ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward)
{
    int* nodeLevel = parametricDrcTypeFeedForward->nodeLevel;
    int* nodeGain  = parametricDrcTypeFeedForward->nodeGain;

    switch (drcCharacteristic) {
        case 7:
            parametricDrcTypeFeedForward->nodeCount = 5;
            nodeLevel[0] = -22; nodeGain[0] = 6;
            nodeLevel[1] = -10; nodeGain[1] = 0;
            nodeLevel[2] = 10;  nodeGain[2] = 0;
            nodeLevel[3] = 20;  nodeGain[3] = -5;
            nodeLevel[4] = 40;  nodeGain[4] = -24;
            break;
        case 8:
            parametricDrcTypeFeedForward->nodeCount = 5;
            nodeLevel[0] = -12; nodeGain[0] = 6;
            nodeLevel[1] = 0;   nodeGain[1] = 0;
            nodeLevel[2] = 5;   nodeGain[2] = 0;
            nodeLevel[3] = 15;  nodeGain[3] = -5;
            nodeLevel[4] = 35;  nodeGain[4] = -24;
            break;
        case 9:
            parametricDrcTypeFeedForward->nodeCount = 4;
            nodeLevel[0] = -34; nodeGain[0] = 12;
            nodeLevel[1] = -10; nodeGain[1] = 0;
            nodeLevel[2] = 10;  nodeGain[2] = 0;
            nodeLevel[3] = 40;  nodeGain[3] = -15;
            break;
        case 10:
            parametricDrcTypeFeedForward->nodeCount = 5;
            nodeLevel[0] = -24; nodeGain[0] = 12;
            nodeLevel[1] = 0;   nodeGain[1] = 0;
            nodeLevel[2] = 5;   nodeGain[2] = 0;
            nodeLevel[3] = 15;  nodeGain[3] = -5;
            nodeLevel[4] = 35;  nodeGain[4] = -24;
            break;
        case 11:
            parametricDrcTypeFeedForward->nodeCount = 5;
            nodeLevel[0] = -19; nodeGain[0] = 15;
            nodeLevel[1] = 0;   nodeGain[1] = 0;
            nodeLevel[2] = 5;   nodeGain[2] = 0;
            nodeLevel[3] = 15;  nodeGain[3] = -5;
            nodeLevel[4] = 35;  nodeGain[4] = -24;
            break;
        default:
            parametricDrcTypeFeedForward->disableParamtricDrc = 1;
    }
    
    return 0;
}

int
parametricDrcTypeFeedForwardInitializeDrcGainSmoothParameters(int drcCharacteristic,
                                                              ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward)
{
    switch (drcCharacteristic) {
        case 7:
            parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow      = 100;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow     = 3000;
            parametricDrcTypeFeedForward->gainSmoothTimeFastPresent     = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast      = 10;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast     = 1000;
            parametricDrcTypeFeedForward->gainSmoothThresholdPresent    = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackThreshold     = 15;
            parametricDrcTypeFeedForward->gainSmoothReleaseThreshold    = 20;
            parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent = 1;
            parametricDrcTypeFeedForward->gainSmoothHoldOff             = 10;
            break;
        case 8:
            parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow      = 100;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow     = 3000;
            parametricDrcTypeFeedForward->gainSmoothTimeFastPresent     = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast      = 10;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast     = 1000;
            parametricDrcTypeFeedForward->gainSmoothThresholdPresent    = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackThreshold     = 15;
            parametricDrcTypeFeedForward->gainSmoothReleaseThreshold    = 20;
            parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent = 1;
            parametricDrcTypeFeedForward->gainSmoothHoldOff             = 10;
            break;
        case 9:
            parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow      = 100;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow     = 3000;
            parametricDrcTypeFeedForward->gainSmoothTimeFastPresent     = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast      = 10;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast     = 1000;
            parametricDrcTypeFeedForward->gainSmoothThresholdPresent    = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackThreshold     = 15;
            parametricDrcTypeFeedForward->gainSmoothReleaseThreshold    = 20;
            parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent = 1;
            parametricDrcTypeFeedForward->gainSmoothHoldOff             = 10;
            break;
        case 10:
            parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow      = 100;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow     = 10000;
            parametricDrcTypeFeedForward->gainSmoothTimeFastPresent     = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast      = 10;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast     = 1000;
            parametricDrcTypeFeedForward->gainSmoothThresholdPresent    = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackThreshold     = 15;
            parametricDrcTypeFeedForward->gainSmoothReleaseThreshold    = 20;
            parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent = 1;
            parametricDrcTypeFeedForward->gainSmoothHoldOff             = 10;
            break;
        case 11:
            parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow      = 100;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow     = 1000;
            parametricDrcTypeFeedForward->gainSmoothTimeFastPresent     = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast      = 10;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast     = 200;
            parametricDrcTypeFeedForward->gainSmoothThresholdPresent    = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackThreshold     = 10;
            parametricDrcTypeFeedForward->gainSmoothReleaseThreshold    = 10;
            parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent = 1;
            parametricDrcTypeFeedForward->gainSmoothHoldOff             = 10;
            break;
        default:
            parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow      = 100;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow     = 3000;
            parametricDrcTypeFeedForward->gainSmoothTimeFastPresent     = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast      = 10;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast     = 1000;
            parametricDrcTypeFeedForward->gainSmoothThresholdPresent    = 1;
            parametricDrcTypeFeedForward->gainSmoothAttackThreshold     = 15;
            parametricDrcTypeFeedForward->gainSmoothReleaseThreshold    = 20;
            parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent = 1;
            parametricDrcTypeFeedForward->gainSmoothHoldOff             = 10;
            break;
    }
    
    return 0;
}

int
parametricDrcTypeFeedForwardInitializeParameters(int drcCharacteristic,
                                             int drcFrameSizeParametricDrc,
                                             ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward)
{
    int err = 0;
    
    parametricDrcTypeFeedForward->levelEstimKWeightingType = 2;
    parametricDrcTypeFeedForward->levelEstimIntegrationTime = drcFrameSizeParametricDrc;
    
    err = parametricDrcTypeFeedForwardInitializeDrcCurveParameters(drcCharacteristic, parametricDrcTypeFeedForward);
    if (err) return(err);
    
    err = parametricDrcTypeFeedForwardInitializeDrcGainSmoothParameters(drcCharacteristic, parametricDrcTypeFeedForward);
    if (err) return(err);

    return 0;
}

int
parseParametricDrcTypeFeedForward(robitbufHandle bitstream,
                                  int drcFrameSizeParametricDrc,
                                  ParametricDrcTypeFeedForward* parametricDrcTypeFeedForward)
{
    int i = 0, err = 0, tmp = 0;

    parametricDrcTypeFeedForward->disableParamtricDrc = 0;
    
    err = getBits(bitstream, 2, &(parametricDrcTypeFeedForward->levelEstimKWeightingType));
    if (err) return(err);
    
    err = getBits(bitstream, 1, &(parametricDrcTypeFeedForward->levelEstimIntegrationTimePresent));
    if (err) return(err);
    
    if (parametricDrcTypeFeedForward->levelEstimIntegrationTimePresent) {
        err = getBits(bitstream, 6, &tmp);
        if (err) return(err);
        parametricDrcTypeFeedForward->levelEstimIntegrationTime = (tmp+1)*drcFrameSizeParametricDrc;
    } else {
        parametricDrcTypeFeedForward->levelEstimIntegrationTime = drcFrameSizeParametricDrc;
    }
    
    err = getBits(bitstream, 1, &(parametricDrcTypeFeedForward->drcCurveDefinitionType));
    if (err) return(err);
    
    if (parametricDrcTypeFeedForward->drcCurveDefinitionType == 0) {
        err = getBits(bitstream, 7, &(parametricDrcTypeFeedForward->drcCharacteristic));
        if (err) return(err);
        err = parametricDrcTypeFeedForwardInitializeDrcCurveParameters(parametricDrcTypeFeedForward->drcCharacteristic, parametricDrcTypeFeedForward);
        if (err) return(err);
    } else {
        parametricDrcTypeFeedForward->drcCharacteristic = 0;
        
        err = getBits(bitstream, 3, &tmp);
        if (err) return(err);
        parametricDrcTypeFeedForward->nodeCount = tmp+2;
        
        for (i=0; i<parametricDrcTypeFeedForward->nodeCount; i++) {
            if (i==0) {
                err = getBits(bitstream, 6, &tmp);
                if (err) return(err);
                parametricDrcTypeFeedForward->nodeLevel[0] = -11-tmp;
            } else {
                err = getBits(bitstream, 5, &tmp);
                if (err) return(err);
                parametricDrcTypeFeedForward->nodeLevel[i] = parametricDrcTypeFeedForward->nodeLevel[i-1]+1+tmp;
            }
            err = getBits(bitstream, 6, &tmp);
            if (err) return(err);
            parametricDrcTypeFeedForward->nodeGain[i] = tmp-39;
        }
    }
    
    err = parametricDrcTypeFeedForwardInitializeDrcGainSmoothParameters(parametricDrcTypeFeedForward->drcCharacteristic, parametricDrcTypeFeedForward);
    if (err) return(err);

    err = getBits(bitstream, 1, &(parametricDrcTypeFeedForward->drcGainSmoothParametersPresent));
    if (err) return(err);
    
    if (parametricDrcTypeFeedForward->drcGainSmoothParametersPresent) {
        err = getBits(bitstream, 8, &tmp);
        if (err) return(err);
        parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow = tmp*5;
        
        err = getBits(bitstream, 8, &tmp);
        if (err) return(err);
        parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow = tmp*40;

        err = getBits(bitstream, 1, &(parametricDrcTypeFeedForward->gainSmoothTimeFastPresent));
        if (err) return(err);
        
        if (parametricDrcTypeFeedForward->gainSmoothTimeFastPresent) {

            err = getBits(bitstream, 8, &tmp);
            if (err) return(err);
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast = tmp*5;
            
            err = getBits(bitstream, 8, &tmp);
            if (err) return(err);
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast = tmp*20;
            
            err = getBits(bitstream, 1, &(parametricDrcTypeFeedForward->gainSmoothThresholdPresent));
            if (err) return(err);
            
            if (parametricDrcTypeFeedForward->gainSmoothThresholdPresent) {
                err = getBits(bitstream, 5, &(parametricDrcTypeFeedForward->gainSmoothAttackThreshold));
                if (err) return(err);
                if ( parametricDrcTypeFeedForward->gainSmoothAttackThreshold == 31) {
                    parametricDrcTypeFeedForward->gainSmoothAttackThreshold = 1000;
                }
                
                err = getBits(bitstream, 5, &(parametricDrcTypeFeedForward->gainSmoothReleaseThreshold));
                if (err) return(err);
                if (parametricDrcTypeFeedForward->gainSmoothReleaseThreshold == 31) {
                    parametricDrcTypeFeedForward->gainSmoothReleaseThreshold = 1000;
                }
            }
        } else {
            parametricDrcTypeFeedForward->gainSmoothAttackTimeFast = parametricDrcTypeFeedForward->gainSmoothAttackTimeSlow;
            parametricDrcTypeFeedForward->gainSmoothReleaseTimeFast = parametricDrcTypeFeedForward->gainSmoothReleaseTimeSlow;
        }
        
        err = getBits(bitstream, 1, &(parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent));
        if (err) return(err);
        
        if (parametricDrcTypeFeedForward->gainSmoothHoldOffCountPresent) {
            err = getBits(bitstream, 7, &(parametricDrcTypeFeedForward->gainSmoothHoldOff));
            if (err) return(err);
        }
    }
    return 0;
}

#ifdef AMD1_PARAMETRIC_LIMITER
int
parseParametricDrcTypeLim(robitbufHandle bitstream,
                          ParametricDrcTypeLim* parametricDrcTypeLim)
{
    int err = 0, tmp = 0;
    
    parametricDrcTypeLim->disableParamtricDrc = 0;
    
    err = getBits(bitstream, 1, &(parametricDrcTypeLim->parametricLimThresholdPresent));
    if (err) return(err);
    
    if (parametricDrcTypeLim->parametricLimThresholdPresent) {
        err = getBits(bitstream, 8, &tmp);
        if (err) return(err);
        parametricDrcTypeLim->parametricLimThreshold = - tmp * 0.125f;
    } else {
        parametricDrcTypeLim->parametricLimThreshold = PARAM_DRC_TYPE_LIM_THRESHOLD_DEFAULT;
    }
    
    err = getBits(bitstream, 1, &(parametricDrcTypeLim->parametricLimReleasePresent));
    if (err) return(err);
    
    if (parametricDrcTypeLim->parametricLimReleasePresent) {
        err = getBits(bitstream, 8, &tmp);
        if (err) return(err);
        parametricDrcTypeLim->parametricLimRelease = tmp*10;
    } else {
        parametricDrcTypeLim->parametricLimRelease = PARAM_DRC_TYPE_LIM_RELEASE_DEFAULT;
    }
    
    parametricDrcTypeLim->parametricLimAttack = PARAM_DRC_TYPE_LIM_ATTACK_DEFAULT;
    parametricDrcTypeLim->drcCharacteristic = 0;
    
    return 0;
}
#endif

int
parseParametricDrcInstructions(robitbufHandle bitstream,
                               int drcFrameSizeParametricDrc,
                               DrcParamsBsDec* drcParams,
                               ParametricDrcInstructions* parametricDrcInstructions)
{
    int i = 0, err = 0;
    int bitSizeLen, lenSizeBits, bitSize, otherBit;

    parametricDrcInstructions->drcCharacteristic = 0;
    parametricDrcInstructions->disableParamtricDrc = 0;
    
    err = getBits(bitstream, 4, &(parametricDrcInstructions->parametricDrcId));
    if (err) return(err);
    
    err = getBits(bitstream, 1, &(parametricDrcInstructions->parametricDrcLookAheadPresent));
    if (err) return(err);
    
    if (parametricDrcInstructions->parametricDrcLookAheadPresent) {
        err = getBits(bitstream, 7, &parametricDrcInstructions->parametricDrcLookAhead);
        if (err) return(err);
    } else {
        parametricDrcInstructions->parametricDrcLookAhead = 0;
    }
    
    err = getBits(bitstream, 1, &(parametricDrcInstructions->parametricDrcPresetIdPresent));
    if (err) return(err);
    
    if (parametricDrcInstructions->parametricDrcPresetIdPresent) {
        err = getBits(bitstream, 7, &(parametricDrcInstructions->parametricDrcPresetId));
        if (err) return(err);
        
        switch (parametricDrcInstructions->parametricDrcPresetId) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
                parametricDrcInstructions->drcCharacteristic = parametricDrcInstructions->parametricDrcPresetId + 7;
                parametricDrcInstructions->parametricDrcType = PARAM_DRC_TYPE_FF;
                err = parametricDrcTypeFeedForwardInitializeParameters(parametricDrcInstructions->drcCharacteristic, drcFrameSizeParametricDrc, &(parametricDrcInstructions->parametricDrcTypeFeedForward));
                if (err) return (err);
                break;
            default:
                parametricDrcInstructions->disableParamtricDrc = 1;
                break;
        }
    } else {
        err = getBits(bitstream, 3, &(parametricDrcInstructions->parametricDrcType));
        if (err) return(err);
        
        if (parametricDrcInstructions->parametricDrcType == PARAM_DRC_TYPE_FF) {
            err = parseParametricDrcTypeFeedForward(bitstream, drcFrameSizeParametricDrc, &(parametricDrcInstructions->parametricDrcTypeFeedForward));
            if (err) return (err);
            parametricDrcInstructions->disableParamtricDrc = parametricDrcInstructions->parametricDrcTypeFeedForward.disableParamtricDrc;
            parametricDrcInstructions->drcCharacteristic = parametricDrcInstructions->parametricDrcTypeFeedForward.drcCharacteristic;
#ifdef AMD1_PARAMETRIC_LIMITER
        } else if (parametricDrcInstructions->parametricDrcType == PARAM_DRC_TYPE_LIM) {
            err = parseParametricDrcTypeLim(bitstream, &(parametricDrcInstructions->parametricDrcTypeLim));
            if (err) return (err);
            parametricDrcInstructions->disableParamtricDrc = parametricDrcInstructions->parametricDrcTypeLim.disableParamtricDrc;
            parametricDrcInstructions->drcCharacteristic = parametricDrcInstructions->parametricDrcTypeLim.drcCharacteristic;
            if (parametricDrcInstructions->parametricDrcLookAheadPresent) {
                parametricDrcInstructions->parametricDrcTypeLim.parametricLimAttack = parametricDrcInstructions->parametricDrcLookAhead;
            }
#endif
        } else {
            err = getBits(bitstream, 3, &bitSizeLen );
            if (err) return(err);
            lenSizeBits = bitSizeLen + 4;
            
            err = getBits(bitstream, lenSizeBits, &bitSize );
            if (err) return(err);
            parametricDrcInstructions->lenBitSize = bitSize + 1;
            
            switch(parametricDrcInstructions->parametricDrcType)
            {
                /* add future extensions here */
                default:
                    parametricDrcInstructions->disableParamtricDrc = 1;
                    for(i = 0; i<parametricDrcInstructions->lenBitSize; i++)
                    {
                        err = getBits(bitstream, 1, &otherBit );
                        if (err) return(err);
                    }
                    break;
            }
        }
    }
    
    return 0;
}

int
parseParametricDrcGainSetParams(robitbufHandle bitstream,
                                 UniDrcConfig* uniDrcConfig,
                                 ParametricDrcGainSetParams* parametricDrcGainSetParams)
{
    int i = 0, err = 0, bsDrcInputLoudness = 0, addChannel = 0, bsChannelWeight = 0;

    err = getBits(bitstream, 4, &(parametricDrcGainSetParams->parametricDrcId));
    if (err) return(err);
    
    err = getBits(bitstream, 3, &(parametricDrcGainSetParams->sideChainConfigType));
    if (err) return(err);
    
    if (parametricDrcGainSetParams->sideChainConfigType) {
        err = getBits(bitstream, 7, &(parametricDrcGainSetParams->downmixId));
        if (err) return(err);
        
        err = getBits(bitstream, 1, &(parametricDrcGainSetParams->levelEstimChannelWeightFormat));
        if (err) return(err);
        
        if (parametricDrcGainSetParams->downmixId == ID_FOR_BASE_LAYOUT) {
            parametricDrcGainSetParams->channelCountFromDownmixId = uniDrcConfig->channelLayout.baseChannelCount;
        } else if (parametricDrcGainSetParams->downmixId == ID_FOR_ANY_DOWNMIX) {
            parametricDrcGainSetParams->channelCountFromDownmixId = 1;
        } else {
            for(i=0; i<uniDrcConfig->downmixInstructionsCount; i++)
            {
                if (parametricDrcGainSetParams->downmixId == uniDrcConfig->downmixInstructions[i].downmixId) break;
            }
            if (i == uniDrcConfig->downmixInstructionsCount)
            {
                fprintf(stderr, "ERROR: downmixInstructions expected but not found: %d\n", uniDrcConfig->downmixInstructionsCount);
                return(UNEXPECTED_ERROR);
            }
            parametricDrcGainSetParams->channelCountFromDownmixId = uniDrcConfig->downmixInstructions[i].targetChannelCount;  /*  channelCountFromDownmixId */
        }
            
        for (i=0; i<parametricDrcGainSetParams->channelCountFromDownmixId; i++) {
            if (parametricDrcGainSetParams->levelEstimChannelWeightFormat == 0) {
                err = getBits(bitstream, 1, &addChannel);
                if (err) return(err);
                parametricDrcGainSetParams->levelEstimChannelWeight[i] = (float)addChannel;
            } else {
                err = getBits(bitstream, 4, &bsChannelWeight);
                if (err) return(err);
                parametricDrcGainSetParams->levelEstimChannelWeight[i] = (float)pow(10.0f, 0.05f * channelWeight[bsChannelWeight]);
            }
        }
    } else {
        parametricDrcGainSetParams->downmixId = 0;
        parametricDrcGainSetParams->channelCountFromDownmixId = 0;
    }
    
    err = getBits(bitstream, 1, &(parametricDrcGainSetParams->drcInputLoudnessPresent));
    if (err) return(err);
    
    if (parametricDrcGainSetParams->drcInputLoudnessPresent) {
        err = getBits(bitstream, 8, &bsDrcInputLoudness);
        if (err) return (err);
        parametricDrcGainSetParams->drcInputLoudness = -57.75f + bsDrcInputLoudness * 0.25f;
    }
    
    return 0;
}

int
parseDrcCoefficientsParametricDrc(robitbufHandle bitstream,
                                  UniDrcConfig* uniDrcConfig,
                                  DrcCoefficientsParametricDrc* drcCoefficientsParametricDrc)
{
    int i = 0, err = 0, code = 0, mu = 0, nu = 0;
    
    err = getBits(bitstream, 4, &(drcCoefficientsParametricDrc->drcLocation));
    if (err) return(err);
    
    err = getBits(bitstream, 1, &(drcCoefficientsParametricDrc->parametricDrcFrameSizeFormat));
    if (err) return(err);
    
#if AMD2_COR2
    if (drcCoefficientsParametricDrc->parametricDrcFrameSizeFormat == 1) {
#else
    if (drcCoefficientsParametricDrc->parametricDrcFrameSize) {
#endif
        err = getBits(bitstream, 15, &code);
        if (err) return(err);
        drcCoefficientsParametricDrc->parametricDrcFrameSize = code + 1;
    } else {
        err = getBits(bitstream, 4, &code);
        if (err) return(err);
        drcCoefficientsParametricDrc->parametricDrcFrameSize = 1 << code;
    }
    
    err = getBits(bitstream, 1, &(drcCoefficientsParametricDrc->parametricDrcDelayMaxPresent));
    if (err) return(err);
    if (drcCoefficientsParametricDrc->parametricDrcDelayMaxPresent) {
        err = getBits(bitstream, 5, &mu);
        if (err) return(err);
        err = getBits(bitstream, 3, &nu);
        if (err) return(err);
        drcCoefficientsParametricDrc->parametricDrcDelayMax = 16 * mu * (1<<nu);
    }

    err = getBits(bitstream, 1, &(drcCoefficientsParametricDrc->resetParametricDrc));
    if (err) return(err);
    
    err = getBits(bitstream, 6, &(drcCoefficientsParametricDrc->parametricDrcGainSetCount));
    if (err) return(err);
    for(i=0; i<drcCoefficientsParametricDrc->parametricDrcGainSetCount; i++)
    {
        err = parseParametricDrcGainSetParams(bitstream, uniDrcConfig, &(drcCoefficientsParametricDrc->parametricDrcGainSetParams[i]));
        if (err) return (err);
    }
    
    return 0;
}

int
generateVirtualGainSetsForParametricDrc(UniDrcConfig* uniDrcConfig)
{
    
    int i = 0, j = 0, cV1 = -1, cV0 = -1, parametricDrcId = 0, drcCharacteristic = 0;
    DrcCoefficientsUniDrc* drcCoefficientsUniDrc;
    ParametricDrcInstructions* parametricDrcInstructions;
    DrcCoefficientsParametricDrc* drcCoefficientsParametricDrc = &(uniDrcConfig->uniDrcConfigExt.drcCoefficientsParametricDrc);
    
    for(i=0; i<uniDrcConfig->drcCoefficientsUniDrcCount; i++)
    {
        if (uniDrcConfig->drcCoefficientsUniDrc[i].drcLocation == drcCoefficientsParametricDrc->drcLocation)
        {
            if (uniDrcConfig->drcCoefficientsUniDrc[i].version == 0)
            {
                cV0 = i;
            }
            else
            {
                cV1 = i;
            }
        }
    }
    if (cV1 >= 0) {
        drcCoefficientsUniDrc = &(uniDrcConfig->drcCoefficientsUniDrc[cV1]);
    }
    else if (cV0 >= 0) {
        drcCoefficientsUniDrc = &(uniDrcConfig->drcCoefficientsUniDrc[cV0]);
    }
    else {
        drcCoefficientsUniDrc = &uniDrcConfig->drcCoefficientsUniDrc[uniDrcConfig->drcCoefficientsUniDrcCount];
        
        drcCoefficientsUniDrc->version = 1;
        drcCoefficientsUniDrc->drcLocation = drcCoefficientsParametricDrc->drcLocation;
        drcCoefficientsUniDrc->drcFrameSizePresent = 0;

        drcCoefficientsUniDrc->gainSetCount = 0;
        drcCoefficientsUniDrc->gainSetCountPlus = 0;
        
#if MPEG_D_DRC_EXTENSION_V1
        drcCoefficientsUniDrc->drcCharacteristicLeftPresent = 0;
        drcCoefficientsUniDrc->drcCharacteristicRightPresent = 0;
        drcCoefficientsUniDrc->shapeFiltersPresent = 0;
        drcCoefficientsUniDrc->gainSequenceCount = 0;
#endif
        uniDrcConfig->drcCoefficientsUniDrcCount += 1;
    }
    drcCoefficientsUniDrc->gainSetCountPlus = drcCoefficientsUniDrc->gainSetCount + drcCoefficientsParametricDrc->parametricDrcGainSetCount;
    for (i=drcCoefficientsUniDrc->gainSetCount; i<drcCoefficientsUniDrc->gainSetCountPlus; i++)
    {
        drcCoefficientsUniDrc->gainSetParams[i].bandCount = 1;
        
        parametricDrcId = uniDrcConfig->uniDrcConfigExt.drcCoefficientsParametricDrc.parametricDrcGainSetParams[i-drcCoefficientsUniDrc->gainSetCount].parametricDrcId;
        
        for(j=0; j<uniDrcConfig->uniDrcConfigExt.parametricDrcInstructionsCount; j++)
        {
            if (parametricDrcId == uniDrcConfig->uniDrcConfigExt.parametricDrcInstructions[j].parametricDrcId) break;
        }
        if (j == uniDrcConfig->uniDrcConfigExt.parametricDrcInstructionsCount)
        {
            fprintf(stderr, "ERROR: parametricDrcInstructions expected but not found.\n");
            return(UNEXPECTED_ERROR);
        }
        parametricDrcInstructions = &uniDrcConfig->uniDrcConfigExt.parametricDrcInstructions[j];
        
        drcCharacteristic = 0;
        if (parametricDrcInstructions->parametricDrcPresetIdPresent) {
            drcCharacteristic = parametricDrcInstructions->drcCharacteristic;
        } else if (parametricDrcInstructions->parametricDrcType == PARAM_DRC_TYPE_FF) {
            if (parametricDrcInstructions->parametricDrcTypeFeedForward.drcCurveDefinitionType == 0) {
                drcCharacteristic = parametricDrcInstructions->parametricDrcTypeFeedForward.drcCharacteristic;
            }
        }
        if (drcCharacteristic != 0) {
#if MPEG_D_DRC_EXTENSION_V1
            drcCoefficientsUniDrc->gainSetParams[i].gainParams[0].drcCharacteristicPresent = 1;
            drcCoefficientsUniDrc->gainSetParams[i].gainParams[0].drcCharacteristicFormatIsCICP = 1;
#endif
            drcCoefficientsUniDrc->gainSetParams[i].gainParams[0].drcCharacteristic = drcCharacteristic;
        } else {
#if MPEG_D_DRC_EXTENSION_V1
            drcCoefficientsUniDrc->gainSetParams[i].gainParams[0].drcCharacteristicPresent = 0;
            drcCoefficientsUniDrc->gainSetParams[i].gainParams[0].drcCharacteristicFormatIsCICP = 0;
#endif
            drcCoefficientsUniDrc->gainSetParams[i].gainParams[0].drcCharacteristic = 0;
        }
    }
    
    return 0;
}
#endif /* AMD1_SYNTAX 1 */

#if MPEG_D_DRC_EXTENSION_V1
int
parseFilterElement(robitbufHandle bitstream,
                   FilterElement* filterElement)
{
    int err = 0;
    
    err = getBits(bitstream, 6, &(filterElement->filterElementIndex));
    if (err) return(err);
    err = getBits(bitstream, 1, &(filterElement->filterElementGainPresent));
    if (err) return(err);
    if (filterElement->filterElementGainPresent) {
        int bsFilterElementGain;
        err = getBits(bitstream, 10, &bsFilterElementGain);
        if (err) return(err);
        filterElement->filterElementGain = bsFilterElementGain * 0.125f - 96.0f;
    }
    return(0);
}

int
parseFilterBlock(robitbufHandle bitstream,
                 FilterBlock* filterBlock)
{
    int err = 0, k;
    
    err = getBits(bitstream, 6, &(filterBlock->filterElementCount));
    if (err) return(err);
    for (k=0; k<filterBlock->filterElementCount; k++) {
        err = parseFilterElement(bitstream, &(filterBlock->filterElement[k]));
        if (err) return(err);
    }
    return(0);
}

int
parseUniqueTdFilterElement(robitbufHandle bitstream,
                           UniqueTdFilterElement* uniqueTdFilterElement)
{
    int err = 0, m, sign;
    float tmp;
    
    err = getBits(bitstream, 1, &(uniqueTdFilterElement->eqFilterFormat));
    if (err) return(err);
    if (uniqueTdFilterElement->eqFilterFormat == 0) {
        int bsRealZeroRadius, bsGenericZeroRadius, bsGenericZeroAngle;
        int bsRealPoleRadius, bsComplexPoleRadius, bsComplexPoleAngle;
        int bsRealZeroRadiusOneCount;
        err = getBits(bitstream, 3, &(bsRealZeroRadiusOneCount));
        if (err) return(err);
        uniqueTdFilterElement->realZeroRadiusOneCount = 2 * bsRealZeroRadiusOneCount;
        err = getBits(bitstream, 6, &(uniqueTdFilterElement->realZeroCount));
        if (err) return(err);
        err = getBits(bitstream, 6, &(uniqueTdFilterElement->genericZeroCount));
        if (err) return(err);
        err = getBits(bitstream, 4, &(uniqueTdFilterElement->realPoleCount));
        if (err) return(err);
        err = getBits(bitstream, 4, &(uniqueTdFilterElement->complexPoleCount));
        if (err) return(err);
        for (m=0; m<uniqueTdFilterElement->realZeroRadiusOneCount; m++)
        {
            err = getBits(bitstream, 1, &uniqueTdFilterElement->zeroSign[m]);
            if (err) return(err);
        }
        for (m=0; m<uniqueTdFilterElement->realZeroCount; m++)
        {
            err = getBits(bitstream, 7, &bsRealZeroRadius);
            if (err) return(err);
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            tmp = 1.0f - zeroPoleRadiusTable[bsRealZeroRadius];
            if (sign == 1) tmp = -tmp;
            uniqueTdFilterElement->realZeroRadius[m] = tmp;
        }
        for (m=0; m<uniqueTdFilterElement->genericZeroCount; m++)
        {
            err = getBits(bitstream, 7, &bsGenericZeroRadius);
            if (err) return(err);
            uniqueTdFilterElement->genericZeroRadius[m] = 1.0f - zeroPoleRadiusTable[bsGenericZeroRadius];
            err = getBits(bitstream, 7, &bsGenericZeroAngle);
            if (err) return(err);
            uniqueTdFilterElement->genericZeroAngle[m] = zeroPoleAngleTable[bsGenericZeroAngle];
        }
        for (m=0; m<uniqueTdFilterElement->realPoleCount; m++)
        {
            err = getBits(bitstream, 7, &bsRealPoleRadius);
            if (err) return(err);
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            tmp = 1.0f - zeroPoleRadiusTable[bsRealPoleRadius];
            if (sign == 1) tmp = -tmp;
            uniqueTdFilterElement->realPoleRadius[m] = tmp;
        }
        for (m=0; m<uniqueTdFilterElement->complexPoleCount; m++)
        {
            err = getBits(bitstream, 7, &bsComplexPoleRadius);
            if (err) return(err);
            uniqueTdFilterElement->complexPoleRadius[m] = 1.0f - zeroPoleRadiusTable[bsComplexPoleRadius];
            err = getBits(bitstream, 7, &bsComplexPoleAngle);
            if (err) return(err);
            uniqueTdFilterElement->complexPoleAngle[m] = zeroPoleAngleTable[bsComplexPoleAngle];
        }
    }
    else {
        err = getBits(bitstream, 7, &(uniqueTdFilterElement->firFilterOrder));
        if (err) return(err);
        err = getBits(bitstream, 1, &(uniqueTdFilterElement->firSymmetry));
        if (err) return(err);
        for (m=0; m<uniqueTdFilterElement->firFilterOrder/2+1; m++)
        {
            int sign, bsFirCoefficient;
            float tmp;
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            err = getBits(bitstream, 10, &bsFirCoefficient);
            if (err) return(err);
            tmp = pow(10.0f, -0.05f * bsFirCoefficient * 0.0625f);
            if (sign) {
                tmp = - tmp;
            }
            uniqueTdFilterElement->firCoefficient[m] = tmp;
        }
    }
    return(0);
}

int
decodeEqSlopeCode(robitbufHandle bitstream,
                  float* eqSlope)
{
    int err = 0, bits = 0;
    err = getBits(bitstream, 1, &bits);
    if (err) return(err);
    if (bits == 0x1) {
        *eqSlope = 0.0f;
    }
    else {
        err = getBits(bitstream, 4, &bits);
        if (err) return(err);
        *eqSlope = eqSlopeTable[bits];
    }
    return(0);
}

int
decodeEqFreqDeltaCode(robitbufHandle bitstream,
                      int* eqFreqDelta)
{
    int err = 0;
    int bits;
    
    err = getBits(bitstream, 4, &bits);
    if (err) return(err);
    *eqFreqDelta = bits+1;
    return(0);
}

int
decodeGainInitialCode(robitbufHandle bitstream,
                      float* eqGainInitial)
{
    int err = 0;
    int bits;
    
    err = getBits(bitstream, 2, &bits);
    if (err) return(err);
    switch (bits) {
        case 0x0:
            err = getBits(bitstream, 5, &bits);
            if (err) return(err);
            *eqGainInitial = 0.5f * bits - 8.0f;
            break;
        case 0x1:
            err = getBits(bitstream, 4, &bits);
            if (err) return(err);
            if (bits<8) {
                *eqGainInitial = bits - 16.0f;
            }
            else {
                *eqGainInitial = (float) bits;
            }
            break;
        case 0x2:
            err = getBits(bitstream, 4, &bits);
            if (err) return(err);
            if (bits<8) {
                *eqGainInitial = 2.0f * bits - 32.0f;
            }
            else {
                *eqGainInitial = 2.0f * bits;
            }
            break;
        case 0x3:
            err = getBits(bitstream, 3, &bits);
            if (err) return(err);
            *eqGainInitial = 4.0f * bits - 64.0f;
            break;
            
        default:
            break;
    }
    return(0);
}

int
decodeGainDeltaCode(robitbufHandle bitstream,
                    float* eqGainDelta)
{
    int err = 0;
    int bits;
    
    err = getBits(bitstream, 5, &bits);
    if (err) return(err);
    *eqGainDelta = eqGainDeltaTable[bits];
    return(0);
}

int
parseEqSubbandGainSpline(robitbufHandle bitstream,
                         const int eqSubbandGainCount,
                         EqSubbandGainSpline* eqSubbandGainSpline)
{
    int err=0, bsEqNodesCount, k;
    err = getBits(bitstream, 5, &bsEqNodesCount);
    if (err) return (err);
    eqSubbandGainSpline->nEqNodes = bsEqNodesCount + 2;
    for (k=0; k<eqSubbandGainSpline->nEqNodes; k++) {
        err = decodeEqSlopeCode(bitstream, &(eqSubbandGainSpline->eqSlope[k]));
        if (err) return (err);
    }
    for (k=1; k<eqSubbandGainSpline->nEqNodes; k++) {
        err = decodeEqFreqDeltaCode(bitstream, &(eqSubbandGainSpline->eqFreqDelta[k]));
        if (err) return (err);
    }
    err = decodeGainInitialCode(bitstream, &(eqSubbandGainSpline->eqGainInitial));
    if (err) return (err);
    for (k=1; k<eqSubbandGainSpline->nEqNodes; k++) {
        err = decodeGainDeltaCode(bitstream, &(eqSubbandGainSpline->eqGainDelta[k]));
        if (err) return (err);
    }
    return (0);
}

int
parseEqSubbandGainVector(robitbufHandle bitstream,
                         const int eqSubbandGainCount,
                         EqSubbandGainVector* eqSubbandGainVector)
{
    int err = 0, m;
    for (m=0; m<eqSubbandGainCount; m++) {
        int sign, bsEqSubbandGain;
        err = getBits(bitstream, 1, &sign);
        if (err) return(err);
        err = getBits(bitstream, 8, &bsEqSubbandGain);
        if (err) return(err);
        if (sign) {
            bsEqSubbandGain = -bsEqSubbandGain;
        }
        eqSubbandGainVector->eqSubbandGain[m] = bsEqSubbandGain * 0.125f;
    }
    return (0);
}

int
parseEqCoefficients(robitbufHandle bitstream,
                    EqCoefficients* eqCoefficients)
{
    int err = 0;
    int j, k, bsEqGainCount, mu, nu;

    err = getBits(bitstream, 1, &(eqCoefficients->eqDelayMaxPresent));
    if (err) return(err);
    if (eqCoefficients->eqDelayMaxPresent) {
        err = getBits(bitstream, 5, &mu);
        if (err) return(err);
        err = getBits(bitstream, 3, &nu);
        if (err) return(err);
        eqCoefficients->eqDelayMax = 16 * mu * (1<<nu);
    }
    
    err = getBits(bitstream, 6, &(eqCoefficients->uniqueFilterBlockCount));
    if (err) return(err);
    for (j=0; j<eqCoefficients->uniqueFilterBlockCount; j++) {
        err = parseFilterBlock (bitstream, &(eqCoefficients->filterBlock[j]));
        if (err) return(err);
    }
    err = getBits(bitstream, 6, &(eqCoefficients->uniqueTdFilterElementCount));
    if (err) return(err);
    for (j=0; j<eqCoefficients->uniqueTdFilterElementCount; j++) {
        err = parseUniqueTdFilterElement (bitstream, &(eqCoefficients->uniqueTdFilterElement[j]));
        if (err) return(err);
    }
    err = getBits(bitstream, 6, &(eqCoefficients->uniqueEqSubbandGainsCount));
    if (err) return(err);
    if (eqCoefficients->uniqueEqSubbandGainsCount>0) {
        err = getBits(bitstream, 1, &(eqCoefficients->eqSubbandGainRepresentation));
        if (err) return(err);
        err = getBits(bitstream, 4, &(eqCoefficients->eqSubbandGainFormat));
        if (err) return(err);
        switch (eqCoefficients->eqSubbandGainFormat) {
            case GAINFORMAT_QMF32:
                eqCoefficients->eqSubbandGainCount = 32;
                break;
            case GAINFORMAT_QMFHYBRID39:
                eqCoefficients->eqSubbandGainCount = 39;
                break;
            case GAINFORMAT_QMF64:
                eqCoefficients->eqSubbandGainCount = 64;
                break;
            case GAINFORMAT_QMFHYBRID71:
                eqCoefficients->eqSubbandGainCount = 71;
                break;
            case GAINFORMAT_QMF128:
                eqCoefficients->eqSubbandGainCount = 128;
                break;
            case GAINFORMAT_QMFHYBRID135:
                eqCoefficients->eqSubbandGainCount = 135;
                break;
            case GAINFORMAT_UNIFORM:
            default:
                err = getBits(bitstream, 8, &bsEqGainCount);
                if (err) return(err);
                eqCoefficients->eqSubbandGainCount = bsEqGainCount + 1;
                break;
        }
        for (k=0; k<eqCoefficients->uniqueEqSubbandGainsCount; k++)
        {
            if (eqCoefficients->eqSubbandGainRepresentation == 1) {
                err = parseEqSubbandGainSpline(bitstream, eqCoefficients->eqSubbandGainCount, &(eqCoefficients->eqSubbandGainSpline[k]));
                if (err) return(err);
            }
            else {
                err = parseEqSubbandGainVector(bitstream, eqCoefficients->eqSubbandGainCount, &(eqCoefficients->eqSubbandGainVector[k]));
                if (err) return(err);
            }
        }
    }
    return(0);
}

int
parseFilterBlockRefs (robitbufHandle bitstream,
                      FilterBlockRefs* filterBlockRefs)
{
    int err = 0, i;
    err = getBits(bitstream, 4, &filterBlockRefs->filterBlockCount);
    if (err) return(err);
    for (i=0; i<filterBlockRefs->filterBlockCount; i++)
    {
        err = getBits(bitstream, 7, &filterBlockRefs->filterBlockIndex[i]);
        if (err) return(err);
    }
    return (0);
}

int
parseTdFilterCascade (robitbufHandle bitstream,
                      EqInstructions* eqInstructions,
                      TdFilterCascade* tdFilterCascade)
{
    int err=0, i, k;
    int bsEqCascadeGain;
    for (i=0; i<eqInstructions->eqChannelGroupCount; i++) {
        err = getBits(bitstream, 1, &(tdFilterCascade->eqCascadeGainPresent[i]));
        if (err) return(err);
        if (tdFilterCascade->eqCascadeGainPresent[i]) {
            err = getBits(bitstream, 10, &bsEqCascadeGain);
            if (err) return(err);
            tdFilterCascade->eqCascadeGain[i] = 0.125f * bsEqCascadeGain - 96.0f;
        }
        else
        {
            tdFilterCascade->eqCascadeGain[i] = 0.0f;
        }
        err = parseFilterBlockRefs (bitstream, &(tdFilterCascade->filterBlockRefs[i]));
        if (err) return(err);
    }
    err = getBits(bitstream, 1, &(tdFilterCascade->eqPhaseAlignmentPresent));
    if (err) return(err);
    {
        for (i=0; i<eqInstructions->eqChannelGroupCount; i++) {
            for (k=i+1; k<eqInstructions->eqChannelGroupCount; k++) {
                if (tdFilterCascade->eqPhaseAlignmentPresent) {
                    err = getBits(bitstream, 1, &(tdFilterCascade->eqPhaseAlignment[i][k]));
                    if (err) return(err);
                }
                else {
                    tdFilterCascade->eqPhaseAlignment[i][k] = 1;
                }
            }
        }
    }
    return(0);
}

int
parseEqInstructions(robitbufHandle bitstream,
                    UniDrcConfig* uniDrcConfig,
                    EqInstructions* eqInstructions)
{
    int err = 0;
    int i, k, channelCount;
    int downmixIdPresent, additionalDownmixIdPresent, additionalDownmixIdCount=0;
    int additionalDrcSetIdPresent, additionalDrcSetIdCount;
    
    err = getBits(bitstream, 6, &(eqInstructions->eqSetId));
    if (err) return(err);
    err = getBits(bitstream, 4, &(eqInstructions->eqSetComplexityLevel));
    if (err) return(err);
    err = getBits(bitstream, 1, &downmixIdPresent);
    if (err) return(err);
    if (downmixIdPresent) {
        err = getBits(bitstream, 7, &(eqInstructions->downmixId[0]));
        if (err) return(err);
        err = getBits(bitstream, 1, &(eqInstructions->eqApplyToDownmix));
        if (err) return(err);
        err = getBits(bitstream, 1, &additionalDownmixIdPresent);
        if (err) return(err);
        if (additionalDownmixIdPresent) {
            err = getBits(bitstream, 7, &additionalDownmixIdCount);
            if (err) return(err);
            for (i=0; i<additionalDownmixIdCount; i++) {
                err = getBits(bitstream, 7, &(eqInstructions->downmixId[1+i]));
                if (err) return(err);
            }
        }
    }
    else {
        eqInstructions->downmixId[0] = 0;
    }
    eqInstructions->downmixIdCount = 1 + additionalDownmixIdCount;
    
    err = getBits(bitstream, 6, &(eqInstructions->drcSetId[0]));
    if (err) return(err);
    err = getBits(bitstream, 1, &additionalDrcSetIdPresent);
    if (err) return(err);
    if (additionalDrcSetIdPresent) {
        err = getBits(bitstream, 6, &additionalDrcSetIdCount);
        if (err) return(err);
        for (i=0; i<additionalDrcSetIdCount; i++) {
            err = getBits(bitstream, 6, &eqInstructions->drcSetId[1+i]);
            if (err) return(err);
        }
    }
    else {
        additionalDrcSetIdCount = 0;
    }
    eqInstructions->drcSetIdCount = 1 + additionalDrcSetIdCount;
    err = getBits(bitstream, 16, &(eqInstructions->eqSetPurpose));
    if (err) return(err);
    err = getBits(bitstream, 1, &(eqInstructions->dependsOnEqSetPresent));
    if (err) return(err);
    if (eqInstructions->dependsOnEqSetPresent) {
        err = getBits(bitstream, 6, &(eqInstructions->dependsOnEqSet));
        if (err) return(err);
    }
    else {
        err = getBits(bitstream, 1, &(eqInstructions->noIndependentEqUse));
        if (err) return(err);
    }
    
    eqInstructions->eqChannelCount = channelCount = uniDrcConfig->channelLayout.baseChannelCount;
    if ((downmixIdPresent == 1) && (eqInstructions->eqApplyToDownmix == 1) &&
        (eqInstructions->downmixId[0] != 0) && (eqInstructions->downmixId[0] != ID_FOR_ANY_DOWNMIX) && (eqInstructions->downmixIdCount==1)) {
        for(i=0; i<uniDrcConfig->downmixInstructionsCount; i++)
        {
            if (eqInstructions->downmixId[0] == uniDrcConfig->downmixInstructions[i].downmixId) break;
        }
        if (i == uniDrcConfig->downmixInstructionsCount)
        {
            fprintf(stderr, "ERROR: downmixInstructions expected but not found: %d\n", uniDrcConfig->downmixInstructionsCount);
            return(UNEXPECTED_ERROR);
        }
        eqInstructions->eqChannelCount = channelCount = uniDrcConfig->downmixInstructions[i].targetChannelCount;  /*  targetChannelCountFromDownmixId*/
    }
    else if ((eqInstructions->downmixId[0] == ID_FOR_ANY_DOWNMIX) || (eqInstructions->downmixIdCount > 1))
    {
        /* TODO downmixIdRequested == 0x7F could also be a downmixed signal --> also for MPEG-H it is always after format converter */
        channelCount = 1; /* eqInstructions->eqChannelCount is still base channel count */
    }
    
    eqInstructions->eqChannelGroupCount = 0;
    for (i=0; i<channelCount; i++) {
        int newGroup = 1;
        err = getBits(bitstream, 7, &(eqInstructions->eqChannelGroupForChannel[i]));
        if (err) return(err);
        for (k=0; k<i; k++) {
            if (eqInstructions->eqChannelGroupForChannel[i] == eqInstructions->eqChannelGroupForChannel[k]) {
                newGroup = 0;
            }
        }
        if (newGroup == 1) {
            eqInstructions->eqChannelGroupCount += 1;
        }
    }
    err = getBits(bitstream, 1, &(eqInstructions->tdFilterCascadePresent));
    if (err) return(err);
    if (eqInstructions->tdFilterCascadePresent) {
        parseTdFilterCascade (bitstream, eqInstructions, &(eqInstructions->tdFilterCascade));
    }
    err = getBits(bitstream, 1, &(eqInstructions->subbandGainsPresent));
    if (err) return(err);
    if (eqInstructions->subbandGainsPresent) {
        for (i=0; i<eqInstructions->eqChannelGroupCount; i++) {
            err = getBits(bitstream, 6, &(eqInstructions->subbandGainsIndex[i]));
            if (err) return(err);
        }
    }
    err = getBits(bitstream, 1, &(eqInstructions->eqTransitionDurationPresent));
    if (err) return(err);
    if (eqInstructions->eqTransitionDurationPresent) {
        int bsEqTransitionDuration;
        err = getBits(bitstream, 5, &(bsEqTransitionDuration));
        if (err) return(err);
        eqInstructions->eqTransitionDuration = 0.001f * pow(2.0f, 2.0f + bsEqTransitionDuration * 0.0625f);
    }
    return(0);
}

int
parseLoudEqInstructions(robitbufHandle bitstream,
                        LoudEqInstructions* loudEqInstructions)
{
    int err = 0;
    int i, bsLoudEqScaling, bsLoudEqOffset;
    int downmixIdPresent, additionalDownmixIdPresent, additionalDownmixIdCount = 0;
    int drcSetIdPresent, additionalDrcSetIdPresent, additionalDrcSetIdCount = 0;
    int eqSetIdPresent, additionalEqSetIdPresent, additionalEqSetIdCount = 0;
    
    err = getBits(bitstream, 4, &(loudEqInstructions->loudEqSetId));
    if (err) return(err);
    err = getBits(bitstream, 4, &(loudEqInstructions->drcLocation));
    if (err) return(err);
    err = getBits(bitstream, 1, &downmixIdPresent);
    if (err) return(err);
    if (downmixIdPresent) {
        err = getBits(bitstream, 7, &(loudEqInstructions->downmixId[0]));
        if (err) return(err);
        err = getBits(bitstream, 1, &additionalDownmixIdPresent);
        if (err) return(err);
        if (additionalDownmixIdPresent) {
            err = getBits(bitstream, 7, &additionalDownmixIdCount);
            if (err) return(err);
            for (i=0; i<additionalDownmixIdCount; i++) {
                err = getBits(bitstream, 7, &(loudEqInstructions->downmixId[i+1]));
                if (err) return(err);
            }
        }
    }
    else {
        loudEqInstructions->downmixId[0] = 0;
    }
    loudEqInstructions->downmixIdCount = 1 + additionalDownmixIdCount;
    
    err = getBits(bitstream, 1, &drcSetIdPresent);
    if (err) return(err);
    if (drcSetIdPresent) {
        err = getBits(bitstream, 6, &(loudEqInstructions->drcSetId[0]));
        if (err) return(err);
        err = getBits(bitstream, 1, &additionalDrcSetIdPresent);
        if (err) return(err);
        if (additionalDrcSetIdPresent) {
            err = getBits(bitstream, 6, &additionalDrcSetIdCount);
            if (err) return(err);
            for (i=0; i<additionalDrcSetIdCount; i++) {
                err = getBits(bitstream, 6, &(loudEqInstructions->drcSetId[i+1]));
                if (err) return(err);
            }
        }
    }
    else
    {
        loudEqInstructions->drcSetId[0] = 0;
    }
    loudEqInstructions->drcSetIdCount = 1 + additionalDrcSetIdCount;
    
    err = getBits(bitstream, 1, &eqSetIdPresent);
    if (err) return(err);
    if (eqSetIdPresent) {
        err = getBits(bitstream, 6, &(loudEqInstructions->eqSetId[0]));
        if (err) return(err);
        err = getBits(bitstream, 1, &additionalEqSetIdPresent);
        if (err) return(err);
        if (additionalEqSetIdPresent) {
            err = getBits(bitstream, 6, &additionalEqSetIdCount);
            if (err) return(err);
            for (i=0; i<additionalEqSetIdCount; i++) {
                err = getBits(bitstream, 6, &(loudEqInstructions->eqSetId[i+1]));
                if (err) return(err);
            }
        }
    }
    else
    {
        loudEqInstructions->eqSetId[0] = 0;
    }
    loudEqInstructions->eqSetIdCount = 1 + additionalEqSetIdCount;
    
    err = getBits(bitstream, 1, &(loudEqInstructions->loudnessAfterDrc));
    if (err) return(err);
    err = getBits(bitstream, 1, &(loudEqInstructions->loudnessAfterEq));
    if (err) return(err);
    err = getBits(bitstream, 6, &(loudEqInstructions->loudEqGainSequenceCount));
    if (err) return(err);
    for (i=0; i<loudEqInstructions->loudEqGainSequenceCount; i++) {
        err = getBits(bitstream, 6, &(loudEqInstructions->gainSequenceIndex[i]));
        if (err) return(err);
        err = getBits(bitstream, 1, &(loudEqInstructions->drcCharacteristicFormatIsCICP[i]));
        if (err) return(err);
        if (loudEqInstructions->drcCharacteristicFormatIsCICP[i]) {
            err = getBits(bitstream, 7, &(loudEqInstructions->drcCharacteristic[i]));
            if (err) return(err);
        }
        else {
            err = getBits(bitstream, 4, &(loudEqInstructions->drcCharacteristicLeftIndex[i]));
            if (err) return(err);
            err = getBits(bitstream, 4, &(loudEqInstructions->drcCharacteristicRightIndex[i]));
            if (err) return(err);
        }
        err = getBits(bitstream, 6, &(loudEqInstructions->frequencyRangeIndex[i]));
        if (err) return(err);
        err = getBits(bitstream, 3, &bsLoudEqScaling);
        if (err) return(err);
        loudEqInstructions->loudEqScaling[i] = pow (2.0f, -0.5f * bsLoudEqScaling);
        err = getBits(bitstream, 5, &bsLoudEqOffset);
        if (err) return(err);
        loudEqInstructions->loudEqOffset[i] = 1.5f * bsLoudEqOffset - 16.0f;
        
    }
    return(0);
}

int
parseDrcExtensionV1(robitbufHandle bitstream,
                    DrcParamsBsDec* drcParams,
                    UniDrcConfig* uniDrcConfig,
                    UniDrcConfigExt* uniDrcConfigExt)
{
    int downmixInstructionsV1Present;
    int downmixInstructionsV1Count;
    int drcCoeffsAndInstructionsUniDrcV1Present;
    int drcCoefficientsUniDrcV1Count;
    int drcInstructionsUniDrcV1Count;
    
    int i = 0, err = 0;
    const int version = 1;
    
    err = getBits(bitstream, 1, &downmixInstructionsV1Present);
    if (err) return(err);
    if (downmixInstructionsV1Present == 1)
    {
        err = getBits(bitstream, 7, &downmixInstructionsV1Count);
        if (err) return(err);
        for (i=0; i<downmixInstructionsV1Count; i++)
        {
            err = parseDownmixInstructions(bitstream, version, drcParams, &uniDrcConfig->channelLayout, &uniDrcConfig->downmixInstructions[i + uniDrcConfig->downmixInstructionsCount]);
            if (err) return(err);
        }
        uniDrcConfig->downmixInstructionsCount += downmixInstructionsV1Count;
    }
    
    err = getBits(bitstream, 1, &drcCoeffsAndInstructionsUniDrcV1Present);
    if (err) return(err);
    if (drcCoeffsAndInstructionsUniDrcV1Present == 1)
    {
        err = getBits(bitstream, 3, &drcCoefficientsUniDrcV1Count);
        if (err) return(err);
        for (i=0; i<drcCoefficientsUniDrcV1Count; i++)
        {
            err = parseDrcCoefficientsUniDrc(bitstream, version, drcParams, &uniDrcConfig->drcCoefficientsUniDrc[i + uniDrcConfig->drcCoefficientsUniDrcCount]);
            if (err) return(err);
        }
        uniDrcConfig->drcCoefficientsUniDrcCount += drcCoefficientsUniDrcV1Count;
        
        err = getBits(bitstream, 6, &drcInstructionsUniDrcV1Count);
        if (err) return(err);
        for (i=0; i<drcInstructionsUniDrcV1Count; i++)
        {
            err = parseDrcInstructionsUniDrc(bitstream, version, uniDrcConfig, &uniDrcConfig->channelLayout, drcParams, &uniDrcConfig->drcInstructionsUniDrc[i + uniDrcConfig->drcInstructionsUniDrcCount]);
            if (err) return(err);
        }
        uniDrcConfig->drcInstructionsUniDrcCount += drcInstructionsUniDrcV1Count;
    }
    
    err = getBits(bitstream, 1, &(uniDrcConfigExt->loudEqInstructionsPresent));
    if (err) return(err);
    if (uniDrcConfigExt->loudEqInstructionsPresent == 1)
    {
        err = getBits(bitstream, 4, &(uniDrcConfigExt->loudEqInstructionsCount));
        if (err) return(err);
        for (i=0; i<uniDrcConfigExt->loudEqInstructionsCount; i++)
        {
            err = parseLoudEqInstructions(bitstream, &uniDrcConfigExt->loudEqInstructions[i]);
            if (err) return(err);
        }
    }
    else
    {
        uniDrcConfigExt->loudEqInstructionsCount = 0;
    }
    
    err = getBits(bitstream, 1, &(uniDrcConfigExt->eqPresent));
    if (err) return(err);
    if (uniDrcConfigExt->eqPresent == 1)
    {
        err = parseEqCoefficients(bitstream, &uniDrcConfigExt->eqCoefficients);
        if (err) return(err);
        err = getBits(bitstream, 4, &(uniDrcConfigExt->eqInstructionsCount));
        if (err) return(err);
        for (i=0; i<uniDrcConfigExt->eqInstructionsCount; i++)
        {
            err = parseEqInstructions(bitstream, uniDrcConfig, &uniDrcConfigExt->eqInstructions[i]);
            if (err) return(err);
        }
    }
    return 0;
}
#endif /* MPEG_D_DRC_EXTENSION_V1 */

int
parseUniDrcConfigExtension(robitbufHandle bitstream,
                           DrcParamsBsDec* drcParams,
                           UniDrcConfig* uniDrcConfig,
                           UniDrcConfigExt* uniDrcConfigExt)
{
    int err = 0, i, k;
    int bitSizeLen, extSizeBits, bitSize, otherBit;
    
    k = 0;
    err = getBits(bitstream, 4, &(uniDrcConfigExt->uniDrcConfigExtType[k]));
    if (err) return(err);
    while(uniDrcConfigExt->uniDrcConfigExtType[k] != UNIDRCCONFEXT_TERM)
    {
        err = getBits(bitstream, 4, &bitSizeLen );
        if (err) return(err);
        extSizeBits = bitSizeLen + 4;

        err = getBits(bitstream, extSizeBits, &bitSize );
        if (err) return(err);
        uniDrcConfigExt->extBitSize[k] = bitSize + 1;

        switch(uniDrcConfigExt->uniDrcConfigExtType[k])
        {
#if AMD1_SYNTAX
            case UNIDRCCONFEXT_PARAM_DRC:
                uniDrcConfigExt->parametricDrcPresent = 1;
                err = parseDrcCoefficientsParametricDrc(bitstream, uniDrcConfig, &(uniDrcConfigExt->drcCoefficientsParametricDrc));
                if (err) return(err);
                err = getBits(bitstream, 4, &uniDrcConfigExt->parametricDrcInstructionsCount);
                if (err) return(err);
                for (i=0; i<uniDrcConfigExt->parametricDrcInstructionsCount; i++) {
                    err = parseParametricDrcInstructions(bitstream, uniDrcConfigExt->drcCoefficientsParametricDrc.parametricDrcFrameSize, drcParams, &(uniDrcConfigExt->parametricDrcInstructions[i]));
                    if (err) return (err);
                }
                break;
#endif /* AMD1_SYNTAX */
#if MPEG_D_DRC_EXTENSION_V1
            case UNIDRCCONFEXT_V1:
                uniDrcConfigExt->drcExtensionV1Present = 1;
                err = parseDrcExtensionV1(bitstream, drcParams, uniDrcConfig, uniDrcConfigExt);
                if (err) return(err);
                break;
#endif /* MPEG_D_DRC_EXTENSION_V1 */
            /* add future extensions here */
            default:
                for(i = 0; i<uniDrcConfigExt->extBitSize[k]; i++)
                {
                    err = getBits(bitstream, 1, &otherBit );
                    if (err) return(err);
                }
                break;
        }
        k++;
        err = getBits(bitstream, 4, &(uniDrcConfigExt->uniDrcConfigExtType[k]));
        if (err) return(err);
    }

    return (0);
}

/* Parser for in-stream DRC configuration */
int
parseUniDrcConfig(robitbufHandle bitstream,
                  DrcParamsBsDec* drcParams,
                  UniDrcConfig* uniDrcConfig
#if MPEG_H_SYNTAX
                  ,LoudnessInfoSet* loudnessInfoSet
#endif
                  )
{
    int i, err = 0;
    const int version = 0;
    
#if MPEG_H_SYNTAX
    /* uniDrcConfig->sampleRate needs to be applied */
    err = getBits(bitstream, 3, &(uniDrcConfig->drcCoefficientsUniDrcCount));
    if (err) return(err);
    err = getBits(bitstream, 6, &(uniDrcConfig->drcInstructionsUniDrcCount));
    if (err) return(err);

    err = parseMpegh3daUniDrcChannelLayout(bitstream, drcParams, &uniDrcConfig->channelLayout);

    for(i=0; i<uniDrcConfig->drcCoefficientsUniDrcCount; i++)
    {
        err = parseDrcCoefficientsUniDrc(bitstream, version, drcParams, &(uniDrcConfig->drcCoefficientsUniDrc[i]));
        if (err) return(err);
    }
    
    for(i=0; i<uniDrcConfig->drcInstructionsUniDrcCount; i++)
    {
        err = getBits(bitstream, 1, &(uniDrcConfig->drcInstructionsUniDrc[i].drcInstructionsType));
        if (uniDrcConfig->drcInstructionsUniDrc[i].drcInstructionsType != 0)
        {
            err = getBits(bitstream, 1, &(uniDrcConfig->drcInstructionsUniDrc[i].drcInstructionsType));
            uniDrcConfig->drcInstructionsUniDrc[i].drcInstructionsType = uniDrcConfig->drcInstructionsUniDrc[i].drcInstructionsType | (1<<1);
            if (uniDrcConfig->drcInstructionsUniDrc[i].drcInstructionsType == 2)
            {
                err = getBits(bitstream, 7, &(uniDrcConfig->drcInstructionsUniDrc[i].mae_groupID));
            }
            else if(uniDrcConfig->drcInstructionsUniDrc[i].drcInstructionsType == 3)
            {
                err = getBits(bitstream, 5, &(uniDrcConfig->drcInstructionsUniDrc[i].mae_groupPresetID));
            }
        }
        else
        {
            uniDrcConfig->drcInstructionsUniDrc[i].mae_groupID = -1;
            uniDrcConfig->drcInstructionsUniDrc[i].mae_groupPresetID = -1;
        }
        err = parseDrcInstructionsUniDrc(bitstream, version, uniDrcConfig, &uniDrcConfig->channelLayout, drcParams, &(uniDrcConfig->drcInstructionsUniDrc[i]));
        if (err) return(err);
    }

    err = getBits(bitstream, 1, &(uniDrcConfig->uniDrcConfigExtPresent));
    if (uniDrcConfig->uniDrcConfigExtPresent == 1)
    {
        err = parseUniDrcConfigExtension(bitstream, drcParams, uniDrcConfig, &(uniDrcConfig->uniDrcConfigExt));
        if (err) return(err);
    }

    err = getBits(bitstream, 1, &(uniDrcConfig->loudnessInfoSetPresent));
    if (uniDrcConfig->loudnessInfoSetPresent == 1)
    {
        err = parseMpegh3daLoudnessInfoSet(bitstream, loudnessInfoSet);
        if (err) return(err);
    }
#else
    err = getBits(bitstream, 1, &(uniDrcConfig->sampleRatePresent));
    if (err) return(err);
    
    if(uniDrcConfig->sampleRatePresent == 1)
    {
        int bsSampleRate;
        err = getBits(bitstream, 18, &bsSampleRate);
        if (err) return(err);
        uniDrcConfig->sampleRate = bsSampleRate + 1000;
    }
#if AMD2_COR2
    else {
        uniDrcConfig->sampleRate = drcParams->sampleRateDefault;
    }
#endif
    
    err = getBits(bitstream, 7, &(uniDrcConfig->downmixInstructionsCount));
    if (err) return(err);
    err = getBits(bitstream, 1, &(uniDrcConfig->drcDescriptionBasicPresent));
    if (err) return(err);
    if (uniDrcConfig->drcDescriptionBasicPresent == 1)
    {
        err = getBits(bitstream, 3, &(uniDrcConfig->drcCoefficientsBasicCount));
        if (err) return(err);
        err = getBits(bitstream, 4, &(uniDrcConfig->drcInstructionsBasicCount));
        if (err) return(err);
    }
    else
    {
        uniDrcConfig->drcCoefficientsBasicCount = 0;
        uniDrcConfig->drcInstructionsBasicCount = 0;
    }
    err = getBits(bitstream, 3, &(uniDrcConfig->drcCoefficientsUniDrcCount));
    if (err) return(err);
    err = getBits(bitstream, 6, &(uniDrcConfig->drcInstructionsUniDrcCount));
    if (err) return(err);
    
    err = parseChannelLayout(bitstream, drcParams, &uniDrcConfig->channelLayout);
    if (err) return(err);
    
    for(i=0; i<uniDrcConfig->downmixInstructionsCount; i++)
    {
        err = parseDownmixInstructions(bitstream, version, drcParams, &uniDrcConfig->channelLayout, &(uniDrcConfig->downmixInstructions[i]));
        if (err) return(err);
    }
    for(i=0; i<uniDrcConfig->drcCoefficientsBasicCount ; i++)
    {
        err = parseDrcCoefficientsBasic(bitstream, &(uniDrcConfig->drcCoefficientsBasic[i]));
        if (err) return(err);
    }
    for(i=0; i<uniDrcConfig->drcInstructionsBasicCount; i++)
    {
        err = parseDrcInstructionsBasic(bitstream, uniDrcConfig, &(uniDrcConfig->drcInstructionsBasic[i]));
        if (err) return(err);
    }
    for(i=0; i<uniDrcConfig->drcCoefficientsUniDrcCount; i++)
    {
        err = parseDrcCoefficientsUniDrc(bitstream, version, drcParams, &(uniDrcConfig->drcCoefficientsUniDrc[i]));
        if (err) return(err);
    }
    for(i=0; i<uniDrcConfig->drcInstructionsUniDrcCount; i++)
    {
        err = parseDrcInstructionsUniDrc(bitstream, version, uniDrcConfig, &uniDrcConfig->channelLayout, drcParams, &(uniDrcConfig->drcInstructionsUniDrc[i]));
        if (err) return(err);
    }

    err = getBits(bitstream, 1, &(uniDrcConfig->uniDrcConfigExtPresent));
    if (err) return(err);

    if (uniDrcConfig->uniDrcConfigExtPresent == 1)
    {
        err = parseUniDrcConfigExtension(bitstream, drcParams, uniDrcConfig, &(uniDrcConfig->uniDrcConfigExt));
        if (err) return(err);
    }
#endif /* MPEG_H_SYNTAX */
    
#if AMD1_SYNTAX
    /* generate virtual gainSets for parametric DRC coefficients */
    if ( uniDrcConfig->uniDrcConfigExt.parametricDrcPresent ) {
        err = generateVirtualGainSetsForParametricDrc(uniDrcConfig);
        if (err) return(err);
    }
#endif /* AMD1_SYNTAX */
    
    for(i=0; i<uniDrcConfig->drcInstructionsUniDrcCount; i++)
    {
        err = generateDrcInstructionsDerivedData(uniDrcConfig, drcParams, &(uniDrcConfig->drcInstructionsUniDrc[i]));
        if (err) return(err);
    }
    
    err = generateDrcInstructionsForDrcOff(uniDrcConfig, drcParams);
    if (err) return(err);
    return(0);
}

/* Parser for in-stream loudnessInfoSet */
int
parseLoudnessInfoSet( robitbufHandle bitstream,
                      DrcParamsBsDec* drcParams,
                      LoudnessInfoSet* loudnessInfoSet)
{
    int err = 0, i, version = 0, offset;
    int loudnessInfoAlbumCount, loudnessInfoCount;
    
    err = getBits(bitstream, 6, &loudnessInfoAlbumCount);
    if (err) return(err);
    err = getBits(bitstream, 6, &loudnessInfoCount);
    if (err) return(err);
    
    offset = loudnessInfoSet->loudnessInfoAlbumCount;    /*  offest should be 0 for now */
    loudnessInfoSet->loudnessInfoAlbumCount += loudnessInfoAlbumCount;
    for (i = 0; i< loudnessInfoSet->loudnessInfoAlbumCount; i++)
    {
        err = parseLoudnessInfo(bitstream, version, &(loudnessInfoSet->loudnessInfoAlbum[i+offset]));
        if (err) return(err);
    }
    
    offset = loudnessInfoSet->loudnessInfoCount;
    loudnessInfoSet->loudnessInfoCount += loudnessInfoCount;
    for (i = 0; i<loudnessInfoSet->loudnessInfoCount; i++)
    {
        err = parseLoudnessInfo(bitstream, version, &(loudnessInfoSet->loudnessInfo[i+offset]));
        if (err) return(err);
    }
    
    err = getBits(bitstream, 1, &(loudnessInfoSet->loudnessInfoSetExtPresent));
    if (err) return(err);
    
    if ( loudnessInfoSet->loudnessInfoSetExtPresent == 1)
    {
        err = parseLoudnessInfoSetExtension(bitstream, loudnessInfoSet);
        if (err) return(err);
    }
    
    return (0);
}

/* ====================================================================================
Parsing of ISOBMFF configuration
====================================================================================*/
#if ISOBMFF_SYNTAX
#if AMD1_SYNTAX
#if !MPEG_H_SYNTAX
int
decGainModifiersIsobmff(robitbufHandle bitstream,
                        const int version,
                        const int bandCount,
                        GainModifiers* gainModifiers)
{
        
    int err = 0, sign, bsGainOffset, bsAttenuationScaling, bsAmplificationScaling, reserved;
    
    if (version > 0) {
        int b;
        for (b=0; b<bandCount; b++) {
            err = getBits(bitstream, 4, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 1, &(gainModifiers->targetCharacteristicLeftPresent[b]));
            if (err) return(err);
            err = getBits(bitstream, 1, &(gainModifiers->targetCharacteristicRightPresent[b]));
            if (err) return(err);
            err = getBits(bitstream, 1, &(gainModifiers->gainScalingPresent[b]));
            if (err) return(err);
            err = getBits(bitstream, 1, &gainModifiers->gainOffsetPresent[b]);
            if (err) return(err);
            
            if (gainModifiers->targetCharacteristicLeftPresent[b]) {
                err = getBits(bitstream, 4, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 4, &(gainModifiers->targetCharacteristicLeftIndex[b]));
                if (err) return(err);
            }
            if (gainModifiers->targetCharacteristicRightPresent[b]) {
                err = getBits(bitstream, 4, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 4, &(gainModifiers->targetCharacteristicRightIndex[b]));
                if (err) return(err);
            }
            if (gainModifiers->gainScalingPresent[b]) {
                err = getBits(bitstream, 4, &(bsAttenuationScaling));
                if (err) return(err);
                gainModifiers->attenuationScaling[b] = bsAttenuationScaling * 0.125f;
                err = getBits(bitstream, 4, &(bsAmplificationScaling));
                if (err) return(err);
                gainModifiers->amplificationScaling[b] = bsAmplificationScaling * 0.125f;
            }
            if (gainModifiers->gainOffsetPresent[b])
            {
                float gainOffset;
                err = getBits(bitstream, 2, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 1, &sign);
                if (err) return(err);
                err = getBits(bitstream, 5, &bsGainOffset);
                if (err) return(err);
                gainOffset = (1+bsGainOffset) * 0.25f;
                if (sign)
                {
                    gainOffset = - gainOffset;
                }
                gainModifiers->gainOffset[b] = gainOffset;
            }
        }
        if (bandCount == 1) {
            err = getBits(bitstream, 1, &(gainModifiers->shapeFilterPresent));
            if (err) return(err);
            if (gainModifiers->shapeFilterPresent) {
                err = getBits(bitstream, 3, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 4, &(gainModifiers->shapeFilterIndex));
                if (err) return(err);
            } else {
                err = getBits(bitstream, 7, &reserved);
                if (err) return(err);
            }
        }
    }
    else if (version == 0)
    {
        int b, gainScalingPresent, gainOffsetPresent;
        float attenuationScaling = 1.0f, amplificationScaling = 1.0f, gainOffset = 0.0f;
        
        err = getBits(bitstream, 7, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 1, &gainScalingPresent);
        if (err) return(err);
        if(gainScalingPresent)
        {
            err = getBits(bitstream, 4, &bsAttenuationScaling);
            if (err) return(err);
            attenuationScaling = bsAttenuationScaling * 0.125f;
            err = getBits(bitstream, 4, &bsAmplificationScaling);
            if (err) return(err);
            amplificationScaling = bsAmplificationScaling * 0.125f;
        }
        err = getBits(bitstream, 7, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 1, &gainOffsetPresent);
        if (err) return(err);
        if(gainOffsetPresent)
        {
            err = getBits(bitstream, 2, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            err = getBits(bitstream, 5, &bsGainOffset);
            if (err) return(err);
            gainOffset = (1+bsGainOffset) * 0.25f;
            if (sign)
            {
                gainOffset = - gainOffset;
            }
        }
        for (b=0; b<bandCount; b++) {
            gainModifiers->targetCharacteristicLeftPresent[b] = 0;
            gainModifiers->targetCharacteristicRightPresent[b] = 0;
            gainModifiers->gainScalingPresent[b] = gainScalingPresent;
            gainModifiers->attenuationScaling[b] = attenuationScaling;
            gainModifiers->amplificationScaling[b] = amplificationScaling;
            gainModifiers->gainOffsetPresent[b] = gainOffsetPresent;
            gainModifiers->gainOffset[b] = gainOffset;
        }
        gainModifiers->shapeFilterPresent = 0;
    }
    return (0);
}
    
int
decDuckingScalingIsobmff(robitbufHandle bitstream,
                         int* duckingScalingPresent,
                         float* duckingScaling)
{
    int err, duckingScalingPresentTmp, bsDuckingScaling, sigma, mu, reserved;
    
    err = getBits(bitstream, 1, &duckingScalingPresentTmp);
    if (err) return(err);
    
    if (duckingScalingPresentTmp == 0)
    {
        *duckingScalingPresent = FALSE;
        *duckingScaling = 1.0f;
    }
    else
    {
        *duckingScalingPresent = TRUE;
        err = getBits(bitstream, 4, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 4, &bsDuckingScaling);
        if (err) return(err);
        
        sigma = bsDuckingScaling >> 3;
        mu = bsDuckingScaling & 0x7;
        
        if (sigma == 0)
        {
            *duckingScaling = 1.0f + 0.125f * (1.0f + mu);
        }
        else
        {
            *duckingScaling = 1.0f - 0.125f * (1.0f + mu);
        }
    }
    return (0);
}
    
int
parseIsobmffDrcInstructionsUniDrc( robitbufHandle bitstream,
                                   const int version,
                                   UniDrcConfig* uniDrcConfig,
                                   DrcParamsBsDec* drcParams,
                                   DrcInstructionsUniDrc* drcInstructionsUniDrc,
                                   int* drcInstructionsUniDrcCount)
{
    int err = 0, c = 0, g = 0, i = 0, j = 0, k = 0;
    int reserved = 0, DRC_instructions_count = 0, idx = 0, match = 0;
    int duckingSequence = -1;
    int downmixIdPresent = 0, additionalDownmixIdCount = 0, bsLimiterPeakTarget = 0;
    int drcChannelCount = 0;
    int uniqueIndex[CHANNEL_COUNT_MAX];
    float uniqueScaling[CHANNEL_COUNT_MAX];
    DrcCoefficientsUniDrc* drcCoefficientsUniDrc = NULL;
    float factor;
    
    if (version >= 1) {
        err = getBits(bitstream, 2, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 6, drcInstructionsUniDrcCount);
        if (err) return(err);
        DRC_instructions_count = *drcInstructionsUniDrcCount;
    } else {
        DRC_instructions_count = 1;
        *drcInstructionsUniDrcCount = *drcInstructionsUniDrcCount + 1;
    }
    
    for (i=0; i<DRC_instructions_count; i++) {
        downmixIdPresent = 1;
        if (version == 0) {
            err = getBits(bitstream, 3, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 6, &(drcInstructionsUniDrc[i].drcSetId));
            if (err) return(err);
            drcInstructionsUniDrc[i].drcSetComplexityLevel = DRC_COMPLEXITY_LEVEL_MAX;
            err = getBits(bitstream, 5, &(drcInstructionsUniDrc[i].drcLocation));
            if (err) return(err);
            err = getBits(bitstream, 7, &(drcInstructionsUniDrc[i].downmixId[0]));
            if (err) return(err);
            if (drcInstructionsUniDrc[i].downmixId[0] == 0) {
                drcInstructionsUniDrc[i].drcApplyToDownmix = 0;
            }
            else {
                drcInstructionsUniDrc[i].drcApplyToDownmix = 1;
            }
            err = getBits(bitstream, 3, &additionalDownmixIdCount);
            if (err) return(err);
            for(j=0; j<additionalDownmixIdCount; j++)
            {
                err = getBits(bitstream, 1, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 7, &(drcInstructionsUniDrc[i].downmixId[j+1]));
                if (err) return(err);
            }
            drcInstructionsUniDrc[i].downmixIdCount = 1 + additionalDownmixIdCount;
        } else {
            err = getBits(bitstream, 6, &(drcInstructionsUniDrc[i].drcSetId));
            if (err) return(err);
            err = getBits(bitstream, 4, &(drcInstructionsUniDrc[i].drcSetComplexityLevel));
            if (err) return(err);
            err = getBits(bitstream, 5, &(drcInstructionsUniDrc[i].drcLocation));
            if (err) return(err);
            err = getBits(bitstream, 1, &downmixIdPresent);
            if (err) return(err);
            if (downmixIdPresent == 1) {
                err = getBits(bitstream, 5, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 7, &(drcInstructionsUniDrc[i].downmixId[0]));
                if (err) return(err);
                err = getBits(bitstream, 1, &drcInstructionsUniDrc[i].drcApplyToDownmix);
                if (err) return(err);
                err = getBits(bitstream, 3, &additionalDownmixIdCount);
                if (err) return(err);
                for(j=0; j<additionalDownmixIdCount; j++)
                {
                    err = getBits(bitstream, 1, &reserved);
                    if (err) return(err);
                    err = getBits(bitstream, 7, &(drcInstructionsUniDrc[i].downmixId[j+1]));
                    if (err) return(err);
                }
                drcInstructionsUniDrc[i].downmixIdCount = 1 + additionalDownmixIdCount;
            } else {
                drcInstructionsUniDrc[i].downmixId[0] = 0;
                drcInstructionsUniDrc[i].downmixIdCount = 1;
            }
        }
        
        err = getBits(bitstream, 16, &(drcInstructionsUniDrc[i].drcSetEffect));
        if (err) return(err);
        
        if ((drcInstructionsUniDrc[i].drcSetEffect & (EFFECT_BIT_DUCK_OTHER | EFFECT_BIT_DUCK_SELF)) == 0)
        {
            err = getBits(bitstream, 7, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 1, &(drcInstructionsUniDrc[i].limiterPeakTargetPresent));
            if (err) return(err);
            if (drcInstructionsUniDrc[i].limiterPeakTargetPresent)
            {
                err = getBits(bitstream, 8, &bsLimiterPeakTarget);
                if (err) return(err);
                drcInstructionsUniDrc[i].limiterPeakTarget = - bsLimiterPeakTarget * 0.125f;
            }
        }
        
        err = getBits(bitstream, 7, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 1, &(drcInstructionsUniDrc[i].drcSetTargetLoudnessPresent));
        if (err) return(err);
        
        /* set default values */
        drcInstructionsUniDrc[i].drcSetTargetLoudnessValueUpper = 0;
        drcInstructionsUniDrc[i].drcSetTargetLoudnessValueLower = -63;
        
        if (drcInstructionsUniDrc[i].drcSetTargetLoudnessPresent == 1)
        {
            int bsDrcSetTargetLoudnessValueUpper, bsDrcSetTargetLoudnessValueLower;
            err = getBits(bitstream, 4, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 6, &bsDrcSetTargetLoudnessValueUpper);
            if (err) return(err);
            drcInstructionsUniDrc[i].drcSetTargetLoudnessValueUpper = bsDrcSetTargetLoudnessValueUpper - 63;
            err = getBits(bitstream, 6, &bsDrcSetTargetLoudnessValueLower);
            if (err) return(err);
            drcInstructionsUniDrc[i].drcSetTargetLoudnessValueLower = bsDrcSetTargetLoudnessValueLower - 63;
        }
        
        if (version == 0) {
            err = getBits(bitstream, 1, &reserved);
            if (err) return(err);
            drcInstructionsUniDrc[i].requiresEq = 0;
        } else {
            err = getBits(bitstream, 1, &(drcInstructionsUniDrc[i].requiresEq));
            if (err) return(err);
        }
        
        err = getBits(bitstream, 6, &(drcInstructionsUniDrc[i].dependsOnDrcSet));
        if (err) return(err);
        
        if (drcInstructionsUniDrc[i].dependsOnDrcSet == 0) {
            err = getBits(bitstream, 1, &(drcInstructionsUniDrc[i].noIndependentUse));
            if (err) return(err);
        } else {
            err = getBits(bitstream, 1, &reserved);
            if (err) return(err);
            drcInstructionsUniDrc[i].noIndependentUse = 0;
        }
        
        err = selectDrcCoefficients(uniDrcConfig, drcInstructionsUniDrc[i].drcLocation, &drcCoefficientsUniDrc);
        if (err) return (err);
        
        err = getBits(bitstream, 8, &drcChannelCount);
        if (err) return(err);
        
        g = 0;
        for (c=0; c<CHANNEL_COUNT_MAX; c++)
        {
            uniqueIndex[c] = -10;
            uniqueScaling[c] = -10.0f;
        }
        for (j=0;j<drcChannelCount;j++) {
            err = getBits(bitstream, 8, &idx);
            if (err) return(err);
            idx = idx - 1;
            drcInstructionsUniDrc[i].channelGroupForChannel[j] = idx;
            match = FALSE;
            if (idx>=0) {
                for (k=0; k<g; k++) {
                    if (uniqueIndex[k] == idx) {
                        match = TRUE;
                        drcInstructionsUniDrc->channelGroupForChannel[j] = k;
                        break;
                    }
                }
                if (match == FALSE)
                {
                    uniqueIndex[g] = idx;
                    g++;
                }
            }
            else {
                drcInstructionsUniDrc->channelGroupForChannel[j] = -1;
            }
        }
        drcInstructionsUniDrc[i].nDrcChannelGroups = g;
        
        for (j=0;j<drcInstructionsUniDrc[i].nDrcChannelGroups;j++) {
            int bsGainSetIndex;
            err = getBits(bitstream, 2, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 6, &bsGainSetIndex);
            if (err) return(err);
            drcInstructionsUniDrc[i].gainSetIndexForChannelGroup[j] = bsGainSetIndex - 1;
        }
        
        /* derive gainSetIndex */
        for (j=0;j<drcChannelCount;j++) {
            if(drcInstructionsUniDrc->channelGroupForChannel[j] == -1) {
                drcInstructionsUniDrc[i].gainSetIndex[j] = -1;
            }
            else {
                drcInstructionsUniDrc[i].gainSetIndex[j] = drcInstructionsUniDrc[i].gainSetIndexForChannelGroup[drcInstructionsUniDrc->channelGroupForChannel[j]];
            }
        }
        
        if (drcInstructionsUniDrc[i].drcSetEffect & (EFFECT_BIT_DUCK_OTHER | EFFECT_BIT_DUCK_SELF))
        {
            duckingSequence = -1;
            if (drcInstructionsUniDrc->drcSetEffect & EFFECT_BIT_DUCK_OTHER) {
                
                for (j=0;j<drcInstructionsUniDrc[i].nDrcChannelGroups;j++) {
                    idx = drcInstructionsUniDrc->gainSetIndex[j];
                    factor = drcInstructionsUniDrc->duckingModifiersForChannelGroup[c].duckingScaling;
                    if (idx >= 0) {
                        if ((duckingSequence >= 0) && (duckingSequence != idx)) {
                            fprintf(stderr, "ERROR: DRC for ducking can only have one ducking gain sequence.\n");
                            return(UNEXPECTED_ERROR);
                        }
                        duckingSequence = idx;
                    }
                }
                for (j=0;j<drcInstructionsUniDrc[i].nDrcChannelGroups;j++) {
                    err = getBits(bitstream, 7, &reserved);
                    if (err) return(err);
                    
                    decDuckingScalingIsobmff(bitstream,
                                             &(drcInstructionsUniDrc[i].duckingModifiersForChannelGroup[j].duckingScalingPresent),
                                             &(drcInstructionsUniDrc[i].duckingModifiersForChannelGroup[j].duckingScaling));
                    
                    drcInstructionsUniDrc[i].bandCountForChannelGroup[j] = 1;
                }
            }
        } else {
            /* get gainModifiers and sequenceIndex for each channel group */
            for (j=0;j<drcInstructionsUniDrc[i].nDrcChannelGroups;j++) {
                /* the DRC channel groups are ordered according to increasing channel indices */
                /* Whenever a new sequenceIndex appears the channel group index is incremented */
                /* The same order has to be observed for all subsequent processing */
                
                if (version == 0) {
                    drcInstructionsUniDrc[i].bandCountForChannelGroup[j] = 1;
                } else {
                    err = getBits(bitstream, 4, &reserved);
                    if (err) return(err);
                    err = getBits(bitstream, 4, &drcInstructionsUniDrc[i].bandCountForChannelGroup[j]);
                    if (err) return(err);
                }
                
                err = decGainModifiersIsobmff(bitstream, version, drcInstructionsUniDrc[i].bandCountForChannelGroup[j], &(drcInstructionsUniDrc[i].gainModifiersForChannelGroup[j]));
                if (err) return (err);
            }
        }
    }
    return 0;
}
    
/* Parser for ISO base media file format 'udi2' box */
int
parseIsobmffUdi2( robitbufHandle bitstream,
                  UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                  UniDrcConfig* uniDrcConfig)
{
        int err = 0, version = 0, flags = 0;
        int drcInstructionsUniDrcCount = 0;
    
        /* version */
        err = getBits(bitstream, 8, &version);
        if (err) return(err);
        
        /* flags */
        err = getBits(bitstream, 24, &flags);
        if (err) return(err);
        
        /* parse content */
        err = parseIsobmffDrcInstructionsUniDrc(bitstream, version, uniDrcConfig, &pUniDrcBsDecStruct->drcParams, &uniDrcConfig->drcInstructionsUniDrc[drcInstructionsUniDrcCount], &drcInstructionsUniDrcCount);
        if (err) return(err);
        
        /* drcInstructionsUniDrcCount */
        uniDrcConfig->drcInstructionsUniDrcCount = drcInstructionsUniDrcCount;
        
        return (0);
}
   
int
parseIsobmffSplitDrcCharacteristic(robitbufHandle bitstream, const int side, SplitDrcCharacteristic* splitDrcCharacteristic)
{
    int err = 0, i = 0, reserved = 0;
    
    err = getBits(bitstream, 7, &reserved);
    if (err) return(err);
    err = getBits(bitstream, 1, &(splitDrcCharacteristic->characteristicFormat));
    if (err) return(err);
    if (splitDrcCharacteristic->characteristicFormat == 0) {
        int bsGain, bsIoRatio, bsExp;
        err = getBits(bitstream, 1, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 6, &bsGain);
        if (err) return(err);
        if (side == LEFT_SIDE) {
            splitDrcCharacteristic->gain = bsGain;
        }
        else {
            splitDrcCharacteristic->gain = - bsGain;
        }
        err = getBits(bitstream, 4, &bsIoRatio);
        if (err) return(err);
        splitDrcCharacteristic->ioRatio = 0.05f + 0.15f * bsIoRatio;
        err = getBits(bitstream, 4, &bsExp);
        if (err) return(err);
        if (bsExp<15) {
            splitDrcCharacteristic->exp = 1.0f + 2.0f * bsExp;
        }
        else {
            splitDrcCharacteristic->exp = 1000.0f;
        }
        err = getBits(bitstream, 1, &(splitDrcCharacteristic->flipSign));
        if (err) return(err);
    }
    else {
        int bsCharacteristicNodeCount, bsNodeLevelDelta, bsNodeGain;
        err = getBits(bitstream, 6, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 2, &(bsCharacteristicNodeCount));
        if (err) return(err);
        splitDrcCharacteristic->characteristicNodeCount = bsCharacteristicNodeCount + 1;
        splitDrcCharacteristic->nodeLevel[0] = DRC_INPUT_LOUDNESS_TARGET;
        splitDrcCharacteristic->nodeGain[0] = 0.0f;
        for (i=1; i<=splitDrcCharacteristic->characteristicNodeCount; i++) {
            err = getBits(bitstream, 3, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 5, &bsNodeLevelDelta);
            if (err) return(err);
            if (side == LEFT_SIDE) {
                splitDrcCharacteristic->nodeLevel[i] = splitDrcCharacteristic->nodeLevel[i-1] - (1.0f + bsNodeLevelDelta);
            }
            else {
                splitDrcCharacteristic->nodeLevel[i] = splitDrcCharacteristic->nodeLevel[i-1] + (1.0f + bsNodeLevelDelta);
            }
            err = getBits(bitstream, 8, &bsNodeGain);
            if (err) return(err);
            splitDrcCharacteristic->nodeGain[i] = 0.5f * bsNodeGain - 64.0f;
        }
    }
    return(0);
}
    
int
parseIsobmffShapeFilterParams(robitbufHandle bitstream, ShapeFilterParams* shapeFilterParams)
{
    int err = 0, reserved = 0;
    err = getBits(bitstream, 3, &reserved);
    if (err) return(err);
    err = getBits(bitstream, 3, &(shapeFilterParams->cornerFreqIndex));
    if (err) return(err);
    err = getBits(bitstream, 2, &(shapeFilterParams->filterStrengthIndex));
    if (err) return(err);
    return (0);
}
    
int
parseIsobmffGainSetParamsCharacteristics(robitbufHandle bitstream,
                                         const int version,
                                         GainParams* gainParams)
{
    int err = 0, reserved = 0;
    
    if (version == 0) {
        err = getBits(bitstream, 1, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 7, &(gainParams->drcCharacteristic));
        if (err) return(err);
        if (gainParams->drcCharacteristic > 0) {
            gainParams->drcCharacteristicPresent = 1;
            gainParams->drcCharacteristicFormatIsCICP = 1;
        }
        else {
            gainParams->drcCharacteristicPresent = 0;
        }
    }
    else {
        err = getBits(bitstream, 1, &(gainParams->drcCharacteristicPresent));
        if (err) return(err);
        err = getBits(bitstream, 1, &(gainParams->drcCharacteristicFormatIsCICP));
        if (err) return(err);
        if (gainParams->drcCharacteristicPresent) {
            if (gainParams->drcCharacteristicFormatIsCICP) {
                err = getBits(bitstream, 1, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 7, &(gainParams->drcCharacteristic));
                if (err) return(err);
            }
            else {
                err = getBits(bitstream, 4, &(gainParams->drcCharacteristicLeftIndex));
                if (err) return(err);
                err = getBits(bitstream, 4, &(gainParams->drcCharacteristicRightIndex));
                if (err) return(err);
            }
        }
    }
    return(0);
}
    
int
parseIsobmffGainSetParamsCrossoverFreqIndex(robitbufHandle bitstream,
                                            GainParams* gainParams,
                                            int drcBandType)
{
    int err = 0, reserved = 0;
    
    if (drcBandType) {
        err = getBits(bitstream, 4, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 4, &(gainParams->crossoverFreqIndex));
        if (err) return(err);
    } else {
        err = getBits(bitstream, 6, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 10, &(gainParams->startSubBandIndex));
        if (err) return(err);
    }
    return(0);
}
    
int
parseIsobmffGainSetParams(robitbufHandle bitstream,
                          const int version,
                          int* gainSequenceIndex,
                          GainSetParams* gainSetParams)
{
    int err = 0, i = 0, reserved = 0;
    
    err = getBits(bitstream, 2, &reserved);
    if (err) return(err);
    err = getBits(bitstream, 2, &(gainSetParams->gainCodingProfile));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->gainInterpolationType));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->fullFrame));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->timeAlignment));
    if (err) return(err);
    err = getBits(bitstream, 1, &(gainSetParams->timeDeltaMinPresent));
    if (err) return(err);
    
    if(gainSetParams->timeDeltaMinPresent)
    {
        int bsTimeDeltaMin;
        err = getBits(bitstream, 5, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 11, &bsTimeDeltaMin);
        if (err) return(err);
        gainSetParams->timeDeltaMin = bsTimeDeltaMin + 1;
    }
    
    if (gainSetParams->gainCodingProfile == GAIN_CODING_PROFILE_CONSTANT) {
        gainSetParams->bandCount = 1;
        *gainSequenceIndex = (*gainSequenceIndex) + 1;
#if AMD2_COR2
        gainSetParams->gainParams[0].gainSequenceIndex = *gainSequenceIndex;
#endif
    }
    else
    {
        err = getBits(bitstream, 3, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 4, &(gainSetParams->bandCount));
        if (err) return(err);
        err = getBits(bitstream, 1, &(gainSetParams->drcBandType));
        if (err) return(err);
        
        for(i=0; i<gainSetParams->bandCount; i++)
        {
            if (version == 0) {
                *gainSequenceIndex = (*gainSequenceIndex) + 1;
            }
            else {
                int bsIndex;
                err = getBits(bitstream, 6, &bsIndex);
                if (err) return(err);
                *gainSequenceIndex = bsIndex;
            }
            gainSetParams->gainParams[i].gainSequenceIndex = *gainSequenceIndex;
            err = parseIsobmffGainSetParamsCharacteristics(bitstream, version, &(gainSetParams->gainParams[i]));
            if (err) return(err);
        }
        for(i=1; i<gainSetParams->bandCount; i++)
        {
            err = parseIsobmffGainSetParamsCrossoverFreqIndex(bitstream, &(gainSetParams->gainParams[i]), gainSetParams->drcBandType);
            if (err) return(err);
        }
    }
    return(0);
}
    
int
parseIsobmffDrcCoefficientsUniDrc( robitbufHandle bitstream,
                                   const int version,
                                   DrcParamsBsDec* drcParams,
                                   DrcCoefficientsUniDrc* drcCoefficientsUniDrc)
{
    int err = 0, i = 0, code = 0;
    int reserved = 0;
    int gainSequenceIndex = -1;
    int gainSequenceCount = 0;
    
    drcCoefficientsUniDrc->version = version;
    
    err = getBits(bitstream, 2, &reserved);
    if (err) return(err);
    
    err = getBits(bitstream, 5, &drcCoefficientsUniDrc->drcLocation);
    if (err) return(err);
    
    err = getBits(bitstream, 1, &drcCoefficientsUniDrc->drcFrameSizePresent);
    if (err) return(err);
    
    if (drcCoefficientsUniDrc->drcFrameSizePresent == 1) {
        err = getBits(bitstream, 1, &reserved);
        if (err) return(err);
        
        err = getBits(bitstream, 15, &code);
        if (err) return(err);
        drcCoefficientsUniDrc->drcFrameSize = code + 1;
    }
    
    if (version == 0) {
        
        drcCoefficientsUniDrc->drcCharacteristicLeftPresent = 0;
        drcCoefficientsUniDrc->drcCharacteristicRightPresent = 0;
        drcCoefficientsUniDrc->shapeFiltersPresent = 0;
        
    } else {
        
        for (i=0; i<SEQUENCE_COUNT_MAX; i++) {
            drcCoefficientsUniDrc->gainSetParamsIndexForGainSequence[i] = -1;
        }
        
        err = getBits(bitstream, 5, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 1, &drcCoefficientsUniDrc->drcCharacteristicLeftPresent);
        if (err) return(err);
        err = getBits(bitstream, 1, &drcCoefficientsUniDrc->drcCharacteristicRightPresent);
        if (err) return(err);
        err = getBits(bitstream, 1, &drcCoefficientsUniDrc->shapeFiltersPresent);
        if (err) return(err);
        
        if (drcCoefficientsUniDrc->drcCharacteristicLeftPresent == 1) {
            err = getBits(bitstream, 4, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 4, &drcCoefficientsUniDrc->characteristicLeftCount);
            if (err) return(err);
            
            for (i=1;i<=drcCoefficientsUniDrc->characteristicLeftCount;i++) {
                err = parseIsobmffSplitDrcCharacteristic(bitstream, LEFT_SIDE, &(drcCoefficientsUniDrc->splitCharacteristicLeft[i]));
                if (err) return(err);
            }
            
        }
        
        if (drcCoefficientsUniDrc->drcCharacteristicRightPresent == 1) {
            err = getBits(bitstream, 4, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 4, &drcCoefficientsUniDrc->characteristicRightCount);
            if (err) return(err);
            
            for (i=1;i<=drcCoefficientsUniDrc->characteristicRightCount;i++) {
                err = parseIsobmffSplitDrcCharacteristic(bitstream, RIGHT_SIDE, &(drcCoefficientsUniDrc->splitCharacteristicRight[i]));
                if (err) return(err);
            }
        }
        
        if (drcCoefficientsUniDrc->shapeFiltersPresent == 1) {
            ShapeFilterBlockParams* shapeFilterBlockParams;
            err = getBits(bitstream, 4, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 4, &(drcCoefficientsUniDrc->shapeFilterCount));
            if (err) return(err);
            for (i=1; i<=drcCoefficientsUniDrc->shapeFilterCount; i++) {
                shapeFilterBlockParams = &(drcCoefficientsUniDrc->shapeFilterBlockParams[i]);
                err = getBits(bitstream, 4, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->lfCutFilterPresent));
                if (err) return(err);
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->lfBoostFilterPresent));
                if (err) return(err);
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->hfCutFilterPresent));
                if (err) return(err);
                err = getBits(bitstream, 1, &(shapeFilterBlockParams->hfBoostFilterPresent));
                if (err) return(err);
                
                if (shapeFilterBlockParams->lfCutFilterPresent == 1) {
                    err = parseIsobmffShapeFilterParams(bitstream, &(shapeFilterBlockParams->lfCutParams));
                    if (err) return(err);
                }
                if (shapeFilterBlockParams->lfBoostFilterPresent == 1) {
                    err = parseIsobmffShapeFilterParams(bitstream, &(shapeFilterBlockParams->lfBoostParams));
                    if (err) return(err);
                }
                if (shapeFilterBlockParams->hfCutFilterPresent == 1) {
                    err = parseIsobmffShapeFilterParams(bitstream, &(shapeFilterBlockParams->hfCutParams));
                    if (err) return(err);
                }
                if (shapeFilterBlockParams->hfBoostFilterPresent == 1) {
                    err = parseIsobmffShapeFilterParams(bitstream, &(shapeFilterBlockParams->hfBoostParams));
                    if (err) return(err);
                }
            }
        }
    }
        
    err = getBits(bitstream, 1, &reserved);
    if (err) return(err);
    err = getBits(bitstream, 1, &(drcParams->delayMode));
    if (err) return(err);
        
    if (version >= 1) {
        err = getBits(bitstream, 2, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 6, &(drcCoefficientsUniDrc->gainSequenceCount));
        if (err) return(err);
    }
    
    err = getBits(bitstream, 6, &(drcCoefficientsUniDrc->gainSetCount));
    if (err) return(err);
    
    drcCoefficientsUniDrc->gainSetCountPlus = drcCoefficientsUniDrc->gainSetCount;

    for (i=0; i<drcCoefficientsUniDrc->gainSetCount; i++) {
        err = parseIsobmffGainSetParams(bitstream, version, &gainSequenceIndex, &(drcCoefficientsUniDrc->gainSetParams[i]));
        if (err) return (err);
        
        if (drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMinPresent)
        {
            if (drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin > drcParams->drcFrameSize) /* assumes that provided audioFrameSize is equal to present drcFrameSize in bitstream */
            {
                fprintf(stderr, "ERROR: DRC time interval (deltaTmin) cannot exceed audio frame size. %d %d\n", drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin, drcParams->drcFrameSize);
                return(PARAM_ERROR);
            }
            drcCoefficientsUniDrc->gainSetParams[i].nGainValuesMax = drcParams->drcFrameSize / drcCoefficientsUniDrc->gainSetParams[i].timeDeltaMin;
            err = initTables(drcCoefficientsUniDrc->gainSetParams[i].nGainValuesMax, &(drcCoefficientsUniDrc->gainSetParams[i].tables));
            if (err) return (err);
        }
        if (version == 0) {
            gainSequenceCount += drcCoefficientsUniDrc->gainSetParams[i].bandCount;
        }
    }
    if (version == 0) {
        drcCoefficientsUniDrc->gainSequenceCount = gainSequenceCount;
    }
    for(i=0; i<drcCoefficientsUniDrc->gainSetCount; i++)
    {
        int b;
        for (b=0; b<drcCoefficientsUniDrc->gainSetParams[i].bandCount; b++) {
            drcCoefficientsUniDrc->gainSetParamsIndexForGainSequence[drcCoefficientsUniDrc->gainSetParams[i].gainParams[b].gainSequenceIndex] = i;
        }
    }    
    return 0;
}
    
/* Parser for ISO base media file format 'udc2' box */
int
parseIsobmffUdc2( robitbufHandle bitstream,
                  UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                  UniDrcConfig* uniDrcConfig)
{
    int err = 0, version = 0, flags = 0;
    int drcCoefficientsUniDrcCount = uniDrcConfig->drcCoefficientsUniDrcCount;
    
    /* version */
    err = getBits(bitstream, 8, &version);
    if (err) return(err);
        
    /* flags */
    err = getBits(bitstream, 24, &flags);
    if (err) return(err);
    
    /* parse content */
    err = parseIsobmffDrcCoefficientsUniDrc(bitstream, version, &pUniDrcBsDecStruct->drcParams, &uniDrcConfig->drcCoefficientsUniDrc[drcCoefficientsUniDrcCount]);
    if (err) return(err);
    
    /* drcCoefficientsUniDrcCount */
    uniDrcConfig->drcCoefficientsUniDrcCount = drcCoefficientsUniDrcCount + 1;
        
    return (0);
}
    
int
parseIsobmffDownmixInstructions( robitbufHandle bitstream,
                                 const int version,
                                 DrcParamsBsDec* drcParams,
                                 ChannelLayout* channelLayout,
                                 DownmixInstructions* downmixInstructions,
                                 int *downmixInstructionsCount)
{
    int err = 0, i = 0, j = 0, k = 0, l = 0, size = 0;
    int reserved = 0, in_stream = 0, downmix_instructions_count = 0;
    
    if (version >= 1) {
        err = getBits(bitstream, 1, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 7, downmixInstructionsCount);
        if (err) return(err);
        downmix_instructions_count = *downmixInstructionsCount;
    } else {
        downmix_instructions_count = 1;
        *downmixInstructionsCount = *downmixInstructionsCount + 1;
    }
    
    for (i=0; i<downmix_instructions_count;i++) {
        
        err = getBits(bitstream, 8, &downmixInstructions[i].targetLayout);
        if (err) return(err);
        
        err = getBits(bitstream, 1, &reserved);
        if (err) return(err);
        
        err = getBits(bitstream, 7, &downmixInstructions[i].targetChannelCount);
        if (err) return(err);
        
        err = getBits(bitstream, 1, &in_stream);
        if (err) return(err);
        
        err = getBits(bitstream, 7, &downmixInstructions[i].downmixId);
        if (err) return(err);
        
        if (in_stream == 0) {
            downmixInstructions[i].downmixCoefficientsPresent = 1;
            
            if (version >= 1) {
                int bsDownmixCoefficientV1, bsDownmixOffset;
                float a, b, downmixOffset, sum;
                
                err = getBits(bitstream, 4, &bsDownmixOffset);
                if (err) return(err);
                size = 4;
                l=0;
                k = robitbuf_GetBitsAvail(bitstream);
                for (j=0; j<downmixInstructions[i].targetChannelCount; j++)
                {
                    for (k=0; k<channelLayout->baseChannelCount; k++) {
                        err = getBits(bitstream, 5, &bsDownmixCoefficientV1);
                        if (err) return(err);
                        downmixInstructions[i].downmixCoefficient[l] = downmixCoeffV1[bsDownmixCoefficientV1];
                        l++;
                        size += 5;
                    }
                }
                size = size % 8;
                if (size) {
                    err = getBits(bitstream, 8-size, &reserved);
                    if (err) return(err);
                }
                switch (bsDownmixOffset) {
                    case 0:
                        downmixOffset = 0.0f;
                        break;
                    case 1:
                        a = 20.0f * log10((float) downmixInstructions[i].targetChannelCount / (float)channelLayout->baseChannelCount);
                        downmixOffset = 0.5f * floor(0.5f + a);
                        break;
                    case 2:
                        a = 20.0f * log10((float) downmixInstructions[i].targetChannelCount / (float)channelLayout->baseChannelCount);
                        downmixOffset = 0.5f * floor(0.5f + 2.0f * a);
                        break;
                    case 3:
                        sum = 0.0f;
                        for (k=0; k<downmixInstructions[i].targetChannelCount * channelLayout->baseChannelCount; k++)
                        {
                            sum += pow(10.0f, 0.1f * downmixInstructions[i].downmixCoefficient[k]);
                        }
                        b = 10.0f * log10(sum);
                        downmixOffset = 0.5f * floor(0.5f + 2.0f * b);
                        break;
                    default:
                        /* Error: this case is not allowed */
                        return (BITSTREAM_ERROR);
                        break;
                }
                for (k=0; k<downmixInstructions[i].targetChannelCount * channelLayout->baseChannelCount; k++)
                {
                    downmixInstructions[i].downmixCoefficient[k] = (float)pow(10.0f, 0.05f * (downmixInstructions[i].downmixCoefficient[k] + downmixOffset));
                }
            } else {
                int bsDownmixCoefficient;
                l = 0;
                for (j=0; j<downmixInstructions[i].targetChannelCount; j++)
                {
                    for (k=0; k<channelLayout->baseChannelCount; k++) {
                        err = getBits(bitstream, 4, &bsDownmixCoefficient);
                        if (err) return(err);
                        if (drcParams->lfeChannelMap[k]) {
                            downmixInstructions[i].downmixCoefficient[l] = (float)pow(10.0f, 0.05f * downmixCoeffLfe[bsDownmixCoefficient]);
                        } else {
                            downmixInstructions[i].downmixCoefficient[l] = (float)pow(10.0f, 0.05f * downmixCoeff[bsDownmixCoefficient]);
                        }
                        l++;
                    }
                }
            }
        }
    }
    
    return(0);
}
    
/* Parser for ISO base media file format 'dmix' box */
int
parseIsobmffDmix( robitbufHandle bitstream,
                  UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                  UniDrcConfig* uniDrcConfig)
{
    int err = 0, version = 0, flags = 0;
    int downmixInstructionsCount = 0;
    
    /* version */
    err = getBits(bitstream, 8, &version);
    if (err) return(err);
    
    /* flags */
    err = getBits(bitstream, 24, &flags);
    if (err) return(err);
    
    /* parse content */
    err = parseIsobmffDownmixInstructions(bitstream, version, &pUniDrcBsDecStruct->drcParams, &uniDrcConfig->channelLayout, uniDrcConfig->downmixInstructions, &downmixInstructionsCount);
    if (err) return(err);
    
    /* downmixInstructionsCount */
    uniDrcConfig->downmixInstructionsCount = downmixInstructionsCount;
    
    return (0);
}
    
/* Parser for ISO base media file format 'chnl' box */
int
parseIsobmffChnl( robitbufHandle bitstream,
                  UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                  UniDrcConfig* uniDrcConfig)
{
    int err = 0, i = 0, version = 0, flags = 0, reserved = 0, code = 0;
    int stream_structure = 0, channelStructured = 1, objectStructured = 2;
    int format_ordering = 0, channel_order_definition = 0, omitted_channels_present = 0;
    int objectCount = 0, layout_channel_count = 0;
    int azimuth[SPEAKER_POS_COUNT_MAX] = {0}, elevation[SPEAKER_POS_COUNT_MAX] = {0};
    
    /* version */
    err = getBits(bitstream, 8, &version);
    if (err) return(err);
        
    /* flags */
    err = getBits(bitstream, 24, &flags);
    if (err) return(err);
    
    if (version == 0) {
        err = getBits(bitstream, 8, &stream_structure);
        if (err) return(err);
        if (stream_structure & channelStructured) {
            err = getBits(bitstream, 8, &uniDrcConfig->channelLayout.definedLayout);
            if (err) return(err);
            if (uniDrcConfig->channelLayout.definedLayout == 0) {
                /* unsupported layout_channel_count from sample entry */
                return -1;
                for (i = 0; i<layout_channel_count; i++) {
                    err = getBits(bitstream, 8, &uniDrcConfig->channelLayout.speakerPosition[i]);
                    if (err) return(err);
                    if (uniDrcConfig->channelLayout.speakerPosition[i] == 126) {
                        err = getBits(bitstream, 16, &azimuth[i]);
                        if (err) return(err);
                        err = getBits(bitstream, 8, &elevation[i]);
                        if (err) return(err);
                    }
                }
            } else {
                code  = 0;
                /* omittedChannelsMap not supported */
                err = getBits(bitstream, 32, &code);
                if (err) return(err);
                err = getBits(bitstream, 32, &code);
                if (err) return(err);
            }
        }
        if (stream_structure & objectStructured) {
            err = getBits(bitstream, 8, &objectCount);
            if (err) return(err);
        }
    } else {
        err = getBits(bitstream, 4, &stream_structure);
        if (err) return(err);
        err = getBits(bitstream, 4, &format_ordering);
        if (err) return(err);
        err = getBits(bitstream, 8, &uniDrcConfig->channelLayout.baseChannelCount);
        if (err) return(err);
        if (stream_structure & channelStructured) {
            err = getBits(bitstream, 8, &uniDrcConfig->channelLayout.definedLayout);
            if (err) return(err);
            if (uniDrcConfig->channelLayout.definedLayout == 0) {
                err = getBits(bitstream, 8, &layout_channel_count);
                if (err) return(err);
                for (i = 0; i<layout_channel_count; i++) {
                    err = getBits(bitstream, 8, &uniDrcConfig->channelLayout.speakerPosition[i]);
                    if (err) return(err);
                    if (uniDrcConfig->channelLayout.speakerPosition[i] == 126) {
                        err = getBits(bitstream, 16, &azimuth[i]);
                        if (err) return(err);
                        err = getBits(bitstream, 8, &elevation[i]);
                        if (err) return(err);
                    }
                }
            } else {
                err = getBits(bitstream, 4, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 3, &channel_order_definition);
                if (err) return(err);
                err = getBits(bitstream, 1, &omitted_channels_present);
                if (err) return(err);
                if (omitted_channels_present == 1) {
                    code  = 0;
                    /* omittedChannelsMap not supported */
                    err = getBits(bitstream, 32, &code);
                    if (err) return(err);
                    err = getBits(bitstream, 32, &code);
                    if (err) return(err);
                }
            }
        }
        if (stream_structure & objectStructured) {
            objectCount = uniDrcConfig->channelLayout.baseChannelCount - layout_channel_count;
        }
    }
    
    return (0);
}
    
int
parseIsobmffLoudnessMeasure(robitbufHandle bitstream,
                                    LoudnessMeasure* loudnessMeasure)
{
    int err = 0, reserved = 0;
        
    err = getBits(bitstream, 8, &(loudnessMeasure->methodDefinition));
    if (err) return(err);
    if (loudnessMeasure->methodDefinition == 7) {
        err = getBits(bitstream, 3, &reserved);
        if (err) return(err);
    } else if (loudnessMeasure->methodDefinition == 8) {
        err = getBits(bitstream, 6, &reserved);
        if (err) return(err);
    }
    err = decMethodValue(bitstream, loudnessMeasure->methodDefinition, &(loudnessMeasure->methodValue));
    if (err) return(err);
    err = getBits(bitstream, 4, &(loudnessMeasure->measurementSystem));
    if (err) return(err);
    err = getBits(bitstream, 4, &(loudnessMeasure->reliability));
    if (err) return(err);
    return (0);
}
    
int
parseIsobmffLoudnessInfo( robitbufHandle bitstream,
                          const int version,
                          LoudnessInfo* loudnessInfo,
                          int *loudnessInfoCount)
{
    int err = 0, bsSamplePeakLevel = 0, bsTruePeakLevel = 0, i = 0, j = 0;
    int code = 0, reserved = 0, loudnessBaseCount = 0, startCount = 0;
    
    if (version >= 1) {
        err = getBits(bitstream, 2, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 6, loudnessInfoCount);
        if (err) return(err);
        loudnessBaseCount = *loudnessInfoCount;
    } else {
        startCount = *loudnessInfoCount;
        *loudnessInfoCount = *loudnessInfoCount + 1;
        loudnessBaseCount = *loudnessInfoCount;
    }
    
    for (i=startCount; i<loudnessBaseCount;i++) {
        if (version >= 1) {
            err = getBits(bitstream, 2, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 6, &code);
            if (err) return(err);
            loudnessInfo[i].eqSetId = code;
        }
        
        err = getBits(bitstream, 3, &reserved);
        if (err) return(err);
        
        err = getBits(bitstream, 7, &loudnessInfo[i].downmixId);
        if (err) return(err);
        
        err = getBits(bitstream, 6, &loudnessInfo[i].drcSetId);
        if (err) return(err);
        
        err = getBits(bitstream, 12, &bsSamplePeakLevel);
        if (err) return(err);
        if (bsSamplePeakLevel == 0)
        {
            loudnessInfo[i].samplePeakLevelPresent = 0;
            loudnessInfo[i].samplePeakLevel = 0.0f;
        }
        else
        {
            loudnessInfo[i].samplePeakLevelPresent = 1;
            loudnessInfo[i].samplePeakLevel = 20.0f - bsSamplePeakLevel * 0.03125f;
        }
        
        err = getBits(bitstream, 12, &bsTruePeakLevel);
        if (err) return(err);
        if (bsTruePeakLevel == 0)
        {
            loudnessInfo[i].truePeakLevelPresent = 0;
            loudnessInfo[i].truePeakLevel = 0.0f;
        }
        else
        {
            loudnessInfo[i].truePeakLevelPresent = 1;
            loudnessInfo[i].truePeakLevel = 20.0f - bsTruePeakLevel * 0.03125f;
        }
        
        err = getBits(bitstream, 4, &(loudnessInfo[i].truePeakLevelMeasurementSystem));
        if (err) return(err);
        err = getBits(bitstream, 4, &(loudnessInfo[i].truePeakLevelReliability));
        if (err) return(err);
        
        err = getBits(bitstream, 8, &(loudnessInfo[i].measurementCount));
        if (err) return(err);
        
        for (j=0; j<loudnessInfo[i].measurementCount; j++)
        {
            err = parseIsobmffLoudnessMeasure(bitstream, &(loudnessInfo[i].loudnessMeasure[j]));
            if (err) return(err);
        }
    }
    
    return(0);
}
    
/* Parser for ISO base media file format 'ludt' box */
int
parseIsobmffLudt( robitbufHandle bitstream,
                  int *sizeLeft,
                  LoudnessInfoSet* loudnessInfoSet)
    {
        int err = 0, version = 0, flags = 0, size = 0, code = 0, i, sizeLeftInternal;
        int loudnessInfoAlbumCount = 0, loudnessInfoCount = 0;
        char type[5];
        char tlou[] = "tlou";
        char alou[] = "alou";
        
        /* switch */
        while ( *sizeLeft ) {
            /* class LoudnessBaseBox extends FullBox(loudnessType) */
            /* box size */
            err = getBits(bitstream, 32, &size);
            if (err) return(err);
            
            /* box type */
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[0] = (char)code;
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[1] = (char)code;
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[2] = (char)code;
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[3] = (char)code;
            type[4] = '\0';
            
            /* extended size */
            if (size == 1) {
                /* currently unsupported */
                return -1;
            } else if (size == 0) {
                size = robitbuf_GetBitsRead(bitstream);
            }

            /* version */
            err = getBits(bitstream, 8, &version);
            if (err) return(err);
            
            /* flags */
            err = getBits(bitstream, 24, &flags);
            if (err) return(err);
            
            sizeLeftInternal = size - 12; /* 8 bytes for the box size and box type, 1 byte for version, and 3 bytes for flags */
            
            /* switch 'tlou' and 'alou' */
            if (strcmp(type, tlou) == 0) {
                
                /* loudnessBaseBox */
                err = parseIsobmffLoudnessInfo(bitstream, version, loudnessInfoSet->loudnessInfo, &loudnessInfoCount);
                if (err) return(err);
                
            } else if (strcmp(type, alou) == 0) {
                
                /* loudnessBaseBox */
                err = parseIsobmffLoudnessInfo(bitstream, version, loudnessInfoSet->loudnessInfoAlbum, &loudnessInfoAlbumCount);
                if (err) return(err);
                
            } else {
                
                fprintf(stderr, "WARNING: Unknown ISOBMFF box discarded.\n");
                
                /* discard bytes of unknown box */
                for (i=0; i<sizeLeftInternal; i++) {
                    err = getBits(bitstream, 8, &code);
                    if (err) return(err);
                }
            }
            
            /* left size */
            *sizeLeft = *sizeLeft - size;
        }
        
        /* loudnessInfoCount */
        loudnessInfoSet->loudnessInfoCount = loudnessInfoCount;
        loudnessInfoSet->loudnessInfoAlbumCount = loudnessInfoAlbumCount;
        
        return (0);
}

int
parseIsobmffPdc1( robitbufHandle bitstream,
                  DrcCoefficientsParametricDrc *drcCoefficientsParametricDrc)
{
    int err = 0, bsSamplePeakLevel = 0, bsTruePeakLevel = 0, i = 0, j = 0;
    int code = 0, reserved = 0, loudnessBaseCount = 0, startCount = 0; /*TODO check for unused variables */
    
    err = getBits(bitstream, 1, &reserved);
    if (err) return(err);
    err = getBits(bitstream, 5, &drcCoefficientsParametricDrc->drcLocation);
    if (err) return(err);
    err = getBits(bitstream, 1, &drcCoefficientsParametricDrc->parametricDrcFrameSizeFormat);
    if (err) return(err);
    err = getBits(bitstream, 1, &drcCoefficientsParametricDrc->resetParametricDrc);
    if (err) return(err);
    
    if(drcCoefficientsParametricDrc->parametricDrcFrameSizeFormat == 0) {
        err = getBits(bitstream, 4, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 4, &code);
        if (err) return(err);
        drcCoefficientsParametricDrc->parametricDrcFrameSize = 1 << code;
    }
    else {
        err = getBits(bitstream, 1, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 15, &code);
        if (err) return(err);
        drcCoefficientsParametricDrc->parametricDrcFrameSize = 1 + code;
    }
    
    err = getBits(bitstream, 1, &reserved);
    if (err) return(err);
    err = getBits(bitstream, 1, &drcCoefficientsParametricDrc->parametricDrcDelayMaxPresent);
    if (err) return(err);
    
    if(drcCoefficientsParametricDrc->parametricDrcDelayMaxPresent == 1) {
        int mu, nu;
        err = getBits(bitstream, 5, &mu);
        if (err) return(err);
        err = getBits(bitstream, 3, &nu);
        if (err) return(err);
        drcCoefficientsParametricDrc->parametricDrcDelayMax = 16 * mu * (1<<nu);
        if (err) return(err);
    }
    
    err = getBits(bitstream, 6, &drcCoefficientsParametricDrc->parametricDrcGainSetCount);
    if (err) return(err);
    
    for(i = 0; i < drcCoefficientsParametricDrc->parametricDrcGainSetCount; i++) {
        err = getBits(bitstream, 4, &drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].parametricDrcId);
        if (err) return(err);
        err = getBits(bitstream, 1, &drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].drcInputLoudnessPresent);
        if (err) return(err);
        err = getBits(bitstream, 3, &drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].sideChainConfigType);
        if (err) return(err);
        
        if(drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].drcInputLoudnessPresent) {
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].drcInputLoudness = -57.75f + code * 0.25f;
        }
        
        if(drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].sideChainConfigType == 1) {
            err = getBits(bitstream, 1, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 7, &drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].downmixId);
            if (err) return(err);
            err = getBits(bitstream, 1, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 7, &drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].channelCountFromDownmixId);
            if (err) return(err);
            
            for(j = 0; j < drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].channelCountFromDownmixId; j++) {
                err = getBits(bitstream, 4, &code);
                if (err) return(err);
                drcCoefficientsParametricDrc->parametricDrcGainSetParams[i].levelEstimChannelWeight[j] = (float)pow(10.0f, 0.05f * channelWeight[code]);
            }
        }
    }
    
    return(0);
}

int
parseIsobmffPdi1( robitbufHandle bitstream,
                  ParametricDrcInstructions *parametricDrcInstructions,
                  int *parametricDrcInstructionsCount,
                  int drcFrameSizeParametricDrc)
{
    int err = 0, bsSamplePeakLevel = 0, bsTruePeakLevel = 0, i = 0, j = 0;
    int code = 0, reserved = 0, loudnessBaseCount = 0, startCount = 0; /*TODO check for unused variables */

    err = getBits(bitstream, 4, &reserved);
    if (err) return(err);
    err = getBits(bitstream, 4, parametricDrcInstructionsCount);
    if (err) return(err);
    
    for(i = 0; i < *parametricDrcInstructionsCount; i++) {
        err = getBits(bitstream, 2, &reserved);
        if (err) return(err);
        err = getBits(bitstream, 4, &parametricDrcInstructions[i].parametricDrcId);
        if (err) return(err);
        err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcLookAheadPresent);
        if (err) return(err);
        err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcPresetIdPresent);
        if (err) return(err);
        
        if(parametricDrcInstructions[i].parametricDrcLookAheadPresent == 1) {
            err = getBits(bitstream, 1, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 7, &parametricDrcInstructions[i].parametricDrcLookAhead);
            if (err) return(err);
        }
        else {
            parametricDrcInstructions[i].parametricDrcLookAhead = 0;
        }
        
        if(parametricDrcInstructions[i].parametricDrcPresetIdPresent == 1) {
            err = getBits(bitstream, 1, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 7, &parametricDrcInstructions[i].parametricDrcPresetId);
            if (err) return(err);
            
            switch (parametricDrcInstructions[i].parametricDrcPresetId) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                    parametricDrcInstructions[i].drcCharacteristic = parametricDrcInstructions[i].parametricDrcPresetId + 7;
                    parametricDrcInstructions[i].parametricDrcType = PARAM_DRC_TYPE_FF;
                    err = parametricDrcTypeFeedForwardInitializeParameters(parametricDrcInstructions[i].drcCharacteristic, drcFrameSizeParametricDrc, &(parametricDrcInstructions[i].parametricDrcTypeFeedForward));
                    if (err) return (err);
                    break;
                default:
                    parametricDrcInstructions[i].disableParamtricDrc = 1;
                    break;
            }
        }
        else {
            err = getBits(bitstream, 5, &reserved);
            if (err) return(err);
            err = getBits(bitstream, 3, &parametricDrcInstructions[i].parametricDrcType);
            if (err) return(err);
            
            if(parametricDrcInstructions[i].parametricDrcType == PARAM_DRC_TYPE_FF) {
                err = getBits(bitstream, 3, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 2, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.levelEstimKWeightingType);
                if (err) return(err);
                err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.levelEstimIntegrationTimePresent);
                if (err) return(err);
                err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcCurveDefinitionType);
                if (err) return(err);
                err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcGainSmoothParametersPresent);
                if (err) return(err);
                if(parametricDrcInstructions[i].parametricDrcTypeFeedForward.levelEstimIntegrationTimePresent == 1) {
                    err = getBits(bitstream, 2, &reserved);
                    if (err) return(err);
                    err = getBits(bitstream, 6, &code);
                    if (err) return(err);
                    parametricDrcInstructions[i].parametricDrcTypeFeedForward.levelEstimIntegrationTime = (code+1)*drcFrameSizeParametricDrc;
                }
                else {
                    parametricDrcInstructions[i].parametricDrcTypeFeedForward.levelEstimIntegrationTime = drcFrameSizeParametricDrc;
                }
                if(parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcCurveDefinitionType == 0) {
                    err = getBits(bitstream, 1, &reserved);
                    if (err) return(err);
                    err = getBits(bitstream, 7, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcCharacteristic);
                    if (err) return(err);
                    err = parametricDrcTypeFeedForwardInitializeDrcCurveParameters(parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcCharacteristic, &parametricDrcInstructions[i].parametricDrcTypeFeedForward);
                    if (err) return(err);
                }
                else {
                    parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcCharacteristic = 0;
                    
                    err = getBits(bitstream, 5, &reserved);
                    if (err) return(err);
                    err = getBits(bitstream, 3, &code);
                    if (err) return(err);
                    parametricDrcInstructions[i].parametricDrcTypeFeedForward.nodeCount = code + 2;
                    
                    for (j=0; j<parametricDrcInstructions[i].parametricDrcTypeFeedForward.nodeCount; j++) {
                        if (j==0) {
                            err = getBits(bitstream, 2, &reserved);
                            if (err) return(err);
                            err = getBits(bitstream, 6, &code);
                            if (err) return(err);
                            parametricDrcInstructions[i].parametricDrcTypeFeedForward.nodeLevel[0] = -11-code;
                        } else {
                            err = getBits(bitstream, 3, &reserved);
                            if (err) return(err);
                            err = getBits(bitstream, 5, &code);
                            if (err) return(err);
                            parametricDrcInstructions[i].parametricDrcTypeFeedForward.nodeLevel[j] = parametricDrcInstructions[i].parametricDrcTypeFeedForward.nodeLevel[j-1]+1+code;
                        }
                        err = getBits(bitstream, 2, &reserved);
                        if (err) return(err);
                        err = getBits(bitstream, 6, &code);
                        if (err) return(err);
                        parametricDrcInstructions[i].parametricDrcTypeFeedForward.nodeGain[j] = code-39;
                    }
                }
                
                err = parametricDrcTypeFeedForwardInitializeDrcGainSmoothParameters(parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcCharacteristic, &parametricDrcInstructions[i].parametricDrcTypeFeedForward);
                if (err) return(err);
                
                if(parametricDrcInstructions[i].parametricDrcTypeFeedForward.drcGainSmoothParametersPresent != 0) {
                    err = getBits(bitstream, 6, &reserved);
                    if (err) return(err);
                    err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothTimeFastPresent);
                    if (err) return(err);
                    err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothHoldOffCountPresent);
                    if (err) return(err);
                    err = getBits(bitstream, 8, &code);
                    if (err) return(err);
                    parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothAttackTimeSlow = code*5;
                    err = getBits(bitstream, 8, &code);
                    if (err) return(err);
                    parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothReleaseTimeSlow = code*40;
                    
                    if(parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothTimeFastPresent == 1) {
                        err = getBits(bitstream, 8, &code);
                        if (err) return(err);
                        parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothAttackTimeFast = code*5;
                        err = getBits(bitstream, 8, &code);
                        if (err) return(err);
                        parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothReleaseTimeFast = code*20;
                        err = getBits(bitstream, 7, &reserved);
                        if (err) return(err);
                        err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothThresholdPresent);
                        if (err) return(err);
                        
                        if(parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothThresholdPresent == 1) {
                            err = getBits(bitstream, 3, &reserved);
                            if (err) return(err);
                            err = getBits(bitstream, 5, &(parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothAttackThreshold));
                            if (err) return(err);
                            if ( parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothAttackThreshold == 31) {
                                parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothAttackThreshold = 1000;
                            }
                            
                            err = getBits(bitstream, 3, &reserved);
                            if (err) return(err);
                            err = getBits(bitstream, 5, &(parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothReleaseThreshold));
                            if (err) return(err);
                            if (parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothReleaseThreshold == 31) {
                                parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothReleaseThreshold = 1000;
                            }
                        }
                    }
                    else {
                        parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothAttackTimeFast = parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothAttackTimeSlow;
                        parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothReleaseTimeFast = parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothReleaseTimeSlow;
                    }
                    
                    if(parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothHoldOffCountPresent == 1) {
                        err = getBits(bitstream, 1, &reserved);
                        if (err) return(err);
                        err = getBits(bitstream, 7, &parametricDrcInstructions[i].parametricDrcTypeFeedForward.gainSmoothHoldOff);
                        if (err) return(err);
                    }
                }
            }
            else if(parametricDrcInstructions[i].parametricDrcType == PARAM_DRC_TYPE_LIM) {
                err = getBits(bitstream, 6, &reserved);
                if (err) return(err);
                err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimThresholdPresent);
                if (err) return(err);
                err = getBits(bitstream, 1, &parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimReleasePresent);
                if (err) return(err);
                
                if(parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimThresholdPresent) {
                    err = getBits(bitstream, 8, &code);
                    if (err) return(err);
                    parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimThreshold = - code * 0.125f;
                }
                else {
                    parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimThreshold = PARAM_DRC_TYPE_LIM_THRESHOLD_DEFAULT;
                }
                
                parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimAttack = parametricDrcInstructions[i].parametricDrcLookAhead;
                
                if(parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimReleasePresent) {
                    err = getBits(bitstream, 8, &code);
                    if (err) return(err);
                    parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimRelease = code*10;
                }
                else {
                    parametricDrcInstructions[i].parametricDrcTypeLim.parametricLimRelease = PARAM_DRC_TYPE_LIM_RELEASE_DEFAULT;
                }
            }
            else {
                parametricDrcInstructions[i].disableParamtricDrc;
            }
        }
    }
    
    return(0);
}

/* Parser for ISO base media file format 'udex' box */
int
parseIsobmffUdex( robitbufHandle bitstream,
                  int *sizeLeft,
                  UniDrcConfig* uniDrcConfig)
    {
        int err = 0, version = 0, flags = 0, size = 0, code = 0, i, sizeLeftInternal;
        int loudnessInfoAlbumCount = 0, loudnessInfoCount = 0;
        char type[5];
        char pdc1[] = "pdc1";
        char pdi1[] = "pdi1";
        
        /* switch */
        while ( *sizeLeft ) {
            /* class LoudnessBaseBox extends FullBox(loudnessType) */
            /* box size */
            err = getBits(bitstream, 32, &size);
            if (err) return(err);
            
            /* box type */
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[0] = (char)code;
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[1] = (char)code;
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[2] = (char)code;
            err = getBits(bitstream, 8, &code);
            if (err) return(err);
            type[3] = (char)code;
            type[4] = '\0';
            
            /* extended size */
            if (size == 1) {
                /* currently unsupported */
                return -1;
            } else if (size == 0) {
                size = robitbuf_GetBitsRead(bitstream);
            }
            
            /* version */
            err = getBits(bitstream, 8, &version);
            if (err) return(err);
            
            /* flags */
            err = getBits(bitstream, 24, &flags);
            if (err) return(err);
            
            sizeLeftInternal = size - 12; /* 8 bytes for the box size and box type, 1 byte for version, and 3 bytes for flags */
            
            /* switch 'pdc1' and 'pdi1' */
            if (strcmp(type, pdc1) == 0) {
                
                /* DRCCoefficientsParametricDrc */
                err = parseIsobmffPdc1(bitstream, &uniDrcConfig->uniDrcConfigExt.drcCoefficientsParametricDrc);
                if (err) return(err);
                
            } else if (strcmp(type, pdi1) == 0) {
                
                /* ParametricDrcInstructions */
                err = parseIsobmffPdi1(bitstream,
                                       uniDrcConfig->uniDrcConfigExt.parametricDrcInstructions,
                                       &uniDrcConfig->uniDrcConfigExt.parametricDrcInstructionsCount,
                                       uniDrcConfig->uniDrcConfigExt.drcCoefficientsParametricDrc.parametricDrcFrameSize);
                if (err) return(err);
                
            } else {
                
                fprintf(stderr, "WARNING: Unknown ISOBMFF box discarded.\n");
                
                /* discard bytes of unknown box */
                for (i=0; i<sizeLeftInternal; i++) {
                    err = getBits(bitstream, 8, &code);
                    if (err) return(err);
                }
            }
            
            /* left size */
            *sizeLeft = *sizeLeft - size;
        }
        
        uniDrcConfig->uniDrcConfigExtPresent = 1;
        uniDrcConfig->uniDrcConfigExt.parametricDrcPresent = 1;
        
        return (0);
}


/* Parser for ISO base media file format boxes */
int
parseIsobmff( robitbufHandle bitstream,
              UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
              UniDrcConfig* uniDrcConfig,
              LoudnessInfoSet* loudnessInfoSet,
              int baseChannelCount)
{
    int err = 0, i = 0, size = 0, sizeLeft = 0, code = 0;
    
    char type[5];
    char ludt[] = "ludt";
    char chnl[] = "chnl";
    char dmix[] = "dmix";
    char udc1[] = "udc1";
    char udc2[] = "udc2";
    char udi1[] = "udi1";
    char udi2[] = "udi2";
    char udex[] = "udex";

    uniDrcConfig->channelLayout.baseChannelCount = baseChannelCount;
    uniDrcConfig->sampleRate = pUniDrcBsDecStruct->drcParams.sampleRateDefault;
    
    sizeLeft = robitbuf_GetBitsAvail(bitstream);

    while (sizeLeft) {
        /* class Box */
        /* box size */
        err = getBits(bitstream, 32, &size);
        if (err) return(err);
        
        /* box type */
        err = getBits(bitstream, 8, &code);
        if (err) return(err);
        type[0] = (char)code;
        err = getBits(bitstream, 8, &code);
        if (err) return(err);
        type[1] = (char)code;
        err = getBits(bitstream, 8, &code);
        if (err) return(err);
        type[2] = (char)code;
        err = getBits(bitstream, 8, &code);
        if (err) return(err);
        type[3] = (char)code;
        type[4] = '\0';
        
        /* extended size */
        if (size == 1) {
            /* currently unsupported */
            return -1;
        } else if (size == 0) {
            size = robitbuf_GetBitsRead(bitstream);
        }
        
        /* left size */
        sizeLeft = size - 8;
        
        if (strcmp(type, ludt) == 0) {
            
            /* ISO base media file format precedence */
            memset(loudnessInfoSet, 0, sizeof(LoudnessInfoSet));
            
            /* parse 'ludt' box content */
            parseIsobmffLudt(bitstream, &sizeLeft, loudnessInfoSet);
            
        } else if (strcmp(type, chnl) == 0) {
            
            /* parse 'chnl' box content */
            parseIsobmffChnl(bitstream, pUniDrcBsDecStruct, uniDrcConfig);
            
        } else if (strcmp(type, dmix) == 0) {
            
            /* parse 'dmix' box content */
            parseIsobmffDmix(bitstream, pUniDrcBsDecStruct, uniDrcConfig);
            
        } else if (strcmp(type, udc2) == 0) {
            
            /* parse 'udc2' box content */
            parseIsobmffUdc2(bitstream, pUniDrcBsDecStruct, uniDrcConfig);
            
        } else if (strcmp(type, udi2) == 0) {

            /* parse 'udi2' box content */
            parseIsobmffUdi2(bitstream, pUniDrcBsDecStruct, uniDrcConfig);
            
        } else if (strcmp(type, udex) == 0) {
            
            /* parse 'udex' box content */
            parseIsobmffUdex(bitstream,  &sizeLeft, uniDrcConfig);
            
        } else if ( (strcmp(type, udc1) == 0) || (strcmp(type, udi1) == 0) ) {
        
            fprintf(stderr, "WARNING: Following ISOBMFFF boxes are currently unsupported: 'chnl','udc1','udi1','udex'.\n");

            /* discard bytes of unknown box */
            for (i=0; i<sizeLeft; i++) {
                err = getBits(bitstream, 8, &code);
                if (err) return(err);
            }
            
        } else {
            
            fprintf(stderr, "WARNING: Unknown ISOBMFF box discarded.\n");

            /* discard bytes of unknown box */
            for (i=0; i<sizeLeft; i++) {
                err = getBits(bitstream, 8, &code);
                if (err) return(err);
            }
            
        }
        
        sizeLeft = robitbuf_GetBitsAvail(bitstream);
    }
    
    /* deriving data */
    for(i=0; i<uniDrcConfig->drcInstructionsUniDrcCount; i++)
    {
        err = generateDrcInstructionsDerivedData(uniDrcConfig, &(pUniDrcBsDecStruct->drcParams), &(uniDrcConfig->drcInstructionsUniDrc[i]));
        if (err) return(err);
    }
    
    err = generateDrcInstructionsForDrcOff(uniDrcConfig, &(pUniDrcBsDecStruct->drcParams));
    if (err) return(err);
    
    return (0);
}
#endif
#endif
#endif
/* ====================================================================================
                           Parsing of DRC gain sequences
   ====================================================================================*/

int
decInitialGain(robitbufHandle bitstream,
               const int gainCodingProfile,
               float* gainInitial)
{
    int err = 0, sign, magn;
    switch (gainCodingProfile)
    {
        case GAIN_CODING_PROFILE_REGULAR:
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            err = getBits(bitstream, 8, &magn);
            if (err) return(err);
            *gainInitial = magn * 0.125f;
            if (sign) *gainInitial = - *gainInitial;
            break;
        case GAIN_CODING_PROFILE_FADING:
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            if (sign==0) *gainInitial = 0.0f;
            else
            {
                err = getBits(bitstream, 10, &magn);
                if (err) return(err);
                *gainInitial = - (magn + 1) * 0.125f;
            }
            break;
        case GAIN_CODING_PROFILE_CLIPPING:
            err = getBits(bitstream, 1, &sign);
            if (err) return(err);
            if (sign==0) *gainInitial = 0.0f;
            else
            {
                err = getBits(bitstream, 8, &magn);
                if (err) return(err);
                *gainInitial = - (magn + 1) * 0.125f;
            }
            break;
        case GAIN_CODING_PROFILE_CONSTANT:
            break;
        default:
            return(UNEXPECTED_ERROR);
    }
    return (0);
}

int
decGains(robitbufHandle bitstream,
         Tables* tables,
         const int nNodes,
         const int deltaTmin,
         const int gainCodingProfile,
         Node* node)
{
    int err = 0, k, e, m;
    int bit;
    int nBitsRead;
    int code;
    int codeFound;
    float drcGainDelta = 0;
    const DeltaGainCodeEntry *deltaGainCodeTable;
    int nDeltaGainValuesInTable;
    
    err = decInitialGain(bitstream, gainCodingProfile, &(node[0].gainDb));
    if (err) return(err);
    
    getDeltaGainCodeTable(gainCodingProfile, &deltaGainCodeTable, &nDeltaGainValuesInTable);
    for (k=1; k<nNodes; k++)
    {
        /* decode a delta gain value */
        nBitsRead = 0;
        code = 0;
        codeFound = FALSE;
        e = 0;
        while ((e < nDeltaGainValuesInTable) && (!codeFound))
        {
            for (m=0; m<deltaGainCodeTable[e].size - nBitsRead; m++)
            {
                err = getBits(bitstream, 1, &bit);
                if (err) return(err);
                code = (code << 1) + bit;
                nBitsRead++;
            }
            while (nBitsRead == deltaGainCodeTable[e].size)
            {
                if (code == deltaGainCodeTable[e].code)
                {
                    drcGainDelta = deltaGainCodeTable[e].value;
                    codeFound = TRUE;
                    break;
                }
                e++;
            }
        }
        if (codeFound == FALSE)
        {
            fprintf(stderr, "ERROR: Huffman code for gain delta not found\n");
            return (UNEXPECTED_ERROR);
        }
        node[k].gainDb = node[k-1].gainDb + drcGainDelta;
    }
    return (0);
}

int
decSlopes(robitbufHandle bitstream,
          int* nNodes,
          const int gainInterpolationType,
          Node* node)
{
    int err = 0, k, e, m, bit;
    int code;
    int codeFound;
    float slopeValue = 0;
    bool end_marker = FALSE;
    int nBitsRead;
    const SlopeCodeTableEntry* slopeCodeTable;
    int nSlopeCodeTableEntries;
    getSlopeCodeTableAndSize(&slopeCodeTable, &nSlopeCodeTableEntries);
    
    /* decode end markers to get nNodes */
    k=0;
    while(end_marker != 1)
    {
        k++;
        err = getBits(bitstream, 1, &end_marker);
        if (err) return(err);
    }
    *nNodes = k;
    
    if (gainInterpolationType == GAIN_INTERPOLATION_TYPE_SPLINE) {
        /* decode slope values */
        for (k=0; k<*nNodes; k++)
        {
            nBitsRead = 0;
            code = 0;
            codeFound = FALSE;
            e = 0;
            while ((e < nSlopeCodeTableEntries) && (!codeFound))
            {
                for (m=0; m<slopeCodeTable[e].size - nBitsRead; m++)
                {
                    err = getBits(bitstream, 1, &bit);
                    if (err) return(err);
                    code = (code << 1) + bit;
                    nBitsRead++;
                }
                while (nBitsRead == slopeCodeTable[e].size)
                {
                    if (code == slopeCodeTable[e].code)
                    {
                        slopeValue = slopeCodeTable[e].value;
                        codeFound = TRUE;
                        break;
                    }
                    e++;
                }
            }
            node[k].slope = slopeValue;
        }
    }
    else {
        for (k=0; k<*nNodes; k++)
        {
            node[k].slope = 0.0f; /* slope will be neglected */
        }
    }
    return(0);
}

int
decTimes(robitbufHandle bitstream,
         Tables* tables,
         const int nNodes,
         const int deltaTmin,
         const int drcFrameSize,
         const int fullFrame,
         const int timeOffset,
         Node* node)
{
    int err = 0, k, e, m;
    int bit;
    int nBitsRead;
    int code;
    int codeFound = FALSE;
    int timeDelta = 0;
    int timeOffs = timeOffset;
    int nTimeValuesInTable = N_DELTA_TIME_CODE_TABLE_ENTRIES_MAX;
    DeltaTimeCodeTableEntry* deltaTimeCodeTable = tables->deltaTimeCodeTable;
    bool frameEndFlag;
    int  nodeTimeTmp;
    bool nodeResFlag;
    int exitCount;
    if (fullFrame == 0)
    {
        err = getBits(bitstream, 1, &frameEndFlag);
        if (err) return(err);
    }
    else
    {
        frameEndFlag = 1;
    }
    
    if (frameEndFlag == 1)
    {
        nodeResFlag = 0;
        for (k=0; k<nNodes-1; k++)
        {
            /* decode a delta time value */
            nBitsRead = 0;
            code = 0;
            codeFound = FALSE;
            exitCount = 0;
            /* frameEndFlag == 1 signals that the last node is at the end of the DRC frame */
            e = 1;
            while ((e < nTimeValuesInTable) && (!codeFound))
            {
                exitCount++;
                if (exitCount > 100000) {
                    printf("ERROR: invaild drcGain payload\n");
                    return(BITSTREAM_ERROR);
                }
                for (m=0; m<deltaTimeCodeTable[e].size - nBitsRead; m++)
                {
                    err = getBits(bitstream, 1, &bit);
                    if (err) return(err);
                    code = (code << 1) + bit;
                    nBitsRead++;
                }
                while (nBitsRead == deltaTimeCodeTable[e].size)
                {
                    if (code == deltaTimeCodeTable[e].code)
                    {
                        timeDelta = deltaTimeCodeTable[e].value;
                        codeFound = TRUE;
                        break;
                    }
                    e++;
                }
            }
            nodeTimeTmp = timeOffs + timeDelta * deltaTmin;
            if (nodeTimeTmp > drcFrameSize + timeOffset)
            {
                if (nodeResFlag == 0)
                {
                    node[k].time  = drcFrameSize + timeOffset;
                    nodeResFlag = 1;
                }
                node[k+1].time = nodeTimeTmp;
            }
            else
            {
                node[k].time = nodeTimeTmp;
            }
            timeOffs = nodeTimeTmp;
        }
        if (nodeResFlag == 0)
        {
            node[k].time  = drcFrameSize + timeOffset;
        }
    }
    else
    {
        for (k=0; k<nNodes; k++)
        {
            /* decode a delta time value */
            nBitsRead = 0;
            code = 0;
            codeFound = FALSE;
            e = 1;
            exitCount = 0;
            while ((e < nTimeValuesInTable) && (!codeFound))
            {
                exitCount++;
                if (exitCount > 100000) {
                    printf("ERROR: invaild drcGain payload\n");
                    return(BITSTREAM_ERROR);
                }
                for (m=0; m<deltaTimeCodeTable[e].size - nBitsRead; m++)
                {
                    err = getBits(bitstream, 1, &bit);
                    if (err) return(err);
                    code = (code << 1) + bit;
                    nBitsRead++;
                }
                while (nBitsRead == deltaTimeCodeTable[e].size)
                {
                    if (code == deltaTimeCodeTable[e].code)
                    {
                        timeDelta = deltaTimeCodeTable[e].value;
                        codeFound = TRUE;
                        break;
                    }
                    e++;
                }
            }
            node[k].time = timeOffs + timeDelta * deltaTmin;
            timeOffs = node[k].time;
        }
    }
    return (0);
}

int
parseSplineNodes(robitbufHandle bitstream,
                 UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                 GainSetParams* gainSetParams,
                 SplineNodes* splineNodes)
{
    int err = 0;
    int timeOffset;
    if (gainSetParams->timeAlignment==0) {
        timeOffset = -1;
    }
    else
    {
        if (gainSetParams->timeDeltaMinPresent)
        {
            timeOffset = - gainSetParams->timeDeltaMin + (gainSetParams->timeDeltaMin-1)/2; /* timeOffset = - deltaTmin + floor((deltaTmin-1)/2); */
        }
        else
        {
          timeOffset = - pUniDrcBsDecStruct->drcParams.deltaTminDefault + (pUniDrcBsDecStruct->drcParams.deltaTminDefault-1)/2; /* timeOffset = - deltaTmin + floor((deltaTmin-1)/2); */

        }
    }
    
    /* error handling */
    if(bitstream->charBuffer == NULL)
    {
        float gainDbPrev = splineNodes->node[splineNodes->nNodes-1].gainDb;
        splineNodes->drcGainCodingMode = 0;

        /* One gain value per frame, slope = 0 */
        splineNodes->nNodes = 1;
        
        if (gainDbPrev < 0)
        {
            splineNodes->node[0].gainDb = gainDbPrev;
        }
        else
        {
            splineNodes->node[0].gainDb = 0.f;
        }

        splineNodes->node[0].slope = 0.0;
        splineNodes->node[0].time = (pUniDrcBsDecStruct->drcParams).drcFrameSize  + timeOffset;
    }
    else
    {
        err = getBits(bitstream, 1, &(splineNodes->drcGainCodingMode));
        if (err == PROC_COMPLETE)
        {
            splineNodes->drcGainCodingMode = 0;
            splineNodes->node[0].slope = 0.0;
            splineNodes->node[0].time = (pUniDrcBsDecStruct->drcParams).drcFrameSize  + timeOffset;
            splineNodes->node[0].gainDb = splineNodes->node[splineNodes->nNodes-1].gainDb;  /* repeat last gain until end of audio input*/
            splineNodes->nNodes = 1;
        }
        else
        {
            if (err) return(err);
        }
        if (splineNodes->drcGainCodingMode == 0)
        {
            /* One gain value per frame, slope = 0 */
            splineNodes->nNodes = 1;
            
            err = decInitialGain(bitstream, gainSetParams->gainCodingProfile, &(splineNodes->node[0].gainDb));
            if (err) return(err);
            
            splineNodes->node[0].slope = 0.0;
            splineNodes->node[0].time = (pUniDrcBsDecStruct->drcParams).drcFrameSize  + timeOffset;
        }
        else
        {
            err = decSlopes(bitstream, &splineNodes->nNodes, gainSetParams->gainInterpolationType, splineNodes->node);
            if (err) return(err);
            if (gainSetParams->timeDeltaMinPresent)
            {
                err = decTimes(bitstream, &gainSetParams->tables, splineNodes->nNodes, gainSetParams->timeDeltaMin, (pUniDrcBsDecStruct->drcParams).drcFrameSize, gainSetParams->fullFrame, timeOffset, splineNodes->node);
                if (err) return(err);
                err = decGains(bitstream, &gainSetParams->tables, splineNodes->nNodes, gainSetParams->timeDeltaMin, gainSetParams->gainCodingProfile, splineNodes->node);
                if (err) return(err);
            }
            else
            {
                err = decTimes(bitstream, &pUniDrcBsDecStruct->tablesDefault, splineNodes->nNodes, (pUniDrcBsDecStruct->drcParams).deltaTminDefault, (pUniDrcBsDecStruct->drcParams).drcFrameSize, gainSetParams->fullFrame, timeOffset, splineNodes->node);
                if (err) return(err);
                err = decGains(bitstream, &pUniDrcBsDecStruct->tablesDefault, splineNodes->nNodes, (pUniDrcBsDecStruct->drcParams).deltaTminDefault, gainSetParams->gainCodingProfile, splineNodes->node);
                if (err) return(err);
            }
            
        }
    }
    return(0);
}

int
parseDrcGainSequence(robitbufHandle bitstream,
                     UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                     GainSetParams* gainSetParams,
                     DrcGainSequence* drcGainSequence)
{
    int err = 0, i;
    int timeBufPrevFrame[NODE_COUNT_MAX], timeBufCurFrame[NODE_COUNT_MAX];
    int nNodesNodeRes, nNodesCur, k, m;

    if (((pUniDrcBsDecStruct->drcParams).delayMode == DELAY_MODE_LOW_DELAY) && (gainSetParams->fullFrame == 0))
    {
        return (PARAM_ERROR);  /* Low-delay mode needs fullFrame 1 */
    }
#if MPEG_D_DRC_EXTENSION_V1
    i=0;
#else
    for(i=0; i<gainSetParams->bandCount; i++)
#endif /* MPEG_D_DRC_EXTENSION_V1 */
    {
        err = parseSplineNodes(bitstream, pUniDrcBsDecStruct, gainSetParams, &(drcGainSequence->splineNodes[i]));
        if (err) return(err);
        
        /* count number of nodes in node reservoir */
        nNodesNodeRes = 0;
        nNodesCur     = 0;
        /* count and buffer nodes from node reservoir */
        for (k = 0; k < drcGainSequence->splineNodes[i].nNodes; k++){
            if(drcGainSequence->splineNodes[i].node[k].time >= pUniDrcBsDecStruct->drcParams.drcFrameSize){
                /* write node reservoir times into buffer */
                timeBufPrevFrame[nNodesNodeRes] = drcGainSequence->splineNodes[i].node[k].time;
                nNodesNodeRes++;
            }
            else{ /* times from current frame */
                timeBufCurFrame[nNodesCur] = drcGainSequence->splineNodes[i].node[k].time;
                nNodesCur++;
            }
        }
        /* compose right time order (bit reservoir first) */
        for (k = 0; k < nNodesNodeRes; k++){
            /* subtract two time drcFrameSize: one to remove node reservoir offset and one to get the negative index relative to the current frame */
            drcGainSequence->splineNodes[i].node[k].time = timeBufPrevFrame[k] - 2*pUniDrcBsDecStruct->drcParams.drcFrameSize;
        }
        /* ...and times from current frame */
        for (m = 0; m < nNodesCur; m++, k++){
            drcGainSequence->splineNodes[i].node[k].time = timeBufCurFrame[m];
        }
    }
    return(0);
}


int
parseUniDrcGainExtension(robitbufHandle bitstream,
                         UniDrcGainExt* uniDrcGainExt)
{
    int err = 0, i, k;
    int bitSizeLen, extSizeBits, bitSize, otherBit;
    
    k = 0;
    err = getBits(bitstream, 4, &(uniDrcGainExt->uniDrcGainExtType[k]));
    if (err) return(err);
    while(uniDrcGainExt->uniDrcGainExtType[k] != UNIDRCGAINEXT_TERM)
    {
        err = getBits(bitstream, 3, &bitSizeLen );
        if (err) return(err);
        extSizeBits = bitSizeLen + 4;

        err = getBits(bitstream, extSizeBits, &bitSize );
        if (err) return(err);
        uniDrcGainExt->extBitSize[k] = bitSize + 1;

        switch(uniDrcGainExt->uniDrcGainExtType[k])
        {
            /* add future extensions here */
            default:
                for(i = 0; i<uniDrcGainExt->extBitSize[k]; i++)
                {
                    err = getBits(bitstream, 1, &otherBit );
                    if (err) return(err);
                }
                break;
        }
        k++;
        err = getBits(bitstream, 4, &(uniDrcGainExt->uniDrcGainExtType[k]));
        if (err) return(err);
    }

    return (0);
}

/* Parser for gain sequences */
int
parseUniDrcGain(robitbufHandle bitstream,
                UNI_DRC_BS_DEC_STRUCT *pUniDrcBsDecStruct,
                UniDrcConfig* uniDrcConfig,
                UniDrcGain* uniDrcGain)
{
    int err = 0, seq;
    static int packetlossFrameCount = 0;
    SplineNodes* splineNodes = {0};
#if MPEG_D_DRC_EXTENSION_V1
    {
        int gainSequenceCount = uniDrcConfig->drcCoefficientsUniDrc[0].gainSequenceCount;

        for (seq=0; seq<gainSequenceCount; seq++)
        {
            int index = uniDrcConfig->drcCoefficientsUniDrc[0].gainSetParamsIndexForGainSequence[seq];
#if AMD2_COR3
            GainSetParams* gainSetParams;
            if ( (index < 0) || (index >= uniDrcConfig->drcCoefficientsUniDrc[0].gainSetCount) ) {
                return UNEXPECTED_ERROR;
            }
            gainSetParams = &(uniDrcConfig->drcCoefficientsUniDrc[0].gainSetParams[index]);
#else
            GainSetParams *gainSetParams = &(uniDrcConfig->drcCoefficientsUniDrc->gainSetParams[index]);
#endif
            if (gainSetParams->gainCodingProfile == GAIN_CODING_PROFILE_CONSTANT)
            {
                splineNodes = &(uniDrcGain->drcGainSequence[seq].splineNodes[0]);
                splineNodes->nNodes = 1;
                splineNodes->node[0].slope = 0.0;
                splineNodes->node[0].time = (pUniDrcBsDecStruct->drcParams).drcFrameSize - 1;
                splineNodes->node[0].gainDb = 0.0f;
            }
            else
            {
                err = parseDrcGainSequence(bitstream, pUniDrcBsDecStruct, gainSetParams, &(uniDrcGain->drcGainSequence[seq]));
                if (err) return(err);
            }
        }
    }
#else
    {
        int gainSetCount = uniDrcConfig->drcCoefficientsUniDrc[0].gainSetCount;
        int bandCount;
        int i;
        for (i=0; i<gainSetCount; i++)
        {
            GainSetParams *gainSetParams = &(uniDrcConfig->drcCoefficientsUniDrc->gainSetParams[i]);
            if (gainSetParams->gainCodingProfile == GAIN_CODING_PROFILE_CONSTANT)
            {
                splineNodes = &(uniDrcGain->drcGainSequence[i].splineNodes[0]);
                uniDrcGain->drcGainSequence[i].bandCount = 1;
                splineNodes->nNodes = 1;
                splineNodes->node[0].slope = 0.0;
                splineNodes->node[0].time = (pUniDrcBsDecStruct->drcParams).drcFrameSize - 1;
                splineNodes->node[0].gainDb = 0.0f;
            }
            else
            {
                bandCount = uniDrcConfig->drcCoefficientsUniDrc[0].gainSetParams[i].bandCount;
                uniDrcGain->drcGainSequence[i].bandCount = bandCount;
                err = parseDrcGainSequence(bitstream, pUniDrcBsDecStruct, gainSetParams, &(uniDrcGain->drcGainSequence[i]));
                if (err) return(err);
            }
        }
    }
#endif
    /* error handling */
    if (bitstream->charBuffer == NULL)
    {
        packetlossFrameCount++;
        
        if (packetlossFrameCount*(float)pUniDrcBsDecStruct->drcParams.drcFrameSize/uniDrcConfig->sampleRate > MAXPACKETLOSSTIME)
        {
            uniDrcConfig->applyDrc = 0;
        }
    }
    else
    {
        err = getBits(bitstream, 1, &(uniDrcGain->uniDrcGainExtPresent));
        if (err) return(err);
        if (uniDrcGain->uniDrcGainExtPresent == 1)
        {
            err = parseUniDrcGainExtension(bitstream, &(uniDrcGain->uniDrcGainExt));
            if (err) return(err);
        }
        packetlossFrameCount = 0;
        uniDrcConfig->applyDrc = 1;
    }
    
    return(0);
}

