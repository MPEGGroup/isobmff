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
#include <cstring>
#include "test_data.h"

inline int parseVVCNal(FILE *input, u8 **data, int *data_len)
{
  size_t startPos;
  size_t NALStart = 0;
  size_t NALEnd   = 0;
  u32 NALULen;
  u8 *NALU;

  u8 byte;
  int zerocount;
  int frame = 0;
  u8 nal_header[2];

  // Save start position
  startPos = ftell(input);

  // Search for sync
  zerocount = 0;
  while(1)
  {
    u8 byte;
    if(!fread(&byte, 1, 1, input))
    {
      return -1;
    }
    /* Search for sync */
    if(zerocount >= 2 && byte == 1)
    {
      /* Read NAL unit header */
      fread(nal_header, 2, 1, input);
      /* Include header in the data */
      fseek(input, -2, SEEK_CUR);
      break;
    }
    else if(byte == 0)
    {
      zerocount++;
    }
    else
    {
      zerocount = 0;
    }
  }
  NALStart = ftell(input);

  // Search for next sync
  zerocount = 0;
  while(1)
  {
    if(!fread(&byte, 1, 1, input))
    {
      zerocount = 0;
      break;
    }
    // Sync found
    if(zerocount >= 2 && byte == 1)
    {
      fseek(input, -1 - zerocount, SEEK_CUR);
      NALEnd = ftell(input);
      break;
    }
    else if(byte == 0)
    {
      zerocount++;
    }
    else
    {
      zerocount = 0;
    }
  }
  NALEnd = ftell(input);

  NALULen = NALEnd - NALStart;
  NALU    = (u8 *)malloc(NALULen);
  fseek(input, NALStart, SEEK_SET);
  if(!fread(NALU, NALULen, 1, input))
  {
    return -1;
  }

  /* Extract NAL unit type */
  byte      = nal_header[1] >> 3;
  *data_len = NALULen;
  *data     = NALU;

  return byte;
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

/**
 * @brief Helper function to add HEVC samples given the color pattern
 *
 * @param media media to add the sample to
 * @param strPattern color pattern of samples (r,b,g,y,w,k)
 * @param repeatPattern number of times to repeat the pattern. No samples are added if this is 0
 * @param sampleEntryH sample entry handle (for the first call)
 * @param lengthSize the length in bytes of the NALUnitLength field in an HEVC video sample
 * @return ISOErr error code
 */
inline ISOErr addHEVCSamples(MP4Media media, std::string strPattern, u32 repeatPattern = 1,
                             ISOHandle sampleEntryH = 0, u32 lengthSize = 1)
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
 * @brief Compare data in a MP4Handle with a data from a buffer
 *
 * @param dataH MP4handle holding some data
 * @param comparePtr Pointer to a buffer holding data
 * @param compareSize Size of the buffer
 * @return ISOErr MP4NoErr if data is the same
 */
inline ISOErr compareData(MP4Handle dataH, const u8 *comparePtr, u32 compareSize)
{
  ISOErr err = MP4NoErr;
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
 * @return ISOErr error code
 */
inline ISOErr checkSample(MP4Media media, u32 sampleNr, const u8 *comparePtr, u32 compareSize,
                          u32 lengthSize = 1)
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
  if(handleSize != outSize) return MP4BadDataErr;
  if(outSize != compareSize + lengthSize) return MP4BadDataErr;

  int compareVal = std::memcmp(comparePtr, *sampleH + lengthSize, compareSize);
  if(compareVal != 0) return MP4BadDataErr;

  MP4DisposeHandle(sampleH);
  return err;
}