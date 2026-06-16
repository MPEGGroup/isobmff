#pragma once

#include "ExtractionStrategy.hpp"

// Forward declarations from libisomediafile
extern "C" {
struct MP4TrackRecord;
typedef struct MP4TrackRecord* MP4Track;
}

namespace t35 {

/**
 * Dedicated IT35 metadata track extractor
 * Extracts from tracks with 'it35' sample entry (T35MetadataSampleEntry)
 */
class DedicatedIt35Extractor : public ExtractionStrategy {
public:
    DedicatedIt35Extractor() = default;
    virtual ~DedicatedIt35Extractor();

    std::string getName() const override { return "dedicated-it35"; }

    std::string getDescription() const override {
        return "Extract from dedicated IT35 metadata track";
    }

    bool canExtract(const ExtractionConfig& config,
                   std::string& reason) override;

    MP4Err extract(const ExtractionConfig& config, MetadataMap* outItems = nullptr) override;

private:
    // Cache the track found in canExtract() for use in extract()
    MP4Track m_cachedTrack = nullptr;

    void clearCache();
};

} // namespace t35
