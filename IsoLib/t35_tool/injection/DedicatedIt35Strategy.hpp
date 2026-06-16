#pragma once

#include "InjectionStrategy.hpp"

namespace t35 {

/**
 * @brief Injection strategy using dedicated IT35 metadata track ('it35' sample entry).
 *
 * Creates a dedicated T.35 timed metadata track with:
 * - Sample entry type: 'it35' (T35MetadataSampleEntry)
 * - t35_identifier and description fields in sample entry
 * - Samples contain only T.35 payload (no box wrapper)
 * - Optional 'rndr' track reference to video track
 *
 * This is the standardized approach for dedicated T.35 metadata tracks.
 */
class DedicatedIt35Strategy : public InjectionStrategy {
public:
    DedicatedIt35Strategy() = default;
    ~DedicatedIt35Strategy() override = default;

    std::string getName() const override { return "dedicated-it35"; }

    std::string getDescription() const override {
        return "Dedicated IT35 metadata track with description and t35_identifier";
    }

    bool isApplicable(const MetadataMap& items,
                     const InjectionConfig& config,
                     std::string& reason) const override;

    MP4Err inject(const InjectionConfig& config,
                  const MetadataMap& items,
                  const T35Prefix& prefix) override;
};

} // namespace t35
