# ISOBMFF

This repository is the official repository for the ISO Base Media File Format
Reference Software. 

The ISO base media file format is published by ISO as part 12 of the MPEG-4
specifications, ISO/IEC 14496-12. As such, it implements and conforms to
part of MPEG-4. This part of MPEG-4 is used heavily by standards other than
MPEG-4, and this reference software is often used by the reference software
for those other standards, but still provides, in those contexts, an
implementation "claiming conformance to MPEG-4".

Updates to the reference software can be submitted using Pull Requests but
are subject to approval by MPEG, and a formal input contribution should be
submitted to MPEG.

When possible, it is preferred that separate Pull Requests for
fixes/enhancements to the build system and for fixes/enhancements to the
software features.

## Development

The repository contains the libisomediafile which is a library implementing
the ISO base media file format. In addition, several tools to read and
write files based on this specification are provided.

### Requirements

- [CMake](https://cmake.org/)
- [git](https://git-scm.com/)

### Compiling

Example of commands to build the entire toolset on a Linux platform.

```
git clone https://github.com/MPEGGroup/isobmff.git

cd isobmff

mkdir build

cd build

cmake ..

make
```

#### Cross platform

CMake allows to generate build scripts for different platforms. For instance:

```
cmake -G "Visual Studio 16 2019" -A ARM64
```

For more generator, please see [CMake documentation](https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html).

Note also that certain IDE may be able to natively parse a `CMakeLists.txt`
in which case there is no need to generate specific build scripts. Please refer to your IDE's
documentation on how to best handle CMake-based projects.

#### Individual compilation

If you are only interested in certain tools, you can build them individually.

For instance, the libisomediafile can be built using `make libisomediafile`
when using Unix Makefile.

For a complete list, please refer to the generated build scripts, for instance
with Unix Makefile:

```
$ make help
The following are some of the valid targets for this Makefile:
... all (the default if no target is provided)
... clean
... depend
... rebuild_cache
... edit_cache
... libuniDrcBitstreamDecoderLib
... libwavIO
... libreadonlybitbuf
... libwriteonlybitbuf
... TLibDecoder
... TLibCommon
... libisomediafile
... makeAudioMovieSample
... playAudioMovieSample
... DRC_to_MP4
... MP4_to_DRC
... hevc_muxer
... hevc_demuxer
... hevc_extractors
... protectAudioMovie
... libisoiff
... isoiff_tool
... WAV_to_MP4
... MP4_to_WAV
```

