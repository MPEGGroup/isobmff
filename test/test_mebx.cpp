/**
 * @file test_mebx.cpp
 * @author Dimitri Podborski
 * @brief Multiplexed Metadata testing
 * @version 0.1
 * @date 2022-02-07
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

TEST_CASE("mebx")
{
  MP4Err err;
  std::string strPattern = "RUbFD";
  u32 repeatPattern = 6;
  std::string strMebxMe4cFile = "test_mebx_me4c.mp4";
  std::string strMebxMixedFile = "test_mebx_mixed.mp4";
  std::string strUnMebxFile = "test_unmebx.mp4";
  std::string strReMebxFile = "test_remebx.mp4";

  char localeEN[] = "en-US";
  char localeDE[] = "de-DE";

  SECTION("Open")
  {
    MP4Movie moov;
    err = ISOOpenMovieFile(&moov, "/Users/podborski/Library/CloudStorage/Box-Box/Dolby/files/iphone13_DV_cinematic.MOV", MP4OpenMovieDebug);
    CHECK(err == MP4NoErr);
  }

  SECTION("Create")
  {
    MP4Movie moov;
    MP4Track trakV;
    MP4Track trakM;
    MP4Media mediaV;
    MP4Media mediaM;
    
    MP4Handle spsHandle, ppsHandle, vpsHandle, sampleEntryVH, sampleEntryMH;
    err = MP4NewHandle(sizeof(HEVC::SPS), &spsHandle);
    std::memcpy((*spsHandle), HEVC::SPS, sizeof(HEVC::SPS));
    err = MP4NewHandle(sizeof(HEVC::PPS), &ppsHandle);
    std::memcpy((*ppsHandle), HEVC::PPS, sizeof(HEVC::PPS));
    err = MP4NewHandle(sizeof(HEVC::VPS), &vpsHandle);
    std::memcpy((*vpsHandle), HEVC::VPS, sizeof(HEVC::VPS));
    err = MP4NewHandle(0, &sampleEntryVH);
    err = MP4NewHandle(0, &sampleEntryMH);

    err = MP4NewMovie(&moov, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);

    // create video track
    err = MP4NewMovieTrack(moov, MP4NewTrackIsVisual, &trakV);
    err = MP4NewTrackMedia(trakV, &mediaV, MP4VisualHandlerType, TIMESCALE, NULL);
    err = ISONewHEVCSampleDescription(trakV, sampleEntryVH, 1, 1, spsHandle, ppsHandle, vpsHandle);
    err = addHEVCSamples(mediaV, "", 0, sampleEntryVH);
    err = addHEVCSamples(mediaV, strPattern, repeatPattern);
    REQUIRE(err == MP4NoErr);

    // create mebx track
    err = MP4NewMovieTrack(moov, MP4NewTrackIsMebx, &trakM);
    CHECK(err == MP4NoErr);
    err = MP4NewTrackMedia(trakM, &mediaM, MP4MetaHandlerType, TIMESCALE, NULL);
    CHECK(err == MP4NoErr);
    err = MP4AddTrackReference(trakM, trakV, MP4DescTrackReferenceType, 0);
    CHECK(err == MP4NoErr);

    // create mebx sample entry
    MP4BoxedMetadataSampleEntryPtr mebx;
    MP4Handle redKeyH, redSetupH;
    u32 local_key_id_red = 0;
    err = createHandleFromString(&redKeyH, "redd");
    err = createHandleFromString(&redSetupH, "Config RED");
    err = ISONewMebxSampleDescription(&mebx, 1, MP4KeyNamespace_me4c, redKeyH, 0, redSetupH, &local_key_id_red);
    CHECK(err == MP4NoErr);

    // add other keys to mebx sample entry


    // add mebx sample entry to track's media
    err = ISOGetMebxHandle(mebx, sampleEntryMH);
    CHECK(err == MP4NoErr);
    err = addHEVCSamples(mediaM, "", 0, sampleEntryMH);
    CHECK(err == MP4NoErr);

    // write file
    err = MP4EndMediaEdits(mediaV);
    err = MP4EndMediaEdits(mediaM);
    err = MP4WriteMovieToFile(moov, strMebxMe4cFile.c_str());
    CHECK(err == MP4NoErr);
  }

  SECTION("Un mebx")
  {
    MP4Movie moov;
    err = MP4OpenMovieFile(&moov, strMebxMe4cFile.c_str(), MP4OpenMovieDebug);
    REQUIRE(err == MP4NoErr);
    // TODO: check getters
    // call a function to unmebx mebx track into multiple timed meta tracks
  }

  SECTION("Re mebx")
  {
    // TODO: convert multiple metadata tracks back to a single mebx track
    // compare with 
  }
}
