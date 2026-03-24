#include "SMPTEFolderSource.hpp"
#include "../common/Logger.hpp"
#include "SMPTE_ST2094_50.hpp"

#include <fstream>
#include <filesystem>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace t35
{

SMPTEFolderSource::SMPTEFolderSource(const std::string &folderPath)
  : path(folderPath)
{
}

bool SMPTEFolderSource::validate(std::string &errorMsg)
{
  LOG_DEBUG("Validating SMPTEFolderSource at {}", path);

  // Check if path exists
  if(!std::filesystem::exists(path))
  {
    errorMsg = "Folder does not exist: " + path;
    return false;
  }

  // Check if it's a directory
  if(!std::filesystem::is_directory(path))
  {
    errorMsg = "Path is not a directory: " + path;
    return false;
  }

  // Check if there are any .json files
  bool hasJsonFiles = false;
  try
  {
    for(const auto &entry : std::filesystem::directory_iterator(path))
    {
      if(entry.is_regular_file() && entry.path().extension() == ".json")
      {
        hasJsonFiles = true;
        break;
      }
    }
  }
  catch(const std::filesystem::filesystem_error &e)
  {
    errorMsg = "Failed to read directory: " + std::string(e.what());
    return false;
  }

  if(!hasJsonFiles)
  {
    errorMsg = "No .json files found in directory: " + path;
    return false;
  }

  LOG_DEBUG("SMPTEFolderSource validation passed");
  return true;
}

MetadataMap SMPTEFolderSource::load(const T35Prefix & /* prefix */)
{
  LOG_INFO("Loading SMPTE ST2094-50 metadata from folder: {}", path);

  // Validate first
  std::string errorMsg;
  if(!validate(errorMsg))
  {
    LOG_ERROR("Validation failed: {}", errorMsg);
    throw T35Exception(T35Error::InvalidJSON, errorMsg);
  }

  MetadataMap metadataMap;

  // Collect all .json files
  std::vector<std::filesystem::path> jsonFiles;
  for(const auto &entry : std::filesystem::directory_iterator(path))
  {
    if(entry.is_regular_file() && entry.path().extension() == ".json")
    {
      jsonFiles.push_back(entry.path());
    }
  }

  // Sort files by name for consistent ordering
  std::sort(jsonFiles.begin(), jsonFiles.end());

  LOG_INFO("Found {} JSON files in folder", jsonFiles.size());

  // Process each JSON file
  for(size_t i = 0; i < jsonFiles.size(); ++i)
  {
    const auto &jsonFile = jsonFiles[i];
    LOG_DEBUG("Processing file {}/{}: {}", i + 1, jsonFiles.size(), jsonFile.filename().string());

    try
    {
      // Read JSON file
      std::ifstream file(jsonFile);
      if(!file.is_open())
      {
        LOG_WARN("Failed to open file: {}, skipping", jsonFile.string());
        continue;
      }

      nlohmann::json j;
      try
      {
        file >> j;
      }
      catch(const nlohmann::json::exception &e)
      {
        LOG_WARN("JSON parse error in {}: {}, skipping", jsonFile.filename().string(), e.what());
        continue;
      }

      // Create SMPTE encoder instance
      SMPTE_ST2094_50 smpteEncoder;

      // Decode JSON to metadata items
      bool error = smpteEncoder.decodeJsonToMetadataItems(j);
      if(error)
      {
        LOG_WARN("SMPTE decoding failed for {}, skipping", jsonFile.filename().string());
        continue;
      }

      // Convert metadata items to syntax elements
      smpteEncoder.convertMetadataItemsToSyntaxElements();

      // Write syntax elements to binary data
      smpteEncoder.writeSyntaxElementsToBinaryData();

      // Get the binary payload
      std::vector<uint8_t> payload = smpteEncoder.getPayloadData();

      // Get timing info
      uint32_t frameStart    = smpteEncoder.getTimeIntervalStart();
      uint32_t frameDuration = smpteEncoder.getTimeintervalDuration();

      // Validate payload
      if(payload.empty())
      {
        LOG_WARN("Empty payload for {}, skipping", jsonFile.filename().string());
        continue;
      }

      // Check for duplicate frame_start
      if(metadataMap.find(frameStart) != metadataMap.end())
      {
        LOG_ERROR("Duplicate frame_start: {} in file {}", frameStart, jsonFile.filename().string());
        throw T35Exception(T35Error::InvalidJSON,
                           "Duplicate frame_start: " + std::to_string(frameStart) + " in file " +
                             jsonFile.filename().string());
      }

      // Create metadata item
      MetadataItem metaItem(frameStart, frameDuration, std::move(payload), jsonFile.string());

      LOG_DEBUG("Loaded SMPTE item {}: frame_start={}, frame_duration={}, payload_size={}", i,
                frameStart, frameDuration, metaItem.payload.size());

      // Insert into map
      metadataMap[frameStart] = std::move(metaItem);
    }
    catch(const T35Exception &e)
    {
      // Re-throw T35 exceptions
      throw;
    }
    catch(const std::exception &e)
    {
      LOG_WARN("Error processing {}: {}, skipping", jsonFile.filename().string(), e.what());
      continue;
    }
  }

  if(metadataMap.empty())
  {
    throw T35Exception(T35Error::NoMetadataFound,
                       "No valid SMPTE metadata found in folder: " + path);
  }

  LOG_INFO("Loaded {} SMPTE metadata items from {}", metadataMap.size(), path);

  // Validate the loaded metadata map
  if(!validateMetadataMap(metadataMap, errorMsg))
  {
    throw T35Exception(T35Error::ValidationFailed, errorMsg);
  }

  return metadataMap;
}

} // namespace t35
