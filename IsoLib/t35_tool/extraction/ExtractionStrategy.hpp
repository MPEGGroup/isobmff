#pragma once

#include "../common/MetadataTypes.hpp"
#include "../common/T35Prefix.hpp"

// Forward declarations from libisomediafile
extern "C" {
typedef int MP4Err;
}

#include <memory>
#include <string>

namespace t35 {

/**
 * Abstract interface for metadata extraction strategies
 *
 * An ExtractionStrategy handles:
 * - Finding metadata in MP4 container (tracks, groups, etc.)
 * - Reading metadata samples
 * - Writing output files (binary, JSON, or video with SEI)
 *
 * Different strategies extract from different MP4 storage methods:
 * - MEBX tracks (me4c namespace)
 * - Dedicated metadata tracks
 * - Sample groups
 * - Sample entry boxes
 * - SEI conversion (metadata → video with SEI NALs)
 */
class ExtractionStrategy {
public:
    virtual ~ExtractionStrategy() = default;

    /**
     * Get strategy name
     * @return Name string (e.g., "mebx-me4c", "auto", "sei")
     */
    virtual std::string getName() const = 0;

    /**
     * Get strategy description
     * @return Human-readable description
     */
    virtual std::string getDescription() const = 0;

    /**
     * Check if this strategy can extract from the given movie
     *
     * @param config Extraction configuration (movie, prefix, etc.)
     * @param reason Output parameter for reason if cannot extract
     * @return true if strategy can extract
     */
    virtual bool canExtract(const ExtractionConfig& config,
                           std::string& reason) = 0;

    /**
     * Extract metadata from movie
     *
     * @param config Configuration (movie, output path, prefix, etc.)
     * @param outItems Optional output parameter for in-memory extraction.
     *                 If non-null, metadata is returned in this map instead of writing files.
     *                 If null (default), writes files to config.outputPath as before.
     * @return MP4Err (0 = success)
     * @throws T35Exception on error
     *
     * Output format depends on strategy and outItems parameter:
     * - If outItems == nullptr: Write .bin files + manifest.json (original behavior)
     * - If outItems != nullptr: Populate MetadataMap with in-memory data
     * - SEI extractor: Always writes .hevc/.265 video file (ignores outItems)
     */
    virtual MP4Err extract(const ExtractionConfig& config, MetadataMap* outItems = nullptr) = 0;
};

/**
 * Factory function to create extraction strategy from name
 *
 * Available strategies:
 * - "auto": Auto-detect (tries all strategies)
 * - "mebx-me4c": MEBX track with me4c namespace
 * - "dedicated-it35": Dedicated metadata track
 * - "sample-group": Sample group
 * - "sei": Convert metadata to video with SEI NALs
 *
 * @param strategyName Strategy name
 * @return Unique pointer to ExtractionStrategy
 * @throws T35Exception if strategy is unknown
 */
std::unique_ptr<ExtractionStrategy> createExtractionStrategy(const std::string& strategyName);

} // namespace t35
