/**
 * @file test_sample_groups.cpp
 * @author Dimitri Podborski
 * @brief Perform checks on sample groups, in movie and movie fragments, defragmentation, changning
 *        the sample group mapping types etc.
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
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include "test_data.h"

extern "C"
{
  MP4_EXTERN(MP4Err)
  ISONewHEVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                              u32 dataReferenceIndex, u32 length_size, MP4Handle first_sps,
                              MP4Handle first_pps, MP4Handle first_spsext);
}

const u32 TIMESCALE = 30000;
const u32 FPS       = 30;

const u32 FOURCC_COLOR = MP4_FOUR_CHAR_CODE('c', 'o', 'l', 'r');
const u32 FOURCC_TEST  = MP4_FOUR_CHAR_CODE('t', 'e', 's', 't');
const u32 FOURCC_BLACK = MP4_FOUR_CHAR_CODE('b', 'l', 'c', 'k');

/**
 * @brief Helper function to add group description with the payload of a given string
 *
 * @param media media to add the description to
 * @param fcc grouping_type
 * @param strDescription Playload string
 * @param [out] idx Index of the group entry
 * @return ISOErr error code
 */
ISOErr addGroupDescription(MP4Media media, u32 fcc, std::string strDescription, u32 &idx)
{
  ISOErr err;
  ISOHandle descrH;
  ISONewHandle(strDescription.length() * sizeof(char), &descrH);
  std::memcpy(*descrH, strDescription.data(), strDescription.length() * sizeof(char));

  err = ISOAddGroupDescription(media, fcc, descrH, &idx);

  ISODisposeHandle(descrH);
  return err;
}

/**
 * @brief Helper function to add samples given the color pattern
 *
 * @param media media to add the sample to
 * @param strPattern color pattern of samples (r,b,g,y,w,k)
 * @param repeatPattern number of times to repeat the pattern. No samples are added if this is 0
 * @param sampleEntryH sample entry handle (for the first call)
 * @return ISOErr error code
 */
ISOErr addSamples(MP4Media media, std::string strPattern, u32 repeatPattern = 1,
                  ISOHandle sampleEntryH = 0)
{
  ISOErr err;
  u32 sampleCount = 0;
  ISOHandle sampleDataH, durationsH, sizesH;
  err = ISONewHandle(sizeof(u32), &durationsH);
  CHECK(err == ISONoErr);

  *((u32 *)*durationsH) = TIMESCALE / FPS;

  std::vector<u8> bufferData;
  std::vector<u32> bufferSizes;
  for(std::string::const_iterator it = strPattern.cbegin(); it != strPattern.cend(); ++it)
  {
    switch(*it)
    {
    case 'r':
      bufferData.insert(bufferData.end(), HEVC::auRed, HEVC::auRed + sizeof(HEVC::auRed));
      bufferSizes.push_back(sizeof(HEVC::auRed));
      break;
    case 'b':
      bufferData.insert(bufferData.end(), HEVC::auBlue, HEVC::auBlue + sizeof(HEVC::auBlue));
      bufferSizes.push_back(sizeof(HEVC::auBlue));
      break;
    case 'g':
      bufferData.insert(bufferData.end(), HEVC::auGreen, HEVC::auGreen + sizeof(HEVC::auGreen));
      bufferSizes.push_back(sizeof(HEVC::auGreen));
      break;
    case 'y':
      bufferData.insert(bufferData.end(), HEVC::auYellow, HEVC::auYellow + sizeof(HEVC::auYellow));
      bufferSizes.push_back(sizeof(HEVC::auYellow));
      break;
    case 'w':
      bufferData.insert(bufferData.end(), HEVC::auWhite, HEVC::auWhite + sizeof(HEVC::auWhite));
      bufferSizes.push_back(sizeof(HEVC::auWhite));
      break;
    case 'k':
      bufferData.insert(bufferData.end(), HEVC::auBlack, HEVC::auBlack + sizeof(HEVC::auBlack));
      bufferSizes.push_back(sizeof(HEVC::auBlack));
      break;
    default:
      break;
    }
  }

  // repeat pattern
  std::vector<u8> bufferDataPattern   = bufferData;
  std::vector<u32> bufferSizesPattern = bufferSizes;
  std::string fullPattern             = strPattern;
  for(u32 n = 1; n < repeatPattern; ++n)
  {
    bufferData.insert(bufferData.end(), bufferDataPattern.begin(), bufferDataPattern.end());
    bufferSizes.insert(bufferSizes.end(), bufferSizesPattern.begin(), bufferSizesPattern.end());
    fullPattern += strPattern;
  }

  // create handles and copy data
  err = ISONewHandle(bufferData.size() * sizeof(u8), &sampleDataH);
  CHECK(err == ISONoErr);
  std::memcpy((*sampleDataH), bufferData.data(), bufferData.size() * sizeof(u8));
  err = ISONewHandle(sizeof(u32) * bufferSizes.size(), &sizesH);
  CHECK(err == ISONoErr);
  for(u32 n = 0; n < bufferSizes.size(); n++)
  {
    ((u32 *)*sizesH)[n] = bufferSizes[n];
  }

  err = ISOAddMediaSamples(media, sampleDataH, bufferSizesPattern.size() * repeatPattern,
                           durationsH, sizesH, sampleEntryH, 0, 0);
  CHECK(err == ISONoErr);

  err = ISODisposeHandle(sampleDataH);
  CHECK(err == ISONoErr);
  err = ISODisposeHandle(durationsH);
  CHECK(err == ISONoErr);
  err = ISODisposeHandle(sizesH);
  CHECK(err == ISONoErr);
  return err;
}

/**
 * @brief Helper function to map samples to groups based on pattern
 *
 * @param media media to map sample in
 * @param strPattern color pattern same as used in addSamples
 * @param idRed The index of the Red entry in the COLOR group
 * @param idBlue The index of the Blue entry in the COLOR group
 * @param idGreen The index of the Green entry in the COLOR group
 * @param idYellow The index of the Yellow entry in the COLOR group
 * @param repeatPattern number of times to repeat the pattern. No samples are mapped if this is 0
 * @return ISOErr error code
 */
ISOErr mapSamplesToGroups(MP4Media media, std::string strPattern, u32 idRed, u32 idBlue,
                          u32 idGreen, u32 idYellow, u32 repeatPattern = 1)
{
  ISOErr err              = ISONoErr;
  std::string fullPattern = strPattern;
  for(u32 n = 1; n < repeatPattern; ++n)
  {
    fullPattern += strPattern;
  }

  // map samples to groups
  if(repeatPattern > 0)
  {
    for(u32 n = 0; n < fullPattern.size(); ++n)
    {
      switch(fullPattern[n])
      {
      case 'r':
        err = ISOMapSamplestoGroup(media, FOURCC_COLOR, idRed, n, 1);
        CHECK(err == ISONoErr);
        break;
      case 'b':
        err = ISOMapSamplestoGroup(media, FOURCC_COLOR, idBlue, n, 1);
        CHECK(err == ISONoErr);
        break;
      case 'g':
        err = ISOMapSamplestoGroup(media, FOURCC_COLOR, idGreen, n, 1);
        CHECK(err == ISONoErr);
        break;
      case 'y':
        err = ISOMapSamplestoGroup(media, FOURCC_COLOR, idYellow, n, 1);
        CHECK(err == ISONoErr);
        break;
      default:
        break;
      }
    }
  }
  return err;
}

/**
 * @brief Helper function to check a single sample payload
 *
 * @param media media to take a sample from
 * @param sampleNr sample number in that media
 * @param comparePtr pointer to data to compare out payload to
 * @param compareSize size of the compare data
 * @return ISOErr error code
 */
ISOErr checkSample(MP4Media media, u32 sampleNr, u8 *comparePtr, u32 compareSize)
{
  ISOErr err;
  MP4Handle sampleH;
  u32 outSize, outSampleFlags, outSampleDescIndex;
  u64 outDTS, outDuration;
  s32 outCTSOffset;

  MP4NewHandle(0, &sampleH);
  err = MP4GetIndMediaSample(media, sampleNr, sampleH, &outSize, &outDTS, &outCTSOffset,
                             &outDuration, &outSampleFlags, &outSampleDescIndex);
  CHECK(err == ISONoErr);

  u32 handleSize;
  MP4GetHandleSize(sampleH, &handleSize);
  if(handleSize != outSize) return -1;
  if(outSize != compareSize) return -1;

  int compareVal = std::memcmp(comparePtr, *sampleH, compareSize);
  if(compareVal != 0) return -1;

  MP4DisposeHandle(sampleH);
  return err;
}

/**
 * @brief Helper function to check all samples in the given file
 *
 * @param strFile input file to check
 * @return ISOErr error code
 */
ISOErr checkSamples(std::string strFile)
{
  ISOErr err;
  ISOMovie moov;
  ISOTrack trak;
  ISOMedia media;
  err = ISOOpenMovieFile(&moov, strFile.c_str(), MP4OpenMovieNormal);

  err                 = ISOGetMovieIndTrack(moov, 1, &trak);
  err                 = ISOGetTrackMedia(trak, &media);
  u32 sampleCnt       = 0;
  u32 colorEntryCount = 0;
  u32 blackEntryCount = 0;
  u32 *sampleNumbers;
  u32 groupIdRed    = 0;
  u32 groupIdBlue   = 0;
  u32 groupIdGreen  = 0;
  u32 groupIdYellow = 0;
  u32 groupIdBlack  = 0;

  // parse the entries of the COLOR type and get the indexes
  err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
  CHECK(err == ISONoErr);
  CHECK(4 == colorEntryCount);
  for(u32 n = 0; n < colorEntryCount; ++n)
  {
    MP4Handle entryH;
    u32 size = 0;
    MP4NewHandle(0, &entryH);
    err = ISOGetGroupDescription(media, FOURCC_COLOR, n + 1, entryH);
    CHECK(err == ISONoErr);
    MP4GetHandleSize(entryH, &size);
    if(size == 0) return -1;

    std::string entryString(*entryH, size);
    if(entryString == "Red frames") groupIdRed = n + 1;
    if(entryString == "Blue frames") groupIdBlue = n + 1;
    if(entryString == "Green frames") groupIdGreen = n + 1;
    if(entryString == "Yellow frames") groupIdYellow = n + 1;
    MP4DisposeHandle(entryH);
  }
  CHECK(groupIdRed != 0);
  CHECK(groupIdBlue != 0);
  CHECK(groupIdGreen != 0);
  CHECK(groupIdYellow != 0);
  // parse the entries of the BLACK type and get the indexes
  err = ISOGetGroupDescriptionEntryCount(media, FOURCC_BLACK, &blackEntryCount);
  CHECK(err == ISONoErr);
  CHECK(1 == blackEntryCount);
  for(u32 n = 0; n < blackEntryCount; ++n)
  {
    MP4Handle entryH;
    u32 size = 0;
    MP4NewHandle(0, &entryH);
    err = ISOGetGroupDescription(media, FOURCC_BLACK, n + 1, entryH);
    CHECK(err == ISONoErr);
    MP4GetHandleSize(entryH, &size);
    if(size == 0) return -1;

    std::string entryString(*entryH, size);
    if(entryString == "Single black frame") groupIdBlack = n + 1;
    MP4DisposeHandle(entryH);
  }
  CHECK(groupIdBlack != 0);

  err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdRed, &sampleNumbers, &sampleCnt);
  CHECK(err == ISONoErr);
  CHECK(sampleCnt == 5);
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], HEVC::auRed, sizeof(HEVC::auRed));
    CHECK(err == ISONoErr);
  }
  err =
      ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdBlue, &sampleNumbers, &sampleCnt);
  CHECK(sampleCnt == 5);
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], HEVC::auBlue, sizeof(HEVC::auBlue));
    CHECK(err == ISONoErr);
  }
  err =
      ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdGreen, &sampleNumbers, &sampleCnt);
  CHECK(sampleCnt == 4);
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], HEVC::auGreen, sizeof(HEVC::auGreen));
    CHECK(err == ISONoErr);
  }
  err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdYellow, &sampleNumbers,
                                       &sampleCnt);
  CHECK(sampleCnt == 4);
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], HEVC::auYellow, sizeof(HEVC::auYellow));
    CHECK(err == ISONoErr);
  }
  err = ISOGetSampleGroupSampleNumbers(media, FOURCC_BLACK, groupIdBlack, &sampleNumbers,
                                       &sampleCnt);
  CHECK(sampleCnt == 1);
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], HEVC::auBlack, sizeof(HEVC::auBlack));
    CHECK(err == ISONoErr);
  }
  return err;
}

/**
 * @brief Starting point for this testing case
 *
 */
TEST_CASE("Test sample groups")
{
  std::string strFrag          = "test_samplegroups_fragmeted.mp4";
  std::string strDefragNormal  = "test_samplegroups_defrag_normal.mp4";
  std::string strDefragCompact = "test_samplegroups_defrag_compact.mp4";
  std::string strDefragAuto    = "test_samplegroups_defrag_auto.mp4";

  /**
   * @brief Create a Fragmented File with different sample groups for testing
   *
   * COLOR gropuing with: RED (r), BLUE (b), GREEN (g) and YELLOW (y)
   * Additional frames without groups: WHITE (w) and BLACK (k)
   *
   * Moovie:      Global group description for COLOR RED and BLUE in stbl
   *
   * Fragment 1:  local group description COLOR RED (not used)
   *              local group description TEST with 2 entries
   * Fragment 2:  local group description COLOR GREEN and YELLOW (both used in COMPACT
   *              sampleToGroupBox)
   *              local group description TEST with 2 entries (one of these entries is the same as
   *              in Fragment 1)
   * Fragment 3:  no group description
   * Fragment 4:  local group descrition COLOR GREEN & YELLOW (both used in NORMAL sampleToGroupBox)
   * Fragment 5:  local group description BLACK + mapping a single sample to it (SampleIdx=3)
   *
   * Samples: red (r), blue(b), green (g), yellow (y), white (w), black (k)
   * Groups: global (*), local (.), none ( )
   * |  frag. 1  |  frag. 2  |  frag. 3  |  frag. 4  |  frag. 5  |
   * |r|b|r|b|r|b|g|r|y|g|r|y|w|k|w|k|w|k|b|g|y|b|g|y|w|k|w|k|w|k|
   * |           |           |           |           |           |
   * |*|*|*|*|*|*|.|*|.|.|*|.| | | | | | |*|.|.|*|.|.| | | |.| | |
   *
   */
  SECTION("Check Fragmented file creation with sample groups")
  {
    ISOErr err;
    ISOMovie moov;
    ISOMedia media;
    ISOTrack trak;

    u32 colorEntryCount = 0;
    u32 groupIdRed      = 0;
    u32 groupIdBlue     = 0;
    u32 groupIdGreen    = 0;
    u32 groupIdYellow   = 0;

    ISOHandle spsHandle, ppsHandle, vpsHandle, sampleEntryH;
    err = MP4NewHandle(sizeof(HEVC::SPS), &spsHandle);
    std::memcpy((*spsHandle), HEVC::SPS, sizeof(HEVC::SPS));
    err = MP4NewHandle(sizeof(HEVC::PPS), &ppsHandle);
    std::memcpy((*ppsHandle), HEVC::PPS, sizeof(HEVC::PPS));
    err = MP4NewHandle(sizeof(HEVC::VPS), &vpsHandle);
    std::memcpy((*vpsHandle), HEVC::VPS, sizeof(HEVC::VPS));
    err = MP4NewHandle(0, &sampleEntryH);

    err = MP4NewMovie(&moov, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
    REQUIRE(err == ISONoErr);

    err = ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
    REQUIRE(err == ISONoErr);
    err = MP4AddTrackToMovieIOD(trak);
    CHECK(err == ISONoErr);
    err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, TIMESCALE, NULL);
    REQUIRE(err == ISONoErr);

    err = ISOBeginMediaEdits(media);
    err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 1, spsHandle, ppsHandle, vpsHandle);
    REQUIRE(err == ISONoErr);

    err = ISOSetTrackFragmentDefaults(trak, TIMESCALE / FPS, sizeof(HEVC::auBlue), 1, 0);

    err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
    CHECK(err != ISONoErr);
    CHECK(colorEntryCount == 0);
    // red and blue are in stbl
    err = addGroupDescription(media, FOURCC_COLOR, "Red frames", groupIdRed);
    CHECK(err == ISONoErr);
    err = addGroupDescription(media, FOURCC_COLOR, "Blue frames", groupIdBlue);
    CHECK(err == ISONoErr);

    err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
    CHECK(err == ISONoErr);
    CHECK(colorEntryCount == 2);

    u32 temp = 0;
    err      = addGroupDescription(media, FOURCC_COLOR, "Red frames", temp);
    // this must fail because "Red frames" payload is already added with the same type
    CHECK(err != ISONoErr);

    // just add sample entry, call addSamples with sample count = 0
    err = addSamples(media, "r", 0, sampleEntryH);
    CHECK(err == ISONoErr);
    err = MP4EndMediaEdits(media);
    CHECK(err == ISONoErr);

    // Fragment 1
    err = ISOStartMovieFragment(moov);
    CHECK(err == ISONoErr);
    err = ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
    CHECK(err == ISONoErr);
    err = addSamples(media, "rb", 3);
    CHECK(err == ISONoErr);
    colorEntryCount = 0;
    err             = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
    CHECK(err != ISONoErr); // there sould be no entries in this fragment yet
    CHECK(colorEntryCount == 0);
    err = addGroupDescription(media, FOURCC_TEST, "Duplicate test", temp);
    CHECK(err == ISONoErr);
    err = addGroupDescription(media, FOURCC_COLOR, "Red frames", temp);
    CHECK(err == ISONoErr); // this must pass even if the same type and payload is in stbl
                            // (but it shall not be in defragmented file)
    err = addGroupDescription(media, FOURCC_TEST, "Test", temp);
    CHECK(err == ISONoErr);
    err = addGroupDescription(media, FOURCC_TEST, "Test", temp);
    CHECK(err != ISONoErr); // this must fail because same type and payload already added
    err = mapSamplesToGroups(media, "rb", groupIdRed, groupIdBlue, groupIdGreen, groupIdYellow, 3);
    CHECK(err == ISONoErr);

    err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
    CHECK(err == ISONoErr); // there should be now exactly one entry
    CHECK(colorEntryCount == 1);

    // Fragment 2
    err = ISOStartMovieFragment(moov);
    ISOSetSamplestoGroupType(media, SAMPLE_GROUP_COMPACT);
    // local green and yellow groups
    err = addGroupDescription(media, FOURCC_COLOR, "Green frames", groupIdGreen);
    CHECK(err == ISONoErr);
    err = addGroupDescription(media, FOURCC_COLOR, "Yellow frames", groupIdYellow);
    CHECK(err == ISONoErr);
    err = addSamples(media, "gry", 2);
    CHECK(err == ISONoErr);
    err = addGroupDescription(media, FOURCC_TEST, "Unique entry", temp);
    CHECK(err == ISONoErr);
    err = addGroupDescription(media, FOURCC_TEST, "Duplicate test", temp);
    CHECK(err == ISONoErr);
    err = mapSamplesToGroups(media, "gry", groupIdRed, groupIdBlue, groupIdGreen, groupIdYellow, 2);
    CHECK(err == ISONoErr);

    // Fragment 3
    err = ISOStartMovieFragment(moov);
    CHECK(err == ISONoErr);
    ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
    err = addSamples(media, "wk", 3);
    CHECK(err == ISONoErr);

    // Fragment 4
    err = ISOStartMovieFragment(moov);
    ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
    // local green and yellow groups
    err = addGroupDescription(media, FOURCC_COLOR, "Green frames", groupIdGreen);
    CHECK(err == ISONoErr);
    err = addGroupDescription(media, FOURCC_COLOR, "Yellow frames", groupIdYellow);
    CHECK(err == ISONoErr);
    err = addSamples(media, "bgy", 2);
    CHECK(err == ISONoErr);
    err = mapSamplesToGroups(media, "bgy", groupIdRed, groupIdBlue, groupIdGreen, groupIdYellow, 2);
    CHECK(err == ISONoErr);

    // Fragment 5
    err = ISOStartMovieFragment(moov);
    CHECK(err == ISONoErr);
    ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
    err = addSamples(media, "wk", 3);
    CHECK(err == ISONoErr);
    u32 groupIDBlack = 0;
    err = addGroupDescription(media, FOURCC_BLACK, "Single black frame", groupIDBlack);
    CHECK(err == ISONoErr);
    err = ISOMapSamplestoGroup(media, FOURCC_BLACK, groupIDBlack, 3, 1);
    CHECK(err == ISONoErr);

    err = MP4WriteMovieToFile(moov, strFrag.c_str());
    CHECK(err == ISONoErr);
  }

  /**
   * @brief Create different flavors of defragmented files from the fragmented file
   *
   * create Defragmented file with NORMAL SampleToGroupBox
   * create Defragmented file with COMPACT SampleToGroupBox
   * create Defragmented file with AUTO SampleToGroupBox (smaller size is used automatically)
   *
   */
  SECTION("Check defragmentation")
  {
    ISOErr err;
    ISOMovie moov;
    ISOTrack trak;
    ISOMedia media;
    err = ISOOpenMovieFile(&moov, strFrag.c_str(), MP4OpenMovieNormal);
    REQUIRE(err == ISONoErr);

    err = ISOGetMovieIndTrack(moov, 1, &trak);
    REQUIRE(err == ISONoErr);
    err = ISOGetTrackMedia(trak, &media);
    REQUIRE(err == ISONoErr);

    // write file with compact sample groups
    err = ISOSetSamplestoGroupType(media, SAMPLE_GROUP_COMPACT);
    CHECK(err == ISONoErr);
    err = MP4WriteMovieToFile(moov, strDefragCompact.c_str());
    CHECK(err == ISONoErr);

    // write file with normal sample groups
    err = ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
    CHECK(err == ISONoErr);
    err = MP4WriteMovieToFile(moov, strDefragNormal.c_str());
    CHECK(err == ISONoErr);

    // write file with auto sample groups
    err = ISOSetSamplestoGroupType(media, SAMPLE_GROUP_AUTO);
    CHECK(err == ISONoErr);
    err = MP4WriteMovieToFile(moov, strDefragAuto.c_str());
    CHECK(err == ISONoErr);
  }

  /**
   * @brief Check parse all files, find the COLOR group ids, extract all samples and check if all
   * samples are correclty extracted (check the payload of every extracted sample as well)
   *
   */
  SECTION("Check all samples in fragmented and defragmented files")
  {
    ISOErr err;
    err = checkSamples(strFrag);
    CHECK(err == ISONoErr);
    err = checkSamples(strDefragNormal);
    CHECK(err == ISONoErr);
    err = checkSamples(strDefragCompact);
    CHECK(err == ISONoErr);
    err = checkSamples(strDefragAuto);
    CHECK(err == ISONoErr);
  }
}
