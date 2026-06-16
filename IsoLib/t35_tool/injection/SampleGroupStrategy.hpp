#pragma once

#include "InjectionStrategy.hpp"

namespace t35 {

/**
 * Sample Group injection strategy
 * Injects T.35 metadata into video track using sample groups (sgpd/sbgp)
 *
 * This strategy modifies the video track by:
 * - Adding sample group descriptions (sgpd) with 'it35' grouping type
 * - Adding sample-to-group mappings (sbgp) to associate samples with metadata
 *
 * Supports both static (all samples → one group) and dynamic (different samples → different groups) metadata.
 */
class SampleGroupStrategy : public InjectionStrategy {
public:
    SampleGroupStrategy() = default;
    virtual ~SampleGroupStrategy() = default;

    std::string getName() const override { return "sample-group"; }

    std::string getDescription() const override {
        return "Inject T.35 metadata into video track using sample groups (sgpd/sbgp)";
    }

    bool isApplicable(const MetadataMap& items,
                     const InjectionConfig& config,
                     std::string& reason) const override;

    MP4Err inject(const InjectionConfig& config,
                 const MetadataMap& items,
                 const T35Prefix& prefix) override;
};

} // namespace t35
