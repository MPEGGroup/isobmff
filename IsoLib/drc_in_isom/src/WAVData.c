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
derivative works. Copyright (c) 2014.
*/

#include "WAVData.h"
#include "Logger.h"

#include <stdio.h>
#include <string.h>

#define FILLLASTFRAME 1
#define MODULEDELAY 0

static void updateWavHeader(WAVData *wavData);

MP4Err initWAVDataForReading    (WAVData *wavData, char *inputFileStr)
{
    int    wavErr;
    MP4Err err;
    
    wavErr                      = 0;
    err                         = MP4NoErr;
    wavData->wavIOHandle        = NULL;
    wavData->totalBytesWritten  = 0;
    
    logMsg(LOGLEVEL_TRACE, "Initializing WAV Data with file: %s", inputFileStr);
    
    wavData->file = fopen(inputFileStr, "rb");
    if (wavData->file == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to open/read file: %s", inputFileStr);
        BAILWITHERROR(MP4IOErr);
    }
    
    wavErr = wavIO_init(&wavData->wavIOHandle, BLOCKLENGTH, FILLLASTFRAME, MODULEDELAY);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to initialize wavIO Handle");
        BAILWITHERROR(MP4InternalErr);
    }
    
    wavErr = wavIO_openRead(wavData->wavIOHandle, wavData->file, &wavData->channels, &wavData->sampleRate,
                            &wavData->byteDepth, &wavData->totalSamplesPerChannel, &wavData->samplesPerChannelFilled);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to read wav data");
        BAILWITHERROR(MP4IOErr);
    }
    
    logMsg(LOGLEVEL_DEBUG, "WAVData: Channels = %d, SampleRate = %d, ByteDepth = %d",
           wavData->channels, wavData->sampleRate, wavData->byteDepth);
    logMsg(LOGLEVEL_TRACE, "WAVData: TotalSamplesPerChannel = %d, SamplesPerChannelFilled = %d",
           wavData->totalSamplesPerChannel, wavData->samplesPerChannelFilled);
    
bail:
    return err;
}

MP4Err readFrame        (WAVData *wavData, MP4Handle sampleH, u32 *duration)
{
    MP4Err          err;
    char            *buffer;
    u32             handleSize;
    size_t          elementsRead;
    
    logMsg(LOGLEVEL_TRACE, "Reading wav frame");
    
    err                 = MP4NoErr;
    elementsRead        = 0;
    handleSize          = BLOCKLENGTH * wavData->channels * wavData->byteDepth;
    err                 = MP4SetHandleSize( sampleH, handleSize ); if (err) goto bail;
    buffer              = (char *) *sampleH;
    
    elementsRead = fread(buffer, wavData->channels * wavData->byteDepth, BLOCKLENGTH, wavData->file);
    
    *duration = (u32) elementsRead;
    
    logMsg(LOGLEVEL_TRACE, "Frame read: duration: %d", *duration);
    if (elementsRead == 0)
    {
        logMsg(LOGLEVEL_TRACE, "Reading wav frame: end of file reached!");
        err = MP4EOF;
    }
bail:
    return err;
}

MP4Err  initWAVDataForWriting            (WAVData *wavData, char *outputFileStr, int channelCount, int sampleRate, int bytesPerSample)
{
    int    wavErr;
    MP4Err err;
    
    wavErr                      = 0;
    err                         = MP4NoErr;
    wavData->wavIOHandle        = NULL;
    wavData->totalBytesWritten  = 0;
    
    logMsg(LOGLEVEL_TRACE, "Initializing WAV Data for writing");
    
    wavData->file = fopen(outputFileStr, "wb");
    if (wavData->file == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to open/write file: %s", outputFileStr);
        BAILWITHERROR(MP4IOErr);
    }
    
    wavErr = wavIO_init(&wavData->wavIOHandle, BLOCKLENGTH, FILLLASTFRAME, MODULEDELAY);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to initialize wavIO Handle");
        BAILWITHERROR(MP4InternalErr);
    }
    
    wavData->channels       = channelCount;
    wavData->sampleRate     = sampleRate;
    wavData->byteDepth      = bytesPerSample;
    
    wavErr = wavIO_openWrite(wavData->wavIOHandle, wavData->file, wavData->channels, wavData->sampleRate, wavData->byteDepth);
    if (wavErr != 0)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to write wav data");
        BAILWITHERROR(MP4IOErr);
    }
    
    logMsg(LOGLEVEL_DEBUG, "WAVData: Channels = %d, SampleRate = %d, ByteDepth = %d",
           wavData->channels, wavData->sampleRate, wavData->byteDepth);
    
bail:
    return err;
}

MP4Err  writeFrame                       (WAVData *wavData, MP4Handle sampleH, u32 size)
{
    MP4Err err;
    char *buffer;
    
    logMsg(LOGLEVEL_TRACE, "Writing audio frame to file. Size = %d", size);
    
    err     = MP4NoErr;
    buffer  = (char *) *sampleH;
    
    fwrite(buffer, size, 1, wavData->file);
    
    if (ferror (wavData->file))
    {
        logMsg(LOGLEVEL_ERROR, "Unable to write to wav output file");
        BAILWITHERROR(MP4IOErr);
    }
    
    wavData->totalBytesWritten += size;
    logMsg(LOGLEVEL_TRACE, "Writing audio frame to file finished. total bytes written = %d", wavData->totalBytesWritten);
bail:
    return err;
}

MP4Err  closeWAVDataFile                 (WAVData *wavData)
{
    MP4Err  err;
    err = MP4NoErr;
    
    logMsg(LOGLEVEL_DEBUG, "Updating WAV file header and closing file. total bytes written = %d", wavData->totalBytesWritten);
    updateWavHeader(wavData);
    
    fclose(wavData->file);
bail:
    return err;
}

static int LittleToNativeEndianLong(int x)
{
    char *t     = (char*)(&x);
    char tmp    = t[0];
    
    t[0] = t[3];
    t[3] = tmp;
    tmp  = t[1];
    t[1] = t[2];
    t[2] = tmp;
    
    return *((int*)t);
}

static void updateWavHeader(WAVData *wavData)
{
    int             bytesWritten = 0;
    unsigned int    tmp;
    
    bytesWritten = wavData->totalBytesWritten;
    
    bytesWritten += 36; //WAVIO_RIFF_HEADER_SIZE
    
    fseek(wavData->file, 4, SEEK_SET);
    tmp = LittleToNativeEndianLong(bytesWritten);
    fwrite(&tmp, sizeof(int), 1, wavData->file);
    
    bytesWritten -= 36;
    
    fseek(wavData->file, 40, SEEK_SET);
    tmp = LittleToNativeEndianLong(bytesWritten);
    fwrite(&tmp, sizeof(int), 1, wavData->file);
}

MP4Err  freeWAVData                 (WAVData *wavData)
{
    MP4Err  err;
    err = MP4NoErr;
    
    wavIO_close(wavData->wavIOHandle);
    
    fclose(wavData->file);
bail:
    return err;
}