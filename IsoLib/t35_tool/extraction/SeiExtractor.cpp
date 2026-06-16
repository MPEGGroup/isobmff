#include "SeiExtractor.hpp"
#include "AutoExtractor.hpp"
#include "../common/Logger.hpp"
#include "../common/T35Prefix.hpp"

extern "C"
{
#include "MP4Movies.h"
}

#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <cstring>
#include <cassert>

namespace t35
{

namespace
{

/**
 * Write NAL unit with Annex-B start code
 */
void writeAnnexBNAL(std::ofstream &out, const uint8_t *data, uint32_t size)
{
  static const uint8_t startCode[4] = {0x00, 0x00, 0x00, 0x01};
  out.write(reinterpret_cast<const char *>(startCode), 4);
  out.write(reinterpret_cast<const char *>(data), size);
}

/**
 * Build HEVC SEI NAL unit with T.35 metadata
 *
 * @param payload T.35 metadata payload (without prefix)
 * @param size Payload size in bytes
 * @param t35PrefixHex T.35 prefix hex string (e.g., "B500900001")
 * @return SEI NAL unit bytes
 */
std::vector<uint8_t> buildSeiNalu(const uint8_t *payload, uint32_t size,
                                  const std::string &t35PrefixHex)
{
  std::vector<uint8_t> sei;

  // NAL header: forbidden_zero_bit=0, nal_unit_type=39 (prefix SEI),
  // nuh_layer_id=0, nuh_temporal_id_plus1=1
  sei.push_back(0x00 | (39 << 1) | 0);
  sei.push_back(0x01);

  // Extract hex portion of t35PrefixHex (strip description after ':' if present)
  std::string hexOnly = t35PrefixHex;
  size_t colonPos     = t35PrefixHex.find(':');
  if(colonPos != std::string::npos)
  {
    hexOnly = t35PrefixHex.substr(0, colonPos);
  }

  // Build full T.35 payload = [prefix][metadata]
  // Convert hex string to binary data
  std::vector<uint8_t> prefixBytes;
  if(hexOnly.size() % 2 != 0)
  {
    Logger::error("Invalid hex string length in T.35 prefix (must be even): " + hexOnly);
    return sei; // return incomplete NAL
  }
  for(size_t i = 0; i < hexOnly.size(); i += 2)
  {
    unsigned int byteVal = 0;
    std::string byteStr  = hexOnly.substr(i, 2);
    if(sscanf(byteStr.c_str(), "%02x", &byteVal) != 1)
    {
      Logger::error("Invalid hex substring in T.35 prefix: " + byteStr);
      return sei; // return incomplete NAL
    }
    prefixBytes.push_back(static_cast<uint8_t>(byteVal));
  }
  uint32_t prefixSize = static_cast<uint32_t>(prefixBytes.size());

  std::vector<uint8_t> fullPayload(prefixSize + size);
  std::memcpy(fullPayload.data(), prefixBytes.data(), prefixSize);
  std::memcpy(fullPayload.data() + prefixSize, payload, size);

  // payloadType = 4 (user_data_registered_itu_t_t35)
  sei.push_back(4);

  // payloadSize (in one byte for simplicity, assumes < 255)
  sei.push_back(static_cast<uint8_t>(fullPayload.size()));

  // payload with emulation prevention
  for(size_t i = 0; i < fullPayload.size(); i++)
  {
    uint8_t b = fullPayload[i];
    sei.push_back(b);
    size_t n = sei.size();
    if(n >= 3 && sei[n - 1] <= 0x03 && sei[n - 2] == 0x00 && sei[n - 3] == 0x00)
    {
      sei.push_back(0x03);
    }
  }

  // rbsp_trailing_bits (10000000)
  sei.push_back(0x80);

  return sei;
}

/**
 * Find first video track in movie
 */
MP4Err findFirstVideoTrack(MP4Movie movie, MP4Track *outTrack)
{
  MP4Err err     = MP4NoErr;
  u32 trackCount = 0;

  err = MP4GetMovieTrackCount(movie, &trackCount);
  if(err) return err;

  for(u32 i = 1; i <= trackCount; i++)
  {
    MP4Track track = nullptr;
    err            = MP4GetMovieTrack(movie, i, &track);
    if(err) continue;

    u32 handlerType = 0;
    MP4Media media  = nullptr;
    err             = MP4GetTrackMedia(track, &media);
    if(err) continue;

    err = MP4GetMediaHandlerDescription(media, &handlerType, nullptr);
    if(err) continue;

    if(handlerType == MP4VisualHandlerType)
    {
      *outTrack = track;
      return MP4NoErr;
    }
  }

  return MP4NotFoundErr;
}

/**
 * Get timescale from video track
 */
MP4Err getVideoTimescale(MP4Track videoTrack, u32 *timescale)
{
  MP4Err err     = MP4NoErr;
  MP4Media media = nullptr;

  err = MP4GetTrackMedia(videoTrack, &media);
  if(err) return err;

  err = MP4GetMediaTimeScale(media, timescale);
  return err;
}

/**
 * Get total video sample count
 */
MP4Err getVideoSampleCount(MP4Track videoTrack, u32 *sampleCount)
{
  MP4Err err     = MP4NoErr;
  MP4Media media = nullptr;

  err = MP4GetTrackMedia(videoTrack, &media);
  if(err) return err;

  err = MP4GetMediaSampleCount(media, sampleCount);
  return err;
}

} // anonymous namespace

bool SeiExtractor::canExtract(const ExtractionConfig &config, std::string &reason)
{
  // Check if movie has video track
  MP4Track videoTrack = nullptr;
  MP4Err err          = findFirstVideoTrack(config.movie, &videoTrack);
  if(err != MP4NoErr)
  {
    reason = "No video track found in movie";
    return false;
  }

  // Check if video track is HEVC
  MP4Media media = nullptr;
  err            = MP4GetTrackMedia(videoTrack, &media);
  if(err != MP4NoErr)
  {
    reason = "Cannot get media from video track";
    return false;
  }

  // Try to get sample entry and check if it's HEVC
  MP4Handle sampleEntryH = nullptr;
  err                    = MP4NewHandle(0, &sampleEntryH);
  if(err != MP4NoErr)
  {
    reason = "Cannot create handle for sample entry";
    return false;
  }

  err = MP4GetMediaSampleDescription(media, 1, sampleEntryH, nullptr);
  if(err != MP4NoErr)
  {
    MP4DisposeHandle(sampleEntryH);
    reason = "Cannot get sample description";
    return false;
  }

  // Try to extract HEVC NAL units to verify it's HEVC
  MP4Handle hevcNALs = nullptr;
  err                = MP4NewHandle(0, &hevcNALs);
  if(err != MP4NoErr)
  {
    MP4DisposeHandle(sampleEntryH);
    reason = "Cannot create handle for HEVC NALs";
    return false;
  }

  err = ISOGetHEVCNALUs(sampleEntryH, hevcNALs, 0);
  MP4DisposeHandle(hevcNALs);
  MP4DisposeHandle(sampleEntryH);

  if(err != MP4NoErr)
  {
    reason = "Could not get Sample Entry NALUs from HEVC video track (not HEVC?)";
    return false;
  }

  // Check if there's any metadata (try auto-detection)
  AutoExtractor autoExtractor;
  if(!autoExtractor.canExtract(config, reason))
  {
    reason = "No T.35 metadata found in movie";
    return false;
  }

  return true;
}

MP4Err SeiExtractor::extract(const ExtractionConfig &config, MetadataMap *outItems)
{
  // Note: SeiExtractor always writes to video file, outItems is ignored
  (void)outItems;

  MP4Err err = MP4NoErr;

  Logger::info("Extracting T.35 metadata and converting to HEVC video with SEI NAL units");

  // Step 1: Extract metadata directly to memory using auto-detection
  Logger::info("Reading metadata from container...");

  MetadataMap metadataItems;
  AutoExtractor autoExtractor;

  // Get items directly in memory - no temp files!
  err = autoExtractor.extract(config, &metadataItems);
  if(err != MP4NoErr)
  {
    Logger::error("Failed to extract metadata");
    return err;
  }

  Logger::info("Loaded " + std::to_string(metadataItems.size()) + " metadata items");

  // Step 2: Find video track and get configuration
  MP4Track videoTrack = nullptr;
  err                 = findFirstVideoTrack(config.movie, &videoTrack);
  if(err != MP4NoErr)
  {
    Logger::error("No video track found");
    return err;
  }

  MP4Media videoMedia = nullptr;
  err                 = MP4GetTrackMedia(videoTrack, &videoMedia);
  if(err != MP4NoErr)
  {
    Logger::error("Cannot get video media");
    return err;
  }

  // Get video timescale
  u32 timescale = 0;
  err           = getVideoTimescale(videoTrack, &timescale);
  if(err != MP4NoErr)
  {
    Logger::error("Cannot get video timescale");
    return err;
  }
  Logger::info("Video timescale: " + std::to_string(timescale));

  // Step 3: Extract HEVC decoder configuration
  MP4Handle sampleEntryH = nullptr;
  err                    = MP4NewHandle(0, &sampleEntryH);
  if(err != MP4NoErr) return err;

  err = MP4GetMediaSampleDescription(videoMedia, 1, sampleEntryH, nullptr);
  if(err != MP4NoErr)
  {
    MP4DisposeHandle(sampleEntryH);
    return err;
  }

  MP4Handle hevcNALs = nullptr;
  err                = MP4NewHandle(0, &hevcNALs);
  if(err != MP4NoErr)
  {
    MP4DisposeHandle(sampleEntryH);
    return err;
  }

  err = ISOGetHEVCNALUs(sampleEntryH, hevcNALs, 0);
  if(err != MP4NoErr)
  {
    Logger::error("Failed to extract HEVC NAL units from sample entry");
    MP4DisposeHandle(hevcNALs);
    MP4DisposeHandle(sampleEntryH);
    return err;
  }

  // Get NAL unit length size
  u32 lengthSize = 0;
  err            = ISOGetNALUnitLength(sampleEntryH, &lengthSize);
  MP4DisposeHandle(sampleEntryH);
  if(err != MP4NoErr)
  {
    Logger::error("Failed to get NAL unit length size");
    MP4DisposeHandle(hevcNALs);
    return err;
  }
  Logger::info("HEVC NAL unit length size: " + std::to_string(lengthSize));

  // Step 4: Open output file and write decoder configuration
  std::filesystem::path outputPath = config.outputPath;
  if(outputPath.extension() != ".hevc" && outputPath.extension() != ".265")
  {
    outputPath.replace_extension(".265");
  }

  std::ofstream outFile(outputPath, std::ios::binary);
  if(!outFile)
  {
    Logger::error("Failed to open output file: " + outputPath.string());
    MP4DisposeHandle(hevcNALs);
    return MP4IOErr;
  }

  // Write decoder config NALs
  u32 hevcNALsSize = 0;
  MP4GetHandleSize(hevcNALs, &hevcNALsSize);
  outFile.write(reinterpret_cast<const char *>(*hevcNALs), hevcNALsSize);
  MP4DisposeHandle(hevcNALs);

  Logger::info("Wrote " + std::to_string(hevcNALsSize) + " bytes decoder configuration");

  // Step 5: Iterate video samples and insert SEI
  u32 videoSampleCount = 0;
  err                  = getVideoSampleCount(videoTrack, &videoSampleCount);
  if(err != MP4NoErr)
  {
    Logger::error("Cannot get video sample count");
    outFile.close();
    return err;
  }

  Logger::info("Processing " + std::to_string(videoSampleCount) + " video samples");

  // Track metadata state (sample-aligned)
  auto metadataIter                   = metadataItems.begin();
  u64 metadataRemain                  = 0; // Remaining duration in timescale units
  const MetadataItem *currentMetadata = nullptr;

  for(u32 sampleNum = 1; sampleNum <= videoSampleCount; sampleNum++)
  {
    // Get video sample
    MP4Handle videoSampleH = nullptr;
    u32 videoSize          = 0;
    u64 videoDTS           = 0;
    s32 videoCTSOffset     = 0;
    u64 videoDuration      = 0;
    u32 videoFlags         = 0;
    u32 videoDescIndex     = 0;

    err = MP4NewHandle(0, &videoSampleH);
    if(err != MP4NoErr) break;

    err = MP4GetIndMediaSample(videoMedia, sampleNum, videoSampleH, &videoSize, &videoDTS,
                               &videoCTSOffset, &videoDuration, &videoFlags, &videoDescIndex);
    if(err != MP4NoErr)
    {
      MP4DisposeHandle(videoSampleH);
      break;
    }

    // Check if we need to fetch next metadata item
    if(metadataRemain == 0 && metadataIter != metadataItems.end())
    {
      currentMetadata = &metadataIter->second;
      metadataRemain  = static_cast<u64>(currentMetadata->frame_duration) * timescale /
                       (timescale / 1000); // Convert frames to timescale units (approximation)
      ++metadataIter;
    }

    // If metadata is active for this sample, write SEI NAL
    if(metadataRemain > 0 && currentMetadata)
    {
      std::vector<uint8_t> sei =
        buildSeiNalu(currentMetadata->payload.data(),
                     static_cast<uint32_t>(currentMetadata->payload.size()), config.t35Prefix);
      writeAnnexBNAL(outFile, sei.data(), static_cast<uint32_t>(sei.size()));

      if(metadataRemain > videoDuration)
      {
        metadataRemain -= videoDuration;
      }
      else
      {
        metadataRemain = 0;
      }
    }

    // Convert video sample from length-prefix to Annex-B format
    uint8_t *src = reinterpret_cast<uint8_t *>(*videoSampleH);
    uint8_t *end = src + videoSize;

    while(src + lengthSize <= end)
    {
      // Read NAL length
      u32 nalLen = 0;
      for(u32 i = 0; i < lengthSize; i++)
      {
        nalLen = (nalLen << 8) | src[i];
      }
      src += lengthSize;

      // Write NAL with start code
      if(src + nalLen <= end)
      {
        writeAnnexBNAL(outFile, src, nalLen);
        src += nalLen;
      }
      else
      {
        Logger::warn("Invalid NAL length at sample " + std::to_string(sampleNum));
        break;
      }
    }

    MP4DisposeHandle(videoSampleH);
  }

  outFile.close();

  Logger::info("Finished writing " + outputPath.string());

  return MP4NoErr;
}

} // namespace t35
