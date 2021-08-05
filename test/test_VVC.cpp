#include <catch.hpp>
#include <ISOMovies.h>
#include "testdataPath.h"

#include <string>

const std::string strDataPath = TESTDATA_PATH;


ISOMovie cVVCMovieBox;

TEST_CASE("New VVC test")
{
	SECTION("vvc_basic_track.mp4 which is a single VVC track with the sample entry type ¡®vvc1¡¯") 
	{ 
    const std::string strTestFile1 = strDataPath + "/isobmff/VVC/" + "vvc_basic_track.mp4";
		ISOErr err;
    err = ISOOpenMovieFile(&cVVCMovieBox, strTestFile1.c_str(), MP4OpenMovieNormal);
    REQUIRE(err == MP4NoErr);

    u32 brand, minorversion;
    err = ISOGetMovieBrand(cVVCMovieBox, &brand, &minorversion);
    CHECK(err == MP4NoErr);
    CHECK(brand == MP4_FOUR_CHAR_CODE('m', 'p', '4', '1'));

    uint32_t uiTrackCnt = 0;
    err                 = ISOGetMovieTrackCount(cVVCMovieBox, &uiTrackCnt);
    CHECK(err == MP4NoErr);
    CHECK(uiTrackCnt == 1);

    ISOTrack track1;
    err = ISOGetMovieIndTrack(cVVCMovieBox, 1, &track1);
    CHECK(err == MP4NoErr);

    u32 trackID;
    err = ISOGetTrackID(track1, &trackID);
    CHECK(err == MP4NoErr);
    CHECK(trackID == 1);

    ISOMedia videoMedia;
    err = ISOGetTrackMedia(track1, &videoMedia);
    REQUIRE(err == MP4NoErr);

    ISOHandle outSample;
    u32 outSize;
    u64 outDecodingTime;
    s32 outCTSOffset;
    u64 outDuration;
    u32 outSampleFlags;
    u32 outSampleDescIndex;
    ISONewHandle(0, &outSample);
    err = ISOGetIndMediaSample(videoMedia, 1, outSample, &outSize, &outDecodingTime, &outCTSOffset, &outDuration, &outSampleFlags, &outSampleDescIndex);
    CHECK(err == ISONoErr);

    // get sample entry 
    u32 outDataReferenceIndex = 0;
    err = MP4GetMediaSampleDescription(videoMedia, 1, outSample, &outDataReferenceIndex);
    CHECK(err == ISONoErr);
    CHECK(outDataReferenceIndex == 1);

    u32 typeOut = 0;
    err = ISOGetSampleDescriptionType(outSample, &typeOut);
    CHECK(err == ISONoErr);
    CHECK(typeOut == MP4_FOUR_CHAR_CODE('v', 'v', 'c', '1'));
	}

  SECTION("vvc_subpicture_tracks") 
  {
    const std::string strTestFile1 = strDataPath + "/isobmff/VVC/" + "vvc_subpicture_tracks.mp4";
    ISOErr err;
    err = ISOOpenMovieFile(&cVVCMovieBox, strTestFile1.c_str(), MP4OpenMovieNormal);
    REQUIRE(err == MP4NoErr);

    //Movie Brand
    u32 brand, minorversion;
    err = ISOGetMovieBrand(cVVCMovieBox, &brand, &minorversion);
    CHECK(err == MP4NoErr);
    CHECK(brand == MP4_FOUR_CHAR_CODE('m', 'p', '4', '1'));

    //Num tracks
    uint32_t uiTrackCnt = 0;
    err                 = ISOGetMovieTrackCount(cVVCMovieBox, &uiTrackCnt);
    CHECK(err == MP4NoErr);
    CHECK(uiTrackCnt == 1 + 8 * 2);

    ISOTrack  track1, track2, track3, track4, track5;

    err = ISOGetMovieIndTrack(cVVCMovieBox, 1, &track1);//base track
    CHECK(err == MP4NoErr);
    err = ISOGetMovieIndTrack(cVVCMovieBox, 2, &track2);
    CHECK(err == MP4NoErr);
    err = ISOGetMovieIndTrack(cVVCMovieBox, 3, &track3);
    CHECK(err == MP4NoErr);
    err = ISOGetMovieIndTrack(cVVCMovieBox, 4, &track4);
    CHECK(err == MP4NoErr);

    // 1 + 16 = 17 tracks
    err = ISOGetMovieIndTrack(cVVCMovieBox, 18, &track5);
    CHECK(err != MP4NoErr);

    //track1: vvc1 // other track: vvs1
    ISOMedia videoMedia;
    err = ISOGetTrackMedia(track2, &videoMedia);
    REQUIRE(err == MP4NoErr);

    ISOHandle outSample;
    u32 outSize;
    u64 outDecodingTime;
    s32 outCTSOffset;
    u64 outDuration;
    u32 outSampleFlags;
    u32 outSampleDescIndex;
    ISONewHandle(0, &outSample);
    err = ISOGetIndMediaSample(videoMedia, 1, outSample, &outSize, &outDecodingTime, &outCTSOffset, &outDuration, &outSampleFlags, &outSampleDescIndex);
    CHECK(err == ISONoErr);

    // get sample entry 
    u32 outDataReferenceIndex;
    err = MP4GetMediaSampleDescription(videoMedia, 1, outSample, &outDataReferenceIndex);
    CHECK(err == ISONoErr);
    CHECK(outDataReferenceIndex == 1);

    u32 typeOut = 0;
    err         = ISOGetSampleDescriptionType(outSample, &typeOut);
    CHECK(err == ISONoErr);
    CHECK(typeOut == MP4_FOUR_CHAR_CODE('v', 'v', 's', '1'));

    u32 outTrackID;
    err = ISOGetTrackID(track2, &outTrackID);
    CHECK(outTrackID == 2);

    // track1 reference 8 subpic
    u32 outReferenceIndex;
    err = ISOGetTrackReferenceCount(track1, MP4_FOUR_CHAR_CODE('s', 'u', 'b', 'p'),
                                    &outReferenceIndex);
    CHECK(err == MP4NoErr);
    CHECK(outReferenceIndex == 8);
    
    //how to get the track grouping 'alte'?
    u32 groupType = MP4_FOUR_CHAR_CODE('a', 'l', 't', 'e');

    // get track grouping 'alte'
    // get subpic order sample grouping 'spor'
    // get layout map entry
   
    groupType = MP4_FOUR_CHAR_CODE('s', 'p', 'i', 'd');
    u32 *sampleNum;
    u32 sampleCnt;
    err = ISOGetSampleGroupSampleNumbers(videoMedia, groupType, 0, &sampleNum, &sampleCnt);
    CHECK(err == MP4NoErr);



  }

}