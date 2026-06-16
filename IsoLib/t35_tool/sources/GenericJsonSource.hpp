#pragma once

#include "MetadataSource.hpp"

namespace t35 {

/**
 * Generic JSON source - reads simple manifest with binary file references
 *
 * Expected JSON format:
 * {
 *   "t35_prefix": "B500900001",
 *   "items": [
 *     {"frame_start": 0, "frame_duration": 24, "binary_file": "meta_001.bin"},
 *     {"frame_start": 24, "frame_duration": 24, "binary_file": "meta_002.bin"}
 *   ]
 * }
 */
class GenericJsonSource : public MetadataSource {
public:
    explicit GenericJsonSource(const std::string& jsonPath);
    virtual ~GenericJsonSource() = default;

    std::string getType() const override { return "generic-json"; }
    MetadataMap load(const T35Prefix& prefix) override;
    bool validate(std::string& errorMsg) override;
    std::string getPath() const override { return path; }

private:
    std::string path;
};

} // namespace t35
