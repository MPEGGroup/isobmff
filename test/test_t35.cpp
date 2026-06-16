/**
 * @file test_t35.cpp
 * @author Dimitri Podborski
 * @brief Perform checks on T.35
 * @version 0.1
 * @date 2025-04-18
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

//  fix
// mdat size correction
// 5796 - 85 = 5711 (0x164F)

//  first sample size correction
//  1839 - 85 = 1754
//  0x07 0x2F -> 0x06 0xDA

#include <catch.hpp>
#include "test_helpers.h"
#include "testdataPath.h"
#include <MP4Atoms.h>

const std::string strDataPath = TESTDATA_PATH;
const std::string strTestFile = strDataPath + "/isobmff/hvc1_hdr10plus_original.mp4";

/**
 * @brief Starting point for this testing case
 *
 */
TEST_CASE("T35")
{
  std::string strT35Default = "test_samplegroups_t35_defaultHDR10p.mp4";

  MP4Err err;

  MP4Handle it35_prefix;
  MP4NewHandle(5, &it35_prefix);
  (*it35_prefix)[0] = 0xB5;
  (*it35_prefix)[1] = 0x00;
  (*it35_prefix)[2] = 0x3C;
  (*it35_prefix)[3] = 0x00;
  (*it35_prefix)[4] = 0x01;

  // TODO:  implement a command line tool that will take an HEVC mp4 with T.35 SEIs in samples
  //        then it will parse all these T.35, and try to move them into sample groups to save space.
  // SECTION("TBD")
  // {
  //   MP4Err err;
  //   MP4Movie moov;
  //   MP4Track trak;
  //   MP4Media media;

  //   err = MP4OpenMovieFile(&moov, strTestFile.c_str(), MP4OpenMovieDebug);
  //   err = MP4GetMovieIndTrack(moov, 1, &trak);
  //   err = MP4GetTrackMedia(trak, &media);

  //   u32 codecType = 0;
  //   u32 nalUnitLength = 0;
  //   err = MP4GetMovieIndTrackSampleEntryType(moov, 1, &codecType);
  //   err = MP4GetMovieIndTrackNALUnitLength(moov, 1, &nalUnitLength);
  //   REQUIRE(codecType == ISOHEVCSampleEntryAtomType);
  //   CHECK(nalUnitLength == 4);
    
  //   u32 sampleCnt = 0;
  //   err = MP4GetMediaSampleCount(media, &sampleCnt);
  //   CHECK(30 == sampleCnt);

  //   // TBD iterate through samples and look through T.35 SEI NAL Units
  //   // we need to build up a table that will contain
  //   // T.35 data and the number of sample the same data was detected in. For example data_blob1 in sample 1,2,5. data_blob2 in samples 3,4,5,6 etc.

  // }

  SECTION("Check creation of it35 default sample group using ISOAddT35GroupDescription")
  {
    MP4Movie moov;
    MP4Media media;
    MP4Track trak;

    u32 lengthSize = 4;
    u32 temp       = 0;

    MP4Handle spsHandle, ppsHandle, vpsHandle, sampleEntryH;
    err = MP4NewHandle(sizeof(HEVC::SPS), &spsHandle);
    std::memcpy((*spsHandle), HEVC::SPS, sizeof(HEVC::SPS));
    err = MP4NewHandle(sizeof(HEVC::PPS), &ppsHandle);
    std::memcpy((*ppsHandle), HEVC::PPS, sizeof(HEVC::PPS));
    err = MP4NewHandle(sizeof(HEVC::VPS), &vpsHandle);
    std::memcpy((*vpsHandle), HEVC::VPS, sizeof(HEVC::VPS));
    err = MP4NewHandle(0, &sampleEntryH);

    err = MP4NewMovie(&moov, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    REQUIRE(err == MP4NoErr);

    err = MP4NewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    REQUIRE(err == MP4NoErr);
    err = MP4AddTrackToMovieIOD(trak);
    CHECK(err == MP4NoErr);
    err = MP4NewTrackMedia(trak, &media, MP4VisualHandlerType, TIMESCALE, NULL);
    REQUIRE(err == MP4NoErr);

    err = MP4BeginMediaEdits(media);
    err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, lengthSize, spsHandle, ppsHandle, vpsHandle);
    REQUIRE(err == MP4NoErr);

    // check getter
    err = ISOGetGroupDescriptionEntryCount(media, MP4T35SampleGroupEntry, &temp);
    CHECK(err == MP4NotFoundErr);
    CHECK(temp == 0);

    // just add sample entry, call addHEVCSamples with sample count = 0
    err = addHEVCSamples(media, "", 0, sampleEntryH, lengthSize, true);
    CHECK(err == MP4NoErr);
    err = MP4EndMediaEdits(media);
    CHECK(err == MP4NoErr);
    // add samples
    err = addHEVCSamples(media, "rb", 3, nullptr, lengthSize, true);
    CHECK(err == MP4NoErr);
    
    // Add T.35 sample group description header. Default sample group (all samples have this header)
    err = ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
    CHECK(err == MP4NoErr);
    err = ISOAddT35GroupDescription(media, it35_prefix, 0, &temp);
    CHECK(err == MP4NoErr);
    err = ISOGetGroupDescriptionEntryCount(media, MP4T35SampleGroupEntry, &temp);
    CHECK(temp == 1);

    err = MP4WriteMovieToFile(moov, strT35Default.c_str());
    CHECK(err == MP4NoErr);
  }

  // TODO: Implement default_group_description_index support in getSampleGroupSampleNumbers
  // before re-enabling this test. Currently, ISOGetSampleGroupSampleNumbers only returns
  // samples with explicit sample-to-group mappings (sbgp atom), but does not handle
  // default group assignments via default_group_description_index.
  // See SampleTableAtom.c:742 for the incomplete implementation.
  /*
  SECTION("Check default it35 sample group")
  {
    MP4Movie moov;
    MP4Track trak;
    MP4Media media;

    u32 it35_sg_cnt = 0;
    u32 *sample_numbers;
    u32 sample_cnt = 0;

    err = MP4OpenMovieFile(&moov, strT35Default.c_str(), MP4OpenMovieDebug);
    err = MP4GetMovieIndTrack(moov, 1, &trak);
    err = MP4GetTrackMedia(trak, &media);

    err = ISOGetGroupDescriptionEntryCount(media, MP4T35SampleGroupEntry, &it35_sg_cnt);
    CHECK(err == MP4NoErr);
    CHECK(1 == it35_sg_cnt);

    MP4Handle entryH;
    u32 size = 0;
    MP4NewHandle(0, &entryH);
    err = ISOGetGroupDescription(media, MP4T35SampleGroupEntry, 1, entryH);
    CHECK(err == MP4NoErr);
    MP4GetHandleSize(entryH, &size);
    CHECK(6 == size);

    err = ISOGetSampleGroupSampleNumbers(media, MP4T35SampleGroupEntry, 1, &sample_numbers, &sample_cnt);
    CHECK(err == MP4NoErr);

    u32 check_sample_cnt = 0;
    MP4GetMediaSampleCount(media, &check_sample_cnt);
    CHECK(check_sample_cnt > 0);
    CHECK(check_sample_cnt == sample_cnt);

  }
  */

  MP4DisposeHandle(it35_prefix);
}
