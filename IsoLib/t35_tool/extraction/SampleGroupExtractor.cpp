#include "SampleGroupExtractor.hpp"
#include "../sources/SMPTE_ST2094_50.hpp"
#include "../common/Logger.hpp"
#include "../common/T35Prefix.hpp"

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
}

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <map>

namespace t35
{

// Helper: Find first video track
static MP4Err findFirstVideoTrack(MP4Movie moov, MP4Track *outTrack)
{
  MP4Err err = MP4NoErr;
  *outTrack  = nullptr;

  u32 trackCount = 0;
  err            = MP4GetMovieTrackCount(moov, &trackCount);
  if(err) return err;

  for(u32 i = 1; i <= trackCount; ++i)
  {
    MP4Track trak   = nullptr;
    MP4Media media  = nullptr;
    u32 handlerType = 0;

    err = MP4GetMovieIndTrack(moov, i, &trak);
    if(err) continue;

    err = MP4GetTrackMedia(trak, &media);
    if(err) continue;

    err = MP4GetMediaHandlerDescription(media, &handlerType, nullptr);
    if(err) continue;

    if(handlerType == MP4VisualHandlerType)
    {
      *outTrack = trak;
      return MP4NoErr;
    }
  }

  return MP4NotFoundErr;
}

bool SampleGroupExtractor::canExtract(const ExtractionConfig &config, std::string &reason)
{
  if(!config.movie)
  {
    reason = "No movie provided";
    return false;
  }

  MP4Err err = MP4NoErr;

  // Find video track
  MP4Track videoTrack = nullptr;
  err                 = findFirstVideoTrack(config.movie, &videoTrack);
  if(err != MP4NoErr)
  {
    reason = "No video track found";
    return false;
  }

  // Get video media
  MP4Media videoMedia = nullptr;
  err                 = MP4GetTrackMedia(videoTrack, &videoMedia);
  if(err != MP4NoErr)
  {
    reason = "Failed to get video track media";
    return false;
  }

  // Check if there are any it35 sample groups
  u32 it35_sg_cnt = 0;
  err = ISOGetGroupDescriptionEntryCount(videoMedia, MP4T35SampleGroupEntry, &it35_sg_cnt);
  if(err != MP4NoErr || it35_sg_cnt == 0)
  {
    reason = "No it35 sample groups found in video track";
    return false;
  }

  LOG_DEBUG("Found {} it35 sample group description(s)", it35_sg_cnt);
  return true;
}

MP4Err SampleGroupExtractor::extract(const ExtractionConfig &config, MetadataMap *outItems)
{
  LOG_INFO("Extracting metadata using sample-group extractor");
  LOG_INFO("T.35 prefix: {}", config.t35Prefix);
  if(!outItems)
  {
    LOG_INFO("Output path: {}", config.outputPath);
  }
  else
  {
    LOG_INFO("Output mode: in-memory");
  }

  MP4Err err = MP4NoErr;

  // Find video track
  MP4Track videoTrack = nullptr;
  err                 = findFirstVideoTrack(config.movie, &videoTrack);
  if(err != MP4NoErr)
  {
    LOG_ERROR("Failed to find video track (err={})", err);
    return err;
  }

  u32 videoTrackID = 0;
  MP4GetTrackID(videoTrack, &videoTrackID);
  LOG_INFO("Using video track ID {}", videoTrackID);

  // Get video media
  MP4Media videoMedia = nullptr;
  err                 = MP4GetTrackMedia(videoTrack, &videoMedia);
  if(err != MP4NoErr)
  {
    LOG_ERROR("Failed to get video track media (err={})", err);
    return err;
  }

  // Get timescale
  u32 timescale = 1000; // default
  err           = MP4GetMediaTimeScale(videoMedia, &timescale);
  if(err != MP4NoErr)
  {
    LOG_WARN("Failed to get timescale, using default 1000");
    timescale = 1000;
  }
  LOG_DEBUG("Video track timescale: {}", timescale);

  // Get sample count
  u32 sampleCount = 0;
  err             = MP4GetMediaSampleCount(videoMedia, &sampleCount);
  if(err != MP4NoErr)
  {
    LOG_ERROR("Failed to get sample count (err={})", err);
    return err;
  }
  LOG_INFO("Video track has {} samples", sampleCount);

  // Get it35 sample group count
  u32 it35_sg_cnt = 0;
  err = ISOGetGroupDescriptionEntryCount(videoMedia, MP4T35SampleGroupEntry, &it35_sg_cnt);
  if(err != MP4NoErr || it35_sg_cnt == 0)
  {
    LOG_ERROR("No it35 sample groups found (err={})", err);
    return MP4NotFoundErr;
  }
  LOG_INFO("Found {} it35 sample group description(s)", it35_sg_cnt);

  // Prepare output directory if writing to files
  namespace fs = std::filesystem;
  fs::path outDir;
  if(!outItems)
  {
    outDir = config.outputPath;
    if(!fs::exists(outDir))
    {
      if(!fs::create_directories(outDir))
      {
        LOG_ERROR("Failed to create output directory: {}", config.outputPath);
        return MP4IOErr;
      }
    }
    LOG_INFO("Extracting samples to {}", outDir.string());
  }

  // Extract each group description into memory first
  MetadataMap items;
  std::vector<nlohmann::json> manifestItems;

  for(u32 groupIdx = 1; groupIdx <= it35_sg_cnt; ++groupIdx)
  {
    // Get group description
    MP4Handle entryH = nullptr;
    err              = MP4NewHandle(0, &entryH);
    if(err != MP4NoErr)
    {
      LOG_ERROR("Failed to create handle (err={})", err);
      return err;
    }

    err = ISOGetGroupDescription(videoMedia, MP4T35SampleGroupEntry, groupIdx, entryH);
    if(err != MP4NoErr)
    {
      MP4DisposeHandle(entryH);
      LOG_ERROR("Failed to get group description {} (err={})", groupIdx, err);
      return err;
    }

    u32 descSize = 0;
    MP4GetHandleSize(entryH, &descSize);

    if(descSize < 1)
    {
      MP4DisposeHandle(entryH);
      LOG_ERROR("Invalid T.35 group description size: {}", descSize);
      return MP4BadDataErr;
    }

    // First byte is complete_message_flag (bit 1) + reserved (bit 7)
    u8 prefixByte        = (*entryH)[0];
    bool completeMessage = (prefixByte & 0x80) != 0;

    LOG_DEBUG("Group {}: size={}, complete_message_flag={}", groupIdx, descSize, completeMessage);

    // T.35 payload is after the prefix byte
    u32 payloadSize       = descSize - 1;
    const u8 *payloadData = ((u8 *)*entryH) + 1;

    // Copy payload to memory
    std::vector<uint8_t> payload(payloadData, payloadData + payloadSize);

    // Decode the metadata if they are SMPTE ST 2094-50
    if(config.t35Prefix == "B500900001:SMPTE-ST2094-50")
    {
      SMPTE_ST2094_50 st2094_50;
      st2094_50.decodeBinaryToSyntaxElements(payload);
      st2094_50.convertSyntaxElementsToMetadataItems();
      st2094_50.dbgPrintMetadataItems();
    }

    LOG_INFO("Extracted group {}: {} bytes (complete_message={})", groupIdx, payloadSize,
             completeMessage);

    // Get which samples use this group
    u32 *sampleNumbers = nullptr;
    u32 samplesInGroup = 0;
    err = ISOGetSampleGroupSampleNumbers(videoMedia, MP4T35SampleGroupEntry, groupIdx,
                                         &sampleNumbers, &samplesInGroup);

    if(err == MP4NoErr && samplesInGroup > 0)
    {
      LOG_DEBUG("Group {} is used by {} sample(s)", groupIdx, samplesInGroup);

      // For metadata map, use the first sample as frame_start
      u32 frameStart    = sampleNumbers[0] - 1; // Convert to 0-based
      u32 frameDuration = samplesInGroup;

      // Create metadata item
      std::string sourceInfo = "metadata_" + std::to_string(groupIdx) + ".bin";
      MetadataItem item(frameStart, frameDuration, std::move(payload), sourceInfo);
      items[frameStart] = item;

      // Build manifest item for file output
      if(!outItems)
      {
        nlohmann::json manifestItem;
        manifestItem["frame_start"]      = frameStart;
        manifestItem["frame_duration"]   = frameDuration;
        manifestItem["binary_file"]      = sourceInfo;
        manifestItem["sample_size"]      = payloadSize;
        manifestItem["group_index"]      = groupIdx;
        manifestItem["complete_message"] = completeMessage;
        manifestItem["samples_in_group"] = samplesInGroup;

        // Include sample numbers for reference
        nlohmann::json samplesArray = nlohmann::json::array();
        for(u32 i = 0; i < samplesInGroup; ++i)
        {
          samplesArray.push_back(sampleNumbers[i]);
        }
        manifestItem["sample_numbers"] = samplesArray;

        manifestItems.push_back(manifestItem);
      }

      free(sampleNumbers);
    }
    else
    {
      LOG_WARN("Group {} has no samples mapped (err={})", groupIdx, err);

      // Still add to items but with zero duration
      std::string sourceInfo = "metadata_" + std::to_string(groupIdx) + ".bin";
      MetadataItem item(0, 0, std::move(payload), sourceInfo);
      items[0] = item;

      // Build manifest item for file output
      if(!outItems)
      {
        nlohmann::json manifestItem;
        manifestItem["frame_start"]      = 0;
        manifestItem["frame_duration"]   = 0;
        manifestItem["binary_file"]      = sourceInfo;
        manifestItem["sample_size"]      = payloadSize;
        manifestItem["group_index"]      = groupIdx;
        manifestItem["complete_message"] = completeMessage;
        manifestItem["samples_in_group"] = 0;
        manifestItem["sample_numbers"]   = nlohmann::json::array();

        manifestItems.push_back(manifestItem);
      }
    }

    MP4DisposeHandle(entryH);
  }

  LOG_INFO("Extracted {} it35 group descriptions", it35_sg_cnt);

  // If in-memory mode, return items
  if(outItems)
  {
    *outItems = std::move(items);
    return MP4NoErr;
  }

  // Otherwise write files
  for(const auto &[frameStart, item] : items)
  {
    fs::path binFile = outDir / item.source_info;
    std::ofstream out(binFile, std::ios::binary);
    if(!out)
    {
      LOG_ERROR("Failed to open {} for writing", binFile.string());
      return MP4IOErr;
    }
    out.write((const char *)item.payload.data(), item.payload.size());
    out.close();
  }

  // Write manifest JSON
  if(!manifestItems.empty())
  {
    nlohmann::json manifest;
    manifest["t35_prefix"]  = config.t35Prefix;
    manifest["timescale"]   = timescale;
    manifest["group_count"] = it35_sg_cnt;
    manifest["items"]       = manifestItems;

    fs::path manifestFile = outDir / "manifest.json";
    std::ofstream manifestOut(manifestFile);
    if(manifestOut)
    {
      manifestOut << manifest.dump(2);
      manifestOut.close();
      LOG_INFO("Wrote manifest to {}", manifestFile.string());
    }
    else
    {
      LOG_WARN("Failed to write manifest file");
    }
  }

  LOG_INFO("Extraction complete");
  return MP4NoErr;
}

} // namespace t35
