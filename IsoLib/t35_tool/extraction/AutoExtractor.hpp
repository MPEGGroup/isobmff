#pragma once

#include "ExtractionStrategy.hpp"

namespace t35 {

/**
 * Auto-detection extractor
 * Tries all extraction strategies in priority order
 */
class AutoExtractor : public ExtractionStrategy {
public:
    AutoExtractor() = default;
    virtual ~AutoExtractor() = default;

    std::string getName() const override { return "auto"; }

    std::string getDescription() const override {
        return "Auto-detect extraction method";
    }

    bool canExtract(const ExtractionConfig& config,
                   std::string& reason) override;

    MP4Err extract(const ExtractionConfig& config, MetadataMap* outItems = nullptr) override;
};

} // namespace t35
