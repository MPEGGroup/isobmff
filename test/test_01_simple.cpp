#include <catch.hpp>
#include <ISOMovies.h>
#include "testdataPath.h"

#include <string>

const std::string strDataPath = TESTDATA_PATH;
const std::string strTestFile = strDataPath + + "/isobmff/01_simple.mp4";

// isobmff stuff
ISOMovie  cMovieBox;


TEST_CASE( "1: Check isobmff/01_simple.mp4" ) {
  ISOErr err;

  err = ISOOpenMovieFile(&cMovieBox, strTestFile.c_str(), MP4OpenMovieNormal);
  REQUIRE( err == MP4NoErr );

  SECTION( "Check Movie" ) {
    u32 brand, minorversion;
    err = ISOGetMovieBrand( cMovieBox, &brand, &minorversion );
    REQUIRE( err == MP4NoErr );
    REQUIRE( brand == MP4_FOUR_CHAR_CODE('m', 'p', '4', '2') );
    REQUIRE( minorversion == 1 );

    u32 outTimeScale;
    err = ISOGetMovieTimeScale( cMovieBox, &outTimeScale );
    REQUIRE( err == MP4NoErr );
    REQUIRE( outTimeScale == 600 );

    ISOTrack audioTrack;
    err = ISOGetMovieTrack( cMovieBox, 101, &audioTrack);
    REQUIRE( err == MP4NoErr );
    REQUIRE( audioTrack != 0 );
    err = ISOGetMovieTrack( cMovieBox, 301, &audioTrack); // non existing track
    REQUIRE( err == MP4BadParamErr );

    ISOHandle h;
    err = ISONewHandle(0, &h);
    REQUIRE( err == MP4NoErr ); // todo: move this to utility testing suite
    err = MP4GetMovieInitialObjectDescriptor( cMovieBox, h );
    REQUIRE( err == MP4NoErr );
    ISODisposeHandle(h);
    REQUIRE( err == MP4NoErr ); // todo: move this to utility testing suite

    u8 outFlag;
    err = MP4GetMovieIODInlineProfileFlag ( cMovieBox, &outFlag );
    REQUIRE( err == MP4NoErr );
    // todo: outFlag ???

    u8 outOD;
    u8 outScene;
    u8 outAudio;
    u8 outVisual;
    u8 outGraphics;
    err = MP4GetMovieProfilesAndLevels( cMovieBox,
              &outOD, 
              &outScene, 
              &outAudio, 
              &outVisual, 
              &outGraphics );
    REQUIRE( err == MP4NoErr );
    // todo: profiles and levels of Object Descriptor
    // WARN((u32)outOD);
    // WARN((u32)outScene);
    // WARN((u32)outAudio);
    // WARN((u32)outVisual);
    // WARN((u32)outGraphics);

    u64 outDuration;
    err = ISOGetMovieDuration(cMovieBox, &outDuration);
    REQUIRE( err == MP4NoErr );
    REQUIRE( outDuration == 6074 );
  }

  SECTION( "Check Tracks" ) {

    ISOTrack tempTrack;
    err = ISOGetMovieIndTrack( cMovieBox, 0, &tempTrack );
    REQUIRE( err != MP4NoErr );
    err = ISOGetMovieIndTrack( cMovieBox, 1, &tempTrack );
    REQUIRE( err == MP4NoErr );
    err = ISOGetMovieIndTrack( cMovieBox, 2, &tempTrack );
    REQUIRE( err == MP4NoErr );
    err = ISOGetMovieIndTrack( cMovieBox, 3, &tempTrack );
    REQUIRE( err == MP4NoErr );
    err = ISOGetMovieIndTrack( cMovieBox, 4, &tempTrack );
    REQUIRE( err == MP4NoErr );
    err = ISOGetMovieIndTrack( cMovieBox, 5, &tempTrack );
    REQUIRE( err != MP4NoErr );

    uint32_t uiTrackCnt = 0;
    err = ISOGetMovieTrackCount(cMovieBox, &uiTrackCnt);
    REQUIRE( err == MP4NoErr );
    REQUIRE( uiTrackCnt == 4 );

    u32 outEnabled;
    ISOGetMovieTrack( cMovieBox, 101, &tempTrack);
    err = ISOGetTrackEnabled( tempTrack, &outEnabled );
    REQUIRE( err == MP4NoErr );
    REQUIRE( outEnabled == 1 );
    
    // todo others
  }

}
