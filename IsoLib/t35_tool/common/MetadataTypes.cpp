#include "MetadataTypes.hpp"

namespace t35
{

bool validateMetadataMap(const MetadataMap &items, std::string &errorMsg)
{
  if(items.empty())
  {
    errorMsg = "Metadata map is empty";
    return false;
  }

  // Check each item individually
  for(const auto &[frameStart, item] : items)
  {
    // Check frame_start matches map key
    if(item.frame_start != frameStart)
    {
      errorMsg = "Item frame_start (" + std::to_string(item.frame_start) +
                 ") doesn't match map key (" + std::to_string(frameStart) + ")";
      return false;
    }

    // Check duration is positive
    if(item.frame_duration == 0)
    {
      errorMsg = "Item at frame " + std::to_string(frameStart) + " has zero duration";
      return false;
    }

    // Check payload is not empty
    if(item.payload.empty())
    {
      errorMsg = "Item at frame " + std::to_string(frameStart) + " has empty payload";
      return false;
    }
  }

  // Check for overlaps
  MetadataItem prevItem;
  bool first = true;

  for(const auto &[frameStart, item] : items)
  {
    if(!first)
    {
      if(prevItem.overlaps(item))
      {
        errorMsg = "Overlapping metadata entries: frames [" + std::to_string(prevItem.frame_start) +
                   "-" + std::to_string(prevItem.frameEnd()) + ") and [" +
                   std::to_string(item.frame_start) + "-" + std::to_string(item.frameEnd()) + ")";
        return false;
      }
    }
    prevItem = item;
    first    = false;
  }

  return true;
}

bool isFullyCovering(const MetadataMap &items, uint32_t totalFrames)
{
  if(items.empty() || totalFrames == 0)
  {
    return false;
  }

  // Check if first item starts at frame 0
  if(items.begin()->first != 0)
  {
    return false;
  }

  // Check coverage
  uint32_t expectedNext = 0;
  for(const auto &[frameStart, item] : items)
  {
    if(frameStart != expectedNext)
    {
      return false; // Gap detected
    }
    expectedNext = item.frameEnd();
  }

  // Check if we covered all frames
  return expectedNext >= totalFrames;
}

bool isStaticMetadata(const MetadataMap &items, uint32_t totalFrames)
{
  if(items.size() != 1)
  {
    return false;
  }

  const auto &item = items.begin()->second;
  return item.frame_start == 0 && item.frameEnd() >= totalFrames;
}

} // namespace t35
