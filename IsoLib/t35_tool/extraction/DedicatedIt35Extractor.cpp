#include "DedicatedIt35Extractor.hpp"
#include "../sources/SMPTE_ST2094_50.hpp"
#include "../common/Logger.hpp"
#include "../common/T35Prefix.hpp"

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "ISOMovies.h"
}

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace t35
{

// DedicatedIt35Extractor implementation

DedicatedIt35Extractor::~DedicatedIt35Extractor() { clearCache(); }

void DedicatedIt35Extractor::clearCache() { m_cachedTrack = nullptr; }

// Helper: Find dedicated IT35 metadata track with matching T.35 prefix
static MP4Err findIt35MetadataTrack(MP4Movie moov, const std::string &t35PrefixStr,
                                    MP4Track *outTrack)
{
  MP4Err err = MP4NoErr;
  *outTrack  = nullptr;

  u32 trackCount = 0;
  err            = MP4GetMovieTrackCount(moov, &trackCount);
  if(err) return err;

  LOG_DEBUG("Searching for IT35 track in {} tracks", trackCount);

  // Search for IT35 metadata track
  for(u32 i = 1; i <= trackCount; ++i)
  {
    MP4Track trak   = nullptr;
    MP4Media media  = nullptr;
    u32 handlerType = 0;

    err = MP4GetMovieIndTrack(moov, i, &trak);
    if(err) continue;

    err = MP4GetTrackMedia(trak, &media);
    if(err) continue;

    err = MP4GetMediaHandlerDescription(media, &handlerType, nullptr);
    if(err) continue;

    // Only metadata tracks
    if(handlerType != MP4MetaHandlerType) continue;

    u32 trackID = 0;
    MP4GetTrackID(trak, &trackID);
    LOG_DEBUG("Found metadata track with ID {}", trackID);

    // Get first sample description
    MP4Handle sampleEntryH = nullptr;
    err                    = MP4NewHandle(0, &sampleEntryH);
    if(err) continue;

    err = MP4GetMediaSampleDescription(media, 1, sampleEntryH, nullptr);
    if(err)
    {
      MP4DisposeHandle(sampleEntryH);
      continue;
    }

    // Check if it's 'it35'
    u32 type = 0;
    err      = ISOGetSampleDescriptionType(sampleEntryH, &type);

    if(err || type != MP4T35MetadataSampleEntryType)
    {
      MP4DisposeHandle(sampleEntryH);
      continue;
    }

    LOG_INFO("Found IT35 track with ID {}", trackID);

    // Read t35_identifier and description from the serialized sample entry handle
    u8 *identifier     = nullptr;
    u32 identifierSize = 0;
    char *description  = nullptr;

    MP4Err readErr =
      ISOGetT35SampleEntryFields(sampleEntryH, &identifier, &identifierSize, &description);
    MP4DisposeHandle(sampleEntryH);
    sampleEntryH = nullptr;

    if(readErr || identifier == nullptr || identifierSize == 0)
    {
      LOG_WARN("Could not read t35_identifier from IT35 sample entry");
      free(identifier);
      free(description);
      continue;
    }

    // Convert t35_identifier bytes to hex string
    std::string hexStr;
    hexStr.reserve(identifierSize * 2);
    for(u32 j = 0; j < identifierSize; j++)
    {
      char buf[3];
      snprintf(buf, sizeof(buf), "%02X", identifier[j]);
      hexStr += buf;
    }
    free(identifier);

    // Build prefix string: "HEX:Description"
    std::string filePrefix = hexStr;
    if(description && description[0] != '\0')
    {
      filePrefix += ":";
      filePrefix += description;
    }

    LOG_DEBUG("Parsed 'it35' sample entry: identifier={} ({} bytes), description='{}'", hexStr,
              identifierSize, (description && description[0] != '\0') ? description : "<empty>");
    free(description);

    // Parse both prefixes to compare hex part only
    T35Prefix requestedPrefix(t35PrefixStr);
    T35Prefix filePrefixParsed(filePrefix);

    if(requestedPrefix.hex() != filePrefixParsed.hex())
    {
      LOG_DEBUG("T35 hex '{}' does not match requested hex '{}'", filePrefixParsed.hex(),
                requestedPrefix.hex());
      continue;
    }
    LOG_DEBUG("T35 hex '{}' matches requested hex '{}'", filePrefixParsed.hex(),
              requestedPrefix.hex());

    if(!requestedPrefix.description().empty() && !filePrefixParsed.description().empty() &&
       requestedPrefix.description() != filePrefixParsed.description())
    {
      LOG_WARN("T.35 description mismatch: requested '{}' but file has '{}'",
               requestedPrefix.description(), filePrefixParsed.description());
    }

    // Check for 'rndr' track reference
    MP4Track videoTrack = nullptr;
    err = MP4GetTrackReference(trak, MP4_FOUR_CHAR_CODE('r', 'n', 'd', 'r'), 1, &videoTrack);
    if(err)
    {
      LOG_WARN("IT35 track ID {} has no 'rndr' track reference, continuing anyway", trackID);
    }
    else
    {
      u32 videoTrackID = 0;
      MP4GetTrackID(videoTrack, &videoTrackID);
      LOG_DEBUG("IT35 track references video track ID {}", videoTrackID);
    }

    // Success!
    *outTrack = trak;
    return MP4NoErr;
  }

  LOG_ERROR("No 'it35' metadata track found");
  return MP4NotFoundErr;
}

bool DedicatedIt35Extractor::canExtract(const ExtractionConfig &config, std::string &reason)
{
  if(!config.movie)
  {
    reason = "No movie provided";
    return false;
  }

  // Clear any previous cache
  clearCache();

  // Find and cache the track
  MP4Err err = findIt35MetadataTrack(config.movie, config.t35Prefix, &m_cachedTrack);

  if(err)
  {
    reason = "No dedicated IT35 metadata track found";
    return false;
  }

  return true;
}

MP4Err DedicatedIt35Extractor::extract(const ExtractionConfig &config, MetadataMap *outItems)
{
  LOG_INFO("Extracting metadata using dedicated-it35 extractor");
  LOG_INFO("T.35 prefix: {}", config.t35Prefix);
  if(!outItems)
  {
    LOG_INFO("Output path: {}", config.outputPath);
  }
  else
  {
    LOG_INFO("Output mode: in-memory");
  }

  MP4Err err         = MP4NoErr;
  MP4Track it35Track = nullptr;

  // Use cached track if available (from canExtract), otherwise find it now
  if(m_cachedTrack)
  {
    LOG_DEBUG("Using cached IT35 track");
    it35Track = m_cachedTrack;

    // Clear cache so we don't reuse it by mistake
    m_cachedTrack = nullptr;
  }
  else
  {
    // Fallback: extract() called without canExtract()
    LOG_DEBUG("Finding IT35 track (cache not available)");
    err = findIt35MetadataTrack(config.movie, config.t35Prefix, &it35Track);
    if(err)
    {
      LOG_ERROR("Failed to find IT35 track (err={})", err);
      return err;
    }
  }

  // Get media and timescale
  MP4Media it35Media = nullptr;
  u32 timescale      = 1000; // default
  err                = MP4GetTrackMedia(it35Track, &it35Media);
  if(err != MP4NoErr)
  {
    LOG_ERROR("Failed to get IT35 media (err={})", err);
    return err;
  }

  err = MP4GetMediaTimeScale(it35Media, &timescale);
  if(err != MP4NoErr)
  {
    LOG_WARN("Failed to get timescale, using default 1000");
    timescale = 1000;
  }
  LOG_DEBUG("IT35 track timescale: {}", timescale);

  // Get sample count
  u32 sampleCount = 0;
  err             = MP4GetMediaSampleCount(it35Media, &sampleCount);
  if(err != MP4NoErr)
  {
    LOG_ERROR("Failed to get sample count (err={})", err);
    return err;
  }
  LOG_INFO("IT35 track has {} samples", sampleCount);

  // Create output directory
  namespace fs = std::filesystem;
  fs::path outDir(config.outputPath);

  if(!fs::exists(outDir))
  {
    if(!fs::create_directories(outDir))
    {
      LOG_ERROR("Failed to create output directory: {}", config.outputPath);
      return MP4IOErr;
    }
  }

  LOG_INFO("Extracting samples to {}", outDir.string());

  // Extract all samples
  std::vector<nlohmann::json> manifestItems;
  u32 currentFrame = 0;

  for(u32 i = 1; i <= sampleCount; ++i)
  {
    MP4Handle sampleH   = nullptr;
    u32 sampleSize      = 0;
    u32 sampleFlags     = 0;
    u32 sampleDescIndex = 0;
    u64 dts             = 0;
    u64 duration        = 0;
    s32 ctsOffset       = 0;

    err = MP4NewHandle(0, &sampleH);
    if(err)
    {
      LOG_ERROR("Failed to create sample handle (err={})", err);
      return err;
    }

    err = MP4GetIndMediaSample(it35Media, i, sampleH, &sampleSize, &dts, &ctsOffset, &duration,
                               &sampleFlags, &sampleDescIndex);

    if(err)
    {
      MP4DisposeHandle(sampleH);
      LOG_ERROR("Failed to read sample {} (err={})", i, err);
      return err;
    }

    // Calculate frame duration (simplification - would need video track info for accurate
    // calculation)
    u32 frameDuration = 1;
    if(duration > 0 && timescale > 0)
    {
      // Rough estimate: assume ~24-60fps
      frameDuration = (u32)duration / (timescale / 60);
      if(frameDuration == 0) frameDuration = 1;
    }

    // Decode the metadata if they are SMPTE ST 2094-50
    if(config.t35Prefix == "B500900001:SMPTE-ST2094-50")
    {
      SMPTE_ST2094_50 st2094_50;
      std::vector<uint8_t> binaryData(sampleSize);
      std::memcpy(binaryData.data(), *sampleH, sampleSize);

      st2094_50.decodeBinaryToSyntaxElements(binaryData);
      st2094_50.convertSyntaxElementsToMetadataItems();
      st2094_50.dbgPrintMetadataItems(); // Print decoded metadata from bitstream
    }

    // Write binary file
    fs::path binFile = outDir / ("metadata_" + std::to_string(i) + ".bin");
    std::ofstream out(binFile, std::ios::binary);
    if(!out)
    {
      LOG_ERROR("Failed to open {} for writing", binFile.string());
      MP4DisposeHandle(sampleH);
      return MP4IOErr;
    }

    // Decode the metadata if they are SMPTE ST 2094-50
    if(config.t35Prefix == "B500900001:SMPTE-ST2094-50")
    {
      SMPTE_ST2094_50 st2094_50;
      std::vector<uint8_t> binaryData(sampleSize);
      std::memcpy(binaryData.data(), *sampleH, sampleSize);

      st2094_50.decodeBinaryToSyntaxElements(binaryData);
      st2094_50.convertSyntaxElementsToMetadataItems();
      st2094_50.dbgPrintMetadataItems(); // Print decoded metadata from bitstream
    }

    // Samples are raw payloads (no box wrapper)
    out.write((char *)*sampleH, sampleSize);
    out.close();

    LOG_INFO("Extracted sample {}: {} bytes, DTS={}, duration={} (frame {})", i, sampleSize, dts,
             duration, currentFrame);

    // Add to manifest
    nlohmann::json item;
    item["frame_start"]        = currentFrame;
    item["frame_duration"]     = frameDuration;
    item["binary_file"]        = binFile.filename().string();
    item["sample_size"]        = sampleSize;
    item["dts"]                = static_cast<uint64_t>(dts);
    item["duration_timescale"] = static_cast<uint64_t>(duration);
    manifestItems.push_back(item);

    currentFrame += frameDuration;

    MP4DisposeHandle(sampleH);
  }

  LOG_INFO("Extracted {} metadata samples", sampleCount);

  // Write manifest JSON
  if(!manifestItems.empty())
  {
    nlohmann::json manifest;
    manifest["t35_prefix"]   = config.t35Prefix;
    manifest["timescale"]    = timescale;
    manifest["sample_count"] = sampleCount;
    manifest["items"]        = manifestItems;

    fs::path manifestFile = outDir / "manifest.json";
    std::ofstream manifestOut(manifestFile);
    if(manifestOut)
    {
      manifestOut << manifest.dump(2);
      manifestOut.close();
      LOG_INFO("Wrote manifest to {}", manifestFile.string());
    }
    else
    {
      LOG_WARN("Failed to write manifest file");
    }
  }

  LOG_INFO("Extraction complete");
  return MP4NoErr;
}

} // namespace t35
