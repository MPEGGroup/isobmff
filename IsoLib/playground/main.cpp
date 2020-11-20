/**
 * @file main.cpp
 * @author Dimitri Podborski
 * @brief Writes a HEVC mp4 file with sample groups accoring to colour pattern
 * @version 0.1
 * @date 2020-10-26
 *
 * @copyright Copyright (c) 2020
 *
 */
#include <ISOMovies.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

extern "C"
{
  MP4_EXTERN(MP4Err)
  ISONewHEVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                              u32 dataReferenceIndex, u32 length_size, MP4Handle first_sps,
                              MP4Handle first_pps, MP4Handle first_spsext);
}

u8 VPS[] = {0x40, 0x01, 0x0C, 0x01, 0xFF, 0xFF, 0x04, 0x08, 0x00, 0x00, 0x03, 0x00,
            0x9F, 0xA8, 0x00, 0x00, 0x03, 0x00, 0x00, 0x1E, 0xBA, 0x02, 0x40};

u8 SPS[] = {0x42, 0x01, 0x01, 0x04, 0x08, 0x00, 0x00, 0x03, 0x00, 0x9F, 0xA8, 0x00, 0x00,
            0x03, 0x00, 0x00, 0x1E, 0xA0, 0x20, 0x83, 0x16, 0x5B, 0xAB, 0x93, 0x2B, 0x9A,
            0x02, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x03, 0x00, 0x32, 0x10};

u8 PPS[] = {0x44, 0x01, 0xC1, 0x73, 0xC0, 0x89};

u8 auRed[] = {0x00, 0x00, 0x00, 0x16, 0x28, 0x01, 0xAF, 0x78, 0xF7, 0x04, 0x03, 0xFF, 0xDB,
              0xA3, 0xFF, 0xED, 0x27, 0xD2, 0xF6, 0xC3, 0x94, 0x40, 0x83, 0xC0, 0x00, 0x78};

u8 auBlue[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3F,
               0x9C, 0x43, 0xFF, 0xFA, 0x87, 0x32, 0xAF, 0xFC, 0x5D, 0xFF,
               0xFF, 0xAE, 0x1D, 0xB9, 0xA2, 0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auGreen[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3F,
                0x9C, 0x43, 0xFF, 0xF5, 0x9F, 0x1F, 0xFF, 0xD8, 0x3B, 0xFF,
                0xFD, 0xF0, 0xF5, 0xB9, 0xA2, 0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auYellow[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xA0, 0x3F,
                 0x9C, 0x43, 0x3C, 0xFA, 0x51, 0x1D, 0xFF, 0xFC, 0x5D, 0xFE,
                 0xCB, 0xAE, 0x1D, 0xB9, 0xA2, 0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auWhite[] = {0x00, 0x00, 0x00, 0x13, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3C, 0x64, 0x00,
                0xE7, 0x9F, 0x6C, 0x07, 0x79, 0x0D, 0x1B, 0xFD, 0x7D, 0x7C, 0x87};

u8 auBlack[] = {0x00, 0x00, 0x00, 0x13, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3C, 0x64, 0x00,
                0xFF, 0xFF, 0x72, 0xCA, 0x19, 0x0D, 0x1B, 0xFD, 0x7D, 0x7C, 0x87};

const u32 TIMESCALE = 30000;
const u32 FPS       = 30;

const u32 FOURCC_COLOR = MP4_FOUR_CHAR_CODE('c', 'o', 'l', 'r');
const u32 FOURCC_TEST  = MP4_FOUR_CHAR_CODE('t', 'e', 's', 't');

ISOErr addSamples(MP4Media media, std::string strPattern, u32 repeatPattern = 1,
                  ISOHandle sampleEntryH = 0)
{
  ISOErr err;
  u32 sampleCount = 0;
  ISOHandle sampleDataH, durationsH, sizesH;
  err = ISONewHandle(sizeof(u32), &durationsH);
  if(err) return err;

  *((u32 *)*durationsH) = TIMESCALE / FPS;

  std::vector<u8> bufferData;
  std::vector<u32> bufferSizes;
  for(std::string::const_iterator it = strPattern.cbegin(); it != strPattern.cend(); ++it)
  {
    switch(*it)
    {
    case 'r':
      bufferData.insert(bufferData.end(), auRed, auRed + sizeof(auRed));
      bufferSizes.push_back(sizeof(auRed));
      break;
    case 'b':
      bufferData.insert(bufferData.end(), auBlue, auBlue + sizeof(auBlue));
      bufferSizes.push_back(sizeof(auBlue));
      break;
    case 'g':
      bufferData.insert(bufferData.end(), auGreen, auGreen + sizeof(auGreen));
      bufferSizes.push_back(sizeof(auGreen));
      break;
    case 'y':
      bufferData.insert(bufferData.end(), auYellow, auYellow + sizeof(auYellow));
      bufferSizes.push_back(sizeof(auYellow));
      break;
    case 'w':
      bufferData.insert(bufferData.end(), auWhite, auWhite + sizeof(auWhite));
      bufferSizes.push_back(sizeof(auWhite));
      break;
    case 'k':
      bufferData.insert(bufferData.end(), auBlack, auBlack + sizeof(auBlack));
      bufferSizes.push_back(sizeof(auBlack));
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
  if(err) return err;
  std::memcpy((*sampleDataH), bufferData.data(), bufferData.size() * sizeof(u8));
  err = ISONewHandle(sizeof(u32) * bufferSizes.size(), &sizesH);
  if(err) return err;
  for(u32 n = 0; n < bufferSizes.size(); n++)
  {
    ((u32 *)*sizesH)[n] = bufferSizes[n];
  }

  err = ISOAddMediaSamples(media, sampleDataH, bufferSizesPattern.size() * repeatPattern,
                           durationsH, sizesH, sampleEntryH, 0, 0);
  if(err) return err;

  err = ISODisposeHandle(sampleDataH);
  if(err) return err;
  err = ISODisposeHandle(durationsH);
  if(err) return err;
  err = ISODisposeHandle(sizesH);
  if(err) return err;
  return err;
}

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

ISOErr mapSamplesToGroups(MP4Media media, std::string strPattern, u32 idRed, u32 idBlue, u32 idGreen, u32 idYellow, u32 repeatPattern = 1)
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
        if(err) return err;
        break;
      case 'b':
        err = ISOMapSamplestoGroup(media, FOURCC_COLOR, idBlue, n, 1);
        if(err) return err;
        break;
      case 'g':
        err = ISOMapSamplestoGroup(media, FOURCC_COLOR, idGreen, n, 1);
        if(err) return err;
        break;
      case 'y':
        err = ISOMapSamplestoGroup(media, FOURCC_COLOR, idYellow, n, 1);
        if(err) return err;
        break;
      default:
        break;
      }
    }
  }
  return err;
}

/**
 * @brief Create a Fragmented File with different sample groups for testing
 * 
 * Moovie:      Global group description for COLOR RED and BLUE in stbl
 * 
 * Fragment 1:  local group description COLOR RED (not used)
 *              local group description TEST with 2 entries
 * Fragment 2:  local group description COLOR GREEN and YELLOW (both used in COMPACT sampleToGroupBox)
 *              local group description TEST with 2 entries (one of these entries is the same as in Fragment 1)
 * Fragment 3:  no group description
 * Fragment 4:  local group descrition COLOR GREEN and YELLOW (both used in NORMAL sampleToGroupBox)
 * Fragment 5:  no group description
 * 
 * Samples: red (r), blue(b), green (g), yellow (y), white (w), black (k)
 * |  frag. 1  |  frag. 2  |  frag. 3  |  frag. 4  |
 * |r|b|r|b|r|b|g|r|y|g|r|y|b|g|y|b|g|y|w|k|w|k|w|k|
 * 
 * @param strFilename output filename
 * @return ISOErr error code
 */
ISOErr createFragmentedFile(std::string strFilename)
{
  ISOErr err;
  ISOMovie moov;
  ISOMedia media;
  ISOTrack trak;

  u32 colorEntryCount = 0;
  u32 groupIdRed    = 0;
  u32 groupIdBlue   = 0;
  u32 groupIdGreen  = 0;
  u32 groupIdYellow = 0;

  ISOHandle spsHandle, ppsHandle, vpsHandle, sampleEntryH;
  err = MP4NewHandle(sizeof(SPS), &spsHandle);
  std::memcpy((*spsHandle), SPS, sizeof(SPS));
  err = MP4NewHandle(sizeof(PPS), &ppsHandle);
  std::memcpy((*ppsHandle), PPS, sizeof(PPS));
  err = MP4NewHandle(sizeof(VPS), &vpsHandle);
  std::memcpy((*vpsHandle), VPS, sizeof(VPS));
  err = MP4NewHandle(0, &sampleEntryH);

  err = MP4NewMovie(&moov, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
  if(err) return err;

  err = ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak);
  if(err) return err;
  err = MP4AddTrackToMovieIOD(trak);
  if(err) return err;
  err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, TIMESCALE, NULL);
  if(err) return err;

  err = ISOBeginMediaEdits(media);
  err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 1, spsHandle, ppsHandle, vpsHandle);
  if(err) return -4;

  err = ISOSetTrackFragmentDefaults(trak, TIMESCALE / FPS, sizeof(auBlue), 1, 0);

  err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
  if(err == MP4NoErr) return -1;
  if(colorEntryCount != 0) return -1;
  // red and blue are in stbl
  err = addGroupDescription(media, FOURCC_COLOR, "Red frames", groupIdRed);   if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Blue frames", groupIdBlue); if(err) return err;

  err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
  if(err) return err;
  if(colorEntryCount != 2) return -1;
  
  u32 temp = 0;
  err      = addGroupDescription(media, FOURCC_COLOR, "Red frames", temp);
  if(err == MP4NoErr)
    return -5; // this must fail because "Red frames" payload is already added with the same type

  // just add sample entry, call addSamples with sample count = 0
  err = addSamples(media, "r", 0, sampleEntryH);
  if(err) return err;
  err = MP4EndMediaEdits(media);

  // Fragment 1
  err = ISOStartMovieFragment(moov);
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
  if(err) return err;
  err = addSamples(media, "rb", 3);
  if(err) return err;
  colorEntryCount = 0;
  err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
  if(err == MP4NoErr) return -1; // there sould be no entries in this fragment yet
  if(colorEntryCount != 0) return -1;
  err = addGroupDescription(media, FOURCC_TEST, "Duplicate test", temp);
  if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Red frames", temp);
  if(err) return err; // this must pass even if the same type and payload is in stbl 
                      // (but it shall not be in defragmented file)
  err = addGroupDescription(media, FOURCC_TEST, "Test", temp);
  if(err) return err;
  err = addGroupDescription(media, FOURCC_TEST, "Test", temp);
  if(!err) return -5; // this must fail because same type and payload already added
  err = mapSamplesToGroups(media, "rb", groupIdRed, groupIdBlue, groupIdGreen, groupIdYellow, 3);
  if(err) return err;

  err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
  if(err) return err; // there should be now exactly one entry
  if(colorEntryCount != 1) return -1;

  // Fragment 2
  err = ISOStartMovieFragment(moov);
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_COMPACT);
  // local green and yellow groups
  err = addGroupDescription(media, FOURCC_COLOR, "Green frames", groupIdGreen);   if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Yellow frames", groupIdYellow); if(err) return err;
  err = addSamples(media, "gry", 2);
  if(err) return err;
  err = addGroupDescription(media, FOURCC_TEST, "Unique entry",
                            temp);
  if(err) return err;
  err = addGroupDescription(media, FOURCC_TEST, "Duplicate test", temp);
  if(err) return err;
  err = mapSamplesToGroups(media, "gry", groupIdRed, groupIdBlue, groupIdGreen, groupIdYellow, 2);
  if(err) return err;

  // Fragment 3
  err = ISOStartMovieFragment(moov);
  if(err) return err;
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
  err = addSamples(media, "wk", 3);
  if(err) return err;

  // Fragment 4
  err = ISOStartMovieFragment(moov);
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
  // local green and yellow groups
  err = addGroupDescription(media, FOURCC_COLOR, "Green frames", groupIdGreen);   if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Yellow frames", groupIdYellow); if(err) return err;
  err = addSamples(media, "bgy", 2);
  if(err) return err;
  err = mapSamplesToGroups(media, "bgy", groupIdRed, groupIdBlue, groupIdGreen, groupIdYellow, 2);
  if(err) return err;

  // Fragment 5
  err = ISOStartMovieFragment(moov);
  if(err) return err;
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
  err = addSamples(media, "wk", 3);
  if(err) return err;

  err = MP4WriteMovieToFile(moov, strFilename.c_str());
  return err;
}

/**
 * @brief Create different flavors of defragmented files from the fragmented file
 * 
 * @param inputFrag Input fragmented file
 * @param outputDefragNorm Defragmented file with NORMAL SampleToGroupBox
 * @param outputDefragComp Defragmented file with COMPACT SampleToGroupBox
 * @param outputDefragAuto Defragmented file with AUTO SampleToGroupBox (smaller size is used automatically)
 * @return ISOErr 
 */
ISOErr defragmentFile(std::string inputFrag, std::string outputDefragNorm, std::string outputDefragComp, std::string outputDefragAuto)
{
  ISOErr err;
  ISOMovie moov;
  ISOTrack trak;
  ISOMedia media;
  err = ISOOpenMovieFile(&moov, inputFrag.c_str(), MP4OpenMovieNormal);
  if(err) return err;

  err = ISOGetMovieIndTrack(moov, 1, &trak);
  if(err) return err;
  err = ISOGetTrackMedia(trak, &media);
  if(err) return err;

  // write file with compact sample groups
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_COMPACT);
  err = MP4WriteMovieToFile(moov, outputDefragComp.c_str());
  if(err) return err;

  // write file with normal sample groups
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_NORMAL);
  err = MP4WriteMovieToFile(moov, outputDefragNorm.c_str());
  if(err) return err;

  // write file with auto sample groups
  ISOSetSamplestoGroupType(media, SAMPLE_GROUP_AUTO);
  err = MP4WriteMovieToFile(moov, outputDefragAuto.c_str());
  return err;
}

ISOErr checkSample(MP4Media media, u32 sampleNr, u8 *comparePtr, u32 compareSize)
{
  ISOErr err;
  MP4Handle sampleH;
  u32 outSize, outSampleFlags, outSampleDescIndex;
  u64 outDTS, outDuration;
  s32 outCTSOffset;

  MP4NewHandle(0, &sampleH);
  err = MP4GetIndMediaSample(media, sampleNr, sampleH, &outSize, &outDTS, &outCTSOffset, &outDuration, &outSampleFlags, &outSampleDescIndex);
  if(err) return err;

  u32 handleSize;
  MP4GetHandleSize(sampleH, &handleSize);
  if(handleSize != outSize) return -1;
  if(outSize != compareSize) return -1;

  int compareVal = std::memcmp(comparePtr, *sampleH, compareSize);
  if(compareVal != 0) return -1;

  MP4DisposeHandle(sampleH);
  return err;
}

ISOErr checkSamples(std::string strFile)
{
  ISOErr err;
  ISOMovie moov;
  ISOTrack trak;
  ISOMedia media;
  err = ISOOpenMovieFile(&moov, strFile.c_str(), MP4OpenMovieNormal);

  err = ISOGetMovieIndTrack(moov, 1, &trak);
  err = ISOGetTrackMedia(trak, &media);
  u32 sampleCnt = 0;
  u32 colorEntryCount = 0;
  u32 *sampleNumbers;
  u32 groupIdRed = 0;
  u32 groupIdBlue = 0;
  u32 groupIdGreen = 0; 
  u32 groupIdYellow = 0;

  // parse the entries of the COLOR type and get the indexes 
  err = ISOGetGroupDescriptionEntryCount(media, FOURCC_COLOR, &colorEntryCount);
  if(err) return err;
  if(colorEntryCount != 4) return -1;
  for(u32 n=0; n<colorEntryCount; ++n)
  {
    MP4Handle entryH;
    u32 size = 0;
    MP4NewHandle(0, &entryH);
    err = ISOGetGroupDescription(media, FOURCC_COLOR, n+1, entryH);
    if(err) return err;
    MP4GetHandleSize(entryH, &size);
    if(size == 0) return -1;

    std::string entryString(*entryH, size);
    if(entryString == "Red frames") groupIdRed = n+1;
    if(entryString == "Blue frames") groupIdBlue = n+1;
    if(entryString == "Green frames") groupIdGreen = n+1;
    if(entryString == "Yellow frames") groupIdYellow = n+1;
    MP4DisposeHandle(entryH);
  }
  if(groupIdRed == 0) return -1;
  if(groupIdBlue == 0) return -1;
  if(groupIdGreen == 0) return -1;
  if(groupIdYellow == 0) return -1;
  

  err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdRed, &sampleNumbers, &sampleCnt);
  if(err) return err;
  if(sampleCnt != 5) return -1;
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], auRed, sizeof(auRed));
    if(err) return err;
  }
  err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdBlue, &sampleNumbers,
                                        &sampleCnt);
  if(sampleCnt != 5) return -1;
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], auBlue, sizeof(auBlue));
    if(err) return err;
  }
  err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdGreen, &sampleNumbers,
                                        &sampleCnt);
  if(sampleCnt != 4) return -1;
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], auGreen, sizeof(auGreen));
    if(err) return err;
  }
  err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdYellow, &sampleNumbers,
                                        &sampleCnt);
  if(sampleCnt != 4) return -1;
  for(u32 i = 0; i < sampleCnt; ++i)
  {
    err = checkSample(media, sampleNumbers[i], auYellow, sizeof(auYellow));
    if(err) return err;
  }
  return err;
}

int main()
{
  ISOErr err;
  std::string strFrag          = "test_samplegroups_fragmeted.mp4";
  std::string strDefragNormal  = "test_samplegroups_defrag_normal.mp4";
  std::string strDefragCompact = "test_samplegroups_defrag_compact.mp4";
  std::string strDefragAuto    = "test_samplegroups_defrag_auto.mp4";

  err = createFragmentedFile(strFrag);
  if(err) return err;

  err = defragmentFile(strFrag, strDefragNormal, strDefragCompact, strDefragAuto);
  if(err) return err;

  err = checkSamples(strFrag); if(err) return err;
  err = checkSamples(strDefragNormal);
  err = checkSamples(strDefragCompact);
  err = checkSamples(strDefragAuto);
  return err;
}
