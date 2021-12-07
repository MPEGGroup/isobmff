/**
 * @file test_metadata.cpp
 * @author Dimitri Podborski
 * @brief Metadata testing
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

TEST_CASE("metadata")
{
  MP4Err err;

  SECTION("Check writing of Properties and EntityToGroups")
  {
    ISOMovie moov;
    ISOTrack trak;
    ISOMedia media;
    ISOMeta metaFile, metaMovie, metaTrack;

    MP4NewMovie(&moov, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    MP4AddTrackToMovieIOD(trak);
    ISONewTrackMedia(trak, &media, ISOVisualHandlerType, 90000, NULL);

    err = ISONewFileMeta(moov, MP4_FOUR_CHAR_CODE('f', 'o', 'o', '1'), &metaFile);
    CHECK(err == ISONoErr);

    err = ISONewMovieMeta(moov, MP4_FOUR_CHAR_CODE('f', 'o', 'o', '2'), &metaMovie);
    CHECK(err == ISONoErr);

    err = ISONewTrackMeta(trak, MP4_FOUR_CHAR_CODE('f', 'o', 'o', '3'), &metaTrack);
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
    MP4MediaAtomPtr mediaPtr = (MP4MediaAtom *)trackPtr->trackMedia;

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

    u32 groupID1 = 123;
    u32 groupID2 = 555;
    err          = ISONewEntityGroup(metaFile, MP4AlternativeEntityGroup, groupID1);
    CHECK(err == MP4NoErr);
    err = ISONewEntityGroup(metaFile, MP4AlternativeEntityGroup, groupID1);
    CHECK(err == MP4BadParamErr);
    err = ISONewEntityGroup(metaFile, MP4AlternativeEntityGroup + 1, groupID1);
    CHECK(err == MP4BadParamErr);
    err = ISONewEntityGroup(metaFile, MP4AlternativeEntityGroup, groupID2);
    CHECK(err == MP4NoErr);

    u32 id1 = 1001;
    u32 id2 = 2001;
    err     = ISOAddEntityID(metaFile, groupID1 + 1, id1);
    CHECK(err == MP4NotFoundErr); // no such groupID present
    err = ISOAddEntityID(metaFile, groupID1, id1);
    CHECK(err == MP4NoErr);
    err = ISOAddEntityID(metaFile, groupID1, id2);
    CHECK(err == MP4NoErr);

    u32 temp = 0;
    err      = ISOGetEntityIDCnt(metaFile, groupID1 + 1, &temp);
    CHECK(err == MP4NotFoundErr);
    err = ISOGetEntityIDCnt(metaFile, groupID1, &temp);
    CHECK(err == MP4NoErr);
    CHECK(2 == temp);
    err = ISOGetEntityIDCnt(metaFile, groupID2, &temp);
    CHECK(err == MP4NoErr);
    CHECK(0 == temp);

    u32 entityCnt = 0;
    EntityGroupEntryPtr pEntityGroupEntries;
    err = ISOGetEntityGroupEntries(metaFile, &pEntityGroupEntries, &entityCnt);
    CHECK(err == MP4NoErr);
    CHECK(2 == entityCnt);
    for(uint32_t i = 0; i < entityCnt; i++)
    {
      CHECK((pEntityGroupEntries + i)->grouping_type == MP4AlternativeEntityGroup);
      switch(i)
      {
      case 0:
        CHECK((pEntityGroupEntries + i)->group_id == groupID1);
        CHECK((pEntityGroupEntries + i)->num_entities_in_group == 2);
        CHECK((pEntityGroupEntries + i)->entity_ids != NULL);
        CHECK((pEntityGroupEntries + i)->entity_ids[0] == id1);
        CHECK((pEntityGroupEntries + i)->entity_ids[1] == id2);
        break;
      case 1:
        CHECK((pEntityGroupEntries + i)->group_id == groupID2);
        CHECK((pEntityGroupEntries + i)->num_entities_in_group == 0);
        CHECK((pEntityGroupEntries + i)->entity_ids == NULL);
        break;
      default:
        break;
      }
    }

    err = MP4WriteMovieToFile(moov, "test_metadata.mp4");
    CHECK(err == ISONoErr);
  }

  SECTION("Check parsing of Properties and EntityToGroups")
  {
    MP4Handle handle;
    // create handle with data from buffer
    err = createHandleFromBuffer(&handle, META_FILE1, sizeof(META_FILE1));
    REQUIRE(err == MP4NoErr);

    ISOMovie cMovieBox;
    err = MP4NewMovieFromHandle(&cMovieBox, handle, MP4OpenMovieNormal);
    REQUIRE(err == MP4NoErr);

    ISOMeta metaFile;
    u32 outMetaType = 0;
    err             = ISOGetFileMeta(cMovieBox, &metaFile, 0, &outMetaType);
    CHECK(err == MP4NoErr);
    CHECK(outMetaType == MP4_FOUR_CHAR_CODE('t', 'e', 's', 't'));
    CHECK(metaFile != NULL);

    u32 entityCnt = 0;
    u32 groupID1  = 123;
    u32 groupID2  = 555;
    u32 id1       = 1001;
    u32 id2       = 2001;
    EntityGroupEntryPtr pEntityGroupEntries;
    err = ISOGetEntityGroupEntries(metaFile, &pEntityGroupEntries, &entityCnt);
    CHECK(err == MP4NoErr);
    CHECK(2 == entityCnt);
    for(uint32_t i = 0; i < entityCnt; i++)
    {
      switch(i)
      {
      case 0:
        CHECK((pEntityGroupEntries + i)->grouping_type == MP4AlternativeEntityGroup);
        CHECK((pEntityGroupEntries + i)->group_id == groupID1);
        CHECK((pEntityGroupEntries + i)->num_entities_in_group == 2);
        CHECK((pEntityGroupEntries + i)->entity_ids != NULL);
        CHECK((pEntityGroupEntries + i)->entity_ids[0] == id1);
        CHECK((pEntityGroupEntries + i)->entity_ids[1] == id2);
        break;
      case 1:
        CHECK((pEntityGroupEntries + i)->grouping_type == MP4_FOUR_CHAR_CODE('a', 'l', 't', 'X'));
        CHECK((pEntityGroupEntries + i)->group_id == groupID2);
        CHECK((pEntityGroupEntries + i)->num_entities_in_group == 0);
        CHECK((pEntityGroupEntries + i)->entity_ids == NULL);
        break;
      default:
        break;
      }
    }
  }
}