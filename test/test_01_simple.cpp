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
#include "testdataPath.h"

#include <string>

const std::string strDataPath = TESTDATA_PATH;
const std::string strTestFile = strDataPath + +"/isobmff/01_simple.mp4";

// isobmff stuff
ISOMovie cMovieBox;

TEST_CASE("Check isobmff/01_simple.mp4")
{
  ISOErr err;

  err = ISOOpenMovieFile(&cMovieBox, strTestFile.c_str(), MP4OpenMovieNormal);
  REQUIRE(err == MP4NoErr);

  ISOTrack audioTrack, videoTrack;

  SECTION("Check Movie")
  {
    u32 brand, minorversion;
    err = ISOGetMovieBrand(cMovieBox, &brand, &minorversion);
    CHECK(err == MP4NoErr);
    CHECK(brand == MP4_FOUR_CHAR_CODE('m', 'p', '4', '2'));
    CHECK(minorversion == 1);

    u32 outTimeScale;
    err = ISOGetMovieTimeScale(cMovieBox, &outTimeScale);
    CHECK(err == MP4NoErr);
    CHECK(outTimeScale == 600);

    ISOTrack audioTrack;
    err = ISOGetMovieTrack(cMovieBox, 101, &audioTrack);
    CHECK(err == MP4NoErr);
    CHECK(audioTrack != 0);
    err = ISOGetMovieTrack(cMovieBox, 301, &audioTrack); // non existing track
    CHECK(err == MP4BadParamErr);

    ISOHandle h;
    err = ISONewHandle(0, &h);
    CHECK(err == MP4NoErr); // TODO: move this to utility testing suite
    err = MP4GetMovieInitialObjectDescriptor(cMovieBox, h);
    CHECK(err == MP4NoErr);
    ISODisposeHandle(h);
    CHECK(err == MP4NoErr); // TODO: move this to utility testing suite

    u8 outFlag;
    err = MP4GetMovieIODInlineProfileFlag(cMovieBox, &outFlag);
    CHECK(err == MP4NoErr);
    // TODO: check the value of outFlag

    u8 outOD;
    u8 outScene;
    u8 outAudio;
    u8 outVisual;
    u8 outGraphics;
    err = MP4GetMovieProfilesAndLevels(cMovieBox, &outOD, &outScene, &outAudio, &outVisual,
                                       &outGraphics);
    CHECK(err == MP4NoErr);
    // TODO: profiles and levels of Object Descriptor
    // WARN((u32)outOD);
    // WARN((u32)outScene);
    // WARN((u32)outAudio);
    // WARN((u32)outVisual);
    // WARN((u32)outGraphics);

    u64 outDuration;
    err = ISOGetMovieDuration(cMovieBox, &outDuration);
    CHECK(err == MP4NoErr);
    CHECK(outDuration == 6074);
  }

  SECTION("Check Tracks")
  {
    // ISOGetMovieIndTrack
    ISOTrack track1, track2, track3, track4;
    err = ISOGetMovieIndTrack(cMovieBox, 0, &audioTrack);
    REQUIRE(err != MP4NoErr);
    err = ISOGetMovieIndTrack(cMovieBox, 1, &track1);
    CHECK(err == MP4NoErr);
    err = ISOGetMovieIndTrack(cMovieBox, 2, &track2);
    CHECK(err == MP4NoErr);
    err = ISOGetMovieIndTrack(cMovieBox, 3, &track3);
    CHECK(err == MP4NoErr);
    err = ISOGetMovieIndTrack(cMovieBox, 4, &track4);
    CHECK(err == MP4NoErr);
    err = ISOGetMovieIndTrack(cMovieBox, 5, &track4);
    CHECK(err != MP4NoErr);

    // ISOGetMovieTrackCount
    uint32_t uiTrackCnt = 0;
    err                 = ISOGetMovieTrackCount(cMovieBox, &uiTrackCnt);
    CHECK(err == MP4NoErr);
    CHECK(uiTrackCnt == 4);

    // ISOGetTrackEnabled
    u32 outEnabled;
    ISOGetMovieTrack(cMovieBox, 101, &audioTrack);
    err = ISOGetTrackEnabled(audioTrack, &outEnabled);
    CHECK(err == MP4NoErr);
    CHECK(outEnabled == 1);

    // ISOGetTrackEditlistEntryCount no elst in 01_simple.mp4
    u32 outEditListCnt;
    err = ISOGetTrackEditlistEntryCount(audioTrack, &outEditListCnt);
    CHECK(err == MP4NotFoundErr);

    // ISOGetTrackEditlist n/a for 01_simple.mp4
    // ISOGetTrackID
    u32 outTrackID;
    err = ISOGetTrackID(audioTrack, &outTrackID);
    CHECK(err == MP4NoErr);
    CHECK(outTrackID == 101);

    // ISOGetTrackMedia
    ISOMedia outMedia;
    err = ISOGetTrackMedia(audioTrack, &outMedia);
    CHECK(err == MP4NoErr);

    // ISOGetTrackMovie
    ISOMovie outMovie;
    err = ISOGetTrackMovie(audioTrack, &outMovie);
    CHECK(err == MP4NoErr);

    // ISOGetTrackOffset
    u32 outMovieOffsetTime;
    err = ISOGetTrackOffset(audioTrack, &outMovieOffsetTime);
    CHECK(err == MP4NoErr);
    CHECK(outMovieOffsetTime == 0);

    // ISOGetTrackReference
    ISOTrack outReferencedTrack;
    err = ISOGetTrackReference(audioTrack, MP4_FOUR_CHAR_CODE('s', 'y', 'n', 'c'), 1,
                               &outReferencedTrack);
    CHECK(err == MP4NoErr);
    ISOGetTrackID(outReferencedTrack, &outTrackID);
    CHECK(outTrackID == 201);

    // ISOGetTrackReferenceCount
    u32 outReferenceIndex;
    err = ISOGetTrackReferenceCount(audioTrack, MP4_FOUR_CHAR_CODE('s', 'y', 'n', 'c'),
                                    &outReferenceIndex);
    CHECK(err == MP4NoErr);
    CHECK(outReferenceIndex == 1);

    // MJ2GetTrackMatrix
    u32 outMatrix[9];
    err = MJ2GetTrackMatrix(audioTrack, outMatrix);
    CHECK(err == MP4NoErr);
    // 65536,0,0,0,65536,0,0,0,1073741824
    CHECK(outMatrix[0] == 65536);
    CHECK(outMatrix[1] == 0);
    CHECK(outMatrix[2] == 0);
    CHECK(outMatrix[3] == 0);
    CHECK(outMatrix[4] == 65536);
    CHECK(outMatrix[5] == 0);
    CHECK(outMatrix[6] == 0);
    CHECK(outMatrix[7] == 0);
    CHECK(outMatrix[8] == 1073741824);

    // MJ2GetTrackLayer
    s16 outLayer;
    err = MJ2GetTrackLayer(audioTrack, &outLayer);
    CHECK(err == MP4NoErr);
    CHECK(outLayer == 0);

    // MJ2GetTrackDimensions
    u32 outWidth, outHeight;
    ISOGetMovieTrack(cMovieBox, 201, &videoTrack);
    err = MJ2GetTrackDimensions(videoTrack, &outWidth, &outHeight);
    CHECK(err == MP4NoErr);
    CHECK(outWidth == 120);
    CHECK(outHeight == 96);

    // MJ2GetTrackVolume
    s16 outVolume;
    err = MJ2GetTrackVolume(audioTrack, &outVolume);
    CHECK(err == MP4NoErr);
    CHECK(outVolume == 256);

    // ISOGetTrackDuration
    u64 outDuration;
    err = ISOGetTrackDuration(audioTrack, &outDuration);
    CHECK(err == MP4NoErr);
    CHECK(outDuration == 6074);
  }

  SECTION("Check Media")
  {
    ISOMedia audioMedia, videoMedia;

    ISOGetMovieTrack(cMovieBox, 101, &audioTrack);
    ISOGetMovieTrack(cMovieBox, 201, &videoTrack);

    err = ISOGetTrackMedia(audioTrack, &audioMedia);
    REQUIRE(err == MP4NoErr);
    err = ISOGetTrackMedia(videoTrack, &videoMedia);
    REQUIRE(err == MP4NoErr);

    // MP4GetMediaDataRefCount
    u32 outCount;
    err = MP4GetMediaDataRefCount(audioMedia, &outCount);
    CHECK(err == MP4NoErr);
    CHECK(outCount == 1);

    // ISOGetIndMediaSample: read second sample of the audio track
    ISOHandle outSample;
    u32 outSize;
    u64 outDecodingTime;
    s32 outCTSOffset;
    u64 outDuration;
    u32 outSampleFlags;
    u32 outSampleDescIndex;
    ISONewHandle(0, &outSample);
    err = ISOGetIndMediaSample(audioMedia, 2, outSample, &outSize, &outDecodingTime, &outCTSOffset,
                               &outDuration, &outSampleFlags, &outSampleDescIndex);
    CHECK(err == MP4NoErr);
    CHECK(outSize == 141);
    CHECK(outDecodingTime == 1024);
    CHECK(outCTSOffset == 0);
    CHECK(outDuration == 1024);
    CHECK(outSampleFlags == 0);
    CHECK(outSampleDescIndex == 1);

    // TODO: implement MP4GetIndMediaSampleReference
    // TODO: implement ISOGetIndMediaSampleWithPad
    // TODO: implement ISOGetMediaDataReference
    // TODO: implement ISOGetMediaDuration
    // TODO: implement ISOGetMediaHandlerDescription
    // TODO: implement ISOGetMediaLanguage
    // TODO: implement ISOGetMediaExtendedLanguageTag

    // ISOGetMediaSample
    u64 outCompositionTime = 0;
    ISOHandle outSampleDescription;
    ISONewHandle(0, &outSampleDescription);
    outDecodingTime    = 0;
    outDuration        = 0;
    outSampleDescIndex = 0;
    err                = ISOGetMediaSample(audioMedia, outSample, &outSize, 3000, &outDecodingTime,
                            &outCompositionTime, &outDuration, outSampleDescription,
                            &outSampleDescIndex, &outSampleFlags);
    CHECK(err == MP4NoErr);
    CHECK(outDecodingTime == 2048);
    CHECK(outCompositionTime == 2048);
    CHECK(outDuration == 1024);
    CHECK(outSampleDescIndex == 1);
    CHECK(outSampleFlags == 0);

    // TODO: implement ISOGetMediaSampleWithPad

    // ISOGetMediaSampleCount
    err = ISOGetMediaSampleCount(audioMedia, &outCount);
    CHECK(err == MP4NoErr);
    CHECK(outCount == 218);

    // TODO: implement ISOGetMediaTimeScale
    // TODO: implement ISOGetMediaTrack
    // TODO: implement MP4GetMediaDecoderConfig
    // TODO: implement MP4GetMediaDecoderType
    // TODO: implement MP4GetMediaDecoderInformation
    // TODO: implement MJ2GetMediaGraphicsMode
    // TODO: implement MJ2GetMediaSoundBalance
    // TODO: implement ISOGetSampletoGroupMap
    // TODO: implement ISOGetGroupDescription
    // TODO: implement ISOGetSampleDependency
  }
}
