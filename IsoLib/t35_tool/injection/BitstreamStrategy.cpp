#include "BitstreamStrategy.hpp"
#include "StrategyHelpers.hpp"
#include "../common/Logger.hpp"
#include <cstdlib>

extern "C"
{
#include "MP4Movies.h"
}

#include <vector>
#include <cstring>
#include <algorithm>

namespace t35
{

bool BitstreamStrategy::isApplicable(const MetadataMap &items, const InjectionConfig &config,
                                     std::string &reason) const
{
  if(!config.movie)
  {
    reason = "No movie provided";
    return false;
  }

  if(items.empty())
  {
    reason = "No metadata items to inject";
    return false;
  }

  // Find video track
  MP4Track trakV = nullptr;
  MP4Err err     = findFirstVideoTrack(config.movie, &trakV);
  if(err || !trakV)
  {
    reason = "No video track found";
    return false;
  }

  // Get media
  MP4Media mediaV = nullptr;
  err             = MP4GetTrackMedia(trakV, &mediaV);
  if(err || !mediaV)
  {
    reason = "Failed to get video media";
    return false;
  }

  // Get sample description
  MP4Handle sampleDescH = nullptr;
  MP4NewHandle(0, &sampleDescH);
  u32 dataRefIndex = 0;
  err              = MP4GetMediaSampleDescription(mediaV, 1, sampleDescH, &dataRefIndex);
  if(err)
  {
    reason = "Failed to get sample description";
    MP4DisposeHandle(sampleDescH);
    return false;
  }

  u32 type = 0;
  ISOGetSampleDescriptionType(sampleDescH, &type);
  MP4DisposeHandle(sampleDescH);

  if(type != MP4_FOUR_CHAR_CODE('a', 'v', '0', '1') &&
     type != MP4_FOUR_CHAR_CODE('h', 'v', 'c', '1') &&
     type != MP4_FOUR_CHAR_CODE('h', 'e', 'v', '1'))
  {
    reason = "Only AV1 and HEVC bitstreams are supported";
    return false;
  }

  return true;
}

MP4Err BitstreamStrategy::inject(const InjectionConfig &config, const MetadataMap &items,
                                 const T35Prefix &prefix)
{
  LOG_INFO("Injecting metadata into video bitstream");
  LOG_INFO("Using T.35 prefix: {}", prefix.toString());

  MP4Err err            = MP4NoErr;
  MP4Track trakV        = nullptr;
  MP4Media mediaV       = nullptr;
  u32 sampleCount       = 0;
  u32 type              = 0;
  MP4Handle sampleDescH = nullptr;

  err = findFirstVideoTrack(config.movie, &trakV);
  if(err) return err;

  err = MP4GetTrackMedia(trakV, &mediaV);
  if(err) return err;

  err = MP4GetMediaSampleCount(mediaV, &sampleCount);
  if(err) return err;

  // Build mapping from composition frame to decoding sample index
  struct SampleTimeInfo
  {
    u32 decodingIndex; // 1-based
    u64 cts;
  };
  std::vector<SampleTimeInfo> sampleTimes;
  sampleTimes.reserve(sampleCount);

  for(u32 i = 1; i <= sampleCount; ++i)
  {
    MP4Handle sampleH = nullptr;
    u32 outSize, outSampleFlags, outSampleDescIndex;
    u64 outDTS, outDuration;
    s32 outCTSOffset;

    MP4NewHandle(0, &sampleH);
    err = MP4GetIndMediaSample(mediaV, i, sampleH, &outSize, &outDTS, &outCTSOffset, &outDuration,
                               &outSampleFlags, &outSampleDescIndex);
    if(err)
    {
      if(sampleH) MP4DisposeHandle(sampleH);
      return err;
    }

    u64 cts = outDTS;
    if(outSampleFlags & MP4MediaSampleHasCTSOffset)
    {
      cts = static_cast<u64>(static_cast<s64>(outDTS) + outCTSOffset);
    }
    sampleTimes.push_back({i, cts});

    if(sampleH) MP4DisposeHandle(sampleH);
  }

  // Sort by CTS to get composition order
  std::sort(sampleTimes.begin(), sampleTimes.end(),
            [](const SampleTimeInfo &a, const SampleTimeInfo &b) { return a.cts < b.cts; });

  // Map decoding index (1-based) to MetadataItem
  std::map<u32, MetadataItem> decodingIndexToMetadata;
  for(u32 compIdx = 0; compIdx < sampleTimes.size(); ++compIdx)
  {
    auto it = items.find(compIdx);
    if(it != items.end())
    {
      decodingIndexToMetadata[sampleTimes[compIdx].decodingIndex] = it->second;
    }
  }

  MP4NewHandle(0, &sampleDescH);
  u32 dataRefIndex = 0;
  err              = MP4GetMediaSampleDescription(mediaV, 1, sampleDescH, &dataRefIndex);
  if(err)
  {
    MP4DisposeHandle(sampleDescH);
    return err;
  }

  ISOGetSampleDescriptionType(sampleDescH, &type);
  MP4DisposeHandle(sampleDescH);

  bool isAV1  = (type == MP4_FOUR_CHAR_CODE('a', 'v', '0', '1'));
  bool isHEVC = (type == MP4_FOUR_CHAR_CODE('h', 'v', 'c', '1') ||
                 type == MP4_FOUR_CHAR_CODE('h', 'e', 'v', '1'));

  LOG_INFO("Bitstream codec: {}", isAV1 ? "AV1" : "HEVC");

  for(u32 i = 1; i <= sampleCount; ++i)
  {
    MP4Handle sampleH = nullptr;
    u32 outSize, outSampleFlags, outSampleDescIndex;
    u64 outDTS;
    s32 outCTSOffset;
    u64 outDuration;

    MP4NewHandle(0, &sampleH);
    err = MP4GetIndMediaSample(mediaV, i, sampleH, &outSize, &outDTS, &outCTSOffset, &outDuration,
                               &outSampleFlags, &outSampleDescIndex);
    if(err)
    {
      MP4DisposeHandle(sampleH);
      return err;
    }

    auto it = decodingIndexToMetadata.find(i);
    if(it != decodingIndexToMetadata.end())
    {
      const MetadataItem &item = it->second;
      LOG_DEBUG("Injecting metadata into sample {} (mapped from composition frame {})", i,
                item.frame_start);

      std::vector<uint8_t> t35PrefixBytes = T35Prefix(config.t35Prefix).toBytes();

      MP4Handle newSampleH = nullptr;

      if(isAV1)
      {
        std::vector<uint8_t> obu;
        obu.push_back(0x2A); // Forbidden=0, Type=5(METADATA), Ext=0, HasSize=1, Reserved=0

        // Size includes: metadata_type (1) + t35PrefixBytes + payload + trailing byte (1)
        uint64_t size = 1 + t35PrefixBytes.size() + item.payload.size() + 1;
        while(size >= 0x80)
        {
          obu.push_back(static_cast<uint8_t>((size & 0x7F) | 0x80));
          size >>= 7;
        }
        obu.push_back(static_cast<uint8_t>(size));

        obu.push_back(0x04); // OBU type / metadata_type = 4 (OBU_METADATA_TYPE_ITUT_T35)
        obu.insert(obu.end(), t35PrefixBytes.begin(), t35PrefixBytes.end());
        obu.insert(obu.end(), item.payload.begin(), item.payload.end());
        obu.push_back(0x80); // trailing byte

        u32 newSize = outSize + obu.size();
        MP4NewHandle(newSize, &newSampleH);

        char *newSampleData = *newSampleH;
        std::memcpy(newSampleData, obu.data(), obu.size());
        std::memcpy(newSampleData + obu.size(), *sampleH, outSize);
      }
      else if(isHEVC)
      {
        std::vector<uint8_t> nalu;
        nalu.push_back(0x4E); // nal_unit_type = 39 (PREFIX_SEI_NUT), layer_id = 0
        nalu.push_back(0x01); // temporal_id_plus1 = 1

        nalu.push_back(4); // payload_type = 4 (user_data_registered_itu_t_t35)

        // Size includes: t35PrefixBytes + payload + trailing byte (1)
        size_t size = t35PrefixBytes.size() + item.payload.size() + 1;
        while(size >= 255)
        {
          nalu.push_back(0xFF);
          size -= 255;
        }
        nalu.push_back(static_cast<uint8_t>(size));

        nalu.insert(nalu.end(), t35PrefixBytes.begin(), t35PrefixBytes.end());
        nalu.insert(nalu.end(), item.payload.begin(), item.payload.end());
        nalu.push_back(0x80); // trailing byte

        u32 naluSize = nalu.size();
        std::vector<uint8_t> lengthPrefixed;
        lengthPrefixed.push_back((naluSize >> 24) & 0xFF);
        lengthPrefixed.push_back((naluSize >> 16) & 0xFF);
        lengthPrefixed.push_back((naluSize >> 8) & 0xFF);
        lengthPrefixed.push_back(naluSize & 0xFF);
        lengthPrefixed.insert(lengthPrefixed.end(), nalu.begin(), nalu.end());

        u32 newSize = outSize + lengthPrefixed.size();
        MP4NewHandle(newSize, &newSampleH);

        char *newSampleData = *newSampleH;
        std::memcpy(newSampleData, lengthPrefixed.data(), lengthPrefixed.size());
        std::memcpy(newSampleData + lengthPrefixed.size(), *sampleH, outSize);
      }

      u32 actualNewSize = 0;
      MP4GetHandleSize(newSampleH, &actualNewSize);

      err = MP4UpdateMediaSample(config.movie, mediaV, i, newSampleH, actualNewSize);
      MP4DisposeHandle(newSampleH);
      if(err)
      {
        MP4DisposeHandle(sampleH);
        return err;
      }
    }

    MP4DisposeHandle(sampleH);
  }

  return MP4NoErr;
}

} // namespace t35
