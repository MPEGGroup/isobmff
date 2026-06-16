/**
 * @file t35_tool.cpp
 * @brief T.35 Metadata Tool - New Architecture
 *
 * This is the new modular implementation with clean separation of concerns:
 * - Sources (input formats)
 * - Injection strategies (MP4 container methods)
 * - Extraction strategies (output formats)
 */

// Standard library
#include <iostream>
#include <string>

// Third-party
#include <CLI/CLI.hpp>

// libisomediafile
extern "C"
{
#include "MP4Movies.h"
}

// T35 tool
#include "common/Logger.hpp"
#include "common/MetadataTypes.hpp"
#include "common/T35Prefix.hpp"
#include "sources/MetadataSource.hpp"
#include "injection/InjectionStrategy.hpp"
#include "extraction/ExtractionStrategy.hpp"

using namespace t35;

// ============================================================================
// Helper Functions
// ============================================================================

void printVersion()
{
  std::cout << "t35_tool v2.0 - T.35 Metadata Tool (New Architecture)\n";
  std::cout << "Built: " << __DATE__ << " " << __TIME__ << "\n";
}

void printAvailableOptions()
{
  std::cout << "\n";
  std::cout << "Available source types:\n";
  std::cout << "  json-manifest (or generic-json)  - Simple JSON with binary file references\n";
  std::cout << "  smpte-folder (or json-folder)    - Folder with SMPTE ST2094-50 JSON files\n";
  std::cout << "\n";
  std::cout << "Available injection methods:\n";
  std::cout << "  mebx-me4c                - MEBX track with me4c namespace (default)\n";
  std::cout << "  dedicated-it35           - Dedicated metadata track\n";
  std::cout << "  sample-group             - Sample group\n";
  std::cout << "  bitstream                - Inject into video bitstream (AV1/HEVC)\n";
  std::cout << "\n";
  std::cout << "Available extraction methods:\n";
  std::cout << "  auto                     - Auto-detect (default)\n";
  std::cout << "  mebx-me4c                - MEBX with me4c namespace\n";
  std::cout << "  dedicated-it35           - Dedicated metadata track\n";
  std::cout << "  sample-group             - Sample group\n";
  std::cout << "  sei                      - Convert to video with SEI (stub)\n";
  std::cout << "\n";
}

// ============================================================================
// Inject Command
// ============================================================================

int doInject(const std::string &inputFile, const std::string &outputFile,
             const std::string &sourceSpec, const std::string &methodName,
             const std::string &prefixStr)
{

  LOG_INFO("=== T.35 Metadata Injection ===");
  LOG_INFO("Input:  {}", inputFile);
  LOG_INFO("Output: {}", outputFile);
  LOG_INFO("Source: {}", sourceSpec);
  LOG_INFO("Method: {}", methodName);
  LOG_INFO("Prefix: {}", prefixStr);

  try
  {
    // Parse T.35 prefix
    T35Prefix prefix(prefixStr);
    if(!prefix.isValid())
    {
      LOG_ERROR("Invalid T.35 prefix: {}", prefixStr);
      return 1;
    }
    LOG_INFO("T.35 Prefix: {} ({})", prefix.hex(), prefix.description());

    // Create source
    auto source = createMetadataSource(sourceSpec);
    LOG_INFO("Created source: {} at {}", source->getType(), source->getPath());

    // Validate source
    std::string errorMsg;
    if(!source->validate(errorMsg))
    {
      LOG_ERROR("Source validation failed: {}", errorMsg);
      return 1;
    }

    // Load metadata
    MetadataMap items = source->load(prefix);

    // Validate metadata
    if(!validateMetadataMap(items, errorMsg))
    {
      LOG_ERROR("Metadata validation failed: {}", errorMsg);
      return 1;
    }

    // Open input movie
    LOG_INFO("Opening input movie...");
    MP4Movie movie = nullptr;
    MP4Err err     = MP4OpenMovieFile(&movie, inputFile.c_str(), MP4OpenMovieNormal);
    if(err || !movie)
    {
      LOG_ERROR("Failed to open input movie: {} (err={})", inputFile, err);
      return 1;
    }

    // Create injection strategy
    auto strategy = createInjectionStrategy(methodName);
    LOG_INFO("Created injection strategy: {}", strategy->getName());

    // Prepare injection config
    InjectionConfig config;
    config.movie     = movie;
    config.t35Prefix = prefix.hex();
    // TODO: Find video track and get sample durations

    // Check applicability
    std::string reason;
    if(!strategy->isApplicable(items, config, reason))
    {
      LOG_ERROR("Strategy '{}' not applicable: {}", methodName, reason);
      MP4DisposeMovie(movie);
      return 1;
    }

    // Inject
    err = strategy->inject(config, items, prefix);
    if(err)
    {
      LOG_ERROR("Injection failed with error: {}", err);
      MP4DisposeMovie(movie);
      return 1;
    }

    // Write output
    LOG_INFO("Writing output movie...");
    err = MP4WriteMovieToFile(movie, outputFile.c_str());
    if(err)
    {
      LOG_ERROR("Failed to write output movie: {}", err);
      MP4DisposeMovie(movie);
      return 1;
    }

    MP4DisposeMovie(movie);
    return 0;
  }
  catch(const T35Exception &e)
  {
    LOG_ERROR("T.35 Error: {}", e.what());
    return 1;
  }
  catch(const std::exception &e)
  {
    LOG_ERROR("Error: {}", e.what());
    return 1;
  }
}

// ============================================================================
// Extract Command
// ============================================================================

int doExtract(const std::string &inputFile, const std::string &outputPath,
              const std::string &methodName, const std::string &prefixStr)
{

  LOG_INFO("=== T.35 Metadata Extraction ===");
  LOG_INFO("Input:  {}", inputFile);
  LOG_INFO("Output: {}", outputPath);
  LOG_INFO("Method: {}", methodName);
  LOG_INFO("Prefix: {}", prefixStr);

  try
  {
    // Parse T.35 prefix
    T35Prefix prefix(prefixStr);
    if(!prefix.isValid())
    {
      LOG_ERROR("Invalid T.35 prefix: {}", prefixStr);
      return 1;
    }
    LOG_INFO("T.35 Prefix: {} ({})", prefix.hex(), prefix.description());

    // Open input movie
    LOG_INFO("Opening input movie...");
    MP4Movie movie = nullptr;
    MP4Err err     = MP4OpenMovieFile(&movie, inputFile.c_str(), MP4OpenMovieNormal);
    if(err || !movie)
    {
      LOG_ERROR("Failed to open input movie: {} (err={})", inputFile, err);
      return 1;
    }

    // Create extraction strategy
    auto strategy = createExtractionStrategy(methodName);
    LOG_INFO("Created extraction strategy: {}", strategy->getName());

    // Prepare extraction config
    ExtractionConfig config;
    config.movie      = movie;
    config.outputPath = outputPath;
    config.t35Prefix  = prefix.toString(); // Use full string with description

    // Only validate for non-auto strategies (auto tries all strategies internally)
    if(methodName != "auto")
    {
      std::string reason;
      if(!strategy->canExtract(config, reason))
      {
        LOG_ERROR("Cannot extract with '{}': {}", methodName, reason);
        MP4DisposeMovie(movie);
        return 1;
      }
    }

    // Extract
    err = strategy->extract(config);
    if(err)
    {
      LOG_ERROR("Extraction failed with error: {}", err);
      MP4DisposeMovie(movie);
      return 1;
    }

    MP4DisposeMovie(movie);
    return 0;
  }
  catch(const T35Exception &e)
  {
    LOG_ERROR("T.35 Error: {}", e.what());
    return 1;
  }
  catch(const std::exception &e)
  {
    LOG_ERROR("Error: {}", e.what());
    return 1;
  }
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char **argv)
{
  CLI::App app{"T.35 Metadata Tool - Modular Architecture"};
  app.set_version_flag("--version,-v", "2.0");
  app.footer("Use --help with subcommands for more information");

  // Global options
  int verbose = 2; // 0=error, 1=warn, 2=info, 3=debug
  app.add_option("--verbose", verbose, "Verbosity level (0-3)")
    ->default_val(2)
    ->check(CLI::Range(0, 3));

  bool listOptions = false;
  app.add_flag("--list-options", listOptions, "List available source types and methods");

  // ========== INJECT SUBCOMMAND ==========
  auto inject = app.add_subcommand("inject", "Inject metadata into MP4");

  std::string injectInput, injectOutput, injectSource;
  std::string injectMethod = "mebx-me4c";
  std::string injectPrefix = "B500900001:SMPTE-ST2094-50";

  inject->add_option("input", injectInput, "Input MP4 file")->required();
  inject->add_option("output", injectOutput, "Output MP4 file")->required();
  inject->add_option("--source,-s", injectSource, "Source spec (type:path)")->required();
  inject->add_option("--method,-m", injectMethod, "Injection method")->default_val("mebx-me4c");
  inject->add_option("--t35-prefix,-p", injectPrefix, "T.35 prefix (hex[:description])")
    ->default_val("B500900001:SMPTE-ST2094-50");

  // ========== EXTRACT SUBCOMMAND ==========
  auto extract = app.add_subcommand("extract", "Extract metadata from MP4");

  std::string extractInput, extractOutput;
  std::string extractMethod = "auto";
  std::string extractPrefix = "B500900001:SMPTE-ST2094-50";

  extract->add_option("input", extractInput, "Input MP4 file")->required();
  extract->add_option("output", extractOutput, "Output directory or file")->required();
  extract->add_option("--method,-m", extractMethod, "Extraction method")->default_val("auto");
  extract->add_option("--t35-prefix,-p", extractPrefix, "T.35 prefix (hex[:description])")
    ->default_val("B500900001:SMPTE-ST2094-50");

  // ========== PARSE ==========
  CLI11_PARSE(app, argc, argv);

  // Initialize logger
  Logger::init(verbose);

  if(listOptions)
  {
    printVersion();
    printAvailableOptions();
    return 0;
  }

  // Execute subcommand
  if(*inject)
  {
    return doInject(injectInput, injectOutput, injectSource, injectMethod, injectPrefix);
  }
  else if(*extract)
  {
    return doExtract(extractInput, extractOutput, extractMethod, extractPrefix);
  }
  else
  {
    std::cout << app.help() << "\n";
    return 0;
  }
}
