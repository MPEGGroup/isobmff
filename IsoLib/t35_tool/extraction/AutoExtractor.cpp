#include "AutoExtractor.hpp"
#include "MebxMe4cExtractor.hpp"
#include "DedicatedIt35Extractor.hpp"
#include "SampleGroupExtractor.hpp"
#include "../common/Logger.hpp"

namespace t35
{

bool AutoExtractor::canExtract(const ExtractionConfig &config, std::string &reason)
{
  // Try extractors in priority order
  std::vector<std::unique_ptr<ExtractionStrategy>> extractors;
  extractors.push_back(std::make_unique<MebxMe4cExtractor>());
  extractors.push_back(std::make_unique<DedicatedIt35Extractor>());
  extractors.push_back(std::make_unique<SampleGroupExtractor>());

  for(const auto &extractor : extractors)
  {
    std::string extractorReason;
    if(extractor->canExtract(config, extractorReason))
    {
      LOG_INFO("Auto-detected strategy: {}", extractor->getName());
      return true;
    }
  }

  reason = "No compatible metadata tracks found (tried: mebx-me4c, dedicated-it35, sample-group)";
  return false;
}

MP4Err AutoExtractor::extract(const ExtractionConfig &config, MetadataMap *outItems)
{
  LOG_INFO("Auto-detecting extraction strategy");

  // Try extractors in priority order
  std::vector<std::unique_ptr<ExtractionStrategy>> extractors;
  extractors.push_back(std::make_unique<MebxMe4cExtractor>());
  extractors.push_back(std::make_unique<DedicatedIt35Extractor>());
  extractors.push_back(std::make_unique<SampleGroupExtractor>());

  for(auto &extractor : extractors)
  {
    std::string reason;
    if(extractor->canExtract(config, reason))
    {
      LOG_INFO("Using auto-detected strategy: {}", extractor->getName());
      return extractor->extract(config, outItems); // Pass through outItems!
    }
    else
    {
      LOG_DEBUG("Strategy '{}' cannot extract: {}", extractor->getName(), reason);
    }
  }

  LOG_ERROR("No compatible extraction strategy found");
  throw T35Exception(
    T35Error::ExtractionFailed,
    "No compatible metadata tracks found (tried: mebx-me4c, dedicated-it35, sample-group)");
}

} // namespace t35
