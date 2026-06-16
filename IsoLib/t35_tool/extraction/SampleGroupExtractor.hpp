#pragma once

#include "ExtractionStrategy.hpp"

namespace t35 {

/**
 * Sample Group extractor
 * Extracts T.35 metadata from video track sample groups (sgpd/sbgp)
 */
class SampleGroupExtractor : public ExtractionStrategy {
public:
    SampleGroupExtractor() = default;
    virtual ~SampleGroupExtractor() = default;

    std::string getName() const override { return "sample-group"; }

    std::string getDescription() const override {
        return "Extract T.35 metadata from video track sample groups";
    }

    bool canExtract(const ExtractionConfig& config,
                   std::string& reason) override;

    MP4Err extract(const ExtractionConfig& config, MetadataMap* outItems = nullptr) override;
};

} // namespace t35
