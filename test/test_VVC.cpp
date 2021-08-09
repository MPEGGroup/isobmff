#include <catch.hpp>
#include <ISOMovies.h>
#include "testdataPath.h"

#include <string>

#include "test_helpers.h"

const std::string strDataPath = TESTDATA_PATH;


TEST_CASE("New VVC test")
{
  SECTION("demo")
  { 
    ISOErr err;
    ISOMovie moov;
    ISOMedia media;
    ISOTrack trak;

    std::string strHEVC = "test_sampledescription_HEVC.mp4";

    ISOHandle TS, sampleEntryH;
    err = MP4NewHandle(sizeof(VVC::TS), &TS);
    std::memcpy((*TS), VVC::TS, sizeof(VVC::TS));
    err = MP4NewHandle(0, &sampleEntryH);

    err = MP4NewMovie(&moov, 0, 0, 0, 0, 0, 0);
    REQUIRE(err == ISONoErr);
    err = ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    REQUIRE(err == ISONoErr);
    err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, TIMESCALE, NULL);
    REQUIRE(err == ISONoErr);


    err = ISONewVVCSampleDescription(trak, sampleEntryH, 1, TS);
    CHECK(err == ISONoErr);

    u32 typeOut = 0;
    err         = ISOGetSampleDescriptionType(sampleEntryH, &typeOut);
    CHECK(err == ISONoErr);
    CHECK(typeOut == MP4_FOUR_CHAR_CODE('v', 'v', 'c', '1'));



  }

}