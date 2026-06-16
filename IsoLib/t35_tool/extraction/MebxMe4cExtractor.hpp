#pragma once

#include "ExtractionStrategy.hpp"

// Forward declarations from libisomediafile
extern "C" {
struct MP4TrackReaderRecord;
typedef struct MP4TrackReaderRecord* MP4TrackReader;
struct MP4TrackRecord;
typedef struct MP4TrackRecord* MP4Track;
}

namespace t35 {

/**
 * MEBX track with me4c namespace extractor
 * Extracts from mebx tracks using me4c namespace with 'it35' key_value
 */
class MebxMe4cExtractor : public ExtractionStrategy {
public:
    MebxMe4cExtractor() = default;
    virtual ~MebxMe4cExtractor();

    std::string getName() const override { return "mebx-me4c"; }

    std::string getDescription() const override {
        return "Extract from MEBX metadata track with me4c namespace";
    }

    bool canExtract(const ExtractionConfig& config,
                   std::string& reason) override;

    MP4Err extract(const ExtractionConfig& config, MetadataMap* outItems = nullptr) override;

private:
    // Cache the reader and track found in canExtract() for use in extract()
    MP4TrackReader m_cachedReader = nullptr;
    MP4Track m_cachedTrack = nullptr;

    void clearCache();
};

} // namespace t35
