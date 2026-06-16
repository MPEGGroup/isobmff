#pragma once

#include "InjectionStrategy.hpp"

namespace t35 {

/**
 * @brief Injection strategy that inserts T.35 metadata directly into the video bitstream.
 *
 * For AV1: Inserts metadata OBU of type T35.
 * For HEVC: Inserts SEI NALU (nal_unit_type=39, payload_type=4).
 *
 * This strategy modifies the video samples in place using the new MP4UpdateMediaSample API.
 */
class BitstreamStrategy : public InjectionStrategy {
public:
    BitstreamStrategy() = default;
    ~BitstreamStrategy() override = default;

    std::string getName() const override { return "bitstream"; }

    std::string getDescription() const override {
        return "Inject T.35 metadata directly into the video bitstream (AV1/HEVC)";
    }

    bool isApplicable(const MetadataMap& items,
                     const InjectionConfig& config,
                     std::string& reason) const override;

    MP4Err inject(const InjectionConfig& config,
                  const MetadataMap& items,
                  const T35Prefix& prefix) override;
};

} // namespace t35
