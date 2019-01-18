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

#include <math.h>
#include <string.h>
#include "wavIO.h"

#define CLIP_WARNING 1
#ifdef CLIP_WARNING
#include <stdio.h>
#endif

typedef struct _AUDIO_DATA_BUFFER
{
  char  *inBufferChar;
  float *inBufferFloat;
  char  *outBufferChar;
  float *outBufferFloat;

  int readPosition;

} AUDIO_DATA_BUFFER, *H_AUDIO_DATA_BUFFER ;

struct _WAVIO
{
  unsigned int framesize;
  unsigned int fillLastInclompleteFrame;
  int delay;

  FILE* fIn;
  unsigned int nInputChannels;
  unsigned int nInputSamplerate;
  unsigned int nInputBytesperSample;
  unsigned long int nInputSamplesPerChannel;
  unsigned int inputFormatTag;

  FILE* fOut;
  unsigned int nOutputChannels;
  unsigned int nOutputSamplerate;
  unsigned int nOutputBytesperSample;
  unsigned long int nTotalSamplesWrittenPerChannel;
  unsigned int outputFormatTag;

  AUDIO_DATA_BUFFER* audioBuffers;
};

typedef struct RiffWavHeader 
  {
    /* total length of the .wav Header: 44 Byte*/
    /* riff-section*/
    char riff_name[4];          /* 4 Byte, header-signature, contains "RIFF"*/
    unsigned int riff_length;   /* 4 Byte, length of the rest of the header (36 Byte) + <data_length> in Byte*/
    char riff_typ[4];           /* 4 Byte, contains the audio type, here "WAVE"*/

    /* format(fmt)-section*/
    char fmt_name[4];           /* 4 Byte, header-signature, contains "fmt "*/
    unsigned int fmt_length;    /* 4 Byte, length of the rest of the fmt header in Byte*/
    short format_tag;           /* 2 Byte, data format of the audio samples*/
    short channels;             /* 2 Byte, number of used channels*/
    unsigned int fs;            /* 4 Byte, sample frequency in Herz*/
    unsigned int bytes_per_sec; /* 4 Byte, <fs> * <block_align>*/
    short block_align;          /* 2 Byte, <channels> * (<bits/sample> / 8)*/
    short bpsample;             /* 2 Byte, quantizer bit depth, 8, 16 or 24 Bit*/

    /* data-section*/
    char data_name[4];          /* 4 Byte, header-signature, contains "data"*/
    unsigned int data_length;   /* 4 Byte, length of the data chunk in Byte*/
  } RIFFWAVHEADER;


#define WAVIO_RIFF_HEADER_SIZE 36

static int ConvertIntToFloat(float *pfOutBuf, const char *pbInBuf, unsigned int nInBytes, unsigned int nBytesPerSample, unsigned int* bytesLeft);
static void ConvertFloatToInt(char *OutBuf, float *InBuf, unsigned int length, unsigned int nBytesPerSample);
static short LittleToNativeEndianShort(short x);
static int LittleToNativeEndianLong(int x);
static __inline short LittleEndian16 (short v);
static __inline int IsLittleEndian (void);
static unsigned int BigEndian32 (char a, char b, char c, char d);

int wavIO_init(WAVIO_HANDLE *hWavIO, const unsigned int framesize, const unsigned int fillLastIncompleteInputFrame, int delay)
{

  H_AUDIO_DATA_BUFFER hBuffer = NULL;

  WAVIO_HANDLE hWavIOtemp = (WAVIO_HANDLE)calloc(1,sizeof(struct _WAVIO));


  hWavIOtemp->framesize = framesize;
  hWavIOtemp->fillLastInclompleteFrame = fillLastIncompleteInputFrame;
  hWavIOtemp->delay = delay;

  hBuffer = (AUDIO_DATA_BUFFER*) calloc(1,sizeof(AUDIO_DATA_BUFFER));
  hWavIOtemp->audioBuffers = hBuffer;

 *hWavIO = hWavIOtemp;
  
  return 0;

}

int wavIO_setDelay(WAVIO_HANDLE hWavIO, int delay)
{
  hWavIO->delay = delay;

  return 0;
}

unsigned int wavIO_getInputFormatTag(WAVIO_HANDLE hWavIO)
{
    return hWavIO->inputFormatTag;
}

int wavIO_openRead(WAVIO_HANDLE hWavIO, FILE *pInFileName, unsigned int *nInChannels, unsigned int *InSampleRate, unsigned int * InBytedepth, unsigned long *nTotalSamplesPerChannel, int *nSamplesPerChannelFilled)
{
  unsigned int riff = BigEndian32 ('R','I','F','F');
  unsigned int junk = BigEndian32 ('J','U','N','K');
  unsigned int fmt = BigEndian32  ('f','m','t',' ');

  unsigned int i=0;

  unsigned int chunkID;

  RIFFWAVHEADER riffheader = {{ 0 }};
  int fmt_read = 0;

  hWavIO->fIn = pInFileName;
  if (hWavIO->fIn == NULL) 
  {
    return -1;
  }

  while (!fmt_read)
  {
    fread(&chunkID, sizeof(unsigned int),1,hWavIO->fIn);
    if (chunkID == riff)
    {
      memcpy(riffheader.riff_name, (char*) &chunkID,4);
      fread(&riffheader.riff_length, sizeof(unsigned int),1,hWavIO->fIn);
      fread(&riffheader.riff_typ, sizeof(unsigned int), 1, hWavIO->fIn);
    }
    else if (chunkID == junk)
    {
      /* do nothing */
    }
    else if (chunkID == fmt)
    {
      memcpy(riffheader.fmt_name, (char*) &chunkID,4);
      fread(&riffheader.fmt_length, sizeof(unsigned int),1,hWavIO->fIn);
      fread(&riffheader.format_tag, 2, 1, hWavIO->fIn);
      fread(&riffheader.channels,2,1,hWavIO->fIn);
      fread(&riffheader.fs,sizeof(unsigned int),1,hWavIO->fIn);
      fread(&riffheader.bytes_per_sec,sizeof(unsigned int),1,hWavIO->fIn);
      fread(&riffheader.block_align,2,1,hWavIO->fIn);
      fread(&riffheader.bpsample,2,1,hWavIO->fIn);
      fmt_read = 1;
    }
    else
    {
      unsigned long int pos = 0;
      i++;
      if (i > 5000)
      {
        break;
      }
      pos = ftell(hWavIO->fIn);
      fseek(hWavIO->fIn, pos-3, SEEK_SET);
    }
  }

  /* Search for data chunk */
  {
    /* Search data chunk */
    unsigned int i = 0;
    unsigned int dataTypeRead = 0;
    unsigned int dataType = BigEndian32 ('d','a','t','a') ;
    while(1) 
      {
        i++;
        if( i > 5000 ) {
        /* Error */
          break;
        }

        fread( &dataTypeRead, sizeof(unsigned int), 1, hWavIO->fIn );
        if ( dataTypeRead == dataType) {
          /* 'data' chunk found - now read dataSize */
          memcpy(riffheader.data_name, (char*) &dataTypeRead, 4);
          fread(&(riffheader.data_length), sizeof(unsigned int), 1 , hWavIO->fIn);
          break;
        }
        else {
          /* 3 bytes back */
          unsigned long int pos=0;
          pos = ftell(hWavIO->fIn);
          fseek(hWavIO->fIn, pos-3, SEEK_SET);
        }
      }
  }
  hWavIO->nInputBytesperSample = riffheader.bpsample/8;
  hWavIO->nInputChannels = riffheader.channels;
  hWavIO->nInputSamplerate = riffheader.fs;
  hWavIO->inputFormatTag = riffheader.format_tag;

  *nTotalSamplesPerChannel = riffheader.data_length / hWavIO->nInputChannels / hWavIO->nInputBytesperSample;
  *nInChannels = hWavIO->nInputChannels;
  *InSampleRate = hWavIO->nInputSamplerate;
  *InBytedepth = hWavIO->nInputBytesperSample;

  hWavIO->audioBuffers->inBufferChar = NULL;
  hWavIO->audioBuffers->inBufferFloat = NULL;

  hWavIO->audioBuffers->inBufferChar = (char*)calloc(hWavIO->framesize * hWavIO->nInputChannels * hWavIO->nInputBytesperSample, sizeof(char));
  hWavIO->audioBuffers->inBufferFloat = (float*)calloc(hWavIO->framesize * hWavIO->nInputChannels, sizeof(float));
  hWavIO->audioBuffers->readPosition = 0;


  hWavIO->nInputSamplesPerChannel = *nTotalSamplesPerChannel;
  if ( hWavIO->fillLastInclompleteFrame && (*nTotalSamplesPerChannel % hWavIO->framesize) )
  {
    *nSamplesPerChannelFilled = hWavIO->framesize - (*nTotalSamplesPerChannel % hWavIO->framesize);
  }
  else {
    *nSamplesPerChannelFilled = 0;
  }

  return 0;
}

int wavIO_openWrite(WAVIO_HANDLE hWavIO, FILE *pOutFileName, unsigned int nOutChannels, unsigned int OutSampleRate, unsigned int bytesPerSample)
{ 
  RIFFWAVHEADER header = {{ 0 }};
  hWavIO->fOut = pOutFileName;
  if (hWavIO->fOut == NULL) 
  {
    return -1;
  }

  header.riff_name[0]='R';
  header.riff_name[1]='I';
  header.riff_name[2]='F';
  header.riff_name[3]='F';

  
  header.riff_length=LittleToNativeEndianLong(0);

  header.riff_typ[0]='W';
  header.riff_typ[1]='A';
  header.riff_typ[2]='V';
  header.riff_typ[3]='E';

  
  header.fmt_name[0]='f';
  header.fmt_name[1]='m';
  header.fmt_name[2]='t';
  header.fmt_name[3]=' ';

  
  header.fmt_length = LittleToNativeEndianLong(16);

  if ( bytesPerSample == 4 ) {
    header.format_tag    = LittleToNativeEndianShort(3);
  }
  else {
    header.format_tag    = LittleToNativeEndianShort(1);
  }

  header.channels      = LittleToNativeEndianShort(nOutChannels);
  header.fs            = LittleToNativeEndianLong(OutSampleRate);
  header.bpsample      = LittleToNativeEndianShort(bytesPerSample*8);
  header.bytes_per_sec = LittleToNativeEndianLong(nOutChannels * OutSampleRate * bytesPerSample);
  header.block_align   = LittleToNativeEndianShort(nOutChannels * bytesPerSample);

  
  header.data_name[0]='d';
  header.data_name[1]='a';
  header.data_name[2]='t';
  header.data_name[3]='a';

  header.data_length=LittleToNativeEndianLong(0);

  /* write header to OutFile */
  fwrite(&header, sizeof(header), 1, hWavIO->fOut);

  hWavIO->nOutputChannels = nOutChannels;
  hWavIO->nOutputSamplerate = OutSampleRate;
  hWavIO->nOutputBytesperSample = bytesPerSample;
  hWavIO->outputFormatTag = header.format_tag;

  hWavIO->audioBuffers->outBufferChar = NULL;
  hWavIO->audioBuffers->outBufferFloat = NULL;
  hWavIO->audioBuffers->outBufferChar = (char*)calloc(hWavIO->framesize * hWavIO->nOutputChannels * hWavIO->nOutputBytesperSample, sizeof(char));
  hWavIO->audioBuffers->outBufferFloat = (float*)calloc(hWavIO->framesize * hWavIO->nOutputChannels, sizeof(float));

  return 0;
}

int wavIO_readFrame(WAVIO_HANDLE hWavIO, float **inBuffer, unsigned int *nSamplesReadPerChannel, unsigned int *isLastFrame, unsigned int * nZerosPaddedBeginning,  unsigned int * nZerosPaddedEnd)
{
  int i = 0;
  unsigned int left = 0 ;
  unsigned int *h_left = &left;
  float tempfloat = 0;
  unsigned int j;
  int ct_pos = -1;
  int error = 0;
  int samplesToRead = 0;
  int zerosToRead = 0;

  int delaySamples = hWavIO->delay;

  if (delaySamples > 0)
  {
    zerosToRead = min( (unsigned int)delaySamples, hWavIO->framesize );
    samplesToRead = hWavIO->framesize - zerosToRead;
    for (i=0;i<zerosToRead * (int)hWavIO->nInputChannels;i= i+hWavIO->nInputChannels)
    {
      for (j=0;j<hWavIO->nInputChannels;j++)
      {
        hWavIO->audioBuffers->inBufferFloat[i+j] = 0.0f;
      }
    }
    if (hWavIO->nInputBytesperSample > 1)
    {
      fread(hWavIO->audioBuffers->inBufferChar, hWavIO->nInputBytesperSample, samplesToRead * hWavIO->nInputChannels, hWavIO->fIn); 
      error = ConvertIntToFloat(hWavIO->audioBuffers->inBufferFloat, hWavIO->audioBuffers->inBufferChar, samplesToRead * hWavIO->nInputChannels * hWavIO->nInputBytesperSample * sizeof(char), hWavIO->nInputBytesperSample, h_left);
    }
    else
    {
      fread(hWavIO->audioBuffers->inBufferFloat,hWavIO->nInputBytesperSample, samplesToRead * hWavIO->nInputChannels, hWavIO->fIn); 
    }
    delaySamples -= zerosToRead;
    hWavIO->delay = delaySamples;
    *nSamplesReadPerChannel = samplesToRead;
    for (j=0;j<(int)(hWavIO->nInputChannels);j++)
    {
      ct_pos = -1;
      for (i = 0;i <  (int)(samplesToRead * hWavIO->nInputChannels); i = i+hWavIO->nInputChannels)
      {
        ct_pos++;
        tempfloat = hWavIO->audioBuffers->inBufferFloat[i+j];
        inBuffer[j][ct_pos + zerosToRead] = tempfloat;
      }
    }

  }
  else
  {
    int leftSamples = hWavIO->nInputSamplesPerChannel - (hWavIO->audioBuffers->readPosition + hWavIO->framesize);
    if ( leftSamples  > 0)     /* more than one frame left */
    {
      samplesToRead = hWavIO->framesize;
      *isLastFrame = 0;
    }
    else if (leftSamples == 0) /* exactly one frame left */
    {
      samplesToRead = hWavIO->framesize;
      *isLastFrame = 1;
    }
    else                       /* less than one frame left */
    {
      samplesToRead = hWavIO->nInputSamplesPerChannel - hWavIO->audioBuffers->readPosition;
      *isLastFrame = 1;
    }

    if (hWavIO->nInputBytesperSample > 1)
    {
      fread(hWavIO->audioBuffers->inBufferChar, hWavIO->nInputBytesperSample, samplesToRead * hWavIO->nInputChannels, hWavIO->fIn); 
      error = ConvertIntToFloat(hWavIO->audioBuffers->inBufferFloat, hWavIO->audioBuffers->inBufferChar, samplesToRead * hWavIO->nInputChannels * hWavIO->nInputBytesperSample * sizeof(char), hWavIO->nInputBytesperSample, h_left);
    }
    else
    {
      fread(hWavIO->audioBuffers->inBufferFloat,hWavIO->nInputBytesperSample, samplesToRead * hWavIO->nInputChannels, hWavIO->fIn); 
    }
    *nSamplesReadPerChannel = samplesToRead;


    if ( *isLastFrame )
    {
      
      /* Fill up frame with zeros if wanted */
      if ( hWavIO->fillLastInclompleteFrame )
        {
          int i = 0;
          
          /* Calculate number of samples to add  */
          int nSamplesToAdd = hWavIO->framesize - samplesToRead; 
          *nZerosPaddedEnd = nSamplesToAdd;

          for ( i = 0; i <  (int)(hWavIO->nInputChannels * nSamplesToAdd); ++i )
          {
            hWavIO->audioBuffers->inBufferFloat[i + hWavIO->nInputChannels * samplesToRead] = 0.0f;
          }
        }
    }

    for (j=0;j<(int)(hWavIO->nInputChannels);j++)
    {
      ct_pos = -1;
      for (i = 0;i <  (int)(samplesToRead * hWavIO->nInputChannels); i = i+hWavIO->nInputChannels)
      {
        ct_pos++;
        tempfloat = hWavIO->audioBuffers->inBufferFloat[i+j];
        inBuffer[j][ct_pos] = tempfloat;
      }
    }
  }

  hWavIO->audioBuffers->readPosition +=  samplesToRead;
  *nZerosPaddedBeginning = zerosToRead;

  return 0;
}

int wavIO_writeFrame(WAVIO_HANDLE hWavIO, float **outBuffer, unsigned int nSamplesToWritePerChannel, unsigned int *nSamplesWrittenPerChannel)
{
  unsigned int i= 0;
  int ct_pos = -1;
  unsigned int j;
  float tempfloat;
  int delaySamples = hWavIO->delay;

  if (delaySamples < 0)
  {
    unsigned int i= 0;
    int samplesToWrite = nSamplesToWritePerChannel - (-1)*delaySamples;
    int samplesToSkip = 0;
    ct_pos = -1;
    ct_pos = ct_pos + samplesToSkip;
    if (samplesToWrite < 0)
    {
      samplesToWrite = 0;
    }
    samplesToSkip = nSamplesToWritePerChannel - samplesToWrite;

    ct_pos = samplesToSkip-1;
    for (i=0; i < samplesToWrite*hWavIO->nOutputChannels; i=i+hWavIO->nOutputChannels)
    {
      ct_pos++;
      for (j=0;j<hWavIO->nOutputChannels;j++)
      {
        tempfloat = outBuffer[j][ct_pos];
        hWavIO->audioBuffers->outBufferFloat[i+j] = tempfloat;
      }
    }

    ConvertFloatToInt(hWavIO->audioBuffers->outBufferChar, hWavIO->audioBuffers->outBufferFloat, samplesToWrite * hWavIO->nOutputChannels * hWavIO->nOutputBytesperSample, hWavIO->nOutputBytesperSample);
    fwrite(hWavIO->audioBuffers->outBufferChar, hWavIO->nOutputBytesperSample, samplesToWrite * hWavIO->nOutputChannels,hWavIO->fOut);

    if (samplesToWrite == 0)
    {
      delaySamples += nSamplesToWritePerChannel;
    }
    else
    {
      delaySamples = 0;
    }

    hWavIO->delay = delaySamples;
    *nSamplesWrittenPerChannel = samplesToWrite;
  }
  
  else
  {
    /* Interleave channels */
    for (i=0; i < nSamplesToWritePerChannel * hWavIO->nOutputChannels; i=i+hWavIO->nOutputChannels)
    {
      ct_pos++;
      for (j=0;j<hWavIO->nOutputChannels;j++)
      {
        tempfloat = outBuffer[j][ct_pos];
        hWavIO->audioBuffers->outBufferFloat[i+j] = tempfloat;
      }
    }

    ConvertFloatToInt(hWavIO->audioBuffers->outBufferChar, hWavIO->audioBuffers->outBufferFloat, nSamplesToWritePerChannel * hWavIO->nOutputChannels * hWavIO->nOutputBytesperSample, hWavIO->nOutputBytesperSample);
    fwrite(hWavIO->audioBuffers->outBufferChar, hWavIO->nOutputBytesperSample, nSamplesToWritePerChannel * hWavIO->nOutputChannels,hWavIO->fOut);

    *nSamplesWrittenPerChannel = nSamplesToWritePerChannel;
  }

  hWavIO->nTotalSamplesWrittenPerChannel += *nSamplesWrittenPerChannel;
  return 0;
}

int wavIO_writeRawData (WAVIO_HANDLE hWavIO, char* data, int byteSize)
{
  fwrite(data, byteSize, 1, hWavIO->fOut);
  
  if (ferror (hWavIO->fOut))
  {
    return -1;
  }
  hWavIO->nTotalSamplesWrittenPerChannel += byteSize / (hWavIO->nOutputChannels * hWavIO->nOutputBytesperSample);
  return 0;
}

int wavIO_updateWavHeader( WAVIO_HANDLE hWavIO, unsigned long * nTotalSamplesWrittenPerChannel )
{
  unsigned long bytesWritten = 0;
  unsigned int tmp;

  if (hWavIO->fOut) {
    bytesWritten = hWavIO->nTotalSamplesWrittenPerChannel * hWavIO->nOutputChannels * hWavIO->nOutputBytesperSample;

    bytesWritten+=WAVIO_RIFF_HEADER_SIZE;


    fseek(hWavIO->fOut,4,SEEK_SET);
    tmp = LittleToNativeEndianLong(bytesWritten);
    fwrite(&tmp, sizeof(int), 1, hWavIO->fOut);


    bytesWritten-=WAVIO_RIFF_HEADER_SIZE;


    fseek(hWavIO->fOut,40,SEEK_SET);
    tmp = LittleToNativeEndianLong(bytesWritten);
    fwrite(&tmp, sizeof(int), 1, hWavIO->fOut);

    *nTotalSamplesWrittenPerChannel = hWavIO->nTotalSamplesWrittenPerChannel;
  }
  else {
    *nTotalSamplesWrittenPerChannel = 0;
  }
  return 0;
}


int wavIO_close(WAVIO_HANDLE hWavIO)
{
  if ( hWavIO->fIn )
  {
    fclose(hWavIO->fIn);
    free(hWavIO->audioBuffers->inBufferChar);
    free(hWavIO->audioBuffers->inBufferFloat);
  }

  if ( hWavIO->fOut )
  {
    fclose(hWavIO->fOut);
    free(hWavIO->audioBuffers->outBufferChar);
    free(hWavIO->audioBuffers->outBufferFloat);
  }

  free(hWavIO->audioBuffers);

  free(hWavIO);

  return 0;
}



static int ConvertIntToFloat(float *pfOutBuf, const char *pbInBuf, unsigned int nInBytes, unsigned int nBytesPerSample, unsigned int* bytesLeft)
{
  unsigned int i,j, nSamples, nOffset;


  if ( nBytesPerSample == 4 ) {
    memcpy(pfOutBuf, pbInBuf, nInBytes*sizeof(char));
    return 0;
  }

  if (nBytesPerSample == sizeof(short))
    {
      const short* shortBuf = (const short*) pbInBuf;
      for(j = 0; j < nInBytes/nBytesPerSample; j++) 
      {
        pfOutBuf[j] = ((float) LittleEndian16(shortBuf[j]))/32768.f;
      }
    }

  else if ( nBytesPerSample > 2 )
    {

      union { signed long s; char c[sizeof(long)]; } u;
      float fFactor =  (float)(1 << ( nBytesPerSample*8 - 1 ));
      fFactor = 1.0f / fFactor;
      
      u.s = 0;
      
      nOffset = (sizeof(long) - nBytesPerSample) * 8;
      
      nSamples = nInBytes / nBytesPerSample;

      *bytesLeft = nInBytes % nBytesPerSample;
      
      for(j = 0; j < nSamples; j++) 
        {
          int n = j * nBytesPerSample;
          
          /* convert chars to 32 bit */
          if (IsLittleEndian())
            {
              for (i = 0; i < nBytesPerSample; i++)
                {
                  u.c[sizeof(long) - 1 - i] = pbInBuf[n + nBytesPerSample - i - 1];
                }
            } 
          else
            {
              for (i = 0; i < nBytesPerSample; i++)
                {
                  u.c[i] = pbInBuf[n + nBytesPerSample - i - 1];
                }
              
            } 
          
          u.s = u.s >> nOffset;
          
          pfOutBuf[j] = ((float) u.s) * fFactor;

        }
    }
  return 0;
}



static void ConvertFloatToInt(char *OutBuf, float *InBuf, unsigned int length, unsigned int nBytesPerSample)
{  
  unsigned int j;

  if ( nBytesPerSample == 4 ) {
    memcpy(OutBuf, InBuf, length*sizeof(char));
  }
  else if (nBytesPerSample == sizeof(short))
  {
    union { signed short s; char c[sizeof(short)]; } u;
    int i;
    float fFactor   = (float)(1 << ( nBytesPerSample*8 - 1 ));
    u.s			    = 0;
    for (j=0; j < length / nBytesPerSample; j++)
    {
      float maxVal  =  32767.f;
      float minVal  = -32768.f;
      float tmpVal  = 0.f;

      int n = j * nBytesPerSample;

      tmpVal = InBuf[j] * fFactor;

      if ( tmpVal > maxVal ) {
        tmpVal = maxVal;
#ifdef CLIP_WARNING
        fprintf(stderr,"wavIO warning: sample > maxVal clipped\n");
#endif
      }
      if ( tmpVal < minVal ) {
        tmpVal = minVal;
#ifdef CLIP_WARNING
        fprintf(stderr,"wavIO warning: sample < minVal clipped\n");
#endif
      }

      u.s = (signed short) tmpVal;

      if (IsLittleEndian())
      {
        for (i=0; i< (int)nBytesPerSample; i++)
        {
          OutBuf[n + i] = u.c[i];
        }
      }
      else
      {
        for (i = 0; i < (int)nBytesPerSample; i++)
        {
          OutBuf[n + nBytesPerSample - i - 1] = u.c[i + (sizeof(short) - (sizeof(short)-1))];
        }
      }
    }
  }
  else
  {
    union { signed long s; char c[sizeof(long)]; } u;
    int i;

    /* Calculate scaling factor for 24bit */
    float fFactor   = (float)(1 << ( nBytesPerSample*8 - 1 ));
    u.s			    = 0;
    for (j=0; j < length / nBytesPerSample; j++)
    {
      int maxVal = (int) fFactor - 1;
      int minVal = (int) -fFactor;

      int n = j * nBytesPerSample;

      u.s = (signed long) (InBuf[j] * fFactor);

      if ( u.s > maxVal ) {
        u.s = maxVal;
#ifdef CLIP_WARNING
        fprintf(stderr,"wavIO warning: sample > maxVal clipped\n");
#endif
      }
      if ( u.s < minVal ) {
        u.s = minVal;
#ifdef CLIP_WARNING
        fprintf(stderr,"wavIO warning: sample < minVal clipped\n");
#endif
      }

      if (IsLittleEndian())
      {
        for (i=0;i< (int)nBytesPerSample; i++)
        {
          OutBuf[n + i] = u.c[i];
        }
      }
      else
      {
        for (i = 0; i < (int)nBytesPerSample; i++)
        {
          OutBuf[n + nBytesPerSample - i - 1] = u.c[i + (sizeof(long) - 3)];
        }
      }
    }
  }
}

static __inline int IsLittleEndian (void)
{
  short s = 0x01 ;
  
  return *((char *) &s) ? 1 : 0;
}


static __inline short LittleEndian16 (short v)
{ /* get signed little endian (2-compl.) and returns in native format, signed */
  if (IsLittleEndian ()) return v ;
  
  else return ((v << 8) & 0xFF00) | ((v >> 8) & 0x00FF) ;
}

static unsigned int BigEndian32 (char a, char b, char c, char d)
{
  if (IsLittleEndian ())
  {
    return (unsigned int) d << 24 |
      (unsigned int) c << 16 |
      (unsigned int) b <<  8 |
      (unsigned int) a ;
  }
  else
  {
    return (unsigned int) a << 24 |
      (unsigned int) b << 16 |
      (unsigned int) c <<  8 |
      (unsigned int) d ;
  }
}


#if defined __BIG_ENDIAN__
static short LittleToNativeEndianShort(short x) 
{
  char *t = (char*)(&x);
  char tmp = t[0];
  t[0] = t[1];
  t[1] = tmp;  
  return *((short*)t);
}

static int LittleToNativeEndianLong(int x) 
{
  char *t = (char*)(&x); 
  char tmp = t[0]; 
  t[0] = t[3]; 
  t[3] = tmp;
  tmp  = t[1]; 
  t[1] = t[2];
  t[2] = tmp;
  return *((int*)t); 
}

#elif defined (_M_IX86) || defined (__i386) || defined (__amd64) || defined (__x86_64__) || defined (WIN64) || defined (_WIN64 ) || (defined(__GNUC__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__))
static short LittleToNativeEndianShort(short x) { return x; }
static int LittleToNativeEndianLong(int x) { return x; }

#else
#error "Not sure if we are big or little endian"
#endif

