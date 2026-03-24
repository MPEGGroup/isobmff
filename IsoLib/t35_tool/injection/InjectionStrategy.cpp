#include "InjectionStrategy.hpp"
#include "MebxMe4cStrategy.hpp"
#include "DedicatedIt35Strategy.hpp"
#include "SampleGroupStrategy.hpp"
#include "../common/Logger.hpp"

namespace t35
{

std::unique_ptr<InjectionStrategy> createInjectionStrategy(const std::string &strategyName)
{
  LOG_DEBUG("Creating injection strategy: '{}'", strategyName);

  if(strategyName == "mebx-me4c")
  {
    return std::make_unique<MebxMe4cStrategy>();
  }
  else if(strategyName == "dedicated-it35")
  {
    return std::make_unique<DedicatedIt35Strategy>();
  }
  else if(strategyName == "sample-group")
  {
    return std::make_unique<SampleGroupStrategy>();
  }
  else if(strategyName == "sei")
  {
    throw T35Exception(T35Error::NotImplemented,
                       "Injection strategy '" + strategyName + "' is not yet implemented");
  }
  else
  {
    throw T35Exception(T35Error::InjectionFailed, "Unknown injection strategy: " + strategyName);
  }
}

} // namespace t35
