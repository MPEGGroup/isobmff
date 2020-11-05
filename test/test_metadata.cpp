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
#include <MP4Atoms.h>

TEST_CASE("Check Metadata functions")
{
  SECTION("Check Properties")
  {
    ISOErr err;
    ISOMovie moov;
    ISOTrack trak;
    ISOMedia media;
    ISOMeta metaFile, metaMovie, metaTrack;

    MP4NewMovie(&moov, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    MP4AddTrackToMovieIOD(trak);
    ISONewTrackMedia(trak, &media, ISOVisualHandlerType, 90000, NULL);

    err = ISONewFileMeta( moov, 123, &metaFile);
    CHECK(err == ISONoErr);

    err = ISONewMovieMeta( moov, 456, &metaMovie);
    CHECK(err == ISONoErr);

    err = ISONewTrackMeta( trak, 789, &metaTrack);
    CHECK(err == ISONoErr);

    u16 outRefIdx;
    ISOHandle urlHandle, urnHandle;
    ISONewHandle(0, &urlHandle);
    ISONewHandle(0, &urnHandle);
    err = ISOAddMetaDataReference(metaFile, &outRefIdx, urlHandle, urnHandle);
    CHECK(err == ISONoErr);
    CHECK(outRefIdx == 1);

    ISOMetaItem outItem;
    err = ISOAddMetaItem(metaFile, &outItem, 0, 0);
    CHECK(err == ISONoErr);

    // MP4GenericAtom foo;
    MP4TrackAtomPtr trackPtr = (MP4TrackAtomPtr)trak;
    MP4MediaAtomPtr mediaPtr = (MP4MediaAtom*)trackPtr->trackMedia;

    // put tkhd and mdhd as properties in the media item
    err = ISOAddMetaItemProperty(outItem, (MP4GenericAtom *)trackPtr->trackHeader, 0);
    CHECK(err == ISONoErr);
    err = ISOAddMetaItemProperty(outItem, (MP4GenericAtom *)mediaPtr->mediaHeader, 0);
    CHECK(err == ISONoErr);

    MP4GenericAtom *properties;
    u32 propertiesFound;
    err = ISOGetProperitesOfMetaItem(outItem, &properties, &propertiesFound);
    CHECK(err == ISONoErr);
    REQUIRE(2 == propertiesFound);

    MP4TrackAtomPtr prop1 = (MP4TrackAtomPtr)properties[0];
    MP4MediaAtomPtr prop2 = (MP4MediaAtomPtr)properties[1];
    CHECK(MP4_FOUR_CHAR_CODE('t', 'k', 'h', 'd') == prop1->type);
    CHECK(MP4_FOUR_CHAR_CODE('m', 'd', 'h', 'd') == prop2->type);
  
    err = MP4WriteMovieToFile(moov, "temp.mp4");
    CHECK(err==ISONoErr);
  }
}