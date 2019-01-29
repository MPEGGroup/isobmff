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

#include "WAVData.h"
#include "Logger.h"

#include <stdio.h>
#include <string.h>

#define FILLLASTFRAME   0
#define MODULEDELAY     0

int initWAVDataForReading    (WAVData *wavData, char *inputFileStr)
{
    int    wavErr;

    wavErr                      = 0;
    wavData->wavIOHandle        = NULL;
    wavData->totalBytesWritten  = 0;
    
    logMsg(LOGLEVEL_TRACE, "Initializing WAV Data with file: %s", inputFileStr);
    
    wavData->file = fopen(inputFileStr, "rb");
    if (wavData->file == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to open/read file: %s", inputFileStr);
        return(-1);
    }
    
    wavErr = wavIO_init(&wavData->wavIOHandle, CHUNKSIZE, FILLLASTFRAME, MODULEDELAY);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to initialize wavIO Handle");
        return(-1);
    }
    
    wavErr = wavIO_openRead(wavData->wavIOHandle, wavData->file, &wavData->channels, &wavData->sampleRate,
                            &wavData->byteDepth, &wavData->totalSamplesPerChannel, &wavData->samplesPerChannelFilled);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to read wav data");
        return(-1);
    }
    wavData->formatTag = wavIO_getInputFormatTag(wavData->wavIOHandle);

    logMsg(LOGLEVEL_DEBUG, "WAVData: Channels = %d, SampleRate = %d, ByteDepth = %d, FormatTag = %u",
           wavData->channels, wavData->sampleRate, wavData->byteDepth, wavData->formatTag);
    logMsg(LOGLEVEL_TRACE, "WAVData: TotalSamplesPerChannel = %d, SamplesPerChannelFilled = %d",
           wavData->totalSamplesPerChannel, wavData->samplesPerChannelFilled);
    
bail:
    return wavErr;
} 

int  initWAVDataForWriting            (WAVData *wavData, char *outputFileStr, int channelCount, int sampleRate, int bytesPerSample)
{
    int    wavErr;

    wavErr                      = 0;
    wavData->wavIOHandle        = NULL;
    wavData->totalBytesWritten  = 0;
    
    logMsg(LOGLEVEL_TRACE, "Initializing WAV Data for writing");
    
    wavData->file = fopen(outputFileStr, "wb");
    if (wavData->file == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to open/write file: %s", outputFileStr);
        return(-1);
    }
    
    wavErr = wavIO_init(&wavData->wavIOHandle, CHUNKSIZE, FILLLASTFRAME, MODULEDELAY);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to initialize wavIO Handle");
        return(-1);
    }
    
    wavData->channels       = channelCount;
    wavData->sampleRate     = sampleRate;
    wavData->byteDepth      = bytesPerSample;
    
    wavErr = wavIO_openWrite(wavData->wavIOHandle, wavData->file, wavData->channels, wavData->sampleRate, wavData->byteDepth);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to write wav data");
        return(-1);
    }
    
    logMsg(LOGLEVEL_DEBUG, "WAVData: Channels = %d, SampleRate = %d, ByteDepth = %d",
           wavData->channels, wavData->sampleRate, wavData->byteDepth);
    
bail:
    return wavErr;
}

int  closeWAVDataFile                 (WAVData *wavData)
{
    int  wavErr;
    wavErr = 0;
    
    logMsg(LOGLEVEL_DEBUG, "Updating WAV file header and closing file. total bytes written = %d", wavData->totalBytesWritten);
    unsigned long nTotalSamplesWrittenPerChannel;
    wavErr = wavIO_updateWavHeader(wavData->wavIOHandle, &nTotalSamplesWrittenPerChannel);

    fclose(wavData->file);
bail:
    return wavErr;
}

int  freeWAVData                 (WAVData *wavData)
{
    int  wavErr;
    wavErr = 0;
    
    wavIO_close(wavData->wavIOHandle);
    
    fclose(wavData->file);
bail:
    return wavErr;
}
