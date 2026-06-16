#pragma once

#include "ExtractionStrategy.hpp"

namespace t35 {

/**
 * SEI extractor for HEVC
 * Extracts T.35 metadata from any container method and converts to HEVC video with SEI NAL units
 *
 * This strategy:
 * - Auto-detects metadata storage method (MEBX, sample groups, etc.)
 * - Reads video track and decoder configuration
 * - Inserts SEI NAL units (user_data_registered_itu_t_t35) before video samples
 * - Converts video from length-prefix format to Annex-B format with start codes
 * - Outputs to .265 file
 *
 * If metadata spans multiple video samples, the same SEI is written for each video sample
 * (redundant metadata for sample alignment)
 */
class SeiExtractor : public ExtractionStrategy {
public:
    SeiExtractor() = default;
    virtual ~SeiExtractor() = default;

    std::string getName() const override { return "sei"; }

    std::string getDescription() const override {
        return "Extract metadata and convert to HEVC video with SEI NAL units";
    }

    bool canExtract(const ExtractionConfig& config,
                   std::string& reason) override;

    MP4Err extract(const ExtractionConfig& config, MetadataMap* outItems = nullptr) override;
};

} // namespace t35
