#pragma once

#include "../common/MetadataTypes.hpp"
#include "../common/T35Prefix.hpp"

// Forward declarations from libisomediafile
extern "C" {
typedef int MP4Err;
}

#include <memory>
#include <string>

namespace t35 {

/**
 * Abstract interface for metadata injection strategies
 *
 * An InjectionStrategy handles:
 * - Creating MP4 container structures (tracks, sample entries, etc.)
 * - Adding metadata samples to the container
 * - Linking metadata to video track
 *
 * Different strategies implement different MP4 storage methods:
 * - MEBX tracks (me4c namespace)
 * - Dedicated metadata tracks
 * - Sample groups
 * - Sample entry boxes
 */
class InjectionStrategy {
public:
    virtual ~InjectionStrategy() = default;

    /**
     * Get strategy name
     * @return Name string (e.g., "mebx-me4c", "dedicated-it35")
     */
    virtual std::string getName() const = 0;

    /**
     * Get strategy description
     * @return Human-readable description
     */
    virtual std::string getDescription() const = 0;

    /**
     * Check if this strategy is applicable to the given metadata
     *
     * Some strategies have constraints:
     * - Static metadata only (sample-entry-box, default-sample-group)
     * - Specific codec requirements
     *
     * @param items Metadata to check
     * @param config Injection configuration
     * @param reason Output parameter for reason if not applicable
     * @return true if strategy can be used
     */
    virtual bool isApplicable(const MetadataMap& items,
                             const InjectionConfig& config,
                             std::string& reason) const = 0;

    /**
     * Inject metadata into movie
     *
     * @param config Configuration (movie, video track, etc.)
     * @param items Metadata to inject
     * @param prefix T.35 prefix for this metadata
     * @return MP4Err (0 = success)
     * @throws T35Exception on error
     */
    virtual MP4Err inject(const InjectionConfig& config,
                         const MetadataMap& items,
                         const T35Prefix& prefix) = 0;
};

/**
 * Factory function to create injection strategy from name
 *
 * Available strategies:
 * - "mebx-me4c": MEBX track with me4c namespace
 * - "dedicated-it35": Dedicated metadata track
 * - "sample-group": Sample group
 *
 * @param strategyName Strategy name
 * @return Unique pointer to InjectionStrategy
 * @throws T35Exception if strategy is unknown
 */
std::unique_ptr<InjectionStrategy> createInjectionStrategy(const std::string& strategyName);

} // namespace t35
