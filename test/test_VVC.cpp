/**
 * @file test_VVC.cpp
 * @author Pengjian
 * @author Dimitri Podborski
 * @brief Testing of VVC related functions
 * @version 0.1
 * @date 2020-11-20
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
#include <MP4LinkedList.c>

TEST_CASE("Test VVC related stuff")
{
  MP4Err err = MP4NoErr;

  SECTION("Parsing simple vvcC")
  {
    MP4Handle w_handle1;
    ISOVVCConfigAtomPtr box;

    err = createHandleFromBuffer(&w_handle1, VVC::VVCC_1, sizeof(VVC::VVCC_1));
    REQUIRE(err == MP4NoErr);

    err = MP4ParseAtomFromHandle(w_handle1, (MP4AtomPtr *)&box);
    REQUIRE(err == MP4NoErr);

    CHECK(box->version == 0);
    CHECK(box->flags == 0);
    CHECK(box->LengthSizeMinusOne == 1);
    CHECK(box->ptl_present_flag == 0);
    CHECK(box->num_of_arrays == 0);

    // clean up
    MP4DisposeHandle(w_handle1);
  }

  SECTION("Parsing vvcC from vvc_basic_track.mp4")
  {
    MP4Handle w_handle1;
    ISOVVCConfigAtomPtr box;

    err = createHandleFromBuffer(&w_handle1, VVC::VVCC_2, sizeof(VVC::VVCC_2));
    REQUIRE(err == MP4NoErr);

    err = MP4ParseAtomFromHandle(w_handle1, (MP4AtomPtr *)&box);
    REQUIRE(err == MP4NoErr);

    CHECK(box->version == 0);
    CHECK(box->flags == 0);
    CHECK(box->LengthSizeMinusOne == 3);
    CHECK(box->ptl_present_flag == 1);
    CHECK(box->ols_idx == 0);

    CHECK(box->num_sublayers == 5);
    CHECK(box->constant_frame_rate == 0);
    CHECK(box->chroma_format_idc == 1);
    CHECK(box->bit_depth_minus8 == 7);
    CHECK(box->native_ptl.num_bytes_constraint_info == 1);
    CHECK(box->native_ptl.general_profile_idc == 1);
    CHECK(box->native_ptl.general_tier_flag == 0);
    CHECK(box->native_ptl.general_level_idc == 35);
    CHECK(box->native_ptl.ptl_frame_only_constraint_flag == 1);
    CHECK(box->native_ptl.ptl_multi_layer_enabled_flag == 0);

    // TODO: add more checks based on what supposed to be in vvcC from vvc_basic_track.mp4

    CHECK(box->max_picture_width == 416);
    CHECK(box->max_picture_height == 240);
    CHECK(box->avg_frame_rate == 0);
    CHECK(box->num_of_arrays == 2);

    for(u32 i = 0; i < box->num_of_arrays; i++)
    {
      CHECK(box->arrays[i].array_completeness == 1);
      CHECK(box->arrays[i].nalList->entryCount == 1);
      switch(i)
      {
      case 0:
        CHECK(box->arrays[i].NAL_unit_type == 15); // SPS
        // TODO: check nalu length and nalu data after implementing functions to get them
        MP4Handle sps;
        u32 sps_length, sps_cnt;
        err = MP4GetListEntryCount(box->arrays[i].nalList, &sps_cnt);
        CHECK(err == MP4NoErr);
        CHECK(sps_cnt == 1);
        err = MP4GetListEntry(box->arrays[i].nalList, 0, (char **)&sps);
        err = MP4GetHandleSize(sps, &sps_length);
        CHECK(err == MP4NoErr);
        // 0x00C5
        CHECK(sps_length == 197);
        break;
      case 1:
        CHECK(box->arrays[i].NAL_unit_type == 16); // PPS
        // TODO: check nalu length and nalu data after implementing functions to get them
        MP4Handle pps;
        u32 pps_length, pps_cnt;
        err = MP4GetListEntryCount(box->arrays[i].nalList, &pps_cnt);
        CHECK(err == MP4NoErr);
        CHECK(pps_cnt == 1);
        err = MP4GetListEntry(box->arrays[i].nalList, 0, (char **)&pps);
        err = MP4GetHandleSize(pps, &pps_length);
        CHECK(err == MP4NoErr);
        // 0x000D
        CHECK(pps_length == 13);
        break;
      default:
        break;
      }
    }

    // TODO: add more checks based on what supposed to be in vvcC from vvc_basic_track.mp4

    // clean up
    MP4DisposeHandle(w_handle1);
  }

  SECTION("creat vvcC from SPS")
  {
    MP4Handle sps, sampleEntryH, pps;

    ISOMovie moov;
    ISOMedia media;
    ISOTrack trak;
    err = MP4NewMovie(&moov, 0, 0, 0, 0, 0, 0);
    REQUIRE(err == ISONoErr);
    err = ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    REQUIRE(err == ISONoErr);
    err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, TIMESCALE, NULL);
    REQUIRE(err == ISONoErr);

    //u8 data[132];
    //FILE *input;
    //input = fopen("C:\\Users\\29968\\Desktop\\sps.bin", "rb");
    //fread(data, 132, 1, input);
    //err = createHandleFromBuffer(&sps, data, sizeof(data));
    //REQUIRE(err == MP4NoErr);

    err = createHandleFromBuffer(&sps, VVC::SPS, sizeof(VVC::SPS));
    REQUIRE(err == MP4NoErr);

    err = createHandleFromBuffer(&pps, VVC::PPS, sizeof(VVC::PPS));
    REQUIRE(err == MP4NoErr);

    u32 lengthSize = 4;
    err            = MP4NewHandle(0, &sampleEntryH);
    err            = ISONewVVCSampleDescription(trak, sampleEntryH, 1, lengthSize, sps, pps);
    CHECK(err == ISONoErr);

    u32 lengthSizeOut;
    err = ISOGetNALUnitLength(sampleEntryH, &lengthSizeOut);
    CHECK(err == ISONoErr);
    CHECK(lengthSizeOut == lengthSize);

    // get sampledescription type
    u32 typeOut = 0;
    err         = ISOGetSampleDescriptionType(sampleEntryH, &typeOut);
    CHECK(err == ISONoErr);
    CHECK(typeOut == MP4_FOUR_CHAR_CODE('v', 'v', 'c', '1'));

    u32 dataReferenceIndex, length_size, count;
    err = ISOGetVVCSampleDescription(sampleEntryH, &dataReferenceIndex, &length_size, VVCsps, &count);
    CHECK(dataReferenceIndex == 1);
    CHECK(length_size == 4);
    CHECK(count == 1);

    // test get parameter sets
    ISOHandle spsHandleOut;
    ISONewHandle(0, &spsHandleOut);
    err = ISOGetVVCSampleDescriptionPS(sampleEntryH, spsHandleOut, VVCsps, 1);
    CHECK(err == ISONoErr);
    err = compareData(spsHandleOut, VVC::SPS, sizeof(VVC::SPS));
    CHECK(err == ISONoErr);

    u32 num_nalus, nal_unit_length;
    err = ISOGetVVCNaluNums(sampleEntryH, VVCsps, &num_nalus);
    CHECK(err == ISONoErr);
    CHECK(num_nalus == 1);
    //CHECK(nal_unit_length == 197);

    err = createHandleFromBuffer(&pps, VVC::PPS, sizeof(VVC::PPS));
    REQUIRE(err == MP4NoErr);

    err = ISOAddVVCSampleDescriptionPS(sampleEntryH, pps, VVCpps);
    CHECK(err == ISONoErr);

    err = ISOGetVVCNaluNums(sampleEntryH, VVCpps, &num_nalus);
    CHECK(err == ISONoErr);
    CHECK(num_nalus == 2);
    //CHECK(nal_unit_length == 13);
  }

  SECTION("test vvs1 sample entry") 
  {
    MP4Handle sampleEntryH;

    ISOMovie moov;
    ISOMedia media;
    ISOTrack trak;
    err = MP4NewMovie(&moov, 0, 0, 0, 0, 0, 0);
    REQUIRE(err == ISONoErr);
    err = ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    REQUIRE(err == ISONoErr);
    err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, TIMESCALE, NULL);
    REQUIRE(err == ISONoErr);

    u32 lengthSize = 4;
    err            = MP4NewHandle(0, &sampleEntryH);
    err            = ISONewVVCSubpicSampleDescription(trak, sampleEntryH, 1, 100,100, lengthSize);   
    CHECK(err == ISONoErr);

    u32 typeOut = 0;
    err         = ISOGetSampleDescriptionType(sampleEntryH, &typeOut);
    CHECK(err == ISONoErr);
    CHECK(typeOut == MP4_FOUR_CHAR_CODE('v', 'v', 's', '1'));

    u32 length;
    err = MP4GetHandleSize(sampleEntryH, &length);
    CHECK(err == ISONoErr);
    /* the vvs1 extend VisualSampleEntry and including the vvnC box, which is a fullBox */
    CHECK(length == 99);
    
    ISODisposeHandle(sampleEntryH);
  }
 }