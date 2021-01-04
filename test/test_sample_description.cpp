/**
 * @file test_sample_description.cpp
 * @author Dimitri Podborski
 * @brief Perform checks for Sample Description functions
 * @version 0.1
 * @date 2021-01-04
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
#include "test_helpers.h"

TEST_CASE("Check Sample Description functions")
{
  SECTION("Check HEVC")
  {
    ISOErr err;
    ISOMovie moov;
    ISOMedia media;
    ISOTrack trak;

    std::string strHEVC = "test_sampledescription_HEVC.mp4";

    ISOHandle spsHandle, ppsHandle, vpsHandle, sampleEntryH;
    err = MP4NewHandle(sizeof(HEVC::SPS), &spsHandle);
    std::memcpy((*spsHandle), HEVC::SPS, sizeof(HEVC::SPS));
    err = MP4NewHandle(sizeof(HEVC::PPS), &ppsHandle);
    std::memcpy((*ppsHandle), HEVC::PPS, sizeof(HEVC::PPS));
    err = MP4NewHandle(sizeof(HEVC::VPS), &vpsHandle);
    std::memcpy((*vpsHandle), HEVC::VPS, sizeof(HEVC::VPS));
    err = MP4NewHandle(0, &sampleEntryH);

    err = MP4NewMovie(&moov, 0, 0, 0, 0, 0, 0);
    REQUIRE(err == ISONoErr);
    err = ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    REQUIRE(err == ISONoErr);
    err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, TIMESCALE, NULL);
    REQUIRE(err == ISONoErr);

    // lengtisize shall only be 1, 2 or 4. Other values MUST fail
    err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 0, spsHandle, ppsHandle, vpsHandle);
    CHECK(err == ISOBadParamErr);
    err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 3, spsHandle, ppsHandle, vpsHandle);
    CHECK(err == ISOBadParamErr);
    err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 5, spsHandle, ppsHandle, vpsHandle);
    CHECK(err == ISOBadParamErr);

    u32 lengthSize = 2;
    err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, lengthSize, spsHandle, ppsHandle, 
                                      vpsHandle);
    CHECK(err == ISONoErr);

    // just add sample entry, call addHEVCSamples with sample count = 0
    err = addHEVCSamples(media, "", 0, sampleEntryH);
    CHECK(err == ISONoErr);

    // add a few dummy samples
    err = addHEVCSamples(media, "rb", 3, 0, lengthSize);
    CHECK(err == ISONoErr);

    // test get parameter sets
    ISOHandle spsHandleOut, ppsHandleOut, vpsHandleOut;
    ISONewHandle(0, &spsHandleOut);
    ISONewHandle(0, &ppsHandleOut);
    ISONewHandle(0, &vpsHandleOut);
    err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, vpsHandleOut, HEVCvps, 1);
    CHECK(err == ISONoErr);
    err = checkData(vpsHandleOut, HEVC::VPS, sizeof(HEVC::VPS));
    CHECK(err == ISONoErr);
    err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, spsHandleOut, HEVCsps, 1);
    CHECK(err == ISONoErr);
    err = checkData(spsHandleOut, HEVC::SPS, sizeof(HEVC::SPS));
    CHECK(err == ISONoErr);
    err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, ppsHandleOut, HEVCpps, 1);
    CHECK(err == ISONoErr);
    err = checkData(ppsHandleOut, HEVC::PPS, sizeof(HEVC::PPS));
    CHECK(err == ISONoErr);
    // no such parameter set
    err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, ppsHandleOut, 35, 1);
    CHECK(err != ISONoErr);

    // get sampledescription type
    u32 typeOut = 0;
    err = ISOGetSampleDescriptionType(sampleEntryH, &typeOut);
    CHECK(err == ISONoErr);
    CHECK(typeOut == MP4_FOUR_CHAR_CODE('h', 'v', 'c', '1'));

    u32 lengthSizeOut = 0;
    err = ISOGetNALUnitLength(sampleEntryH, &lengthSizeOut);
    CHECK(err == ISONoErr);
    CHECK(lengthSizeOut == lengthSize);


    err = MP4WriteMovieToFile(moov, strHEVC.c_str());
    CHECK(err == ISONoErr);
  }
}