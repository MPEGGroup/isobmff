#pragma once

#include "../common/MetadataTypes.hpp"
#include "../common/T35Prefix.hpp"
#include <memory>
#include <string>

namespace t35 {

/**
 * Abstract interface for metadata sources
 *
 * A MetadataSource handles:
 * - Discovery of input metadata files
 * - Parsing of source format (JSON, binary, etc.)
 * - Conversion to binary T.35 payloads
 * - Creation of MetadataMap with timing information
 *
 * Each source type knows its input format and handles conversion internally.
 */
class MetadataSource {
public:
    virtual ~MetadataSource() = default;

    /**
     * Get source type identifier
     * @return Type string (e.g., "generic-json", "smpte-folder")
     */
    virtual std::string getType() const = 0;

    /**
     * Load all metadata items from source
     *
     * @param prefix T.35 prefix for this metadata (may be used for validation)
     * @return MetadataMap with binary payloads ready for injection
     * @throws T35Exception on error
     */
    virtual MetadataMap load(const T35Prefix& prefix) = 0;

    /**
     * Validate source before loading
     *
     * @param errorMsg Output parameter for error message
     * @return true if source is valid and can be loaded
     */
    virtual bool validate(std::string& errorMsg) = 0;

    /**
     * Get source path/location
     * @return Path string for debugging
     */
    virtual std::string getPath() const = 0;
};

/**
 * Factory function to create metadata source from type:path string
 *
 * Format: "type:path"
 * Examples:
 *   - "generic-json:metadata.json"
 *   - "smpte-folder:metadata_dir"
 *
 * @param sourceSpec Source specification string
 * @return Unique pointer to MetadataSource
 * @throws T35Exception if type is unknown or path is invalid
 */
std::unique_ptr<MetadataSource> createMetadataSource(const std::string& sourceSpec);

} // namespace t35
