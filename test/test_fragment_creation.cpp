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
derivative works. Copyright (c) 1999.
*/

#include <catch.hpp>
#include <ISOMovies.h>
#include <string>
#include <vector>
#include <algorithm>
#include "libmd5/MD5.h"

// TODO: move code from playground to this test after finishing all features

extern "C" {
MP4_EXTERN(MP4Err)
ISONewHEVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                            u32 dataReferenceIndex, u32 length_size,
                            MP4Handle first_sps, MP4Handle first_pps,
                            MP4Handle first_spsext);
}

// TODO: after reading the sample data check the md5 to ensure that samples are correct
libmd5::MD5 g_md5;


/* AAC encoding of a 234.375 Hz stereo tone */
u8 aacDecoderSpecificInfo[] = {0x11, 0x80, 0x08, 0xc4, 0x00, 0x00,
                               0x20, 0x00, 0x00, 0x00, 0x00, 0x00};
u8 aacAccessUnit[] = {0x21, 0x09, 0x49, 0x00, 0x00, 0x00, 0x00, 0x29, 0xc0, 0x56, 0x10, 0xf8, 0xc3,
                      0x8e, 0x9f, 0x84, 0x3b, 0xe0, 0x7c, 0xc3, 0x80, 0x00, 0x00, 0x00, 0x00};

u8 VPS[] = {0x40, 0x01, 0x0C, 0x01, 0xFF, 0xFF, 0x04, 0x08, 0x00, 0x00, 0x03, 0x00,
            0x9F, 0xA8, 0x00, 0x00, 0x03, 0x00, 0x00, 0x1E, 0xBA, 0x02, 0x40};

u8 SPS[] = {0x42, 0x01, 0x01, 0x04, 0x08, 0x00, 0x00, 0x03, 0x00, 0x9F, 0xA8, 0x00, 0x00,
            0x03, 0x00, 0x00, 0x1E, 0xA0, 0x20, 0x83, 0x16, 0x5B, 0xAB, 0x93, 0x2B, 0x9A,
            0x02, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x03, 0x00, 0x32, 0x10};

u8 PPS[] = {0x44, 0x01, 0xC1, 0x73, 0xC0, 0x89};

u8 auRed[] = {0x00, 0x00, 0x00, 0x16, 0x28, 0x01, 0xAF, 0x78, 0xF7, 0x04, 0x03, 0xFF, 0xDB,
              0xA3, 0xFF, 0xED, 0x27, 0xD2, 0xF6, 0xC3, 0x94, 0x40, 0x83, 0xC0, 0x00, 0x78};

u8 auBlue[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3F,
               0x9C, 0x43, 0xFF, 0xFA, 0x87, 0x32, 0xAF, 0xFC, 0x5D, 0xFF,
               0xFF, 0xAE, 0x1D, 0xB9, 0xA2, 0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auGreen[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3F,
                0x9C, 0x43, 0xFF, 0xF5, 0x9F, 0x1F, 0xFF, 0xD8, 0x3B, 0xFF,
                0xFD, 0xF0, 0xF5, 0xB9, 0xA2, 0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auYellow[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xA0, 0x3F,
                 0x9C, 0x43, 0x3C, 0xFA, 0x51, 0x1D, 0xFF, 0xFC, 0x5D, 0xFE,
                 0xCB, 0xAE, 0x1D, 0xB9, 0xA2, 0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auWhite[] = {0x00, 0x00, 0x00, 0x13, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3C, 0x64, 0x00,
                0xE7, 0x9F, 0x6C, 0x07, 0x79, 0x0D, 0x1B, 0xFD, 0x7D, 0x7C, 0x87};

u8 auBlack[] = {0x00, 0x00, 0x00, 0x13, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3C, 0x64, 0x00,
                0xFF, 0xFF, 0x72, 0xCA, 0x19, 0x0D, 0x1B, 0xFD, 0x7D, 0x7C, 0x87};

u32 frameCount          = 1;
u32 samplesizefieldsize = 32;
u32 samplesperchunk     = 24;

MP4Err addMySamples(MP4Track theTrack, MP4Media theMedia, u32 do_init);
MP4Err getDecoderSpecificInfo(MP4Handle sampleH);
MP4Err getNextAudioFrame(MP4Handle sampleH);

TEST_CASE("Fragment creation")
{
  MP4Err err;
  MP4Movie moov;
  MP4Track trak;
  MP4Media media;
  u8 OD_profileAndLevel;
  u8 scene_profileAndLevel;
  u8 audio_profileAndLevel;
  u8 visual_profileAndLevel;
  u8 graphics_profileAndLevel;
  u32 initialObjectDescriptorID;
  u32 timeScale;
  u64 mediaDuration;
  MP4Handle rap_desc;
  u32 rap_desc_index;
  u32 frameCounter;

  MP4NewHandle(1, &rap_desc); /* Allocate one byte for rap */
  err                       = MP4NoErr;
  initialObjectDescriptorID = 1;
  OD_profileAndLevel        = 0xff;  /* none required */
  scene_profileAndLevel     = 0xff;  /* none required */
  audio_profileAndLevel     = 0x01;  /* main profile L1 */
  visual_profileAndLevel    = 0xff;  /* none required */
  graphics_profileAndLevel  = 0xff;  /* none required */
  timeScale                 = 48000; /* sampling frequency */
  err = MP4NewMovie(&moov, initialObjectDescriptorID, OD_profileAndLevel, scene_profileAndLevel,
                    audio_profileAndLevel, visual_profileAndLevel, graphics_profileAndLevel);
  CHECK(err == MP4NoErr);

  err = ISOSetMovieTimeScale(moov, 48000);
  CHECK(err == MP4NoErr);

  err = MP4NewMovieTrack(moov, MP4NewTrackIsAudio, &trak);
  CHECK(err == MP4NoErr);

  if(initialObjectDescriptorID != 0)
  {
    err = MP4AddTrackToMovieIOD(trak);
    CHECK(err == MP4NoErr);
  }

  err = MP4NewTrackMedia(trak, &media, MP4AudioHandlerType, timeScale, NULL);
  CHECK(err == MP4NoErr);
  err = ISOSetSampleSizeField(media, samplesizefieldsize);
  CHECK(err == MP4NoErr);
  err = MP4BeginMediaEdits(media);
  CHECK(err == MP4NoErr);

  err = ISOSetTrackFragmentDefaults(trak, 1024, sizeof(aacAccessUnit), 1, 0);
  CHECK(err == MP4NoErr);

  err = addMySamples(trak, media, 1);

  err = MP4EndMediaEdits(media);
  CHECK(err == MP4NoErr);

  err = MP4GetMediaDuration(media, &mediaDuration);
  CHECK(err == MP4NoErr);

  err = MP4InsertMediaIntoTrack(trak, 0, 0, mediaDuration, 1);
  CHECK(err == MP4NoErr);

  err = ISOStartMovieFragment(moov);
  CHECK(err == MP4NoErr);
  frameCount = 1;
  err        = addMySamples(trak, media, 0);
  CHECK(err == MP4NoErr);

  ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), rap_desc, &rap_desc_index);
  for(frameCounter = 1; frameCounter < samplesperchunk; frameCounter++)
  {
    /* Mark RAP frames (CRA/BLA/IDR/IRAP) to the group */
    if(frameCounter % 4 == 0)
    {
      ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), rap_desc_index,
                           frameCounter - 1, 1);
    }
  }

  err = MP4WriteMovieToFile(moov, "test_fragments.mp4");
  CHECK(err == MP4NoErr);
}

MP4Err addMySamples(MP4Track trak, MP4Media media, u32 do_init)
{
  MP4Err err;
  MP4Handle sampleEntryH;
  MP4Handle sampleDataH;
  MP4Handle sampleDurationH;
  MP4Handle sampleSizeH;
  MP4Handle decoderSpecificInfoH;
  u32 objectTypeIndication;
  u32 streamType;
  u32 decoderBufferSize;
  u32 maxBitrate;
  u32 avgBitrate;
  u32 done       = 0;
  u32 fileoffset = 0;
  u32 i;
  MP4Handle pad_bits, depsH;

  err = MP4NoErr;
  if(do_init)
  {
    err = MP4SetMediaLanguage(media, (char*)"und");
    if(err) goto bail;

    objectTypeIndication = 0x40;     /* mpeg-4 audio */
    streamType           = 0x05;     /* audio stream */
    decoderBufferSize    = 2 * 6144; /* stereo */
    maxBitrate           = 128000;
    avgBitrate           = 128000;
    err                  = MP4NewHandle(0, &decoderSpecificInfoH);
    if(err) goto bail;
    err = getDecoderSpecificInfo(decoderSpecificInfoH);
    if(err) goto bail;

    err = MP4NewHandle(0, &sampleEntryH);
    if(err) goto bail;
    err = MP4NewSampleDescription(trak, sampleEntryH, 1, objectTypeIndication, streamType,
                                  decoderBufferSize, maxBitrate, avgBitrate, decoderSpecificInfoH);
    if(err) goto bail;
  }
  else
    sampleEntryH = NULL;

  err = MP4NewHandle(samplesperchunk * sizeof(u32), &sampleDurationH);
  if(err) goto bail;
  err = MP4NewHandle(0, &sampleDataH);
  if(err) goto bail;
  err = MP4NewHandle(samplesperchunk * sizeof(u32), &sampleSizeH);
  if(err) goto bail;
  err = MP4NewHandle(1, &pad_bits);
  if(err) goto bail;
  err = MP4NewHandle(samplesperchunk, &depsH);
  if(err) goto bail;
  *((u8 *)(*pad_bits)) = 0;
  for(i = 0; i < samplesperchunk; i++)
    ((u8 *)*depsH)[i] = does_not_depend_on | is_not_depended_on | has_no_redundancy;

  while(!done)
  {
    u32 frames, sizesofar, i, chunksize, samplesize;

    frames    = 0;
    err       = MP4SetHandleOffset(sampleDataH, 0);
    chunksize = 0;

    for(i = 0; i < samplesperchunk; i++)
    {
      err = getNextAudioFrame(sampleDataH);
      if(err)
      {
        if(err == MP4EOF)
        {
          err  = 0;
          done = 1;
          break;
        }
      }
      else
      {
        frames++;
        err = MP4GetHandleSize(sampleDataH, &samplesize);
        if(err) goto bail;
        chunksize += samplesize;
        err = MP4SetHandleOffset(sampleDataH, chunksize);
        if(err) goto bail;
        ((u32 *)*sampleDurationH)[i] = 1024; /* AAC block length */
        ((u32 *)*sampleSizeH)[i]     = samplesize;
      }
    }
    if(frames > 0)
    {
      err = MP4SetHandleOffset(sampleDataH, 0);
      err = MP4AddMediaSamples(media, sampleDataH, frames, sampleDurationH, sampleSizeH,
                               sampleEntryH, NULL, NULL);
      if(err) goto bail;

      if(sampleEntryH)
      {
        err = MP4DisposeHandle(sampleEntryH);
        if(err) goto bail;
        sampleEntryH = NULL;
      }
    }
  }
bail:
  return err;
}

MP4Err getDecoderSpecificInfo(MP4Handle decoderSpecificInfoH)
{
  MP4Err err;
  u8 *p;
  u32 i, frameLength;

  err = MP4NoErr;
  if((frameLength = sizeof(aacDecoderSpecificInfo)) >= 128)
  {
    err = MP4BadParamErr;
    goto bail;
  }
  err = MP4SetHandleSize(decoderSpecificInfoH, frameLength + 2);
  if(err) goto bail;
  p    = (u8 *)*decoderSpecificInfoH;
  *p++ = 0x05;        /* DecSpecificInfoTag, 14496-1, Sec 8.2.2.2, Table 1*/
  *p++ = frameLength; /* sizeOfInstance, 14496-1, Sec 12.3.3 */
  for(i = 0; i < frameLength; i++)
    *p++ = aacDecoderSpecificInfo[i];
bail:
  return err;
}

MP4Err getNextAudioFrame(MP4Handle sampleH)
{
  MP4Err err;
  u8 *p;
  u32 i, frameLength;

  err = MP4NoErr;
  if(frameCount++ > 48) /* one second of sine wave signal */
    return MP4EOF;
  frameLength = sizeof(aacAccessUnit);
  err         = MP4SetHandleSize(sampleH, frameLength);
  if(err) goto bail;
  p = (u8 *)*sampleH;
  for(i = 0; i < frameLength; i++)
    *p++ = aacAccessUnit[i];
bail:
  return err;
}