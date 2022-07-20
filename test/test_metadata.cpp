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

  std::string strTestItem        = "test_item.mp4";
  std::string strTestEntityGroup = "test_meta_entity.mp4";

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
    err     = ISOAddEntityIDToGroup(metaFile, groupID1 + 1, id1);
    CHECK(err == MP4NotFoundErr); // no such groupID present
    err = ISOAddEntityIDToGroup(metaFile, groupID1, id1);
    CHECK(err == MP4NoErr);
    err = ISOAddEntityIDToGroup(metaFile, groupID1, id2);
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

    err = MP4WriteMovieToFile(moov, strTestEntityGroup.c_str());
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

  SECTION("Check non-timed item creation")
  {
    MP4Movie file;
    ISOMeta meta;
    u32 outMetaType;

    err = ISONewMetaMovie(&file, MP4_FOUR_CHAR_CODE('t', 'e', 's', 't'),
                          MP4_FOUR_CHAR_CODE('f', 'o', 'o', 'o'), 0);
    REQUIRE(err == MP4NoErr);

    err = ISOSetMovieCompatibleBrand(file, MP4_FOUR_CHAR_CODE('b', 'a', 'a', 'r'));
    CHECK(err == MP4NoErr);

    err = ISOGetFileMeta(file, &meta, 0, &outMetaType);
    CHECK(err == MP4NoErr);

    // add 2 items
    ISOMetaItem item1;
    ISOMetaItem item2;

    MP4Handle itemDataHandle1;
    MP4Handle itemDataHandle2;
    MP4Handle itemDataHandle3;
    err = createHandleFromBuffer(&itemDataHandle1, TestData::DEADBEEF, sizeof(TestData::DEADBEEF));
    CHECK(err == MP4NoErr);
    err =
      createHandleFromBuffer(&itemDataHandle2, TestData::FACEDECOCD, sizeof(TestData::FACEDECOCD));
    CHECK(err == MP4NoErr);
    err = createHandleFromBuffer(&itemDataHandle3, TestData::DECAFCODEDOC,
                                 sizeof(TestData::DECAFCODEDOC));
    CHECK(err == MP4NoErr);

    // item 1
    ISOAddMetaItem(meta, &item1, 0, 0);
    CHECK(err == MP4NoErr);
    err = ISOAddItemExtentUsingItemData(item1, itemDataHandle1);
    CHECK(err == MP4NoErr);
    err = ISOSetItemInfo(item1, 0, (char *)"first item", NULL, NULL);
    CHECK(err == MP4NoErr);
    err = ISOSetItemInfoItemType(item1, MP4_FOUR_CHAR_CODE('f', 'r', 's', 't'), NULL);
    CHECK(err == MP4NoErr);

    MP4GenericAtom *pTestProp;
    err = MP4NewForeignAtom((MP4GenericAtom *)&pTestProp, MP4_FOUR_CHAR_CODE('p', 'r', 'o', 'p'),
                            itemDataHandle3);
    CHECK(err == MP4NoErr);
    err = ISOAddMetaItemProperty(item1, pTestProp, 1);
    CHECK(err == MP4NoErr);

    ISOSetPrimaryItem(meta, item1);
    CHECK(err == MP4NoErr);

    // item 2
    ISOAddMetaItem(meta, &item2, 0, 0);
    CHECK(err == MP4NoErr);
    err = ISOAddItemExtentUsingItemData(item2, itemDataHandle2);
    CHECK(err == MP4NoErr);
    err = ISOSetItemInfo(item2, 0, (char *)"second item", NULL, NULL);
    CHECK(err == MP4NoErr);
    err = ISOSetItemInfoItemType(item2, MP4_FOUR_CHAR_CODE('s', 'c', 'n', 'd'), NULL);
    CHECK(err == MP4NoErr);

    err = ISOAddMetaItemProperty(item2, pTestProp, 0);
    CHECK(err == MP4NoErr);

    // add relation
    err = ISOAddItemRelation(item1, item2, MP4_FOUR_CHAR_CODE('p', 't', 'r', '1'));
    REQUIRE(err == MP4NoErr);

    err = MP4WriteMovieToFile(file, strTestItem.c_str());
    REQUIRE(err == MP4NoErr);
  }

  SECTION("Check non-timed item parsing")
  {
    MP4Movie moov;
    ISOMeta meta;

    err = MP4OpenMovieFile(&moov, strTestItem.c_str(), MP4OpenMovieNormal);
    CHECK(err == MP4NoErr);

    u32 outHandleType;
    err = ISOGetFileMeta(moov, &meta, MP4_FOUR_CHAR_CODE('t', 'e', 's', 't'), &outHandleType);
    CHECK(err == MP4NoErr);
    CHECK(outHandleType == MP4_FOUR_CHAR_CODE('t', 'e', 's', 't'));

    // check first item
    ISOMetaItem *items;
    u32 cnt;
    err = ISOGetAllItemsWithType(meta, MP4_FOUR_CHAR_CODE('f', 'r', 's', 't'), &items, &cnt);
    CHECK(err == MP4NoErr);
    CHECK(cnt == 1);
    MP4GenericAtom *properties;
    ISOMetaItem item1 = items[0];
    err               = ISOGetProperitesOfMetaItem(item1, &properties, &cnt);
    CHECK(err == MP4NoErr);
    CHECK(cnt == 1);
    MP4UnknownAtomPtr prop = (MP4UnknownAtomPtr)properties[0];
    CHECK(prop->type == MP4_FOUR_CHAR_CODE('p', 'r', 'o', 'p'));
    MP4Handle propHandle;
    MP4NewHandle(prop->dataSize, &propHandle);
    memcpy(*propHandle, prop->data, prop->dataSize);
    err = compareData(propHandle, TestData::DECAFCODEDOC, sizeof(TestData::DECAFCODEDOC));
    CHECK(err == MP4NoErr);

    // check second item
    err = ISOGetAllItemsWithType(meta, MP4_FOUR_CHAR_CODE('s', 'c', 'n', 'd'), &items, &cnt);
    CHECK(err == MP4NoErr);
    CHECK(cnt == 1);
    ISOMetaItem item2 = items[0];
    err               = ISOGetProperitesOfMetaItem(item2, &properties, &cnt);
    CHECK(err == MP4NoErr);
    CHECK(cnt == 1);
    prop = (MP4UnknownAtomPtr)properties[0];
    CHECK(prop->type == MP4_FOUR_CHAR_CODE('p', 'r', 'o', 'p'));
    memcpy(*propHandle, prop->data, prop->dataSize);
    err = compareData(propHandle, TestData::DECAFCODEDOC, sizeof(TestData::DECAFCODEDOC));
    CHECK(err == MP4NoErr);

    // check references
    u16 refCnt;
    MP4Handle refItemIDsHandle;
    MP4NewHandle(0, &refItemIDsHandle);
    err = ISOGetItemReferences(item1, MP4_FOUR_CHAR_CODE('p', 't', 'r', '1'), &refCnt,
                               refItemIDsHandle);
    CHECK(err == MP4NoErr);
    CHECK(refCnt == 1);
    u32 *refItemIDsArray = (u32 *)*refItemIDsHandle;
    CHECK(refItemIDsArray[0] == 2);

    err = ISOGetItemReferences(item2, MP4_FOUR_CHAR_CODE('p', 't', 'r', '1'), &refCnt,
                               refItemIDsHandle);
    CHECK(err == MP4NoErr);
    CHECK(refCnt == 0);
  }
}
