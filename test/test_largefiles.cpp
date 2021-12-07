/**
 * @file test_largefiles.cpp
 * @author Dimitri Podborski
 * @brief Testing large files
 * @version 0.1
 * @date 2021-12-06
 * 
 * @copyright This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4.
 * Those intending to use this software module in hardware or software products are advised that its
 * use may infringe existing patents. The original developer of this software module and his/her
 * company, the subsequent editors and their companies, and ISO/IEC have no liability for use of
 * this software module or modifications thereof in an implementation. Copyright is not released for
 * non MPEG-4 conforming products. Apple Computer, Inc. retains full right to use the code for its
 * own purpose, assign or donate the code to a third party and to inhibit third parties from using
 * the code for non MPEG-4 conforming products. This copyright notice must be included in all copies
 * or derivative works. Copyright (c) 1999.
 *
 */

#include <catch.hpp>
#include <ISOMovies.h>
#include <MP4Atoms.h>
#include "test_helpers.h"

TEST_CASE("Check large files/boxes")
{
  MP4Err err;
  SECTION("Check writing large mdat box (not fragmented)")
  {
    MP4Movie moov;
    MP4Track trak;
    MP4Media media;
    u32 sampleSize = 104857600; // 100 MB
    u32 numSamples = 41;

    MP4Handle sampleDataH, durationH, sizeH;
    MP4NewHandle(sizeof(u32), &durationH);
    MP4NewHandle(sizeof(u32), &sizeH);
    MP4NewHandle(sampleSize * sizeof(u8), &sampleDataH);
    *((u32 *)*durationH) = 90000 / 30;
    *((u32 *)*sizeH) = sampleSize;

    MP4Handle spsHandle, ppsHandle, vpsHandle, sampleEntryH;
    err = MP4NewHandle(sizeof(HEVC::SPS), &spsHandle);
    std::memcpy((*spsHandle), HEVC::SPS, sizeof(HEVC::SPS));
    err = MP4NewHandle(sizeof(HEVC::PPS), &ppsHandle);
    std::memcpy((*ppsHandle), HEVC::PPS, sizeof(HEVC::PPS));
    err = MP4NewHandle(sizeof(HEVC::VPS), &vpsHandle);
    std::memcpy((*vpsHandle), HEVC::VPS, sizeof(HEVC::VPS));
    err = MP4NewHandle(0, &sampleEntryH);

    MP4NewMovie(&moov, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    MP4NewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    MP4NewTrackMedia(trak, &media, ISOVisualHandlerType, 90000, NULL);
    err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 1, spsHandle, ppsHandle, vpsHandle);
    REQUIRE(err == ISONoErr);
    
    err = addHEVCSamples(media, "r", 0, sampleEntryH);

    for(u32 i = 0; i < numSamples; i++)
    {
      err = MP4AddMediaSamples(media, sampleDataH, 1, durationH, sizeH, 0, 0, 0);
      if(err != MP4NoErr)
      {
        CHECK(err == MP4NoErr);
        break;
      }
    }
    
    err = MP4WriteMovieToFile(moov, "large_mdat.mp4");
    CHECK(err == ISONoErr);
  }

  SECTION("Check reading of the large mdat from the previous test")
  {
    MP4Movie moov;
    MP4Track trak;
    MP4Media media;

    err = ISOOpenMovieFile(&moov, "large_mdat.mp4", MP4OpenMovieNormal);
    CHECK(err == ISONoErr);

    err = MP4GetMovieTrack(moov, 1, &trak);
    CHECK(err == ISONoErr);
    err = MP4GetTrackMedia(trak, &media);

    u32 sampleCnt = 0;
    err = MP4GetMediaSampleCount(media, &sampleCnt);
    CHECK(err == MP4NoErr);
    CHECK(sampleCnt == 41);
  }
}
