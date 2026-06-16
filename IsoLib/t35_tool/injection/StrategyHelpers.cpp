#include "StrategyHelpers.hpp"
#include "../common/Logger.hpp"

#include <algorithm>
#include <cstring>
#include <numeric>

namespace t35
{

MP4Err findFirstVideoTrack(MP4Movie moov, MP4Track *outTrack)
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

MP4Err getVideoSampleDurations(MP4Media mediaV, std::vector<u32> &durations)
{
  MP4Err err      = MP4NoErr;
  u32 sampleCount = 0;

  durations.clear();

  err = MP4GetMediaSampleCount(mediaV, &sampleCount);
  if(err) return err;

  struct SampleInfo
  {
    u64 cts;
    u32 duration;
  };
  std::vector<SampleInfo> samples;
  samples.reserve(sampleCount);

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
    samples.push_back({cts, static_cast<u32>(outDuration)});

    if(sampleH) MP4DisposeHandle(sampleH);
  }

  // Sort by CTS
  std::sort(samples.begin(), samples.end(),
            [](const SampleInfo &a, const SampleInfo &b) { return a.cts < b.cts; });

  durations.reserve(sampleCount);
  for(const auto &s : samples)
  {
    durations.push_back(s.duration);
  }

  LOG_DEBUG("Collected {} video sample durations", durations.size());
  return MP4NoErr;
}

MP4Err buildMetadataDurationsAndSizes(const MetadataMap &items,
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
    u32 totalDur =
      std::accumulate(videoDurations.begin() + startFrame, videoDurations.begin() + endFrame, 0u);

    metadataDurations.push_back(totalDur);
    metadataSizes.push_back(static_cast<u32>(item.payload.size()));

    LOG_DEBUG("Metadata item covers frames [{}-{}] totalDur={} size={} bytes", startFrame,
              endFrame - 1, totalDur, item.payload.size());
  }

  return MP4NoErr;
}

} // namespace t35
