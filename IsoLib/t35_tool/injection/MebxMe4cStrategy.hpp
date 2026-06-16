#pragma once

#include "InjectionStrategy.hpp"

namespace t35 {

/**
 * MEBX track with me4c namespace injection strategy
 * Future implementation
 */
class MebxMe4cStrategy : public InjectionStrategy {
public:
    MebxMe4cStrategy() = default;
    virtual ~MebxMe4cStrategy() = default;

    std::string getName() const override { return "mebx-me4c"; }

    std::string getDescription() const override {
        return "MEBX metadata track with me4c namespace";
    }

    bool isApplicable(const MetadataMap& items,
                     const InjectionConfig& config,
                     std::string& reason) const override;

    MP4Err inject(const InjectionConfig& config,
                 const MetadataMap& items,
                 const T35Prefix& prefix) override;
};

} // namespace t35
