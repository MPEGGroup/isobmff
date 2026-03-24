#include "MebxMe4cExtractor.hpp"
#include "../sources/SMPTE_ST2094_50.hpp"
#include "../common/Logger.hpp"
#include "../common/T35Prefix.hpp"

extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
}

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace t35
{

// Helper: Convert u32 4CC to string representation
static std::string fourCCToString(u32 fourcc)
{
  char buf[5] = {0};
  buf[0]      = (fourcc >> 24) & 0xFF;
  buf[1]      = (fourcc >> 16) & 0xFF;
  buf[2]      = (fourcc >> 8) & 0xFF;
  buf[3]      = fourcc & 0xFF;
  return std::string(buf);
}

// MebxMe4cExtractor implementation

MebxMe4cExtractor::~MebxMe4cExtractor() { clearCache(); }

void MebxMe4cExtractor::clearCache()
{
  if(m_cachedReader)
  {
    MP4DisposeTrackReader(m_cachedReader);
    m_cachedReader = nullptr;
  }
  m_cachedTrack = nullptr;
}

// Helper: Find mebx track with me4c namespace and it35 key_value
static MP4Err findMebxMe4cTrackReader(MP4Movie moov, const std::string &t35PrefixStr,
                                      MP4TrackReader *outReader, MP4Track *outTrack)
{
  MP4Err err = MP4NoErr;
  *outReader = nullptr;
  if(outTrack) *outTrack = nullptr;

  u32 trackCount = 0;
  err            = MP4GetMovieTrackCount(moov, &trackCount);
  if(err) return err;

  LOG_DEBUG("Searching for mebx me4c track in {} tracks", trackCount);

  // Search for mebx track
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

    // Create track reader
    MP4TrackReader reader = nullptr;
    err                   = MP4CreateTrackReader(trak, &reader);
    if(err) continue;

    // Get sample description (we'll reuse this for both type checking and metadata config)
    MP4Handle sampleEntryH = nullptr;
    err                    = MP4NewHandle(0, &sampleEntryH);
    if(err)
    {
      MP4DisposeTrackReader(reader);
      continue;
    }

    err = MP4TrackReaderGetCurrentSampleDescription(reader, sampleEntryH);
    if(err)
    {
      MP4DisposeHandle(sampleEntryH);
      MP4DisposeTrackReader(reader);
      continue;
    }

    // Check if it's 'mebx'
    u32 type = 0;
    err      = ISOGetSampleDescriptionType(sampleEntryH, &type);

    if(err || type != MP4BoxedMetadataSampleEntryType)
    {
      MP4DisposeHandle(sampleEntryH);
      MP4DisposeTrackReader(reader);
      continue;
    }

    LOG_INFO("Found mebx track with ID {}", trackID);

    // Check for 'rndr' track reference
    MP4Track videoTrack = nullptr;
    err = MP4GetTrackReference(trak, MP4_FOUR_CHAR_CODE('r', 'n', 'd', 'r'), 1, &videoTrack);
    if(err)
    {
      LOG_WARN("Mebx track ID {} has no 'rndr' track reference, skipping", trackID);
      MP4DisposeHandle(sampleEntryH);
      MP4DisposeTrackReader(reader);
      continue;
    }

    u32 videoTrackID = 0;
    MP4GetTrackID(videoTrack, &videoTrackID);
    LOG_DEBUG("Mebx track references video track ID {}", videoTrackID);

    // Search for me4c namespace key with 'it35' key_value
    u32 key_namespace   = MP4_FOUR_CHAR_CODE('m', 'e', '4', 'c');
    MP4Handle key_value = nullptr;
    err                 = MP4NewHandle(4, &key_value);
    if(err)
    {
      MP4DisposeHandle(sampleEntryH);
      MP4DisposeTrackReader(reader);
      return err;
    }

    // Key_value = 'it35' (4CC)
    u32 it35_fourcc = MP4_FOUR_CHAR_CODE('i', 't', '3', '5');
    char *keyPtr    = (char *)*key_value;
    keyPtr[0]       = (it35_fourcc >> 24) & 0xFF;
    keyPtr[1]       = (it35_fourcc >> 16) & 0xFF;
    keyPtr[2]       = (it35_fourcc >> 8) & 0xFF;
    keyPtr[3]       = it35_fourcc & 0xFF;

    LOG_DEBUG("Searching for key_namespace='me4c', key_value='it35' (4CC)");

    // Iterate through all matches to find one with matching setupInfo
    u32 selected_local_key_id = 0;
    int found_match           = 0;

    for(u32 matchIdx = 0;; matchIdx++)
    {
      u32 abs_idx      = 0;
      u32 local_key_id = 0;

      // Find the matchIdx-th entry with matching namespace + key_value
      err = MP4FindMebxKeyMatchByIndex(sampleEntryH, key_namespace, key_value, matchIdx, &abs_idx,
                                       &local_key_id);
      if(err)
      {
        // No more matches
        LOG_DEBUG("No more matches after checking {} entries", matchIdx);
        break;
      }

      LOG_DEBUG("Found match {}: abs_idx={}, local_key_id=0x{:08X} ('{}')", matchIdx, abs_idx,
                local_key_id, fourCCToString(local_key_id));

      // Read setupInfo for this match
      MP4Handle read_key_value = nullptr;
      MP4Handle setupInfoH     = nullptr;

      err = MP4NewHandle(0, &read_key_value);
      if(err)
      {
        LOG_ERROR("Failed to create read_key_value handle (err={})", err);
        continue;
      }

      err = MP4NewHandle(0, &setupInfoH);
      if(err)
      {
        LOG_ERROR("Failed to create setupInfo handle (err={})", err);
        MP4DisposeHandle(read_key_value);
        continue;
      }

      u32 read_local_key_id  = 0;
      u32 read_key_namespace = 0;
      err = ISOGetMebxMetadataConfig(sampleEntryH, abs_idx, &read_local_key_id, &read_key_namespace,
                                     read_key_value, nullptr, setupInfoH);

      if(err)
      {
        LOG_WARN("ISOGetMebxMetadataConfig failed for match {} (err={})", matchIdx, err);
        MP4DisposeHandle(setupInfoH);
        MP4DisposeHandle(read_key_value);
        continue;
      }

      LOG_DEBUG("Match {} config: local_key_id=0x{:08X}, namespace=0x{:08X}", matchIdx,
                read_local_key_id, read_key_namespace);

      // Check setupInfo
      u32 setupInfoSize = 0;
      MP4GetHandleSize(setupInfoH, &setupInfoSize);
      LOG_DEBUG("  setupInfo size = {} bytes", setupInfoSize);

      if(setupInfoSize > 0)
      {
        // Parse setupInfo binary format:
        // 1. utf8string description (null-terminated)
        // 2. unsigned int(8) t35_identifier[] (remaining bytes)

        char *setupData = (char *)*setupInfoH;

        // Read null-terminated description
        size_t descLen = 0;
        for(size_t i = 0; i < setupInfoSize; i++)
        {
          if(setupData[i] == '\0')
          {
            descLen = i;
            break;
          }
        }

        std::string desc;
        if(descLen > 0)
        {
          desc = std::string(setupData, descLen);
        }

        // Read remaining bytes as t35_identifier
        u32 identifierStart = descLen + 1; // Skip null terminator
        u32 identifierSize  = setupInfoSize - identifierStart;

        std::vector<uint8_t> identifierBytes;
        if(identifierSize > 0 && identifierStart < setupInfoSize)
        {
          identifierBytes.assign((uint8_t *)(setupData + identifierStart),
                                 (uint8_t *)(setupData + setupInfoSize));
        }

        // Convert identifier bytes to hex string
        std::string hexStr = T35Prefix::bytesToHex(identifierBytes);

        LOG_DEBUG("  Parsed setupInfo: description='{}', identifier={} ({} bytes)",
                  desc.empty() ? "(empty)" : desc, hexStr, identifierBytes.size());

        // Create T35Prefix from parsed components
        T35Prefix filePrefix(hexStr, desc);

        // Parse requested prefix
        T35Prefix requestedPrefix(t35PrefixStr);

        // Verify hex prefix matches (ignore description)
        if(requestedPrefix.hex() == filePrefix.hex())
        {
          LOG_DEBUG("T.35 hex matches requested hex!");

          // Warn if descriptions differ (informative only)
          if(!requestedPrefix.description().empty() && !filePrefix.description().empty() &&
             requestedPrefix.description() != filePrefix.description())
          {
            LOG_WARN("T.35 description mismatch: requested '{}' but file has '{}'",
                     requestedPrefix.description(), filePrefix.description());
          }

          // Found the correct match!
          selected_local_key_id = local_key_id;
          found_match           = 1;
          MP4DisposeHandle(setupInfoH);
          MP4DisposeHandle(read_key_value);
          break;
        }
        else
        {
          LOG_DEBUG("T.35 hex '{}' does not match requested hex '{}', continuing search",
                    filePrefix.hex(), requestedPrefix.hex());
        }
      }
      else
      {
        LOG_WARN("setupInfo is empty for match {}", matchIdx);
      }

      MP4DisposeHandle(setupInfoH);
      MP4DisposeHandle(read_key_value);
    }

    MP4DisposeHandle(key_value);

    if(!found_match)
    {
      LOG_DEBUG("No match found with requested T.35 prefix for track {}", trackID);
      MP4DisposeHandle(sampleEntryH);
      MP4DisposeTrackReader(reader);
      continue;
    }

    // Configure the reader with the selected local_key_id
    err = MP4SetMebxTrackReaderLocalKeyId(reader, selected_local_key_id);
    if(err)
    {
      LOG_ERROR("Failed to set local_key_id (err={})", err);
      MP4DisposeHandle(sampleEntryH);
      MP4DisposeTrackReader(reader);
      continue;
    }

    LOG_INFO("Selected mebx me4c track ID {} with local_key_id = '{}' (0x{:08X})", trackID,
             fourCCToString(selected_local_key_id), selected_local_key_id);

    MP4DisposeHandle(sampleEntryH);

    // Success!
    *outReader = reader;
    if(outTrack) *outTrack = trak;
    return MP4NoErr;
  }

  LOG_ERROR("No mebx track found with me4c namespace and it35 key_value");
  return MP4NotFoundErr;
}

bool MebxMe4cExtractor::canExtract(const ExtractionConfig &config, std::string &reason)
{
  if(!config.movie)
  {
    reason = "No movie provided";
    return false;
  }

  // Clear any previous cache
  clearCache();

  // Find and cache the reader with setupInfo verification
  MP4Err err =
    findMebxMe4cTrackReader(config.movie, config.t35Prefix, &m_cachedReader, &m_cachedTrack);

  if(err)
  {
    reason = "No mebx track with me4c namespace and matching T.35 prefix found";
    return false;
  }

  return true;
}

MP4Err MebxMe4cExtractor::extract(const ExtractionConfig &config, MetadataMap *outItems)
{
  LOG_INFO("Extracting metadata using mebx-me4c extractor");
  LOG_INFO("T.35 prefix: {}", config.t35Prefix);
  if(!outItems)
  {
    LOG_INFO("Output path: {}", config.outputPath);
  }
  else
  {
    LOG_INFO("Output mode: in-memory");
  }

  MP4Err err                = MP4NoErr;
  MP4TrackReader mebxReader = nullptr;
  MP4Track mebxTrack        = nullptr;

  // Use cached reader if available (from canExtract), otherwise find it now
  if(m_cachedReader)
  {
    LOG_DEBUG("Using cached mebx me4c track reader");
    mebxReader = m_cachedReader;
    mebxTrack  = m_cachedTrack;

    // Clear cache so we don't double-dispose
    m_cachedReader = nullptr;
    m_cachedTrack  = nullptr;
  }
  else
  {
    // Fallback: extract() called without canExtract()
    LOG_DEBUG("Finding mebx me4c track (cache not available)");
    err = findMebxMe4cTrackReader(config.movie, config.t35Prefix, &mebxReader, &mebxTrack);
    if(err)
    {
      LOG_ERROR("Failed to find mebx me4c track (err={})", err);
      return err;
    }
  }

  // Get timescale
  MP4Media mebxMedia = nullptr;
  u32 timescale      = 1000; // default
  err                = MP4GetTrackMedia(mebxTrack, &mebxMedia);
  if(err == MP4NoErr)
  {
    MP4GetMediaTimeScale(mebxMedia, &timescale);
  }
  LOG_DEBUG("Mebx track timescale: {}", timescale);

  // Create output directory
  namespace fs = std::filesystem;
  fs::path outDir(config.outputPath);

  if(!fs::exists(outDir))
  {
    if(!fs::create_directories(outDir))
    {
      LOG_ERROR("Failed to create output directory: {}", config.outputPath);
      MP4DisposeTrackReader(mebxReader);
      return MP4IOErr;
    }
  }

  LOG_INFO("Extracting samples to {}", outDir.string());

  // Extract all samples
  std::vector<nlohmann::json> manifestItems;
  u32 sampleCount  = 0;
  u32 currentFrame = 0;

  for(u32 i = 1;; ++i)
  {
    MP4Handle sampleH = nullptr;
    u32 sampleSize = 0, sampleFlags = 0, sampleDuration = 0;
    s32 dts = 0, cts = 0;

    err = MP4NewHandle(0, &sampleH);
    if(err)
    {
      MP4DisposeTrackReader(mebxReader);
      return err;
    }

    err = MP4TrackReaderGetNextAccessUnitWithDuration(mebxReader, sampleH, &sampleSize,
                                                      &sampleFlags, &dts, &cts, &sampleDuration);

    if(err)
    {
      MP4DisposeHandle(sampleH);
      if(err == MP4EOF)
      {
        LOG_DEBUG("Reached end of mebx samples");
        err = MP4NoErr;
        break;
      }
      LOG_ERROR("Failed to read sample (err={})", err);
      MP4DisposeTrackReader(mebxReader);
      return err;
    }

    sampleCount++;

    // Calculate frame duration (assuming constant frame rate)
    u32 frameDuration = 1;
    if(sampleDuration > 0)
    {
      frameDuration = sampleDuration / 512; // rough estimate
      if(frameDuration == 0) frameDuration = 1;
    }

    // Write binary file
    fs::path binFile = outDir / ("metadata_" + std::to_string(i) + ".bin");
    std::ofstream out(binFile, std::ios::binary);
    if(!out)
    {
      LOG_ERROR("Failed to open {} for writing", binFile.string());
      MP4DisposeHandle(sampleH);
      MP4DisposeTrackReader(mebxReader);
      return MP4IOErr;
    }

    out.write((char *)*sampleH, sampleSize);
    out.close();

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

    LOG_INFO("Extracted sample {}: {} bytes, DTS={}, duration={} (frame {})", i, sampleSize, dts,
             sampleDuration, currentFrame);

    // Add to manifest
    nlohmann::json item;
    item["frame_start"]        = currentFrame;
    item["frame_duration"]     = frameDuration;
    item["binary_file"]        = binFile.filename().string();
    item["sample_size"]        = sampleSize;
    item["dts"]                = dts;
    item["duration_timescale"] = sampleDuration;
    manifestItems.push_back(item);

    currentFrame += frameDuration;

    MP4DisposeHandle(sampleH);
  }

  MP4DisposeTrackReader(mebxReader);

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
