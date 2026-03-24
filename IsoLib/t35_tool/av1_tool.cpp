/**
 * @file av1_tool.cpp
 * @brief Implementation of a simple AV1 tool for debugging and analysis.
 * @version 0.1
 * @date 2025-08-26
 *
 * @copyright This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4
 * only for evaluation and testing purposes. Those intending to use this software module in hardware
 * or software products are advised that its use may infringe existing patents. The original
 * developer of this software module and his/her company, the subsequent editors and their
 * companies, and ISO/IEC have no liability for use of this software module or modifications thereof
 * in an implementation.
 *
 * Copyright is not released for non MPEG-4 conforming products. Apple Computer, Inc. retains full
 * right to use the code for its own purpose, assign or donate the code to a third party and to
 * inhibit third parties from using the code for non MPEG-4 conforming products. This copyright
 * notice must be included in all copies or derivative works.
 *
 */

// libisomediafile headers
extern "C"
{
#include "MP4Movies.h"
#include "MP4Atoms.h"
}

// C++ headers
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static void fourccToStr(u32 fcc, char out[5])
{
  out[0] = char((fcc >> 24) & 0xFF);
  out[1] = char((fcc >> 16) & 0xFF);
  out[2] = char((fcc >> 8) & 0xFF);
  out[3] = char((fcc) & 0xFF);
  out[4] = 0;
}

static std::string sampleFlagsToStr(u32 flags)
{
  std::ostringstream oss;
  oss << "0x" << std::hex << flags << std::dec;

  std::vector<std::string> labels;
  if(flags & MP4MediaSampleNotSync) labels.push_back("non-sync");
  else
    labels.push_back("sync");

  if(flags & MP4MediaSampleHasCTSOffset) labels.push_back("CTS-offset");

  if(!labels.empty())
  {
    oss << " [";
    for(size_t i = 0; i < labels.size(); ++i)
    {
      if(i) oss << ",";
      oss << labels[i];
    }
    oss << "]";
  }
  return oss.str();
}

static const char *obuTypeName(unsigned t)
{
  switch(t)
  {
  case 0:
    return "RESERVED_0";
  case 1:
    return "SEQUENCE_HEADER";
  case 2:
    return "TEMPORAL_DELIMITER";
  case 3:
    return "FRAME_HEADER";
  case 4:
    return "TILE_GROUP";
  case 5:
    return "METADATA";
  case 6:
    return "FRAME";
  case 7:
    return "REDUNDANT_FRAME_HEADER";
  case 8:
    return "TILE_LIST";
  case 9:
    return "PADDING";
  default:
    return "RESERVED";
  }
}

struct OBUHeader
{
  unsigned forbidden_bit = 0;
  unsigned obu_type      = 0;
  bool extension_flag    = false;
  bool has_size_field    = false;
  unsigned reserved_1bit = 0;
  unsigned temporal_id   = 0;
  unsigned spatial_id    = 0;
  uint64_t payload_size  = 0;
  size_t header_bytes    = 0; // bytes consumed by the header (incl. ext + leb)
};

// Minimal unsigned LEB128 (8-byte cap, as per AV1 limit for size fields)
static bool readULEB128(const uint8_t *p, size_t avail, uint64_t &value, size_t &used)
{
  value               = 0;
  used                = 0;
  const int MAX_BYTES = 8;
  while(used < avail && used < (size_t)MAX_BYTES)
  {
    uint8_t byte = p[used];
    if(value >> 57) return false; // would overflow 64-bit with next 7 bits
    value |= uint64_t(byte & 0x7F) << (7 * used);
    used++;
    if((byte & 0x80) == 0) return true;
  }
  return false; // ran out or exceeded cap without terminator
}

static bool parseOBUHeader(const uint8_t *data, size_t avail, OBUHeader &h)
{
  if(avail < 1) return false;
  uint8_t b0       = data[0];
  h.forbidden_bit  = (b0 >> 7) & 0x1;
  h.obu_type       = (b0 >> 3) & 0x0F;
  h.extension_flag = ((b0 >> 2) & 0x1) != 0;
  h.has_size_field = ((b0 >> 1) & 0x1) != 0;
  h.reserved_1bit  = b0 & 0x1;
  h.header_bytes   = 1;

  if(h.forbidden_bit != 0) return false; // spec requires 0

  if(h.extension_flag)
  {
    if(avail < h.header_bytes + 1) return false;
    uint8_t ext   = data[h.header_bytes];
    h.temporal_id = (ext >> 5) & 0x7;
    h.spatial_id  = (ext >> 3) & 0x3;
    // lower 3 bits are reserved 0 in spec; we won't strictly enforce
    h.header_bytes += 1;
  }

  if(!h.has_size_field)
  {
    // In AV1 ISOBMFF low overhead bitstream format is a must.
    return false;
  }

  if(h.header_bytes >= avail) return false;
  size_t leb_used  = 0;
  uint64_t payload = 0;
  if(!readULEB128(data + h.header_bytes, avail - h.header_bytes, payload, leb_used)) return false;
  h.payload_size = payload;
  h.header_bytes += leb_used;
  // Do not check payload bounds here; caller will ensure total fits in sample
  return true;
}

static void dumpAv1SampleOBUs(const uint8_t *sample, size_t sampleSize, uint32_t sampleIndex,
                              u32 flags, s32 cts, s32 dts)
{
  std::cout << "    Sample#" << sampleIndex + 1 << " size=" << sampleSize
            << " flags=" << sampleFlagsToStr(flags) << " CTS=" << cts << " DTS=" << dts << "\n";

  size_t off   = 0;
  unsigned idx = 0;
  while(off < sampleSize)
  {
    OBUHeader h;
    if(!parseOBUHeader(sample + off, sampleSize - off, h))
    {
      std::cout << "      !! OBU parse error at +" << off << "\n";
      break;
    }
    size_t total = h.header_bytes + size_t(h.payload_size);
    if(off + total > sampleSize)
    {
      std::cout << "      !! Truncated OBU at +" << off << " need=" << total
                << " have=" << (sampleSize - off) << "\n";
      break;
    }
    std::cout << "      OBU[" << idx++ << "] @+" << off << " type=" << h.obu_type << " ("
              << obuTypeName(h.obu_type) << ")"
              << " spatial_id=" << h.spatial_id << " temporal_id=" << h.temporal_id
              << " hdr=" << h.header_bytes << " payload=" << h.payload_size
              << (h.extension_flag ? " ext tid=" : "")
              << (h.extension_flag ? std::to_string(h.temporal_id) : "")
              << (h.extension_flag ? " sid=" : "")
              << (h.extension_flag ? std::to_string(h.spatial_id) : "") << "\n";
    off += total;
  }
  if(off != sampleSize)
  {
    std::cout << "      note: leftover=" << (sampleSize - off) << " bytes\n";
  }
}

int main(int argc, char **argv)
{
  if(argc < 2)
  {
    std::cerr << "Usage: av1_tool <file.mp4>\n";
    return 1;
  }

  MP4Err err    = MP4NoErr;
  MP4Movie moov = nullptr;

  // Open MP4
  err = MP4OpenMovieFile(&moov, argv[1], MP4OpenMovieNormal);
  if(err)
  {
    std::cerr << "Failed to open " << argv[1] << " (err=" << err << ")\n";
    return err;
  }

  u32 trackCount = 0;
  if((err = MP4GetMovieTrackCount(moov, &trackCount)))
  {
    MP4DisposeMovie(moov);
    return err;
  }
  std::cout << "Movie has " << trackCount << " tracks\n";

  for(u32 trackNumber = 1; trackNumber <= trackCount; ++trackNumber)
  {
    MP4Track trak = nullptr;
    if(MP4GetMovieIndTrack(moov, trackNumber, &trak) != MP4NoErr || !trak) continue;

    MP4TrackReader reader = nullptr;
    if(MP4CreateTrackReader(trak, &reader) != MP4NoErr || !reader) continue;

    MP4Handle sampleEntryH = nullptr;
    MP4NewHandle(0, &sampleEntryH);
    if(!sampleEntryH)
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

    u32 sampleEntryType = 0;
    ISOGetSampleDescriptionType(sampleEntryH, &sampleEntryType);
    char typeStr[5];
    fourccToStr(sampleEntryType, typeStr);
    std::cout << "Track " << trackNumber << " sample entry: " << typeStr;

    // If resv or encv, get the original format
    if(sampleEntryType == MP4RestrictedVideoSampleEntryAtomType ||
       sampleEntryType == MP4EncVisualSampleEntryAtomType)
    {
      ISOGetOriginalFormat(sampleEntryH, &sampleEntryType);
      fourccToStr(sampleEntryType, typeStr);
      std::cout << ":" << typeStr << "\n";
    }
    else
    {
      std::cout << "\n";
    }

    if(sampleEntryType == MP4_FOUR_CHAR_CODE('a', 'v', '0', '1'))
    {
      std::cout << "  AV1 track detected — dumping OBU headers per sample \n";

      MP4Handle auH = nullptr;
      MP4NewHandle(0, &auH);
      if(!auH)
      {
        MP4DisposeHandle(sampleEntryH);
        MP4DisposeTrackReader(reader);
        continue;
      }

      u32 auSize = 0;
      u32 flags  = 0;
      s32 cts = 0, dts = 0;
      u32 auIndex = 0;

      while((err = MP4TrackReaderGetNextAccessUnit(reader, auH, &auSize, &flags, &cts, &dts)) ==
            MP4NoErr)
      {
        const uint8_t *bytes = (const uint8_t *)*auH;
        dumpAv1SampleOBUs(bytes, auSize, auIndex++, flags, cts, dts);
        // Clear for next AU (keeps capacity per lib's handle semantics)
        MP4SetHandleSize(auH, 0);
      }

      if(err != MP4EOF && err != MP4NoErr)
      {
        std::cerr << "  reader error on track " << trackNumber << " (err=" << err << ")\n";
      }

      MP4DisposeHandle(auH);
    }
    else
    {
      std::cout << "  (not an AV1 track, skipping sample dump)\n";
    }

    MP4DisposeHandle(sampleEntryH);
    MP4DisposeTrackReader(reader);
  }

  MP4DisposeMovie(moov);
  return err;
}
