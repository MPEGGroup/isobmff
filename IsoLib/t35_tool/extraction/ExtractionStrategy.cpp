#include "ExtractionStrategy.hpp"
#include "MebxMe4cExtractor.hpp"
#include "DedicatedIt35Extractor.hpp"
#include "AutoExtractor.hpp"
#include "SampleGroupExtractor.hpp"
#include "SeiExtractor.hpp"
#include "../common/Logger.hpp"

namespace t35
{

std::unique_ptr<ExtractionStrategy> createExtractionStrategy(const std::string &strategyName)
{
  LOG_DEBUG("Creating extraction strategy: '{}'", strategyName);

  if(strategyName == "auto")
  {
    return std::make_unique<AutoExtractor>();
  }
  else if(strategyName == "mebx-me4c")
  {
    return std::make_unique<MebxMe4cExtractor>();
  }
  else if(strategyName == "dedicated-it35")
  {
    return std::make_unique<DedicatedIt35Extractor>();
  }
  else if(strategyName == "sample-group")
  {
    return std::make_unique<SampleGroupExtractor>();
  }
  else if(strategyName == "sei")
  {
    return std::make_unique<SeiExtractor>();
  }
  else
  {
    throw T35Exception(T35Error::ExtractionFailed, "Unknown extraction strategy: " + strategyName);
  }
}

} // namespace t35
