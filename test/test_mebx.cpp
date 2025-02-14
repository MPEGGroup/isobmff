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
  std::string strPattern       = "NIbFD";
  u32 repeatPattern            = 6;
  std::string strMebxMe4cFile  = "test_mebx_me4c.mp4";
  std::string strUnMebxFile    = "test_unmebx.mp4";
  std::string strReMebxFile    = "test_remebx.mp4";

  char localeEN[] = "en-US";
  char localeDE[] = "de-DE";

  // SECTION("Open")
  // {
  //   MP4Movie moov;
  //   err = ISOOpenMovieFile(
  //     &moov,
  //     "/Users/podborski/Library/CloudStorage/Box-Box/Dolby/files/iphone13_DV_cinematic.MOV",
  //     MP4OpenMovieDebug);
  //   CHECK(err == MP4NoErr);
  // }

  SECTION("Create")
  {
    MP4Movie moov;
    MP4Track trakV;
    MP4Track trakM;
    MP4Media mediaV;
    MP4Media mediaM;

    u32 local_key_id;
    u32 local_key_id_red      = MP4_FOUR_CHAR_CODE('r', 'e', 'd', 'd');
    u32 local_key_id_blu      = MP4_FOUR_CHAR_CODE('b', 'l', 'u', 'e');
    u32 local_key_id_ylw      = MP4_FOUR_CHAR_CODE('y', 'l', 'o', 'w');
    u32 local_key_id_wht      = MP4_FOUR_CHAR_CODE('w', 'h', 't', 'e');
    u32 local_key_id_blk      = MP4_FOUR_CHAR_CODE('b', 'l', 'c', 'k');

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
    err = ISONewMebxSampleDescription(&mebx, 1);
    CHECK(err == MP4NoErr);

    MP4Handle redSetupH;
    err = createHandleFromString(&redSetupH, "Config RED");
    err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_red, &local_key_id, MP4KeyNamespace_me4c, 0, 0, redSetupH);
    CHECK(err == MP4NoErr);
    CHECK(local_key_id_red == local_key_id);

    // add the same key again. It should be added with a different local_key_id
    err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_red, &local_key_id, MP4KeyNamespace_me4c, 0, 0, redSetupH);
    CHECK(err == MP4NoErr);
    CHECK(local_key_id_red != local_key_id);

    // add one more with the different namespace
    MP4Handle revAddress;
    err = createHandleFromString(&revAddress, "com.foo.bar.zoo");
    err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_red, &local_key_id, QTKeyNamespace_mdta, revAddress, 0, 0);
    CHECK(err == MP4NoErr);
    CHECK(local_key_id_red != local_key_id);

    MP4Handle bluKeyH, bluSetupH;
    err = createHandleFromString(&bluKeyH, "blue");
    err = createHandleFromString(&bluSetupH, "Config BLUE");
    err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_blu, &local_key_id, MP4KeyNamespace_me4c, bluKeyH, 0, bluSetupH);
    CHECK(err == MP4NoErr);
    CHECK(local_key_id_blu == local_key_id);

    MP4Handle ylwKeyH, ylwSetupH;
    err = createHandleFromString(&ylwKeyH, "ylow");
    err = createHandleFromString(&ylwSetupH, "Config YELLOW");
    err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_ylw, &local_key_id, MP4KeyNamespace_me4c, ylwKeyH, 0, ylwSetupH);
    CHECK(err == MP4NoErr);
    CHECK(local_key_id_ylw == local_key_id);

    MP4Handle whtKeyH, whtSetupH;
    err = createHandleFromString(&whtKeyH, "whte");
    err = createHandleFromString(&whtSetupH, "Config WHITE");
    err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_wht, &local_key_id, MP4KeyNamespace_me4c, whtKeyH, 0, whtSetupH);
    CHECK(err == MP4NoErr);
    CHECK(local_key_id_wht == local_key_id);

    MP4Handle blkKeyH, blkSetupH;
    err = createHandleFromString(&blkKeyH, "blck");
    err = createHandleFromString(&blkSetupH, "Config BLACK");
    err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_blk, &local_key_id, MP4KeyNamespace_me4c, blkKeyH, 0, blkSetupH);
    CHECK(err == MP4NoErr);
    CHECK(local_key_id_blk == local_key_id);
    // MP4Handle lblKeyH, lblEnSetupH;
    // err = createHandleFromString(&lblKeyH, "labl");
    // err = createHandleFromString(&lblEnSetupH, "Config lable ENGLISH");
    // err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_lable_en, MP4KeyNamespace_me4c, lblKeyH, localeEN, lblEnSetupH);
    // CHECK(err == MP4NoErr);
    // MP4Handle lblDeSetupH;
    // err = createHandleFromString(&lblDeSetupH, "Config lable GERMAN");
    // err = ISOAddMebxMetadataToSampleEntry(mebx, local_key_id_lable_de, MP4KeyNamespace_me4c, lblKeyH, localeDE, lblDeSetupH);
    // CHECK(err == MP4NoErr);

    // add mebx sample entry to track's media
    err = ISOGetMebxHandle(mebx, sampleEntryMH);
    CHECK(err == MP4NoErr);
    err = addMebxSamples(mediaM, "", 0, sampleEntryMH);
    CHECK(err == MP4NoErr);

    // add samples using local key ids and pattern
    err = addMebxSamples(mediaM, strPattern, repeatPattern, 0, local_key_id_red, local_key_id_blu,
                         local_key_id_ylw, local_key_id_wht, local_key_id_blk, 0);
    CHECK(err == MP4NoErr);

    // write file
    err = MP4EndMediaEdits(mediaV);
    err = MP4EndMediaEdits(mediaM);
    err = MP4WriteMovieToFile(moov, strMebxMe4cFile.c_str());
    CHECK(err == MP4NoErr);
  }

  SECTION("Read")
  {
    MP4Movie moov;
    err = MP4OpenMovieFile(&moov, strMebxMe4cFile.c_str(), MP4OpenMovieNormal);
    REQUIRE(err == MP4NoErr);

    MP4Track trak;
    MP4Media media;
    MP4TrackReader reader;
    err = MP4GetMovieIndTrack(moov, 2, &trak);
    err = MP4GetTrackMedia(trak, &media);
    err = MP4CreateTrackReader(trak, &reader);
    REQUIRE(err == MP4NoErr);

    MP4Handle sampleEntryH;
    MP4NewHandle(1, &sampleEntryH);
    err = MP4TrackReaderGetCurrentSampleDescription(reader, sampleEntryH);
    CHECK(err == MP4NoErr);

    u32 key_cnt = 0;
    err = ISOGetMebxMetadataCount(sampleEntryH, &key_cnt);
    CHECK(err == MP4NoErr);
    CHECK(7 == key_cnt);

    for(u32 i = 0; i < key_cnt; i++)
    {
      u32 local_key_id, key_namespace;
      MP4Handle key_value, setupInfo;
      MP4NewHandle(0, &key_value);
      MP4NewHandle(0, &setupInfo);
      char *locale_string = nullptr;
      err = ISOGetMebxMetadataConfig(sampleEntryH, i, &local_key_id, &key_namespace, key_value, &locale_string, setupInfo);
      CHECK(err == MP4NoErr);
      switch (i)
      {
      case 0:
      {
        MP4Handle redSetupH, key_val;
        err = createHandleFromString(&redSetupH, "Config RED");
        err = createHandleFromString(&key_val, "redd");
        CHECK(local_key_id == MP4_FOUR_CHAR_CODE('r', 'e', 'd', 'd'));
        CHECK(key_namespace == MP4KeyNamespace_me4c);
        err = compareData(key_value, key_val);
        CHECK(err == MP4NoErr);
        CHECK(nullptr == locale_string);
        err = compareData(redSetupH, setupInfo);
        CHECK(err == MP4NoErr);
        break;
      }
      case 3:
      {
        MP4Handle blueSetupH, key_val;
        err = createHandleFromString(&blueSetupH, "Config BLUE");
        err = createHandleFromString(&key_val, "blue");
        CHECK(local_key_id == MP4_FOUR_CHAR_CODE('b', 'l', 'u', 'e'));
        CHECK(key_namespace == MP4KeyNamespace_me4c);
        err = compareData(key_value, key_val);
        CHECK(err == MP4NoErr);
        CHECK(nullptr == locale_string);
        err = compareData(blueSetupH, setupInfo);
        CHECK(err == MP4NoErr);
        break;
      }
      
      default:
        break;
      }
      MP4DisposeHandle(key_value);
      MP4DisposeHandle(setupInfo);
    }


    err = MP4SetMebxTrackReader(reader, MP4_FOUR_CHAR_CODE('r', 'e', 'd', 'd'));
    CHECK(err == MP4NoErr);
    
    u32 n = 0;
    do
    {
      MP4Handle outAccessUnit;
      MP4NewHandle(0, &outAccessUnit);
      u32 outSize, outSampleFlags;
      s32 outCTS, outDTS;
      err = MP4TrackReaderGetNextAccessUnit(reader, outAccessUnit, &outSize, &outSampleFlags, &outCTS, &outDTS);

      if(!err)
      {
        checkRedMebxSamples(strPattern, repeatPattern, outAccessUnit, n);
        n++;
      } 
    } while (err == MP4NoErr);
    CHECK(strPattern.size()*repeatPattern == n);
    
  }

}
