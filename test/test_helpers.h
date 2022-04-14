/**
 * @file test_helpers.h
 * @author Dimitri Podborski
 * @brief Helper functions for testing
 * @version 0.1
 * @date 2021-01-04
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

#pragma once
#include <ISOMovies.h>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cstring>
#include "test_data.h"

/**
 * @brief Append data to buffer with the preceding length field of lengthSize bytes.
 *
 * @param rBuffer Buffer to add data to
 * @param lengthSize length field size in bytes
 * @param data data to append
 * @param size size of data
 */
inline void appendDataWithLengthField(std::vector<u8> &rBuffer, u32 lengthSize, const u8 *data,
                                      u32 size)
{
  std::vector<u8> lengthData(lengthSize, 0x00);
  for(u32 n = 0; n < lengthSize; n++)
  {
    lengthData[lengthSize - n - 1] = size >> (n * 8);
  }
  rBuffer.insert(rBuffer.end(), lengthData.begin(), lengthData.end());
  rBuffer.insert(rBuffer.end(), data, data + size);
}

inline void appendDataWithBoxField(std::vector<u8> &rBuffer, u32 type, std::vector<u8> data)
{
  u32 size = data.size() + 8;
  std::vector<u8> lengthData(4, 0x00);
  for(u32 n = 0; n < 4; n++)
  {
    lengthData[4 - n - 1] = size >> (n * 8);
  }
  std::vector<u8> typeData(4, 0x00);
  for(u32 n = 0; n < 4; n++)
  {
    typeData[4 - n - 1] = type >> (n * 8);
  }
  rBuffer.insert(rBuffer.end(), lengthData.begin(), lengthData.end());
  rBuffer.insert(rBuffer.end(), typeData.begin(), typeData.end());
  rBuffer.insert(rBuffer.end(), data.begin(), data.end());
}

inline std::vector<u8> getMetaSample(u32 x, u32 y, u32 w, u32 h)
{
  std::vector<u8> retVal(4 * 4, 0x00);
  retVal[0]  = x >> (3 * 8);
  retVal[1]  = x >> (2 * 8);
  retVal[2]  = x >> (1 * 8);
  retVal[3]  = x >> (0 * 8);
  retVal[4]  = y >> (3 * 8);
  retVal[5]  = y >> (2 * 8);
  retVal[6]  = y >> (1 * 8);
  retVal[7]  = y >> (0 * 8);
  retVal[8]  = w >> (3 * 8);
  retVal[9]  = w >> (2 * 8);
  retVal[10] = w >> (1 * 8);
  retVal[11] = w >> (0 * 8);
  retVal[12] = h >> (3 * 8);
  retVal[13] = h >> (2 * 8);
  retVal[14] = h >> (1 * 8);
  retVal[15] = h >> (0 * 8);
  return retVal;
}

/**
 * @brief Helper function to add HEVC samples given the color pattern
 *
 * @param media media to add the sample to
 * @param strPattern color pattern of samples (r,b,g,y,w,k,R,U,D,F)
 * @param repeatPattern number of times to repeat the pattern. No samples are added if this is 0
 * @param sampleEntryH sample entry handle (for the first call)
 * @param lengthSize the length in bytes of the NALUnitLength field in an HEVC video sample
 * @return MP4Err error code
 */
inline MP4Err addHEVCSamples(MP4Media media, std::string strPattern, u32 repeatPattern = 1,
                             MP4Handle sampleEntryH = 0, u32 lengthSize = 1)
{
  MP4Err err;
  u32 sampleCount = 0;
  MP4Handle sampleDataH, durationsH, sizesH;
  err = MP4NewHandle(sizeof(u32), &durationsH);
  CHECK(err == MP4NoErr);

  *((u32 *)*durationsH) = TIMESCALE / FPS;

  std::vector<u8> bufferData;
  std::vector<u32> bufferSizes;
  for(std::string::const_iterator it = strPattern.cbegin(); it != strPattern.cend(); ++it)
  {
    switch(*it)
    {
    case 'r':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auRed, sizeof(HEVC::auRed));
      bufferSizes.push_back(sizeof(HEVC::auRed) + lengthSize);
      break;
    case 'b':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auBlue, sizeof(HEVC::auBlue));
      bufferSizes.push_back(sizeof(HEVC::auBlue) + lengthSize);
      break;
    case 'g':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auGreen, sizeof(HEVC::auGreen));
      bufferSizes.push_back(sizeof(HEVC::auGreen) + lengthSize);
      break;
    case 'y':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auYellow, sizeof(HEVC::auYellow));
      bufferSizes.push_back(sizeof(HEVC::auYellow) + lengthSize);
      break;
    case 'w':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auWhite, sizeof(HEVC::auWhite));
      bufferSizes.push_back(sizeof(HEVC::auWhite) + lengthSize);
      break;
    case 'k':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auBlack, sizeof(HEVC::auBlack));
      bufferSizes.push_back(sizeof(HEVC::auBlack) + lengthSize);
      break;
    case 'R':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auRU, sizeof(HEVC::auRU));
      bufferSizes.push_back(sizeof(HEVC::auRU) + lengthSize);
      break;
    case 'U':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auUA, sizeof(HEVC::auUA));
      bufferSizes.push_back(sizeof(HEVC::auUA) + lengthSize);
      break;
    case 'D':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auDE, sizeof(HEVC::auDE));
      bufferSizes.push_back(sizeof(HEVC::auDE) + lengthSize);
      break;
    case 'F':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auFR, sizeof(HEVC::auFR));
      bufferSizes.push_back(sizeof(HEVC::auFR) + lengthSize);
      break;
    case 'N':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auNL, sizeof(HEVC::auNL));
      bufferSizes.push_back(sizeof(HEVC::auNL) + lengthSize);
      break;
    case 'I':
      appendDataWithLengthField(bufferData, lengthSize, HEVC::auID, sizeof(HEVC::auID));
      bufferSizes.push_back(sizeof(HEVC::auID) + lengthSize);
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
  err = MP4NewHandle(bufferData.size() * sizeof(u8), &sampleDataH);
  CHECK(err == MP4NoErr);
  std::memcpy((*sampleDataH), bufferData.data(), bufferData.size() * sizeof(u8));
  err = MP4NewHandle(sizeof(u32) * bufferSizes.size(), &sizesH);
  CHECK(err == MP4NoErr);
  for(u32 n = 0; n < bufferSizes.size(); n++)
  {
    ((u32 *)*sizesH)[n] = bufferSizes[n];
  }

  err = MP4AddMediaSamples(media, sampleDataH, bufferSizesPattern.size() * repeatPattern,
                           durationsH, sizesH, sampleEntryH, 0, 0);
  CHECK(err == MP4NoErr);

  err = MP4DisposeHandle(sampleDataH);
  CHECK(err == MP4NoErr);
  err = MP4DisposeHandle(durationsH);
  CHECK(err == MP4NoErr);
  err = MP4DisposeHandle(sizesH);
  CHECK(err == MP4NoErr);
  return err;
}

/**
 * @brief Compare data in a MP4Handle with a data from a buffer
 *
 * @param dataH MP4handle holding some data
 * @param comparePtr Pointer to a buffer holding data
 * @param compareSize Size of the buffer
 * @return MP4Err MP4NoErr if data is the same
 */
inline MP4Err compareData(MP4Handle dataH, const u8 *comparePtr, u32 compareSize)
{
  MP4Err err = MP4NoErr;
  u32 handleSize;
  MP4GetHandleSize(dataH, &handleSize);
  if(handleSize != compareSize) return MP4BadDataErr;

  int compareVal = std::memcmp(comparePtr, *dataH, compareSize);
  if(compareVal != 0) return MP4BadDataErr;
  return err;
}

/**
 * @brief Compare the payload of a sample with the data in a buffer.
 *
 * lengthSize bytes of the sample payload are skipped for comparison. Single NALU in a sample only.
 *
 * @param media media to take a sample from
 * @param sampleNr sample number in that media
 * @param comparePtr pointer to data to compare the sample payload to
 * @param compareSize size of the compare data
 * @param lengthSize the length in bytes of the NALUnitLength field
 * @return MP4Err error code
 */
inline MP4Err checkSample(MP4Media media, u32 sampleNr, const u8 *comparePtr, u32 compareSize,
                          u32 lengthSize = 1)
{
  MP4Err err;
  MP4Handle sampleH;
  u32 outSize, outSampleFlags, outSampleDescIndex;
  u64 outDTS, outDuration;
  s32 outCTSOffset;

  MP4NewHandle(0, &sampleH);
  err = MP4GetIndMediaSample(media, sampleNr, sampleH, &outSize, &outDTS, &outCTSOffset,
                             &outDuration, &outSampleFlags, &outSampleDescIndex);
  CHECK(err == MP4NoErr);

  u32 handleSize;
  MP4GetHandleSize(sampleH, &handleSize);
  if(handleSize != outSize) return MP4BadDataErr;
  if(outSize != compareSize + lengthSize) return MP4BadDataErr;

  int compareVal = std::memcmp(comparePtr, *sampleH + lengthSize, compareSize);
  if(compareVal != 0) return MP4BadDataErr;

  MP4DisposeHandle(sampleH);
  return err;
}

inline void dumpBufferToFile(std::string filepath, char *buffer, u32 size)
{
  std::ofstream ofs(filepath, std::ios::binary);
  ofs.write(buffer, size);
  ofs.close();
}

inline void dumpHandleToFile(std::string filepath, MP4Handle h)
{
  u32 size   = 0;
  MP4Err err = MP4GetHandleSize(h, &size);
  if(err) return;
  dumpBufferToFile(filepath, *h, size);
}

/**
 * @brief Create a Handle from a buffer
 *
 * @param dataH Output Handle with the data from buffer of provided size
 * @param data Pointer to a buffer with the data which should be copied to handle
 * @param size Size of data in buffer to copy
 * @return MP4Err MP4NoErr on success.
 */
inline MP4Err createHandleFromBuffer(MP4Handle *dataH, const u8 *data, u32 size)
{
  MP4Err err = MP4NoErr;

  if(!data || size == 0) return MP4BadParamErr;
  err = MP4NewHandle(size, dataH);
  if(err) return err;

  std::memcpy((**dataH), data, size);
  return err;
}

/**
 * @brief Create a Handle from a buffer
 *
 * @param dataH Output Handle with the data from buffer of provided size
 * @param str std::string which should be copied to handle
 * @return MP4Err MP4NoErr on success.
 */
inline MP4Err createHandleFromString(MP4Handle *dataH, const std::string str)
{
  MP4Err err = MP4NoErr;
  u32 size   = static_cast<u32>(str.size());

  if(size == 0) return MP4BadParamErr;
  err = MP4NewHandle(size, dataH);
  if(err) return err;

  std::memcpy((**dataH), str.data(), size);
  return err;
}

inline MP4Err addMebxSamples(MP4Media media, std::string strPattern, u32 repeatPattern = 1,
                             MP4Handle sampleEntryH = 0, u32 lk_r = 0, u32 lk_b = 0, u32 lk_y = 0,
                             u32 lk_w = 0, u32 lk_k = 0, u32 lk_g = 0, u32 lk_en = 0, u32 lk_de = 0)
{
  MP4Err err;
  u32 sampleCount = 0;
  MP4Handle sampleDataH, durationsH, sizesH;
  err = MP4NewHandle(sizeof(u32), &durationsH);
  CHECK(err == MP4NoErr);

  *((u32 *)*durationsH) = TIMESCALE / FPS;

  std::vector<u8> bufferData;
  std::vector<u32> bufferSizes;
  for(std::string::const_iterator it = strPattern.cbegin(); it != strPattern.cend(); ++it)
  {
    switch(*it)
    {
    case 'r':
    {
      auto metaSample = getMetaSample(0, 0, 64, 48);
      appendDataWithBoxField(bufferData, lk_r, metaSample);
      bufferSizes.push_back(metaSample.size() + 8);
      break;
    }
    case 'b':
    {
      auto metaSample = getMetaSample(0, 0, 64, 48);
      appendDataWithBoxField(bufferData, lk_b, metaSample);
      bufferSizes.push_back(metaSample.size() + 8);
      break;
    }
    case 'g':
    {
      auto metaSample = getMetaSample(0, 0, 64, 48);
      appendDataWithBoxField(bufferData, lk_g, metaSample);
      bufferSizes.push_back(metaSample.size() + 8);
      break;
    }
    case 'y':
    {
      auto metaSample = getMetaSample(0, 0, 64, 48);
      appendDataWithBoxField(bufferData, lk_y, metaSample);
      bufferSizes.push_back(metaSample.size() + 8);
      break;
    }
    case 'w':
    {
      auto metaSample = getMetaSample(0, 0, 64, 48);
      appendDataWithBoxField(bufferData, lk_y, metaSample);
      bufferSizes.push_back(metaSample.size() + 8);
      break;
    }
    case 'k':
    {
      auto metaSample = getMetaSample(0, 0, 64, 48);
      appendDataWithBoxField(bufferData, lk_k, metaSample);
      bufferSizes.push_back(metaSample.size() + 8);
      break;
    }
    case 'R':
    {
      auto metaSampleRed   = getMetaSample(0, 32, 64, 16);
      auto metaSampleBlue  = getMetaSample(0, 16, 64, 16);
      auto metaSampleWhite = getMetaSample(0, 0, 64, 16);
      u32 sampleSize =
        metaSampleRed.size() + metaSampleBlue.size() + metaSampleWhite.size() + 3 * 8;
      appendDataWithBoxField(bufferData, lk_r, metaSampleRed);
      appendDataWithBoxField(bufferData, lk_b, metaSampleBlue);
      appendDataWithBoxField(bufferData, lk_w, metaSampleWhite);
      bufferSizes.push_back(sampleSize);
      break;
    }
    case 'U':
    {
      auto metaSampleYellow = getMetaSample(0, 24, 64, 24);
      auto metaSampleBlue   = getMetaSample(0, 0, 64, 24);
      u32 sampleSize        = metaSampleYellow.size() + metaSampleBlue.size() + 2 * 8;
      appendDataWithBoxField(bufferData, lk_b, metaSampleBlue);
      appendDataWithBoxField(bufferData, lk_y, metaSampleYellow);
      bufferSizes.push_back(sampleSize);
      break;
    }
    case 'D':
    {
      auto metaSampleBlack  = getMetaSample(0, 0, 64, 16);
      auto metaSampleRed    = getMetaSample(0, 16, 64, 16);
      auto metaSampleYellow = getMetaSample(0, 32, 64, 16);
      u32 sampleSize =
        metaSampleRed.size() + metaSampleBlack.size() + metaSampleYellow.size() + 3 * 8;
      appendDataWithBoxField(bufferData, lk_k, metaSampleBlack);
      appendDataWithBoxField(bufferData, lk_r, metaSampleRed);
      appendDataWithBoxField(bufferData, lk_y, metaSampleYellow);
      bufferSizes.push_back(sampleSize);
      break;
    }
    case 'F':
    {
      auto metaSampleRed   = getMetaSample(44, 0, 20, 48);
      auto metaSampleBlue  = getMetaSample(0, 0, 20, 48);
      auto metaSampleWhite = getMetaSample(20, 0, 24, 48);
      u32 sampleSize =
        metaSampleRed.size() + metaSampleBlue.size() + metaSampleWhite.size() + 3 * 8;
      appendDataWithBoxField(bufferData, lk_r, metaSampleRed);
      appendDataWithBoxField(bufferData, lk_b, metaSampleBlue);
      appendDataWithBoxField(bufferData, lk_w, metaSampleWhite);
      bufferSizes.push_back(sampleSize);
      break;
    }
    case 'N':
    {
      auto metaSampleRed   = getMetaSample(0, 0, 64, 16);
      auto metaSampleBlue  = getMetaSample(0, 32, 64, 16);
      auto metaSampleWhite = getMetaSample(0, 16, 64, 16);
      u32 sampleSize =
        metaSampleRed.size() + metaSampleBlue.size() + metaSampleWhite.size() + 3 * 8;
      appendDataWithBoxField(bufferData, lk_r, metaSampleRed);
      appendDataWithBoxField(bufferData, lk_b, metaSampleBlue);
      appendDataWithBoxField(bufferData, lk_w, metaSampleWhite);
      bufferSizes.push_back(sampleSize);
      break;
    }
    case 'I':
    {
      auto metaSampleRed    = getMetaSample(0, 0, 64, 24);
      auto metaSampleWhite  = getMetaSample(0, 24, 64, 24);
      u32 sampleSize        = metaSampleRed.size() + metaSampleWhite.size() + 2 * 8;
      appendDataWithBoxField(bufferData, lk_r, metaSampleRed);
      appendDataWithBoxField(bufferData, lk_w, metaSampleWhite);
      bufferSizes.push_back(sampleSize);
      break;
    }
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
  err = MP4NewHandle(bufferData.size() * sizeof(u8), &sampleDataH);
  CHECK(err == MP4NoErr);
  std::memcpy((*sampleDataH), bufferData.data(), bufferData.size() * sizeof(u8));
  err = MP4NewHandle(sizeof(u32) * bufferSizes.size(), &sizesH);
  CHECK(err == MP4NoErr);
  for(u32 n = 0; n < bufferSizes.size(); n++)
  {
    ((u32 *)*sizesH)[n] = bufferSizes[n];
  }

  err = MP4AddMediaSamples(media, sampleDataH, bufferSizesPattern.size() * repeatPattern,
                           durationsH, sizesH, sampleEntryH, 0, 0);
  CHECK(err == MP4NoErr);

  err = MP4DisposeHandle(sampleDataH);
  CHECK(err == MP4NoErr);
  err = MP4DisposeHandle(durationsH);
  CHECK(err == MP4NoErr);
  err = MP4DisposeHandle(sizesH);
  CHECK(err == MP4NoErr);
  return err;
}
