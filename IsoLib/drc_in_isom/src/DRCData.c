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

#include "DRCData.h"
#include "Logger.h"

#include <uniDrc.h>
#include <math.h>
#include <uniDrcTables.h>

extern int getBits (robitbufHandle bitstream, const int nBitsRequested, int* result);

MP4Err  copyDrcGainIntoBufferForSample  (DRCData *drcData, MP4Handle dataH, int bitOffset, int bitsRead);
MP4Err  readFileContentIntoBuffer       (char *fileStr, unsigned char **buffer, int *size);

MP4Err initDRCData  (DRCData *drcData, char *inputFileStr, int sampleRate, int channels, int framesize)
{
    MP4Err          err;
    int             drcErr;
    int             bytesRead;
    int             bitsRead;
    
    logMsg(LOGLEVEL_DEBUG, "Initializing DRC data with file %s", inputFileStr);
    
    err                             = MP4NoErr;
    drcErr                          = 0;
    bytesRead                       = 0;
    bitsRead                        = 0;
    drcData->bufferOffset           = 0;
    drcData->bufferIndex            = 0;
    drcData->bufferBytesRemaining   = 0;
    drcData->bitstreamBuffer        = NULL;
    drcData->hUniDrcBsDecStruct     = NULL;
    drcData->hUniDrcConfig          = NULL;
    drcData->hLoudnessInfoSet       = NULL;
    drcData->hUniDrcGain            = NULL;
    drcData->gainCount              = 0;
    
    drcData->bitstream = calloc(1, sizeof(robitbuf));
    
    drcErr = openUniDrcBitstreamDec(&drcData->hUniDrcBsDecStruct, &drcData->hUniDrcConfig,
                                    &drcData->hLoudnessInfoSet, &drcData->hUniDrcGain);
    if (drcErr) BAILWITHERROR(MP4InternalErr);
    
   
    logMsg(LOGLEVEL_DEBUG, "Audio settings for DRC Init: Samplerate: %d, Framesize: %d, Channels: %d", sampleRate, framesize, channels);
    
    drcErr = initUniDrcBitstreamDec(drcData->hUniDrcBsDecStruct, sampleRate, framesize, 0, -1, NULL);

    if (drcErr) BAILWITHERROR(MP4InternalErr);
    
    err = readFileContentIntoBuffer(inputFileStr, &drcData->bitstreamBuffer, &drcData->bufferBytesRemaining); if (err) goto bail;
    
    
    logMsg(LOGLEVEL_TRACE, "Processing initial uniDrcConfig (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
    drcErr = processUniDrcBitstreamDec_uniDrcConfig(drcData->hUniDrcBsDecStruct,
                                                 drcData->hUniDrcConfig,
                                                 drcData->hLoudnessInfoSet,
                                                 drcData->bitstreamBuffer,
                                                 drcData->bufferBytesRemaining,
                                                 drcData->bufferOffset,
                                                 &bitsRead);
    
    if (drcErr)
    {
        logMsg(LOGLEVEL_ERROR, "Processing initial uniDrcConfig failed (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
        BAILWITHERROR(MP4InternalErr);
    }
    
    bytesRead                           = bitsRead / 8;
    drcData->bufferOffset               = bitsRead - bytesRead * 8;
    drcData->bufferIndex                += bytesRead;
    drcData->bufferBytesRemaining       -= bytesRead;
    logMsg(LOGLEVEL_TRACE, "Finished processing initial uniDrcConfig (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
    
bail:
    return err;
}

MP4Err nextUniDRCGain   (DRCData *drcData, MP4Handle dataH)
{
    MP4Err          err;
    int             drcErr;
    int             bytesRead;
    int             bitsRead;
    
    err                             = MP4NoErr;
    drcErr                          = 0;
    bytesRead                       = 0;
    bitsRead                        = 0;
    
    if (drcData->gainCount != 0)
    {
        logMsg(LOGLEVEL_TRACE, "Processing uniDrcConfig (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
        drcErr = processUniDrcBitstreamDec_uniDrcConfig(drcData->hUniDrcBsDecStruct,
                                                 drcData->hUniDrcConfig,
                                                 drcData->hLoudnessInfoSet,
                                                 &drcData->bitstreamBuffer[drcData->bufferIndex],
                                                 drcData->bufferBytesRemaining,
                                                 drcData->bufferOffset,
                                                 &bitsRead);
        if (drcErr > PROC_COMPLETE)
        {
            logMsg(LOGLEVEL_ERROR, " Processing uniDrcConfig failed (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
            BAILWITHERROR(MP4InternalErr);
        }
    
        bytesRead                           = bitsRead / 8;
        drcData->bufferOffset               = bitsRead - bytesRead * 8;
        drcData->bufferIndex                += bytesRead;
        drcData->bufferBytesRemaining       -= bytesRead;
        logMsg(LOGLEVEL_TRACE, "Finished processing uniDrcConfig (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
        
        if (drcErr == PROC_COMPLETE)
            BAILWITHERROR(MP4EOF);
        
        if (bitsRead == 0)
            BAILWITHERROR(MP4EOF);
    }
    
    bytesRead                       = 0;
    bitsRead                        = 0;
    
    logMsg(LOGLEVEL_TRACE, "Processing uniDrcGain (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
    drcErr = processUniDrcBitstreamDec_uniDrcGain(drcData->hUniDrcBsDecStruct,
                                                  drcData->hUniDrcConfig,
                                                  drcData->hUniDrcGain,
                                                  &drcData->bitstreamBuffer[drcData->bufferIndex],
                                                  drcData->bufferBytesRemaining,
                                                  drcData->bufferOffset,
                                                  &bitsRead);
    
    err = copyDrcGainIntoBufferForSample(drcData, dataH, drcData->bufferOffset, bitsRead);
    if (err) goto bail;
    
    drcData->gainCount++;
    logMsg(LOGLEVEL_TRACE, "Gain #%d: %d bits", drcData->gainCount, bitsRead - drcData->bufferOffset);
    
    bytesRead                           = bitsRead / 8;
    drcData->bufferOffset               = bitsRead - bytesRead * 8;
    drcData->bufferIndex                += bytesRead;
    drcData->bufferBytesRemaining       -= bytesRead;
    
    
    logMsg(LOGLEVEL_TRACE, "Finished processing uniDrcGain (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
    
    if (drcErr > PROC_COMPLETE)
    {
        logMsg(LOGLEVEL_ERROR, " Processing uniDrcGain failed (bytePosition: %d, bitOffset: %d, bytesRemaining: %d)", drcData->bufferIndex, drcData->bufferOffset, drcData->bufferBytesRemaining);
        BAILWITHERROR(MP4InternalErr);
    }
    
    if (drcErr == PROC_COMPLETE)
        err = MP4EOF;
    
    if (bitsRead == 0)
    {
        err = MP4EOF;
    }
    
bail:
    return err;
}

MP4Err  copyDrcGainIntoBufferForSample  (DRCData *drcData, MP4Handle dataH, int bitOffset, int bitsRead)
{
    MP4Err          err;
    int             bitErr;
    char            *buffer;
    u32             handleSize;
    int             tmp;
    int             bits;
    
    err     = MP4NoErr;
    bitErr  = 0;
    bits    = bitsRead - bitOffset;
    
    logMsg(LOGLEVEL_TRACE, "Copying DRC gain to Sample Buffer, bits: %d", bits);
    
    robitbufHandle bitstream = drcData->bitstream;
    
    if (bitstream != NULL && drcData->bufferBytesRemaining)
    {
        robitbuf_Init(bitstream, &drcData->bitstreamBuffer[drcData->bufferIndex], drcData->bufferBytesRemaining * 8, 0);
    }
    else
    {
        logMsg(LOGLEVEL_ERROR, "Initializing robitbuf failed");
        BAILWITHERROR(MP4InternalErr);
    }
    
    bitErr = getBits(bitstream, bitOffset, NULL);
    if (bitErr) BAILWITHERROR(MP4InternalErr);
    
    handleSize = bits / 8;
    if (bits % 8 != 0)
        handleSize++;
    
    err                 = MP4SetHandleSize( dataH, handleSize ); if (err) goto bail;
    buffer              = (char *) *dataH;
    
    for (int i = 0; i < handleSize - 1; i++)
    {
        bitErr = getBits(bitstream, 8, (int *) &buffer[i]);
        if (bitErr) BAILWITHERROR(MP4InternalErr);
    }
    
    if (bits % 8 != 0)
    {
        bitErr = getBits(bitstream, bits % 8, &tmp);
        if (bitErr) BAILWITHERROR(MP4InternalErr);
        
        buffer[handleSize-1] = tmp << (8 - bits % 8);
    }
    else
    {
        bitErr = getBits(bitstream, 8, (int *) &buffer[handleSize-1]);
        if (bitErr) BAILWITHERROR(MP4InternalErr);
    }
    
    logMsg(LOGLEVEL_TRACE, "Copying DRC gain to Sample Buffer finished, bytes: %d", handleSize);
    
bail:
    return err;
}

MP4Err readFileContentIntoBuffer    (char *fileStr, unsigned char **buffer, int *size)
{
    MP4Err          err;
    int             drcErr;
    FILE            *inputFile;
    
    err                             = MP4NoErr;
    drcErr                          = 0;
    inputFile = fopen (fileStr , "rb");
    
    if (inputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Unable to open/read file: %s", fileStr);
        BAILWITHERROR(MP4IOErr);
    }
    
    fseek( inputFile , 0L , SEEK_END);
    *size = (int)ftell( inputFile );
    rewind( inputFile );
    
    *buffer = calloc( 1, *size );
    
    if( 1!=fread( *buffer , *size, 1 , inputFile) ) {
        fclose(inputFile);
        free(*buffer);
        logMsg(LOGLEVEL_ERROR, "Unable to read file: %s", fileStr);
        BAILWITHERROR(MP4IOErr);
    }
    fclose(inputFile);
    
bail:
    return err;
}


MP4Err  freeDRCData                  (DRCData *drcData)
{
    MP4Err  err;
    int     drcErr;
    
    err = MP4NoErr;
    
    drcErr = closeUniDrcBitstreamDec(&drcData->hUniDrcBsDecStruct, &drcData->hUniDrcConfig,
                                     &drcData->hLoudnessInfoSet, &drcData->hUniDrcGain);
    if (drcErr) BAILWITHERROR(MP4InternalErr);
    
    free (drcData->bitstream);
    free (drcData->bitstreamBuffer);
bail:
    return err;
}
