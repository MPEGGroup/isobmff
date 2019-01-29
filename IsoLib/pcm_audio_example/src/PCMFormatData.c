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
derivative works. Copyright (c) 2019.
*/

#include <string.h>
#include "PCMFormatData.h"
#include "Logger.h"

void initPCMFormatData (PCMFormatData* pcmFormatData) {
    memset(pcmFormatData, 0, sizeof(PCMFormatData));
}

int setPCMFormatDataFromWAV (WAVData* wavData, PCMFormatData* pcmFormatData) {
    int wavErr;
    wavErr = 0;
    logMsg(LOGLEVEL_DEBUG, "Setting PCM format data from WAV header");

    initPCMFormatData (pcmFormatData);
    pcmFormatData->channelCount = wavData->channels;
    pcmFormatData->sampleRate = wavData->sampleRate;
    pcmFormatData->pcmSampleSize = wavData->byteDepth * 8;
    switch (wavData->formatTag) {
        case WAVE_FORMAT_TAG_INTEGER:
            pcmFormatData->isInteger = 1;
            break;
        case WAVE_FORMAT_TAG_IEEE_FLOAT:
            pcmFormatData->isInteger = 0;
            break;
        default:
            wavErr = -1;
            return wavErr;
    }
    pcmFormatData->isLittleEndian = 1;  // according to WAV spec
    return wavErr;
}
