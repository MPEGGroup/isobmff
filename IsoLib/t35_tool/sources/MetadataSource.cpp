#include "MetadataSource.hpp"
#include "GenericJsonSource.hpp"
#include "SMPTEFolderSource.hpp"
#include "../common/Logger.hpp"

namespace t35
{

std::unique_ptr<MetadataSource> createMetadataSource(const std::string &sourceSpec)
{
  // Parse "type:path"
  size_t colonPos = sourceSpec.find(':');
  if(colonPos == std::string::npos)
  {
    throw T35Exception(T35Error::SourceError,
                       "Invalid source spec format. Expected 'type:path', got: " + sourceSpec);
  }

  std::string type = sourceSpec.substr(0, colonPos);
  std::string path = sourceSpec.substr(colonPos + 1);

  if(path.empty())
  {
    throw T35Exception(T35Error::SourceError, "Empty path in source spec: " + sourceSpec);
  }

  LOG_DEBUG("Creating source: type='{}' path='{}'", type, path);

  if(type == "generic-json" || type == "json-manifest")
  {
    return std::make_unique<GenericJsonSource>(path);
  }
  else if(type == "smpte-folder" || type == "json-folder")
  {
    return std::make_unique<SMPTEFolderSource>(path);
  }
  else
  {
    throw T35Exception(T35Error::SourceError, "Unknown source type: " + type);
  }
}

} // namespace t35
