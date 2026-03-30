#include "DedicatedIt35Strategy.hpp"
#include "../common/Logger.hpp"

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
}

#include <algorithm>
#include <numeric>
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
    u32 totalDur =
      std::accumulate(videoDurations.begin() + startFrame, videoDurations.begin() + endFrame, 0u);

    metadataDurations.push_back(totalDur);
    metadataSizes.push_back(static_cast<u32>(item.payload.size()));

    LOG_DEBUG("Metadata item covers frames [{}-{}] totalDur={} size={} bytes", startFrame,
              endFrame - 1, totalDur, item.payload.size());
  }

  return MP4NoErr;
}

// Helper: Build sample data (just payloads, no box wrapper)
static MP4Err buildSampleData(const std::vector<MetadataItem> &sortedItems,
                              const std::vector<u32> &metadataSizes, MP4Handle *outSampleDataH)
{
  MP4Err err      = MP4NoErr;
  *outSampleDataH = nullptr;

  if(sortedItems.empty())
  {
    return MP4NoErr;
  }

  // Calculate total size (just payloads, no box wrapper)
  u64 totalSize = 0;
  for(u32 size : metadataSizes)
  {
    totalSize += size;
  }

  err = MP4NewHandle(static_cast<u32>(totalSize), outSampleDataH);
  if(err) return err;

  // Copy payloads directly
  char *dst = reinterpret_cast<char *>(**outSampleDataH);
  for(u32 n = 0; n < sortedItems.size(); ++n)
  {
    const MetadataItem &item = sortedItems[n];
    std::memcpy(dst, item.payload.data(), item.payload.size());
    dst += metadataSizes[n];
  }

  return MP4NoErr;
}

bool DedicatedIt35Strategy::isApplicable(const MetadataMap &items, const InjectionConfig &config,
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

MP4Err DedicatedIt35Strategy::inject(const InjectionConfig &config, const MetadataMap &items,
                                     const T35Prefix &prefix)
{
  LOG_INFO("Injecting metadata using dedicated IT35 track strategy");

  MP4Err err            = MP4NoErr;
  MP4Track trakM        = nullptr; // metadata track
  MP4Track trakV        = nullptr; // reference to video track
  MP4Media mediaM       = nullptr;
  MP4Handle sampleDataH = nullptr;
  MP4Handle durationsH  = nullptr;
  MP4Handle sizesH      = nullptr;
  MP4Media videoMedia   = nullptr;
  u32 videoTimescale    = 0;
  u32 sampleCount       = 0;
  std::vector<u32> videoDurations;
  std::vector<u32> metadataDurations;
  std::vector<u32> metadataSizes;
  std::vector<MetadataItem> sortedItems;

  // Find video track
  LOG_DEBUG("Finding first video track");
  err = findFirstVideoTrack(config.movie, &trakV);
  if(err)
  {
    LOG_ERROR("Failed to find video track (err={})", err);
    goto bail;
  }

  // Get video media and timescale
  err = MP4GetTrackMedia(trakV, &videoMedia);
  if(err)
  {
    LOG_ERROR("Failed to get video media (err={})", err);
    goto bail;
  }

  err = MP4GetMediaTimeScale(videoMedia, &videoTimescale);
  if(err)
  {
    videoTimescale = 1000; // default to 1000 if not available
    LOG_WARN("Failed to get video timescale, defaulting to 1000");
  }
  LOG_DEBUG("Video timescale: {}", videoTimescale);

  // Get video sample durations
  err = getVideoSampleDurations(videoMedia, videoDurations);
  if(err)
  {
    LOG_ERROR("Failed to get video sample durations (err={})", err);
    goto bail;
  }

  // Create dedicated IT35 metadata track
  LOG_DEBUG("Creating dedicated IT35 metadata track with T.35 prefix: {}", prefix.toString());
  err = ISONewT35MetadataTrack(config.movie, videoTimescale, prefix.toString().c_str(),
                               trakV,                         // video track reference
                               MP4RndrTrackReferenceAtomType, // 'rndr' track reference
                               &trakM, &mediaM);
  if(err)
  {
    LOG_ERROR("Failed to create IT35 metadata track (err={})", err);
    goto bail;
  }

  // Build metadata durations and sizes
  err = buildMetadataDurationsAndSizes(items, videoDurations, metadataDurations, metadataSizes,
                                       sortedItems);
  if(err)
  {
    LOG_ERROR("Failed to build metadata durations/sizes (err={})", err);
    goto bail;
  }

  sampleCount = static_cast<u32>(sortedItems.size());
  LOG_DEBUG("Prepared {} metadata samples", sampleCount);

  // Build durations handle
  err = MP4NewHandle(sampleCount * sizeof(u32), &durationsH);
  if(err)
  {
    LOG_ERROR("Failed to create durations handle (err={})", err);
    goto bail;
  }

  {
    u32 *durationPtr = reinterpret_cast<u32 *>(*durationsH);
    for(u32 n = 0; n < sampleCount; ++n)
    {
      durationPtr[n] = metadataDurations[n];
    }
  }

  // Build sizes handle
  err = MP4NewHandle(sampleCount * sizeof(u32), &sizesH);
  if(err)
  {
    LOG_ERROR("Failed to create sizes handle (err={})", err);
    goto bail;
  }

  {
    u32 *sizePtr = reinterpret_cast<u32 *>(*sizesH);
    for(u32 n = 0; n < sampleCount; ++n)
    {
      sizePtr[n] = metadataSizes[n];
    }
  }

  // Build sample data (just payloads, no box wrapper)
  err = buildSampleData(sortedItems, metadataSizes, &sampleDataH);
  if(err)
  {
    LOG_ERROR("Failed to build sample data (err={})", err);
    goto bail;
  }

  // Add all samples in one call
  err = MP4AddMediaSamples(mediaM, sampleDataH, sampleCount, durationsH, sizesH,
                           0,  // reuse sample entry
                           0,  // no decoding offsets
                           0); // all sync samples
  if(err)
  {
    LOG_ERROR("MP4AddMediaSamples failed (err={})", err);
    goto bail;
  }

  LOG_INFO("Added {} metadata samples to dedicated IT35 track", sampleCount);

  // Finalize media edits to commit durations
  err = MP4EndMediaEdits(mediaM);
  if(err)
  {
    LOG_ERROR("Failed to end media edits (err={})", err);
    goto bail;
  }

  LOG_INFO("Metadata injection complete");

bail:
  if(sampleDataH) MP4DisposeHandle(sampleDataH);
  if(durationsH) MP4DisposeHandle(durationsH);
  if(sizesH) MP4DisposeHandle(sizesH);

  return err;
}

} // namespace t35
