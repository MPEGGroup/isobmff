#pragma once

#include <cstdint>
#include <exception>
#include <map>
#include <string>
#include <vector>

// Forward declarations from libisomediafile
// These will be properly defined when MP4Movies.h is included
#ifndef MP4Movie
struct MP4MovieRecord;
typedef struct MP4MovieRecord* MP4Movie;
#endif

#ifndef MP4Track
struct MP4TrackRecord;
typedef struct MP4TrackRecord* MP4Track;
#endif

namespace t35 {

// ============================================================================
// Core Data Types
// ============================================================================

/**
 * Single metadata item with timing and binary payload
 */
struct MetadataItem {
    uint32_t frame_start;            // Starting frame number
    uint32_t frame_duration;         // Duration in frames
    std::vector<uint8_t> payload;    // Binary T.35 payload (without T.35 prefix)
    std::string source_info;         // Debug: where this came from (filename, etc.)

    MetadataItem() : frame_start(0), frame_duration(0) {}

    MetadataItem(uint32_t start, uint32_t duration, std::vector<uint8_t> data,
                 const std::string& info = "")
        : frame_start(start)
        , frame_duration(duration)
        , payload(std::move(data))
        , source_info(info)
    {}

    // Frame range end (exclusive)
    uint32_t frameEnd() const { return frame_start + frame_duration; }

    // Check if this item overlaps with another
    bool overlaps(const MetadataItem& other) const {
        return frame_start < other.frameEnd() && frameEnd() > other.frame_start;
    }
};

/**
 * Collection of metadata items indexed by frame_start
 * Sorted map ensures items are in frame order
 */
using MetadataMap = std::map<uint32_t, MetadataItem>;

// ============================================================================
// Configuration Structures
// ============================================================================

/**
 * Configuration passed to injection strategies
 */
struct InjectionConfig {
    MP4Movie movie;                         // Movie to inject into
    MP4Track videoTrack;                    // Reference video track
    std::vector<uint32_t> videoSampleDurations;  // Video sample durations in timescale units
    std::string t35Prefix;                  // T.35 prefix hex string (e.g., "B500900001")

    InjectionConfig()
        : movie(nullptr)
        , videoTrack(nullptr)
    {}
};

/**
 * Configuration passed to extraction strategies
 */
struct ExtractionConfig {
    MP4Movie movie;                         // Movie to extract from
    std::string outputPath;                 // Output directory or file path
    std::string t35Prefix;                  // T.35 prefix to look for (e.g., "B500900001")

    ExtractionConfig()
        : movie(nullptr)
    {}
};

// ============================================================================
// Error Handling
// ============================================================================

/**
 * Error codes for T.35 tool operations
 */
enum class T35Error {
    Success = 0,
    InvalidJSON,
    MissingFiles,
    SourceError,
    InjectionFailed,
    ExtractionFailed,
    NoMetadataFound,
    ValidationFailed,
    MP4Error,
    NotImplemented
};

/**
 * Exception type for T.35 tool errors
 */
class T35Exception : public std::exception {
public:
    T35Exception(T35Error err, const std::string& msg)
        : code(err)
        , message(msg)
    {
        fullMessage = "T35Error(" + std::to_string(static_cast<int>(err)) + "): " + msg;
    }

    const char* what() const noexcept override {
        return fullMessage.c_str();
    }

    T35Error getCode() const { return code; }
    const std::string& getMessage() const { return message; }

private:
    T35Error code;
    std::string message;
    std::string fullMessage;
};

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Validate a MetadataMap
 * Checks for:
 * - No overlapping frame ranges
 * - Valid frame numbers
 * - Non-empty payloads
 *
 * @param items The metadata map to validate
 * @param errorMsg Output parameter for error message
 * @return true if valid, false otherwise
 */
bool validateMetadataMap(const MetadataMap& items, std::string& errorMsg);

/**
 * Check if metadata map covers all frames (no gaps)
 *
 * @param items The metadata map to check
 * @param totalFrames Total number of frames in video
 * @return true if all frames are covered
 */
bool isFullyCovering(const MetadataMap& items, uint32_t totalFrames);

/**
 * Check if metadata map has single static entry covering all frames
 *
 * @param items The metadata map to check
 * @param totalFrames Total number of frames in video
 * @return true if single static entry
 */
bool isStaticMetadata(const MetadataMap& items, uint32_t totalFrames);

} // namespace t35
