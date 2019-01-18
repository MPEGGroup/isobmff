/***********************************************************************************
 
 This software module was originally developed by 
 
 Fraunhofer IIS
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 14496-12, ISO/IEC
 23003-4, and ISO/IEC 23008-3 standards and which satisfy any specified conformance
 criteria. Those intending to use this  software module in products are advised that
 its use may infringe existing patents. ISO/IEC have no liability for use of this
 software module or modifications thereof. Copyright is not released for products
 that do not conform to the ISO/IEC 14496-12, ISO/IEC 23003-4, and ISO/IEC 23008-3
 standards.
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2018.
 
 ***********************************************************************************/

#ifndef _WAVIO_H
#define _WAVIO_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

typedef struct _WAVIO* WAVIO_HANDLE;

int wavIO_init(WAVIO_HANDLE *hWavIO, const unsigned int framesize, const unsigned int fillLastIncompleteInputFrame, int delay);
int wavIO_setDelay(WAVIO_HANDLE hWavIO, int delay);
unsigned int wavIO_getInputFormatTag(WAVIO_HANDLE hWavIO);
int wavIO_openRead(WAVIO_HANDLE hWavIO, FILE *pInFileName, unsigned int *nInChannels, unsigned int *InSampleRate, unsigned int * InBytedepth, unsigned long *nTotalSamplesPerChannel, int *nSamplesPerChannelFilled);
int wavIO_readFrame(WAVIO_HANDLE hWavIO, float **inBuffer, unsigned int *nSamplesReadPerChannel, unsigned int *isLastFrame, unsigned int * nZerosPaddedBeginning,  unsigned int * nZerosPaddedEnd);
int wavIO_openWrite(WAVIO_HANDLE hWavIO, FILE *pOutFileName, unsigned int nOutChannels, unsigned int OutSampleRate, unsigned int bytesPerSample);
int wavIO_writeFrame(WAVIO_HANDLE hWavIO, float **outBuffer, unsigned int nSamplesToWritePerChannel, unsigned int *nSamplesWrittenPerChannel);
int wavIO_writeRawData (WAVIO_HANDLE hWavIO, char* data, int byteSize);
int wavIO_updateWavHeader(WAVIO_HANDLE hWavIO, unsigned long * nTotalSamplesWrittenPerChannel);
int wavIO_close(WAVIO_HANDLE hWavIO);

#ifdef __cplusplus
}
#endif

#endif
