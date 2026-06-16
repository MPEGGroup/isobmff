# T.35 Metadata Tool v2.0

A modular command-line tool for injecting and extracting ITU-T T.35 metadata into/from MP4 video files. This tool is specifically designed for handling SMPTE ST 2094-50 dynamic metadata and other T.35-based metadata formats.

## Overview

The T.35 Metadata Tool provides a clean, modular architecture for working with T.35 metadata in MP4 containers. It supports multiple injection and extraction methods, allowing flexibility in how metadata is embedded and retrieved from video files.

### Key Features

- **Multiple Injection Methods**: Support for MEBX tracks (me4c namespace), dedicated metadata tracks, and sample groups
- **Flexible Extraction**: Auto-detection or manual selection of extraction strategies
- **Multiple Source Formats**: JSON manifests with binary references, SMPTE folder structures
- **T.35 Prefix Support**: Configurable ITU-T T.35 country/terminal provider codes
- **Validation**: Built-in validation for metadata integrity and applicability
- **Clean Architecture**: Separation of concerns with pluggable sources and strategies

## Architecture

The tool follows a modular design with three main components:

1. **Sources** - Handle input metadata formats
   - JSON manifest with binary file references
   - SMPTE folder structures with JSON files

2. **Injection Strategies** - Define how metadata is embedded into MP4
   - MEBX track with me4c namespace
   - Dedicated metadata track (it35)
   - Sample groups

3. **Extraction Strategies** - Define how metadata is retrieved from MP4
   - Auto-detection (tries all methods)
   - MEBX extraction (me4c)
   - Dedicated track extraction
   - Sample group extraction
   - SEI conversion (stub)

## Building

### Prerequisites

- C++17 compatible compiler
- CMake 3.15 or later
- CLI11 library (command-line parsing)
- libisomediafile (MP4 manipulation)

### Build Instructions

```bash
mkdir build
cd build
cmake ..
make
```

The compiled binary will be available as `t35_tool`.

## Usage

### Basic Command Structure

```bash
t35_tool [OPTIONS] SUBCOMMAND
```

### Global Options

- `--verbose <level>` - Set verbosity level (0-3)
  - 0: Errors only
  - 1: Warnings
  - 2: Info (default)
  - 3: Debug
- `--list-options` - Display all available source types and injection/extraction methods
- `--version, -v` - Show version information

### Inject Command

Inject T.35 metadata into an MP4 file.

```bash
t35_tool inject [OPTIONS] input output
```

#### Required Arguments

- `input` - Input MP4 file path
- `output` - Output MP4 file path

#### Options

- `--source, -s <spec>` - Source specification in format `type:path` (required)
  - `json-manifest:/path/to/manifest.json`
  - `smpte-folder:/path/to/folder`
  - `generic-json:/path/to/file.json` (alias for json-manifest)
  - `json-folder:/path/to/folder` (alias for smpte-folder)

- `--method, -m <method>` - Injection method (default: `mebx-me4c`)
  - `mebx-me4c` - MEBX track with me4c namespace
  - `dedicated-it35` - Dedicated metadata track
  - `sample-group` - Sample group approach

- `--t35-prefix, -p <prefix>` - T.35 prefix in format `HEX[:description]`
  - Default: `B500900001:SMPTE-ST2094-50`
  - Format: Country code + terminal provider code (hex) + optional description

#### Examples

```bash
# Inject using JSON manifest with default MEBX-me4c method
t35_tool inject input.mp4 output.mp4 \
  --source json-manifest:metadata.json

# Inject using SMPTE folder with dedicated track method
t35_tool inject input.mp4 output.mp4 \
  --source smpte-folder:./metadata_folder \
  --method dedicated-it35

# Inject using sample groups
t35_tool inject input.mp4 output.mp4 \
  --source json-manifest:metadata.json \
  --method sample-group
```

### Extract Command

Extract T.35 metadata from an MP4 file.

```bash
t35_tool extract [OPTIONS] input output
```

#### Required Arguments

- `input` - Input MP4 file path
- `output` - Output directory or file path

#### Options

- `--method, -m <method>` - Extraction method (default: `auto`)
  - `auto` - Auto-detect extraction method
  - `mebx-it35` - Extract from MEBX track (it35 namespace)
  - `mebx-me4c` - Extract from MEBX track (me4c namespace)
  - `dedicated-it35` - Extract from dedicated metadata track
  - `sample-group` - Extract from sample groups
  - `sei` - Convert to SEI messages (stub)

- `--t35-prefix, -p <prefix>` - T.35 prefix filter
  - Default: `B500900001:SMPTE-ST2094-50`

#### Examples

```bash
# Auto-detect and extract to directory
t35_tool extract input.mp4 ./output_folder

# Extract from MEBX-me4c track
t35_tool extract input.mp4 ./output_folder \
  --method mebx-me4c

# Extract with specific T.35 prefix filter
t35_tool extract input.mp4 ./output_folder \
  --method auto \
  --t35-prefix B500900001:SMPTE-ST2094-50

# Extract from sample groups
t35_tool extract input.mp4 ./output_folder \
  --method sample-group
```

## Source Formats

### JSON Manifest (json-manifest / generic-json)

A simple JSON file that references binary metadata files:

```json
{
  "metadata": [
    {
      "sample": 0,
      "file": "metadata_000.bin"
    },
    {
      "sample": 1,
      "file": "metadata_001.bin"
    }
  ]
}
```

Binary files should contain raw T.35 payload data (without the T.35 prefix).

### SMPTE Folder (smpte-folder / json-folder)

A directory containing individual JSON files following SMPTE ST 2094-50 format:

```
metadata_folder/
├── frame_0000.json
├── frame_0001.json
├── frame_0002.json
└── ...
```

Each JSON file contains SMPTE ST 2094-50 compliant metadata for a single frame.

## T.35 Prefix Format

The T.35 prefix identifies the metadata type using ITU-T T.35 country and terminal provider codes:

- Format: `CCTTTTTTTT[:Description]`
  - `CC` - Country code (2 hex digits)
  - `TTTTTTTT` - Terminal provider code (8 hex digits)
  - `Description` - Optional human-readable description

### Common Prefixes

- `B500900001:SMPTE-ST2094-50` - SMPTE ST 2094-50 dynamic metadata
- `B5003C0001:HDR10Plus` - HDR10+ metadata (example)

## Injection Methods Explained

### MEBX with me4c Namespace

- **Identifier**: `mebx-me4c`
- **Description**: Uses the Metadata Extension Box (MEBX) with the 'me4c' namespace
- **Use Case**: Standard metadata embedding for compatibility with various players
- **Container**: MEBX track in MP4

### Dedicated Metadata Track

- **Identifier**: `dedicated-it35`
- **Description**: Creates a separate metadata track specifically for T.35 data
- **Use Case**: When metadata needs to be clearly separated from video streams
- **Container**: Dedicated track with it35 handler

### Sample Groups

- **Identifier**: `sample-group`
- **Description**: Associates metadata with video samples using sample-to-group mapping
- **Use Case**: Frame-accurate metadata association with minimal overhead
- **Container**: Sample group boxes in MP4

## Extraction Methods

### Auto Detection

- **Identifier**: `auto`
- **Description**: Automatically tries all extraction methods and uses the first successful one
- **Recommended**: Yes, for general use

### Manual Extraction

Specify the exact extraction method when you know the injection method used:

- `mebx-me4c` - For MEBX tracks with me4c namespace
- `dedicated-it35` - For dedicated metadata tracks
- `sample-group` - For sample group based metadata

## Error Handling

The tool provides detailed error messages for common issues:

- **Invalid T.35 prefix**: Malformed prefix string
- **Source validation failed**: Invalid source format or missing files
- **Metadata validation failed**: Corrupt or invalid metadata content
- **Strategy not applicable**: Chosen method incompatible with input
- **Injection/Extraction failed**: MP4 manipulation errors

Exit codes:
- `0` - Success
- `1` - Error occurred (check log output)

## Logging and Debugging

Control verbosity with the `--verbose` flag:

```bash
# Error messages only
t35_tool --verbose 0 inject input.mp4 output.mp4 --source json-manifest:meta.json

# Full debug output
t35_tool --verbose 3 inject input.mp4 output.mp4 --source json-manifest:meta.json
```

Log messages include:
- Input/output file paths
- Source type and validation status
- Injection/extraction method used
- T.35 prefix details
- Success/failure status with error details

## Workflow Examples

### Complete Round-Trip Workflow

1. **Prepare metadata**:
   ```json
   {
     "metadata": [
       {"sample": 0, "file": "frame_000.bin"},
       {"sample": 1, "file": "frame_001.bin"}
     ]
   }
   ```

2. **Inject into video**:
   ```bash
   t35_tool inject original.mp4 with_metadata.mp4 \
     --source json-manifest:manifest.json \
     --method mebx-me4c
   ```

3. **Verify by extracting**:
   ```bash
   t35_tool extract with_metadata.mp4 ./extracted \
     --method auto
   ```

4. **Compare original and extracted metadata**:
   ```bash
   diff -r original_metadata/ extracted/
   ```

### Working with SMPTE Folders

1. **Organize SMPTE JSON files**:
   ```
   smpte_metadata/
   ├── frame_0000.json
   ├── frame_0001.json
   └── ...
   ```

2. **Inject**:
   ```bash
   t35_tool inject video.mp4 video_with_smpte.mp4 \
     --source smpte-folder:./smpte_metadata \
     --method dedicated-it35 \
     --t35-prefix B500900001:SMPTE-ST2094-50
   ```

3. **Extract and verify**:
   ```bash
   t35_tool extract video_with_smpte.mp4 ./extracted_smpte
   ```

## Limitations and Known Issues

- SEI conversion (`--method sei`) is currently a stub implementation
- Sample group extraction requires exact frame alignment
- Large metadata files may impact MP4 file size significantly
- Some extraction methods may not preserve exact binary formatting

## Development

### Project Structure

```
t35_tool/
├── t35_tool.cpp             # Main entry point
├── common/
│   ├── Logger.hpp           # Logging utilities
│   ├── MetadataTypes.hpp    # Core data types
│   └── T35Prefix.hpp        # T.35 prefix handling
├── sources/
│   └── MetadataSource.hpp   # Source abstraction and factory
├── injection/
│   └── InjectionStrategy.hpp # Injection strategy abstraction
└── extraction/
    └── ExtractionStrategy.hpp # Extraction strategy abstraction
```

### Adding New Sources

Implement the `MetadataSource` interface and register in the factory.

### Adding New Injection Methods

Implement the `InjectionStrategy` interface and register in the factory.

### Adding New Extraction Methods

Implement the `ExtractionStrategy` interface and register in the factory.

## Contributing

When contributing, please ensure:
- Code follows existing style conventions
- New features include appropriate validation
- Error messages are clear and actionable
- Documentation is updated accordingly

## License

[Specify your license here]

## References

- ITU-T Recommendation T.35 - Procedure for allocation of ITU-T defined codes
- SMPTE ST 2094-50 - Dynamic Metadata for Color Volume Transform
- ISO/IEC 14496-12 - ISO Base Media File Format

## Support

For issues, questions, or contributions, please [specify contact method or issue tracker].

---

**Version**: 2.0  
**Last Updated**: January 15, 2026
