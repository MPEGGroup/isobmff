#include "MebxMe4cStrategy.hpp"
#include "../common/Logger.hpp"

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
}

#include <algorithm>
#include <cstring>
#include <vector>

namespace t35
{

// Helper: Find first video track
static MP4Err findFirstVideoTrack(MP4Movie moov, MP4Track *outTrack)
{
  MP4Err err     = MP4NoErr;
  u32 trackCount = 0;
  *outTrack      = nullptr;

  err = MP4GetMovieTrackCount(moov, &trackCount);
  if(err) return err;

  MP4Track firstVideo = nullptr;
  u32 videoCount      = 0;

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
      if(!firstVideo)
      {
        firstVideo = trak;
      }
      ++videoCount;
    }
  }

  if(!firstVideo)
  {
    LOG_ERROR("No video track found in movie");
    return MP4NotFoundErr;
  }

  if(videoCount > 1)
  {
    LOG_WARN("Found {} video tracks, using the first one", videoCount);
  }

  *outTrack = firstVideo;
  return MP4NoErr;
}

// Helper: Get video sample durations
static MP4Err getVideoSampleDurations(MP4Media mediaV, std::vector<u32> &durations)
{
  MP4Err err      = MP4NoErr;
  u32 sampleCount = 0;

  durations.clear();

  err = MP4GetMediaSampleCount(mediaV, &sampleCount);
  if(err) return err;

  durations.reserve(sampleCount);

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

    durations.push_back(static_cast<u32>(outDuration));

    if(sampleH) MP4DisposeHandle(sampleH);
  }

  LOG_DEBUG("Collected {} video sample durations", durations.size());
  return MP4NoErr;
}

// Helper: Build metadata durations and sizes
static MP4Err buildMetadataDurationsAndSizes(const MetadataMap &items,
                                             const std::vector<u32> &videoDurations,
                                             std::vector<u32> &metadataDurations,
                                             std::vector<u32> &metadataSizes,
                                             std::vector<MetadataItem> &sortedItems)
{
  metadataDurations.clear();
  metadataSizes.clear();
  sortedItems.clear();

  if(items.empty())
  {
    LOG_ERROR("No metadata items provided");
    return MP4BadParamErr;
  }

  // Sort items by frame_start (already sorted in map, but make vector)
  for(const auto &[start, item] : items)
  {
    sortedItems.push_back(item);
  }

  // Validate coverage
  const auto &last = sortedItems.back();
  u32 maxFrame     = last.frame_start + last.frame_duration;
  if(maxFrame > videoDurations.size())
  {
    LOG_ERROR("Metadata covers up to frame {} but video only has {} samples", maxFrame,
              videoDurations.size());
    return MP4BadParamErr;
  }

  // Compute metadata sample durations and sizes
  for(const auto &item : sortedItems)
  {
    u32 startFrame = item.frame_start;
    u32 endFrame   = startFrame + item.frame_duration;
    u32 totalDur   = 0;

    for(u32 f = startFrame; f < endFrame; ++f)
    {
      totalDur += videoDurations[f];
    }

    metadataDurations.push_back(totalDur);
    metadataSizes.push_back(static_cast<u32>(item.payload.size()));

    LOG_DEBUG("Metadata item covers frames [{}-{}] totalDur={} size={} bytes", startFrame,
              endFrame - 1, totalDur, item.payload.size());
  }

  return MP4NoErr;
}

// Helper: Add all metadata samples
static MP4Err addAllMetadataSamples(MP4Media mediaM, const std::vector<MetadataItem> &sortedItems,
                                    const std::vector<u32> &metadataDurations,
                                    const std::vector<u32> &metadataSizes, u32 local_key_id)
{
  MP4Err err      = MP4NoErr;
  u32 sampleCount = static_cast<u32>(sortedItems.size());

  MP4Handle durationsH  = nullptr;
  MP4Handle sizesH      = nullptr;
  MP4Handle sampleDataH = nullptr;
  u64 totalSize         = 0;

  if(sampleCount == 0)
  {
    LOG_ERROR("No metadata samples to add");
    return MP4BadParamErr;
  }

  // --- Durations handle ---
  {
    bool allSame = std::all_of(metadataDurations.begin(), metadataDurations.end(),
                               [&](u32 d) { return d == metadataDurations[0]; });
    if(allSame)
    {
      err = MP4NewHandle(sizeof(u32), &durationsH);
      if(err) goto bail;
      *((u32 *)*durationsH) = metadataDurations[0];
    }
    else
    {
      err = MP4NewHandle(sizeof(u32) * sampleCount, &durationsH);
      if(err) goto bail;
      for(u32 n = 0; n < sampleCount; ++n)
      {
        ((u32 *)*durationsH)[n] = metadataDurations[n];
      }
    }
  }

  // --- Sizes handle ---
  {
    bool allSame = std::all_of(metadataSizes.begin(), metadataSizes.end(),
                               [&](u32 s) { return s == metadataSizes[0]; });
    if(allSame)
    {
      err = MP4NewHandle(sizeof(u32), &sizesH);
      if(err) goto bail;
      *((u32 *)*sizesH) = metadataSizes[0] + 8; // +4 box_size +4 box_type
    }
    else
    {
      err = MP4NewHandle(sizeof(u32) * sampleCount, &sizesH);
      if(err) goto bail;
      for(u32 n = 0; n < sampleCount; ++n)
      {
        ((u32 *)*sizesH)[n] = metadataSizes[n] + 8; // +4 box_size +4 box_type
      }
    }
  }

  // --- Sample data handle ---
  totalSize = 0;
  for(u32 n = 0; n < sampleCount; ++n)
  {
    totalSize += metadataSizes[n] + 8;
  }
  err = MP4NewHandle((u32)totalSize, &sampleDataH);
  if(err) goto bail;

  {
    char *dst = reinterpret_cast<char *>(*sampleDataH);
    for(u32 n = 0; n < sampleCount; ++n)
    {
      const MetadataItem &item = sortedItems[n];

      u32 boxSize = 8 + metadataSizes[n];
      // write size
      dst[0] = (boxSize >> 24) & 0xFF;
      dst[1] = (boxSize >> 16) & 0xFF;
      dst[2] = (boxSize >> 8) & 0xFF;
      dst[3] = (boxSize) & 0xFF;
      // write type (local_key_id)
      dst[4] = (local_key_id >> 24) & 0xFF;
      dst[5] = (local_key_id >> 16) & 0xFF;
      dst[6] = (local_key_id >> 8) & 0xFF;
      dst[7] = (local_key_id) & 0xFF;
      dst += 8;

      // Copy payload from memory
      std::memcpy(dst, item.payload.data(), item.payload.size());
      dst += metadataSizes[n];
    }
  }

  // --- Add all samples in one call ---
  err = MP4AddMediaSamples(mediaM, sampleDataH, sampleCount, durationsH, sizesH,
                           0,  // reuse sample entry
                           0,  // no decoding offsets
                           0); // all sync samples
  if(err)
  {
    LOG_ERROR("MP4AddMediaSamples failed (err={})", err);
    goto bail;
  }

  LOG_INFO("Added {} metadata samples", sampleCount);

bail:
  if(sampleDataH) MP4DisposeHandle(sampleDataH);
  if(durationsH) MP4DisposeHandle(durationsH);
  if(sizesH) MP4DisposeHandle(sizesH);

  return err;
}

// Helper: Create handle with 4-character code
static MP4Err fourCCToHandle(u32 fourCC, MP4Handle *outHandle)
{
  MP4Err err = MP4NoErr;
  *outHandle = nullptr;

  err = MP4NewHandle(4, outHandle);
  if(err) return err;

  char *data = (char *)**outHandle; // Dereference to get the data pointer
  data[0]    = (fourCC >> 24) & 0xFF;
  data[1]    = (fourCC >> 16) & 0xFF;
  data[2]    = (fourCC >> 8) & 0xFF;
  data[3]    = (fourCC) & 0xFF;

  return MP4NoErr;
}

// Helper: Convert string to handle (as text, not hex)
static MP4Err stringToHandle(const std::string &input, MP4Handle *outHandle)
{
  MP4Err err = MP4NoErr;
  *outHandle = nullptr;

  // Copy input string as text
  u32 byteCount = static_cast<u32>(input.size());
  err           = MP4NewHandle(byteCount, outHandle);
  if(err) return err;

  std::memcpy(**outHandle, input.data(), byteCount);

  return MP4NoErr;
}

bool MebxMe4cStrategy::isApplicable(const MetadataMap &items, const InjectionConfig &config,
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

  return true;
}

MP4Err MebxMe4cStrategy::inject(const InjectionConfig &config, const MetadataMap &items,
                                const T35Prefix &prefix)
{
  LOG_INFO("Injecting metadata using mebx-me4c strategy");

  MP4Err err      = MP4NoErr;
  MP4Track trakM  = nullptr; // metadata track
  MP4Track trakV  = nullptr; // reference to video track
  MP4Media mediaM = nullptr;

  // Find video track
  LOG_DEBUG("Finding first video track");
  err = findFirstVideoTrack(config.movie, &trakV);
  if(err)
  {
    LOG_ERROR("Failed to find video track (err={})", err);
    return err;
  }

  // Create mebx track
  LOG_DEBUG("Creating mebx track");
  err = MP4NewMovieTrack(config.movie, MP4NewTrackIsMebx, &trakM);
  if(err)
  {
    LOG_ERROR("Failed to create mebx track (err={})", err);
    return err;
  }

  // Get video media and timescale
  MP4Media videoMedia = nullptr;
  u32 videoTimescale  = 0;
  err                 = MP4GetTrackMedia(trakV, &videoMedia);
  if(err)
  {
    LOG_ERROR("Failed to get video media (err={})", err);
    return err;
  }

  err = MP4GetMediaTimeScale(videoMedia, &videoTimescale);
  if(err)
  {
    videoTimescale = 1000; // default to 1000 if not available
    LOG_WARN("Failed to get video timescale, defaulting to 1000");
  }
  LOG_DEBUG("Video timescale: {}", videoTimescale);

  // Get video sample durations
  std::vector<u32> videoDurations;
  err = getVideoSampleDurations(videoMedia, videoDurations);
  if(err)
  {
    LOG_ERROR("Failed to get video sample durations (err={})", err);
    return err;
  }

  // Create mebx media with same timescale as video
  LOG_DEBUG("Creating mebx media with timescale {}", videoTimescale);
  err = MP4NewTrackMedia(trakM, &mediaM, MP4MetaHandlerType, videoTimescale, NULL);
  if(err)
  {
    LOG_ERROR("Failed to create mebx media (err={})", err);
    return err;
  }

  // Link metadata track to video track using 'rndr' track reference
  LOG_DEBUG("Adding track reference");
  err = MP4AddTrackReference(trakM, trakV, MP4_FOUR_CHAR_CODE('r', 'n', 'd', 'r'), 0);
  if(err)
  {
    LOG_ERROR("Failed to add track reference (err={})", err);
    return err;
  }

  // Create mebx sample entry
  LOG_DEBUG("Creating mebx sample entry");
  MP4BoxedMetadataSampleEntryPtr mebx = nullptr;
  err                                 = ISONewMebxSampleDescription(&mebx, 1);
  if(err)
  {
    LOG_ERROR("Failed to create mebx sample description (err={})", err);
    return err;
  }

  // For me4c strategy:
  // - key_namespace = 'me4c'
  // - key_value = 'it35' (4-character code)
  // - setupInfo = T.35 prefix string
  LOG_DEBUG("Using me4c namespace with it35 key_value");

  // Build key_value as 'it35' 4CC
  MP4Handle key_value = nullptr;
  err                 = fourCCToHandle(MP4_FOUR_CHAR_CODE('i', 't', '3', '5'), &key_value);
  if(err)
  {
    LOG_ERROR("Failed to create it35 key_value handle (err={})", err);
    return err;
  }
  LOG_DEBUG("Created key_value handle with it35 4CC");

  // Build setupInfo with T.35 prefix in binary format:
  // 1. utf8string description (null-terminated, '\0' if empty)
  // 2. unsigned int(8) t35_identifier[] (binary bytes)
  MP4Handle setupInfo = nullptr;
  {
    const std::string &desc              = prefix.description();
    std::vector<uint8_t> identifierBytes = prefix.toBytes();

    // Calculate total size: description length + null terminator + identifier bytes
    u32 descLen   = desc.empty() ? 1 : (u32)desc.size() + 1; // '\0' if empty, or string + '\0'
    u32 totalSize = descLen + (u32)identifierBytes.size();

    err = MP4NewHandle(totalSize, &setupInfo);
    if(err)
    {
      LOG_ERROR("Failed to create setupInfo handle (err={})", err);
      MP4DisposeHandle(key_value);
      return err;
    }

    char *buffer = *setupInfo;
    u32 offset   = 0;

    // Write description as null-terminated UTF-8 string
    if(desc.empty())
    {
      buffer[offset++] = '\0'; // Just null byte if no description
    }
    else
    {
      memcpy(buffer + offset, desc.c_str(), desc.size());
      offset += desc.size();
      buffer[offset++] = '\0'; // Null terminator
    }

    // Write t35_identifier as binary bytes
    if(!identifierBytes.empty())
    {
      memcpy(buffer + offset, identifierBytes.data(), identifierBytes.size());
      offset += identifierBytes.size();
    }

    LOG_DEBUG("Created setupInfo handle: description='{}' ({} bytes), identifier={} bytes",
              desc.empty() ? "(empty)" : desc, descLen, identifierBytes.size());
  }

  // Add sample entry with me4c namespace
  // For me4c namespace, desired_local_key_id must match the 4CC in key_value
  u32 desired_key_id = MP4_FOUR_CHAR_CODE('i', 't', '3', '5');
  u32 local_key_id   = 0;
  LOG_DEBUG("Calling ISOAddMebxMetadataToSampleEntry with me4c namespace");
  err = ISOAddMebxMetadataToSampleEntry(mebx,
                                        desired_key_id, // Must match key_value for me4c
                                        &local_key_id,
                                        MP4_FOUR_CHAR_CODE('m', 'e', '4', 'c'), // me4c namespace
                                        key_value,                              // 'it35' 4CC
                                        NULL,       // locale_string (not used)
                                        setupInfo); // T.35 prefix string

  MP4DisposeHandle(key_value);
  MP4DisposeHandle(setupInfo);

  if(err)
  {
    LOG_ERROR("Failed to add mebx metadata to sample entry (err={})", err);
    return err;
  }

  MP4Handle sampleEntryMH = nullptr;
  err                     = MP4NewHandle(0, &sampleEntryMH);
  if(err)
  {
    LOG_ERROR("Failed to create sample entry handle (err={})", err);
    return err;
  }

  err = ISOGetMebxHandle(mebx, sampleEntryMH);
  if(err)
  {
    LOG_ERROR("Failed to get mebx handle (err={})", err);
    return err;
  }

  err = MP4AddMediaSamples(mediaM, 0, 0, 0, 0, sampleEntryMH, 0, 0);
  if(err)
  {
    LOG_ERROR("Failed to add sample entry (err={})", err);
    return err;
  }

  LOG_INFO("MEBX track and sample entry created successfully");
  LOG_INFO("Local key ID = {}", local_key_id);
  LOG_INFO("Namespace: me4c, Key: it35, Setup: {}", prefix.toString());

  // Prepare metadata sample durations and sizes
  std::vector<u32> metadataDurations;
  std::vector<u32> metadataSizes;
  std::vector<MetadataItem> sortedItems;

  err = buildMetadataDurationsAndSizes(items, videoDurations, metadataDurations, metadataSizes,
                                       sortedItems);
  if(err)
  {
    LOG_ERROR("Failed to build metadata durations and sizes (err={})", err);
    return err;
  }

  // Add all metadata samples
  err = addAllMetadataSamples(mediaM, sortedItems, metadataDurations, metadataSizes, local_key_id);
  if(err)
  {
    LOG_ERROR("Failed to add metadata samples (err={})", err);
    return err;
  }

  // End media edits
  err = MP4EndMediaEdits(mediaM);
  if(err)
  {
    LOG_ERROR("Failed to end media edits (err={})", err);
    return err;
  }

  LOG_INFO("Metadata injection complete");
  return MP4NoErr;
}

} // namespace t35
