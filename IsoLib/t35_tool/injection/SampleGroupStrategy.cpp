#include "SampleGroupStrategy.hpp"
#include "StrategyHelpers.hpp"
#include "../common/Logger.hpp"
#include "../common/T35Prefix.hpp"
#include "../common/MetadataTypes.hpp"

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
}

#include <algorithm>
#include <cstring>

namespace t35
{

bool SampleGroupStrategy::isApplicable(const MetadataMap &items, const InjectionConfig &config,
                                       std::string &reason) const
{
  (void)config; // Unused for this strategy

  if(items.empty())
  {
    reason = "No metadata items to inject";
    return false;
  }

  // Sample groups can handle both static and dynamic metadata
  return true;
}

MP4Err SampleGroupStrategy::inject(const InjectionConfig &config, const MetadataMap &items,
                                   const T35Prefix &prefix)
{
  LOG_INFO("Injecting metadata using sample-group strategy");
  LOG_INFO("T.35 prefix: {}", prefix.toString());

  MP4Err err = MP4NoErr;

  // Validate
  if(!config.movie)
  {
    LOG_ERROR("Invalid injection config: missing movie");
    return MP4BadParamErr;
  }

  if(items.empty())
  {
    LOG_ERROR("No metadata items to inject");
    return MP4BadParamErr;
  }

  // Find video track
  LOG_DEBUG("Finding first video track");
  MP4Track videoTrack = nullptr;
  err                 = findFirstVideoTrack(config.movie, &videoTrack);
  if(err)
  {
    LOG_ERROR("Failed to find video track (err={})", err);
    return err;
  }

  // Get video media
  MP4Media videoMedia = nullptr;
  err                 = MP4GetTrackMedia(videoTrack, &videoMedia);
  if(err != MP4NoErr)
  {
    LOG_ERROR("Failed to get video track media (err={})", err);
    return err;
  }

  u32 videoTrackID = 0;
  MP4GetTrackID(videoTrack, &videoTrackID);
  LOG_INFO("Using video track ID {}", videoTrackID);

  // Get video sample count
  u32 videoSampleCount = 0;
  err                  = MP4GetMediaSampleCount(videoMedia, &videoSampleCount);
  if(err != MP4NoErr)
  {
    LOG_ERROR("Failed to get video sample count (err={})", err);
    return err;
  }
  LOG_INFO("Video track has {} samples", videoSampleCount);

  // Sort metadata items by frame_start
  std::vector<std::pair<u32, MetadataItem>> sortedItems(items.begin(), items.end());
  std::sort(sortedItems.begin(), sortedItems.end(),
            [](const auto &a, const auto &b) { return a.first < b.first; });

  LOG_INFO("Processing {} metadata items", sortedItems.size());

  // For each metadata item:
  // 1. Add T.35 group description (sgpd)
  // 2. Map video samples to this group (sbgp)

  for(size_t i = 0; i < sortedItems.size(); ++i)
  {
    const auto &[frameStart, item] = sortedItems[i];
    u32 frameEnd                   = frameStart + item.frame_duration;

    LOG_DEBUG("Processing metadata item {}: frames {}-{} ({} frames)", i + 1, frameStart,
              frameEnd - 1, item.frame_duration);

    // Create handle for T.35 payload
    MP4Handle t35DataH = nullptr;
    err                = MP4NewHandle(item.payload.size(), &t35DataH);
    if(err != MP4NoErr)
    {
      LOG_ERROR("Failed to create handle for T.35 data (err={})", err);
      return err;
    }

    std::memcpy(*t35DataH, item.payload.data(), item.payload.size());

    // Add T.35 group description
    u32 groupIndex = 0;
    err            = ISOAddT35GroupDescription(videoMedia, t35DataH,
                                               1, // complete_message_flag = 1
                                               &groupIndex);

    MP4DisposeHandle(t35DataH);

    if(err != MP4NoErr)
    {
      LOG_ERROR("Failed to add T.35 group description (err={})", err);
      return err;
    }

    LOG_DEBUG("Added T.35 group description at index {}", groupIndex);

    // Calculate which video samples correspond to these frames
    // video sample index is 1-based
    u32 firstSample = frameStart + 1; // 1-based
    u32 sampleCount = item.frame_duration;

    // Clamp to video track bounds
    if(firstSample > videoSampleCount)
    {
      LOG_WARN("Metadata starts at frame {} but video only has {} samples, skipping", frameStart,
               videoSampleCount);
      continue;
    }

    if(firstSample + sampleCount - 1 > videoSampleCount)
    {
      u32 oldCount = sampleCount;
      sampleCount  = videoSampleCount - firstSample + 1;
      LOG_WARN("Metadata extends beyond video track: clamping from {} to {} samples", oldCount,
               sampleCount);
    }

    // Map video samples to this group
    err = ISOMapSamplestoGroup(videoMedia,
                               MP4T35SampleGroupEntry, // 'it35'
                               groupIndex, firstSample, sampleCount);

    if(err != MP4NoErr)
    {
      LOG_ERROR("Failed to map samples to group (err={})", err);
      return err;
    }

    LOG_INFO("Mapped video samples {}-{} to T.35 group {} ({} bytes)", firstSample,
             firstSample + sampleCount - 1, groupIndex, item.payload.size());
  }

  LOG_INFO("Successfully injected {} T.35 metadata items into video track using sample groups",
           sortedItems.size());

  return MP4NoErr;
}

} // namespace t35
