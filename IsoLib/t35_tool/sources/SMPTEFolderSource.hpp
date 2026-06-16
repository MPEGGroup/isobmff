#pragma once

#include "MetadataSource.hpp"

namespace t35 {

/**
 * SMPTE ST2094-50 folder source - reads folder with SMPTE JSON files
 *
 * Scans folder for .json files containing SMPTE ST2094-50 metadata
 * Uses existing SMPTE_ST2094_50 encoding logic internally
 */
class SMPTEFolderSource : public MetadataSource {
public:
    explicit SMPTEFolderSource(const std::string& folderPath);
    virtual ~SMPTEFolderSource() = default;

    std::string getType() const override { return "smpte-folder"; }
    MetadataMap load(const T35Prefix& prefix) override;
    bool validate(std::string& errorMsg) override;
    std::string getPath() const override { return path; }

private:
    std::string path;
};

} // namespace t35
