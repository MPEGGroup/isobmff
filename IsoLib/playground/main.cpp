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

extern "C" {
MP4_EXTERN(MP4Err)
ISONewHEVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                            u32 dataReferenceIndex, u32 length_size,
                            MP4Handle first_sps, MP4Handle first_pps,
                            MP4Handle first_spsext);
}

u8 VPS[] = {0x40, 0x01, 0x0C, 0x01, 0xFF, 0xFF, 0x04, 0x08,
                       0x00, 0x00, 0x03, 0x00, 0x9F, 0xA8, 0x00, 0x00,
                       0x03, 0x00, 0x00, 0x1E, 0xBA, 0x02, 0x40};

u8 SPS[] = {0x42, 0x01, 0x01, 0x04, 0x08, 0x00, 0x00, 0x03,
                       0x00, 0x9F, 0xA8, 0x00, 0x00, 0x03, 0x00, 0x00,
                       0x1E, 0xA0, 0x20, 0x83, 0x16, 0x5B, 0xAB, 0x93,
                       0x2B, 0x9A, 0x02, 0x00, 0x00, 0x03, 0x00, 0x02,
                       0x00, 0x00, 0x03, 0x00, 0x32, 0x10};

u8 PPS[] = {0x44, 0x01, 0xC1, 0x73, 0xC0, 0x89};

u8 auRed[] = {0x00, 0x00, 0x00, 0x16, 0x28, 0x01, 0xAF, 0x78, 0xF7, 0x04,
              0x03, 0xFF, 0xDB, 0xA3, 0xFF, 0xED, 0x27, 0xD2, 0xF6,
              0xC3, 0x94, 0x40, 0x83, 0xC0, 0x00, 0x78};

u8 auBlue[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xE0,
                          0x3F, 0x9C, 0x43, 0xFF, 0xFA, 0x87, 0x32, 0xAF,
                          0xFC, 0x5D, 0xFF, 0xFF, 0xAE, 0x1D, 0xB9, 0xA2,
                          0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auGreen[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xE0,
                           0x3F, 0x9C, 0x43, 0xFF, 0xF5, 0x9F, 0x1F, 0xFF,
                           0xD8, 0x3B, 0xFF, 0xFD, 0xF0, 0xF5, 0xB9, 0xA2,
                           0xB4, 0xBC, 0x6D, 0x84, 0x5F};

u8 auYellow[] = {0x00, 0x00, 0x00, 0x1A, 0x28, 0x01, 0xAF, 0x0A, 0xA0,
                            0x3F, 0x9C, 0x43, 0x3C, 0xFA, 0x51, 0x1D, 0xFF,
                            0xFC, 0x5D, 0xFE, 0xCB, 0xAE, 0x1D, 0xB9, 0xA2,
                            0xB4, 0xBC, 0x6D, 0x84, 0x5F};


u8 auWhite[] = {
  0x00, 0x00, 0x00, 0x13, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3C, 0x64, 0x00, 0xE7, 0x9F, 0x6C, 0x07, 0x79, 0x0D, 0x1B, 0xFD, 0x7D, 0x7C, 0x87
};

u8 auBlack[] = {
  0x00, 0x00, 0x00, 0x13, 0x28, 0x01, 0xAF, 0x0A, 0xE0, 0x3C, 0x64, 0x00, 0xFF, 0xFF, 0x72, 0xCA, 0x19, 0x0D, 0x1B, 0xFD, 0x7D, 0x7C, 0x87
};

const u32 TIMESCALE = 30000;
const u32 FPS = 30;

const u32 FOURCC_RED = MP4_FOUR_CHAR_CODE('r', 'e', 'd', ' ');
const u32 FOURCC_BLU = MP4_FOUR_CHAR_CODE('b', 'l', 'u', 'e');
const u32 FOURCC_GRN = MP4_FOUR_CHAR_CODE('g', 'r', 'e', 'n');
const u32 FOURCC_YLW = MP4_FOUR_CHAR_CODE('y', 'e', 'l', 'w');

const u32 FOURCC_COLOR = MP4_FOUR_CHAR_CODE('c', 'o', 'l', 'r');
const u32 FOURCC_TEST  = MP4_FOUR_CHAR_CODE('t', 'e', 's', 't');

u32 groupIdRed = 0;
u32 groupIdBlue = 0;
u32 groupIdGreen = 0;
u32 groupIdYellow = 0;

ISOErr addSamples(MP4Media media, std::string strPattern, u32 repeatPattern = 1, ISOHandle sampleEntryH = 0) {
  ISOErr err;
  u32 sampleCount = 0;
  ISOHandle sampleDataH, durationsH, sizesH;
  err = ISONewHandle(sizeof(u32), &durationsH); if(err) return err;

  *((u32*)*durationsH) = TIMESCALE/FPS;

  std::vector<u8> bufferData;
  std::vector<u32> bufferSizes;
  for(std::string::const_iterator it=strPattern.cbegin(); it != strPattern.cend(); ++it) {
    switch (*it)
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
      std::cerr << "UNKNOWN char in pattern string: " << *it << std::endl;
      break;
    }
  }

  // repeat pattern
  std::vector<u8> bufferDataPattern = bufferData;
  std::vector<u32> bufferSizesPattern = bufferSizes;
  std::string fullPattern = strPattern;
  for(u32 n = 1; n<repeatPattern; ++n) {
    bufferData.insert(bufferData.end(), bufferDataPattern.begin(), bufferDataPattern.end());
    bufferSizes.insert(bufferSizes.end(), bufferSizesPattern.begin(), bufferSizesPattern.end());
    fullPattern += strPattern;
  }
  
  // create handles and copy data
  err = ISONewHandle(bufferData.size()*sizeof(u8), &sampleDataH); if(err) return err;
  memcpy((*sampleDataH), bufferData.data(), bufferData.size()*sizeof(u8));
  err = ISONewHandle( sizeof(u32)*bufferSizes.size(), &sizesH ); if(err) return err;
  for(u32 n=0; n<bufferSizes.size(); n++) {
    ((u32*) *sizesH)[n] = bufferSizes[n];
  }

  err = ISOAddMediaSamples(media, sampleDataH, bufferSizesPattern.size()*repeatPattern, durationsH, sizesH, sampleEntryH, 0, 0); if(err) return err;

  err = ISODisposeHandle(sampleDataH); if(err) return err;
  err = ISODisposeHandle(durationsH); if(err) return err;
  err = ISODisposeHandle(sizesH); if(err) return err;
  return err;
}

ISOErr addGroupDescription(MP4Media media, u32 fcc, std::string strDescription, u32 &idx)
{
  ISOErr err;
  ISOHandle descrH;
  ISONewHandle(strDescription.length()*sizeof(char), &descrH);
  memcpy(*descrH, strDescription.data(), strDescription.length()*sizeof(char));

  err = ISOAddGroupDescription(media, fcc, descrH, &idx);

  ISODisposeHandle(descrH);
  return err;
}

ISOErr addGroupDescriptions(MP4Media media) 
{
  ISOErr err = ISONoErr;
  err = addGroupDescription(media, FOURCC_COLOR, "Red frames", groupIdRed); if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Blue frames", groupIdBlue); if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Green frames", groupIdGreen); if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Yellow frames", groupIdYellow); if(err) return err;
  return err;
}

ISOErr mapSamplesToGroups(MP4Media media, std::string strPattern, u32 repeatPattern = 1) 
{
  ISOErr err = ISONoErr;
  std::string fullPattern = strPattern;
  for(u32 n = 1; n<repeatPattern; ++n) {
    fullPattern += strPattern;
  }

  // add sample groups
  std::cout << "fullPattern = " << fullPattern << std::endl;
  if(repeatPattern>0) {
    size_t nr = std::count(fullPattern.begin(), fullPattern.end(), 'r');
    size_t nb = std::count(fullPattern.begin(), fullPattern.end(), 'b');
    size_t ng = std::count(fullPattern.begin(), fullPattern.end(), 'g');
    size_t ny = std::count(fullPattern.begin(), fullPattern.end(), 'y');
    std::cout << "r = " << nr << std::endl;
    std::cout << "b = " << nb << std::endl;
    std::cout << "g = " << ng << std::endl;
    std::cout << "y = " << ny << std::endl;

    for(u32 n=0; n<fullPattern.size(); ++n){
      std::cout << fullPattern[n] << " ";
      switch (fullPattern[n])
      {
      case 'r':
        ISOMapSamplestoGroup(media, FOURCC_COLOR, groupIdRed, n, 1);
        std::cout << "RED  map sample_index=" << n << " to group_index=" << groupIdRed << std::endl;
        break;
      case 'b':
        ISOMapSamplestoGroup(media, FOURCC_COLOR, groupIdBlue, n, 1);
        std::cout << "BLUE map sample_index=" << n << " to group_index=" << groupIdBlue << std::endl;
        break;
      case 'g':
        ISOMapSamplestoGroup(media, FOURCC_COLOR, groupIdGreen, n, 1);
        std::cout << "GREEN map sample_index=" << n << " to group_index=" << groupIdGreen << std::endl;
        break;
      case 'y':
        ISOMapSamplestoGroup(media, FOURCC_COLOR, groupIdYellow, n, 1);
        std::cout << "YELLOW map sample_index=" << n << " to group_index=" << groupIdYellow << std::endl;
        break;
      default:
        std::cout << "SKIP (" << fullPattern[n] << ") sample_index=" << n << std::endl;
        break;
      }
    }
  }
  return err;
}

ISOErr createFile(std::string strFilename) {
  std::cout << "create file..." << std::endl;
  ISOErr err;
  ISOMovie moov;
  ISOMedia media;
  ISOTrack trak;

  ISOHandle spsHandle, ppsHandle, vpsHandle, sampleEntryH;
  err = ISONewHandle(sizeof(SPS), &spsHandle);
  memcpy((*spsHandle), SPS, sizeof(SPS));
  err = ISONewHandle(sizeof(PPS), &ppsHandle);
  memcpy((*ppsHandle), PPS, sizeof(PPS));
  err = ISONewHandle(sizeof(VPS), &vpsHandle);
  memcpy((*vpsHandle), VPS, sizeof(VPS));
  err = ISONewHandle(0, &sampleEntryH);
  
  u32 initialODID = 1;
  u8 OD_profileAndLevel = 0xff;       // none required
  u8 scene_profileAndLevel = 0xff;    // none required
  u8 audio_profileAndLevel = 0xff;    // none required
  u8 visual_profileAndLevel = 0xff;   // none required
  u8 graphics_profileAndLevel = 0xff; // none required

  err = MP4NewMovie(&moov, OD_profileAndLevel, OD_profileAndLevel,
                    scene_profileAndLevel, audio_profileAndLevel,
                    visual_profileAndLevel, graphics_profileAndLevel);
  if (err) return -1;

  ISOSetMovieBrand(moov, MP4_FOUR_CHAR_CODE('i', 's', 'o', '6'), 0);

  err = ISONewMovieTrack(moov, MP4NewTrackIsVisual, &trak); if (err) return -2;
  err = MP4AddTrackToMovieIOD(trak); if (err) return -3;
  err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, TIMESCALE, NULL); if (err) return -4;

  err = ISOBeginMediaEdits(media);
  err = ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 1, spsHandle, ppsHandle, vpsHandle); if (err) return -4;

  err = ISOSetTrackFragmentDefaults( trak, TIMESCALE/FPS, sizeof(auBlue), 1, 0);

  err = addGroupDescriptions(media); if(err) return err;
  u32 temp = 0;
  err = addGroupDescription(media, FOURCC_COLOR, "Yellow frames", temp); if(err == MP4NoErr) return -5; // this must fail because "Yellow frames" payload is already added with the same type

  // just add sample entry, call addSamples with sample count = 0
  err = addSamples(media, "r", 0, sampleEntryH);  if(err) return err;
  err = MP4EndMediaEdits(media);

  std::cout << "fragment 1" << std::endl;
  ISOSetSamplestoGroupType(media,0);
  err = ISOStartMovieFragment( moov ); if(err) return err;
  err = addSamples(media, "rb", 3);  if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "This must pass even if in stbl", temp); if(err) return err;
  err = addGroupDescription(media, FOURCC_TEST, "This must pass", temp); if(err) return err;
  err = addGroupDescription(media, FOURCC_COLOR, "Yellow frames", temp); if(err) return err; // this must pass even if the same type and payload is in stbl (but it shall not be in defragmented file)
  err = addGroupDescription(media, FOURCC_TEST, "Test", temp); if(err) return err;
  err = addGroupDescription(media, FOURCC_TEST, "Test", temp); if(!err) return -5; // this must fail because same type and payload already added
  err = mapSamplesToGroups(media, "rb", 3); if(err) return err;

  std::cout << "fragment 2 (compressed sample group)" << std::endl;
  ISOSetSamplestoGroupType(media, 1);
  err = ISOStartMovieFragment( moov ); if(err) return err;
  err = addSamples(media, "gry", 2);  if(err) return err;
  err = addGroupDescription(media, FOURCC_TEST, "Same as in previous fragment but different entry", groupIdBlue); if(err) return err;
  err = addGroupDescription(media, FOURCC_TEST, "This must pass", groupIdBlue); if(err) return err;
  err = mapSamplesToGroups(media, "gry", 2); if(err) return err;

  std::cout << "fragment 3" << std::endl;
  ISOSetSamplestoGroupType(media, 0);
  err = ISOStartMovieFragment( moov ); if(err) return err;
  err = addSamples(media, "bgy", 2);  if(err) return err;
  err = mapSamplesToGroups(media, "bgy", 2); if(err) return err;

  // std::cout << "fragment 4" << std::endl;
  // err = ISOStartMovieFragment( moov ); if(err) return err;
  // err = addSamples(media, "wk", 3);  if(err) return err;
  // err = mapSamplesToGroups(media, "wk", 3); if(err) return err;
  
  err = MP4WriteMovieToFile(moov, strFilename.c_str());
  return err;
}

int main() {
  ISOErr err;
  std::string strFileFrag = "hevc_fragments.mp4";
  std::string strFileDefrag = "hevc_defrag.mp4";

  err = createFile(strFileFrag); if(err)
  {
    std::cerr << "could not create the file" << std::endl;
    return err;
  }

  std::cout << "\n\nParse file..." << std::endl;
  ISOMovie moov;
  ISOTrack trak;
  ISOMedia media;
  err = ISOOpenMovieFile(&moov, strFileFrag.c_str(), MP4OpenMovieNormal); // MP4OpenMovieDebug

  u32 trackCount = 0;
  err = MP4GetMovieTrackCount(moov, &trackCount);
  for (u32 trackNumber = 1; trackNumber <= trackCount; ++trackNumber) {
    err = ISOGetMovieIndTrack(moov, trackNumber, &trak);
    err = ISOGetTrackMedia(trak, &media);
    u32 sampleCnt = 0;
    u32* sampleNumbers;
    err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdRed, &sampleNumbers, &sampleCnt);

    std::cout << "RED sampleCnt=" << sampleCnt << std::endl;
    for(u32 i=0; i<sampleCnt; ++i) {
      std::cout  << "sample " << sampleNumbers[i] << std::endl;
    }

    err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdBlue, &sampleNumbers, &sampleCnt);

    std::cout << "BLUE sampleCnt=" << sampleCnt << std::endl;
    for(u32 i=0; i<sampleCnt; ++i) {
      std::cout  << "sample " << sampleNumbers[i] << std::endl;
    }

    err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdGreen, &sampleNumbers, &sampleCnt);

    std::cout << "GREEN sampleCnt=" << sampleCnt << std::endl;
    for(u32 i=0; i<sampleCnt; ++i) {
      std::cout  << "sample " << sampleNumbers[i] << std::endl;
    }

    err = ISOGetSampleGroupSampleNumbers(media, FOURCC_COLOR, groupIdYellow, &sampleNumbers, &sampleCnt);

    std::cout << "YELLOW sampleCnt=" << sampleCnt << std::endl;
    for(u32 i=0; i<sampleCnt; ++i) {
      std::cout  << "sample " << sampleNumbers[i] << std::endl;
    }


  }
  // ISOGetSampleGroupSamples()
  
  err = MP4WriteMovieToFile(moov, strFileDefrag.c_str());
  return err;
}
