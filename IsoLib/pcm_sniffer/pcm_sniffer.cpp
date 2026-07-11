/**
 * @file pcm_sniffer.cpp
 * @brief Implementation of a simple PCM sniffer tool for HEVC
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
extern "C" {
  #include "MP4Movies.h"
  #include "MP4Atoms.h"
}

// HM headers
#include "AnnexBread.h"   // InputByteStream, AnnexBStats, byteStreamNALUnit
#include "NALread.h"      // InputNALUnit, read
#include "TDecCAVLC.h"    // TDecCavlc
#include "TComSlice.h"    // TComSPS, CHANNEL_TYPE_LUMA

// C++ headers
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static void fourccToStr(u32 fcc, char out[5]) {
  out[0] = char((fcc >> 24) & 0xFF);
  out[1] = char((fcc >> 16) & 0xFF);
  out[2] = char((fcc >>  8) & 0xFF);
  out[3] = char((fcc      ) & 0xFF);
  out[4] = 0;
}

class PcmSniffer {
public:
  int run(const std::string& fileName) {
    MP4Err err = MP4NoErr;
    MP4Movie moov = nullptr;

    // Open MP4
    err = MP4OpenMovieFile(&moov, fileName.c_str(), MP4OpenMovieNormal);
    if (err) {
      std::cerr << "Failed to open " << fileName << " (err=" << err << ")\n";
      return err;
    }

    u32 trackCount = 0;
    if ((err = MP4GetMovieTrackCount(moov, &trackCount))) {
      MP4DisposeMovie(moov);
      return err;
    }
    std::cout << "Movie has " << trackCount << " tracks\n";

    for (u32 trackNumber = 1; trackNumber <= trackCount; ++trackNumber) {
      MP4Track trak = nullptr;
      if (MP4GetMovieIndTrack(moov, trackNumber, &trak) != MP4NoErr || !trak) continue;

      MP4TrackReader reader = nullptr;
      if (MP4CreateTrackReader(trak, &reader) != MP4NoErr || !reader) continue;

      MP4Handle sampleEntryH = nullptr;
      MP4NewHandle(0, &sampleEntryH);
      if (!sampleEntryH) { MP4DisposeTrackReader(reader); continue; }

      err = MP4TrackReaderGetCurrentSampleDescription(reader, sampleEntryH);
      if (err) {
        MP4DisposeHandle(sampleEntryH);
        MP4DisposeTrackReader(reader);
        continue;
      }

      u32 sampleEntryType = 0;
      ISOGetSampleDescriptionType(sampleEntryH, &sampleEntryType);
      char typeStr[5]; fourccToStr(sampleEntryType, typeStr);
      std::cout << "Track " << trackNumber << " sample entry: " << typeStr;

      // If resv or encv, get the original format
      if (sampleEntryType == MP4RestrictedVideoSampleEntryAtomType || sampleEntryType == MP4EncVisualSampleEntryAtomType) {
        ISOGetOriginalFormat(sampleEntryH, &sampleEntryType);
        fourccToStr(sampleEntryType, typeStr);
        std::cout << ":" << typeStr << "\n";
      }
      else {
        std::cout << "\n";
      }

      if (sampleEntryType == ISOHEVCSampleEntryAtomType || sampleEntryType == MP4_FOUR_CHAR_CODE('h', 'e', 'v', '1')) {
        MP4Handle nalusH = nullptr;
        MP4NewHandle(0, &nalusH);
        if (!nalusH) {
          MP4DisposeHandle(sampleEntryH);
          MP4DisposeTrackReader(reader);
          continue;
        }

        // extraction_mode = 0 get NALUs from hvcC (HM may have issues with lhvC)
        err = ISOGetHEVCNALUsFromSampleEntry(sampleEntryH, nalusH, 1);
        if (!err) {
          u32 size = 0; MP4GetHandleSize(nalusH, &size);
          // std::cout << "Got " << size << " NALU bytes from sample entry\n";

          if (size > 0) {
            // HM wants a std::istream. Wrap into istringstream, then InputByteStream
            std::string annexB(reinterpret_cast<char*>(*nalusH), size);
            std::istringstream iss(annexB, std::ios::in | std::ios::binary);
            InputByteStream ibs(iss);

            AnnexBStats stats{};
            TDecCavlc cavlc;

            while (true) {
              std::vector<uint8_t> nalUnit;
              Bool eof = byteStreamNALUnit(ibs, nalUnit, stats);
              if (nalUnit.empty() && eof) break;

              // Load NAL bytes into InputNALUnit
              InputNALUnit nalu;
              {
                auto& bs = nalu.getBitstream();  // reference to internal bitstream
                bs.getFifo() = nalUnit;          // copy NAL payload bytes
                bs.resetToStart();               // reset reader indices
              }

              // Parse NAL header. Sets nalu.m_nalUnitType and primes RBSP
              read(nalu);

              if (nalu.m_nalUnitType == NAL_UNIT_SPS) {
                // Tell CAVLC where to read from, then parse SPS
                cavlc.setBitstream(&nalu.getBitstream());
                TComSPS sps;
                cavlc.parseSPS(&sps);

                std::cout << "  SPS id=" << sps.getSPSId()
                          << " res=" << sps.getPicWidthInLumaSamples()
                          << "x"     << sps.getPicHeightInLumaSamples()
                          << " bitDepth=" << sps.getBitDepth(CHANNEL_TYPE_LUMA)
                          << " pcm_enabled_flag=" << sps.getUsePCM()
                          << "\n";
              }
            }
          }
        } else {
          std::cerr << "ISOGetHEVCNALUsFromSampleEntry failed with " << err << "\n";
        }

        MP4DisposeHandle(nalusH);
      }

      MP4DisposeHandle(sampleEntryH);
      MP4DisposeTrackReader(reader);
    }

    MP4DisposeMovie(moov);
    return 0;
  }
};

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Usage: pcm_sniffer <file.mp4>\n";
    return 1;
  }
  PcmSniffer sniffer;
  return sniffer.run(argv[1]);
}
