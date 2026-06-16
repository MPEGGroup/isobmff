#pragma once

#include "../common/MetadataTypes.hpp"
#include <vector>

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
}

namespace t35
{

/**
 * @brief Find the first video track in a movie.
 * @param moov The movie to search.
 * @param outTrack Output parameter for the found track.
 * @return MP4Err (0 = success)
 */
MP4Err findFirstVideoTrack(MP4Movie moov, MP4Track *outTrack);

/**
 * @brief Get durations of all samples in a video media.
 * @param mediaV The video media.
 * @param durations Output vector for durations.
 * @return MP4Err (0 = success)
 */
MP4Err getVideoSampleDurations(MP4Media mediaV, std::vector<u32> &durations);

/**
 * @brief Build metadata durations and sizes from a map of items and video durations.
 * @param items Map of metadata items.
 * @param videoDurations Vector of video sample durations.
 * @param metadataDurations Output vector for metadata durations.
 * @param metadataSizes Output vector for metadata sizes.
 * @param sortedItems Output vector for sorted metadata items.
 * @return MP4Err (0 = success)
 */
MP4Err buildMetadataDurationsAndSizes(const MetadataMap &items,
                                      const std::vector<u32> &videoDurations,
                                      std::vector<u32> &metadataDurations,
                                      std::vector<u32> &metadataSizes,
                                      std::vector<MetadataItem> &sortedItems);

} // namespace t35
