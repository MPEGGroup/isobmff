/***********************************************************************************
 
 This software module was originally developed by 
 
 Fraunhofer IIS
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. Those intending to use this 
 software module in products are advised that its use may infringe existing patents. 
 ISO/IEC have no liability for use of this software module or modifications thereof. 
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
 standard.
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "wavIO.h"

int main(int argc, char *argv[])
{

  unsigned int wavio_blocklength = 1024;
  int error_init = 0;
  int error_read = 0;
  int error_write = 0;
  int error_close = 0;

  int frameNo = -1;
  unsigned long int NumberSamplesWritten = 0;
  unsigned int isLastFrame = 0;

  char *inpath = NULL;
  char *outpath = NULL;

  WAVIO_HANDLE hWavIO = NULL;

  int moduleDelay = 0;
  unsigned int i = 0, j = 0;
  unsigned int nOutChannels  = 0;
  int bFillLastFrame = 0;

  int fs_out = 48000;
  int byteps_out = 3;

  int nSamplesPerChannelFilled;

  FILE *fIn = NULL;
  FILE *fOut = NULL;

  unsigned int nInChannels = 0;
  unsigned int InSampleRate;
  unsigned long nTotalSamplesPerChannel;
  unsigned long nTotalSamplesWrittenPerChannel;
 
  unsigned int bitdepth = 0;
  unsigned int bytedepth = 0;
  unsigned int InBytedepth = 0;

  float **inBuffer = NULL;
  float **outBuffer = NULL;

  fprintf(stderr, "\n\n");
  fprintf(stderr, "****************************** wavIO ******************************\n");
  fprintf(stderr, "*                                                                 *\n");
  fprintf(stderr, "*                    Build Date: %s                      *\n", __DATE__);
  fprintf(stderr, "*                                                                 *\n");
  fprintf(stderr, "*                    (c) 2013 Fraunhofer IIS                      *\n");
  fprintf(stderr, "*                     All Rights Reserved.                        *\n");
  fprintf(stderr, "*                                                                 *\n");
  fprintf(stderr, "* This software and/or program is protected by copyright law and  *\n");
  fprintf(stderr, "*   international treaties. Any reproduction or distribution of   *\n");
  fprintf(stderr, "* this software and/or program, or any portion of it, may result  *\n");
  fprintf(stderr, "* in severe civil and criminal penalties, and will be prosecuted  *\n");
  fprintf(stderr, "*           to the maximum extent possible under law.             *\n");
  fprintf(stderr, "*                                                                 *\n");
  fprintf(stderr, "*******************************************************************\n\n");

  /* Commandline Parsing */
  for ( i = 1; i < (unsigned int) argc; ++i )
  {
    if (!strcmp(argv[i],"-if"))      /* Optional if -of is used */
    {
      inpath = argv[i+1];
      i++;
    }
    else if (!strcmp(argv[i],"-of"))  /* Optional if -if is used */
    {
      outpath = argv[i+1];
      i++;
    } 
    else if (!strcmp(argv[i],"-d")) /* Optional */
    {
      moduleDelay = atoi(argv[i+1]);
      i++;
    }     
    else if (!strcmp(argv[i],"-c")) /* Optional */
    {
      nOutChannels = atoi(argv[i+1]);
      i++;
    } 
    else if (!strcmp(argv[i],"-fill")) /* Optional */
    {
       bFillLastFrame = 1;
    } 
    else if (!strcmp(argv[i],"-bitdepth")) /* Optional */
    {
      bitdepth = atoi(argv[i+1]);
      /* divide by 8 to get byte depth */
      bytedepth = bitdepth>>3;
      i++;
    } 
    else if (!strcmp(argv[i],"-qmf")) /* Optional */
    {
      bitdepth = 32;
      /* divide by 8 to get byte depth */
      bytedepth = bitdepth>>3;
      i++;
    } 

  }

  if ( (inpath == NULL) && (outpath == NULL) )
    {
      fprintf( stderr, "Invalid arguments, usage:\n");
      fprintf( stderr, "-if <input.wav>  path to input wave file\n");
      fprintf( stderr, "-of <output.wav> path to output wave file\n");
      fprintf( stderr, "-d <delay>       compensate delay of <delay> samples (input numbers >= 0)\n");
      fprintf( stderr, "-c <channels>    number of output channels\n");
      fprintf( stderr, "-fill            fill up last read frame with zeros, if incomplete\n");
      fprintf( stderr, "-bitdepth        specify bit depth of output file (16 or 24 )\n");
      fprintf( stderr, "-qmf             read/write qmf values using a wav container\n");
      return -1;
    }

  /* Open input file */
  if ( inpath )
  {
    fIn = fopen(inpath, "rb");
    if ( fIn != NULL) {
      fprintf(stderr, "Found input file: %s.\n", inpath );
    }
    else {
      fprintf(stderr, "Could not open input file: %s.\n", inpath );
      return -1;
    }
  }

  /* Open output file */
  if ( outpath )
  {
    fOut = fopen(outpath, "wb");
    if ( fOut != NULL) {
      fprintf(stderr, "Write to output file: %s.\n", outpath);
    }
    else {
      fprintf(stderr, "Could not open output file: %s.\n", outpath );
      return -1;
    }
  }


  /* ----------------- INIT WAV IO ----------------- */

  error_init = wavIO_init(&hWavIO,
                          wavio_blocklength,
                          bFillLastFrame,
                          moduleDelay);

  if ( fIn )
  {
  error_init = wavIO_openRead(hWavIO,
                              fIn,
                              &nInChannels,
                              &InSampleRate, 
                              &InBytedepth,
                              &nTotalSamplesPerChannel, 
                              &nSamplesPerChannelFilled);
  }

  if ( fOut && (nOutChannels == 0) ) {
    fprintf(stderr, "\nNo number of output channels specified! Using input format (%i channels).\n", nInChannels);
    nOutChannels = nInChannels;
  }

  if ( fOut && (bytedepth == 0) ) {
    fprintf(stderr, "\nNo bit depth specified! Using input format (%i bits).\n", InBytedepth*8);
    bytedepth = InBytedepth;
  }

  if ( fIn )
    fs_out = InSampleRate;

  error_init = wavIO_openWrite(hWavIO,
                               fOut,
                               nOutChannels,
                               fs_out,
                               bytedepth);

  if ( 0 != error_init )
  {
    fprintf(stderr, "Error during initialization.\n");
    return -1;
  }

  /* alloc local buffers */
  if (nInChannels)
  {
    inBuffer = (float**)calloc(nInChannels,sizeof(float*));
    for (i = 0; i< nInChannels; i++)
      inBuffer[i] = (float*)calloc(wavio_blocklength,sizeof(float));
  }
  
  if (nOutChannels)
  {
    outBuffer = (float**)calloc(nOutChannels,sizeof(float*));
    for (i=0; i< (unsigned int) nOutChannels; i++)
      outBuffer[i] = (float*)calloc(wavio_blocklength,sizeof(float));
  }

  /* ----------------- PROCESSING ----------------- */
  do
  {
    unsigned int samplesReadPerChannel = 0;
    unsigned int samplesToWritePerChannel = 0;
    unsigned int samplesWrittenPerChannel = 0;
    unsigned int nZerosPaddedBeginning = 0;
    unsigned int nZerosPaddedEnd = 0;

    frameNo++;

    /* read frame if input file is available */
    if ( fIn )
    {
      error_read = wavIO_readFrame(hWavIO,inBuffer,&samplesReadPerChannel,&isLastFrame,&nZerosPaddedBeginning,&nZerosPaddedEnd);

      /* processing of h_audioData->inputBuffer[channels][0..BLOCKLENGTH-1] write to h_audioData->outputBuffer[channels][0..BLOCKLENGTH-1] */
      /* example: here we simply copy input to output */
      for (i = 0; i < max(samplesReadPerChannel, wavio_blocklength); i++)
      {
        for (j = 0; j < nOutChannels; j++)
        {
          outBuffer[j][i] = inBuffer[j][i];
        }
      }

      /* Add up possible delay and actually read samples */
      samplesToWritePerChannel = nZerosPaddedBeginning + samplesReadPerChannel + nZerosPaddedEnd;
    }

    if ( fOut )
    {
      /* create some dummy data to write */
      if ( !fIn )
      {
        static int lastFrameCounter = 0;
        for (i = 0; i < wavio_blocklength; i++)
        {
          for (j = 0; j < nOutChannels; j++)
          {
            outBuffer[j][i] = 3245678.f;
          }

        }
        lastFrameCounter++;

        samplesToWritePerChannel = wavio_blocklength;

        /* Break after 100 frames */

        if (lastFrameCounter == 100 )
          isLastFrame = 1;

      }

      /* write frame */
      error_write = wavIO_writeFrame(hWavIO,outBuffer,samplesToWritePerChannel,&samplesWrittenPerChannel);
      NumberSamplesWritten += samplesWrittenPerChannel;
    }
  }
  while (! isLastFrame);

  /* ----------------- EXIT WAV IO ----------------- */
  error_close = wavIO_updateWavHeader(hWavIO, &nTotalSamplesWrittenPerChannel);

  assert(nTotalSamplesWrittenPerChannel==NumberSamplesWritten);

  if ( error_close == 0 ) {
    fprintf(stderr, "\nOutput file %s is written.\n", outpath);
  }

  error_close = wavIO_close(hWavIO);

  /* free local buffers */
  if (nInChannels)
  {
    for (i=0; i< nInChannels; i++) {
      free(inBuffer[i]);
    }

    free(inBuffer);
  }

  if (nOutChannels)
  {
    for (i=0; i< nOutChannels; i++) {
      free(outBuffer[i]);
    }

    free(outBuffer);
  }

  return 0;

}
