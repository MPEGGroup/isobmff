#include "GenericJsonSource.hpp"
#include "../common/Logger.hpp"

#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace t35
{

GenericJsonSource::GenericJsonSource(const std::string &jsonPath)
  : path(jsonPath)
{
}

bool GenericJsonSource::validate(std::string &errorMsg)
{
  LOG_DEBUG("Validating GenericJsonSource at {}", path);

  // Check if file exists
  if(!std::filesystem::exists(path))
  {
    errorMsg = "JSON file does not exist: " + path;
    return false;
  }

  // Check if it's a regular file
  if(!std::filesystem::is_regular_file(path))
  {
    errorMsg = "Path is not a regular file: " + path;
    return false;
  }

  // Try to parse JSON
  try
  {
    std::ifstream file(path);
    if(!file.is_open())
    {
      errorMsg = "Cannot open JSON file: " + path;
      return false;
    }

    nlohmann::json j;
    file >> j;

    // Validate JSON schema
    if(!j.contains("items"))
    {
      errorMsg = "JSON missing required field: 'items'";
      return false;
    }

    if(!j["items"].is_array())
    {
      errorMsg = "JSON field 'items' must be an array";
      return false;
    }

    // Get base directory for relative paths
    std::filesystem::path jsonDir = std::filesystem::path(path).parent_path();
    if(jsonDir.empty())
    {
      jsonDir = ".";
    }

    // Validate each item
    const auto &items = j["items"];
    for(size_t i = 0; i < items.size(); ++i)
    {
      const auto &item = items[i];

      // Check required fields
      if(!item.contains("frame_start"))
      {
        errorMsg = "Item " + std::to_string(i) + " missing 'frame_start'";
        return false;
      }
      if(!item.contains("frame_duration"))
      {
        errorMsg = "Item " + std::to_string(i) + " missing 'frame_duration'";
        return false;
      }
      if(!item.contains("binary_file"))
      {
        errorMsg = "Item " + std::to_string(i) + " missing 'binary_file'";
        return false;
      }

      // Check types
      if(!item["frame_start"].is_number_unsigned())
      {
        errorMsg = "Item " + std::to_string(i) + " 'frame_start' must be unsigned integer";
        return false;
      }
      if(!item["frame_duration"].is_number_unsigned())
      {
        errorMsg = "Item " + std::to_string(i) + " 'frame_duration' must be unsigned integer";
        return false;
      }
      if(!item["binary_file"].is_string())
      {
        errorMsg = "Item " + std::to_string(i) + " 'binary_file' must be string";
        return false;
      }

      // Check if binary file exists
      std::string binaryFile           = item["binary_file"].get<std::string>();
      std::filesystem::path binaryPath = jsonDir / binaryFile;

      if(!std::filesystem::exists(binaryPath))
      {
        errorMsg = "Binary file does not exist: " + binaryPath.string();
        return false;
      }

      if(!std::filesystem::is_regular_file(binaryPath))
      {
        errorMsg = "Binary path is not a regular file: " + binaryPath.string();
        return false;
      }
    }

    LOG_DEBUG("GenericJsonSource validation passed");
    return true;
  }
  catch(const nlohmann::json::exception &e)
  {
    errorMsg = "JSON parse error: " + std::string(e.what());
    return false;
  }
  catch(const std::exception &e)
  {
    errorMsg = "Validation error: " + std::string(e.what());
    return false;
  }
}

MetadataMap GenericJsonSource::load(const T35Prefix & /* prefix */)
{
  LOG_INFO("Loading metadata from GenericJsonSource: {}", path);

  // Validate first
  std::string errorMsg;
  if(!validate(errorMsg))
  {
    LOG_ERROR("Validation failed: {}", errorMsg);
    throw T35Exception(T35Error::InvalidJSON, errorMsg);
  }

  // Parse JSON
  std::ifstream file(path);
  nlohmann::json j;
  file >> j;

  // Get base directory for relative paths
  std::filesystem::path jsonDir = std::filesystem::path(path).parent_path();
  if(jsonDir.empty())
  {
    jsonDir = ".";
  }

  MetadataMap metadataMap;

  // Load each item
  const auto &items = j["items"];
  for(size_t i = 0; i < items.size(); ++i)
  {
    const auto &item = items[i];

    uint32_t frameStart    = item["frame_start"].get<uint32_t>();
    uint32_t frameDuration = item["frame_duration"].get<uint32_t>();
    std::string binaryFile = item["binary_file"].get<std::string>();

    // Read binary file
    std::filesystem::path binaryPath = jsonDir / binaryFile;
    std::ifstream binFile(binaryPath, std::ios::binary);
    if(!binFile)
    {
      throw T35Exception(T35Error::MissingFiles,
                         "Failed to open binary file: " + binaryPath.string());
    }

    // Read entire file into vector
    std::vector<uint8_t> payload((std::istreambuf_iterator<char>(binFile)),
                                 std::istreambuf_iterator<char>());

    if(payload.empty())
    {
      LOG_WARN("Binary file is empty: {}", binaryPath.string());
    }

    LOG_DEBUG("Loaded item {}: frame_start={}, frame_duration={}, payload_size={}", i, frameStart,
              frameDuration, payload.size());

    // Create metadata item
    MetadataItem metaItem(frameStart, frameDuration, std::move(payload), binaryPath.string());

    // Insert into map (key = frame_start)
    if(metadataMap.find(frameStart) != metadataMap.end())
    {
      throw T35Exception(T35Error::InvalidJSON,
                         "Duplicate frame_start: " + std::to_string(frameStart));
    }

    metadataMap[frameStart] = std::move(metaItem);
  }

  LOG_INFO("Loaded {} metadata items from {}", metadataMap.size(), path);

  // Validate the loaded metadata map
  if(!validateMetadataMap(metadataMap, errorMsg))
  {
    throw T35Exception(T35Error::ValidationFailed, errorMsg);
  }

  return metadataMap;
}

} // namespace t35
