/**
 * @file MP4Movies.h
 * @brief API
 * @version 0.1
 *
 * @copyright This header file may be freely copied and distributed.
 *
 */

#ifndef INCLUDED_MP4MOVIE_H
#define INCLUDED_MP4MOVIE_H

#include "MP4OSMacros.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ISMACrypt

  /**
   * \defgroup Types Data Types
   *
   * Data types used within the API
   * @{
   */

  /**
   * @brief This is a typedef for function error codes
   *
   * The general rule is that zero is used to indicate success, and negative numbers are errors.
   */
  typedef int MP4Err;

  /**
   * @brief Function error codes
   * @see MP4Err
   */
  enum
  {
    /* JLF 12/00 : support for OD, returned by MP4GetInline... and MP4GetProfiles... */
    MP4HasRootOD                    = 2,    /**< Has root object descriptor */
    MP4EOF                          = 1,    /**< End of file */
    MP4NoErr                        = 0,    /**< Success */
    MP4FileNotFoundErr              = -1,   /**< No such file or directory */
    MP4BadParamErr                  = -2,   /**< Wrong parameter */
    MP4NoMemoryErr                  = -3,   /**< No memory */
    MP4IOErr                        = -4,   /**< I/0 Error */
    MP4NoLargeAtomSupportErr        = -5,   /**< No support for large atoms */
    MP4BadDataErr                   = -6,   /**< Bad data */
    MP4VersionNotSupportedErr       = -7,   /**< Version not supported */
    MP4InvalidMediaErr              = -8,   /**< Invalid media */
    MP4InternalErr                  = -9,   /**< Iternal error */
    MP4NotFoundErr                  = -10,  /**< Not found */
    MP4DataEntryTypeNotSupportedErr = -100, /**< Data entity type not supported */
    MP4NoQTAtomErr                  = -500, /**< No QT atom */
    MP4NotImplementedErr            = -1000 /**< Not implemented */
  };

  /**
   * @brief Modes used by MP4OpenMovieFile()
   */
  enum
  {
    MP4OpenMovieNormal  = 0,        /**< Open in normal mode */
    MP4OpenMovieDebug   = (1 << 0), /**< Open in debug mode, prints the atom tree to stdout */
    MP4OpenMovieInPlace = (1 << 1)  /**< Open in place, discards mdat box */
  };

  /**
   * @brief Track flags
   * @see MP4NewMovieTrack
   */
  enum
  {
    MP4NewTrackIsVisual     = (1 << 1), /**< track contains visual media */
    MP4NewTrackIsAudio      = (1 << 2), /**< track contains audio media */
    MP4NewTrackIsMetadata   = (1 << 3), /**< track contains meta-data */
    MP4NewTrackIsVolumetric = (1 << 4), /**< track contains volumetric media */
    MP4NewTrackIsHaptic     = (1 << 5), /**< track contains haptic media */
    MP4NewTrackIsMebx       = (1 << 6), /**< track contains boxed meta-data */
    MP4NewTrackIsPrivate    = (1 << 8)  /**< track contains a media type unknown to MPEG-4 */
  };

  enum
  {
    MP4KeyNamespace_uiso = MP4_FOUR_CHAR_CODE('u', 'i', 's', 'o'),
    MP4KeyNamespace_me4c = MP4_FOUR_CHAR_CODE('m', 'e', '4', 'c'),
    QTKeyNamespace_mdta  = MP4_FOUR_CHAR_CODE('m', 'd', 't', 'a'),
    QTKeyNamespace_udta  = MP4_FOUR_CHAR_CODE('u', 'd', 't', 'a')
  };

  /**
   * @brief Handler types
   */
  enum
  {
    MP4ObjectDescriptorHandlerType = MP4_FOUR_CHAR_CODE('o', 'd', 's', 'm'),
    MP4ClockReferenceHandlerType   = MP4_FOUR_CHAR_CODE('c', 'r', 's', 'm'),
    MP4SceneDescriptionHandlerType = MP4_FOUR_CHAR_CODE('s', 'd', 's', 'm'),
    MP4VisualHandlerType           = MP4_FOUR_CHAR_CODE('v', 'i', 'd', 'e'),
    MP4AudioHandlerType            = MP4_FOUR_CHAR_CODE('s', 'o', 'u', 'n'),
    MP4MPEG7HandlerType            = MP4_FOUR_CHAR_CODE('m', '7', 's', 'm'),
    MP4OCIHandlerType              = MP4_FOUR_CHAR_CODE('o', 'c', 's', 'm'),
    MP4IPMPHandlerType             = MP4_FOUR_CHAR_CODE('i', 'p', 's', 'm'),
    MP4MPEGJHandlerType            = MP4_FOUR_CHAR_CODE('m', 'j', 's', 'm'),
    MP4HintHandlerType             = MP4_FOUR_CHAR_CODE('h', 'i', 'n', 't'),
    MP4TextHandlerType             = MP4_FOUR_CHAR_CODE('t', 'e', 'x', 't'),
    MP7TextHandlerType             = MP4_FOUR_CHAR_CODE('m', 'p', '7', 't'),
    MP7BinaryHandlerType           = MP4_FOUR_CHAR_CODE('m', 'p', '7', 'b'),
    MP21HandlerType                = MP4_FOUR_CHAR_CODE('m', 'p', '2', '1'),
    MP4NullHandlerType             = MP4_FOUR_CHAR_CODE('n', 'u', 'l', 'l'),
    MP4MetaHandlerType             = MP4_FOUR_CHAR_CODE('m', 'e', 't', 'a'),
    MP4VolumetricHandlerType       = MP4_FOUR_CHAR_CODE('v', 'o', 'l', 'v'),
    MP4HapticHandlerType           = MP4_FOUR_CHAR_CODE('h', 'a', 'p', 't'),

    ISOXMLAtomType       = MP4_FOUR_CHAR_CODE('x', 'm', 'l', ' '),
    ISOBinaryXMLAtomType = MP4_FOUR_CHAR_CODE('b', 'x', 'm', 'l')
  };

  enum
  {
    MP4IPMP_NoControlPoint             = 0x00,
    MP4IPMP_DB_Decoder_ControlPoint    = 0x01,
    MP4IPMP_Decoder_CB_ControlPoint    = 0x02,
    MP4IPMP_CB_Compositor_ControlPoint = 0x03,
    MP4IPMP_BIFSTree_ControlPoint      = 0x04
  };

  /**
   * @brief Sample dependency types
   * @see ISOSetSampleDependency, ISOGetSampleDependency
   */
  enum
  {
    is_leading_dependency = 0x40,    /**< this sample is a leading sample that has a dependency
    before the referenced I-picture (and is therefore not decodable) */
    is_no_leading            = 0x80, /**< this sample is not a leading sample */
    is_leading_no_dependency = 0xC0, /**< this sample is a leading sample that has no dependency
    before the referenced I-picture (and is therefore decodable) */
    does_depend_on     = 0x10,       /**< this sample does depend on others (not an I picture) */
    does_not_depend_on = 0x20,       /**< this sample does not depend on others (I picture) */
    is_depended_on     = 0x4,        /**< other samples may depend on this one (not disposable) */
    is_not_depended_on = 0x8,        /**< no other sample depends on this one (disposable) */
    has_redundancy     = 1,          /**< there is redundant coding in this sample */
    has_no_redundancy  = 2           /**< there is no redundant coding in this sample */
  };

  /**
   * @brief Sample grouping types
   * @see ISOSetSamplestoGroupType()
   */
  typedef enum sampleToGroupType_t
  {
    SAMPLE_GROUP_NORMAL = 0, /**< Use normal sample groups 'sbgp' */
    SAMPLE_GROUP_COMPACT,    /**< Use compact sample groups 'csgp' */
    SAMPLE_GROUP_AUTO        /**< automatically decide based on atom size */
  } sampleToGroupType_t;
/**
 * @brief Get the MP4PrivateMovieRecordPtr moov
 *
 * Declare MP4Err and bail on error.
 * @param arg ISOMovie or MP4Movie object
 */
#define GETMOOV(arg)                            \
  MP4PrivateMovieRecordPtr moov;                \
  MP4Err err;                                   \
  err = MP4NoErr;                               \
  if(arg == NULL) BAILWITHERROR(MP4BadParamErr) \
  moov = (MP4PrivateMovieRecordPtr)arg
/**
 * @brief Get the MP4MovieAtomPtr movieAtom
 * @param arg ISOMovie or MP4Movie object
 */
#define GETMOVIEATOM(arg)    \
  MP4MovieAtomPtr movieAtom; \
  GETMOOV(arg);              \
  movieAtom = (MP4MovieAtomPtr)moov->moovAtomPtr
/**
 * @brief Get the MP4MovieHeaderAtomPtr movieHeaderAtom
 * @param arg ISOMovie or MP4Movie object
 */
#define GETMOVIEHEADERATOM(arg)          \
  MP4MovieHeaderAtomPtr movieHeaderAtom; \
  GETMOVIEATOM(arg);                     \
  movieHeaderAtom = (MP4MovieHeaderAtomPtr)movieAtom->mvhd
/**
 * @brief Get the MP4ObjectDescriptorAtomPtr iodAtom
 * @param arg ISOMovie or MP4Movie object
 */
#define GETIODATOM(arg)               \
  MP4ObjectDescriptorAtomPtr iodAtom; \
  GETMOVIEATOM(arg);                  \
  iodAtom = (MP4ObjectDescriptorAtomPtr)movieAtom->iods;

  struct MP4MovieRecord
  {
    void *data;
  };
  typedef struct MP4MovieRecord MP4MovieRecord;

  /**
   * @brief This is an opaque handle that contains a reference to a movie.
   */
  typedef MP4MovieRecord *MP4Movie;

  struct MP4TrackRecord
  {
    void *data;
  };
  typedef struct MP4TrackRecord MP4TrackRecord;
  /**
   * @brief This is an opaque handle that contains a reference to a track.
   */
  typedef MP4TrackRecord *MP4Track;

  struct MP4MediaRecord
  {
    void *data;
  };
  typedef struct MP4MediaRecord MP4MediaRecord;
  /**
   * @brief This is an opaque handle that contains a reference to media.
   */
  typedef MP4MediaRecord *MP4Media;

  struct MP4TrackReaderRecord
  {
    void *data;
  };
  typedef struct MP4TrackReaderRecord MP4TrackReaderRecord;
  /**
   * @brief This is an opaque handle that contains a reference to a track reader.
   */
  typedef MP4TrackReaderRecord *MP4TrackReader;

  struct MP4UserDataRecord
  {
    void *data;
  };
  typedef struct MP4UserDataRecord MP4UserDataRecord;
  /**
   * @brief An opaque handle that references user data.
   */
  typedef MP4UserDataRecord *MP4UserData;

  struct MP4SLConfigRecord
  {
    void *data;
  };
  typedef struct MP4SLConfigRecord MP4SLConfigRecord;
  typedef MP4SLConfigRecord *MP4SLConfig;

  struct MP4GenericAtomRecord
  {
    void *data;
  };
  typedef struct MP4GenericAtomRecord MP4GenericAtomRecord;
  typedef MP4GenericAtomRecord *MP4GenericAtom;

#ifdef PRAGMA_EXPORT
#pragma export on
#endif

  /**
   * @brief MP4Handle is used to pass sections of dynamically allocated memory to the API.
   *
   * Handles have an internal structure that remember their allocated size, and the indicated size
   * of the data stored in the Handle. It is common to create a zero-length Handle and pass it to a
   * function that places data into it, resizing it appropriately. You can treat handles as (char**)
   * as long as you use the API functions to allocate their memory and change their size.
   */
  typedef char **MP4Handle;

  /**
   * @}
   * \defgroup Utility Utility functions
   *
   * Utility functions which help to operate with data.
   * @{
   */

  /**
   * @brief Creates a new handle, and allocates handleSize bytes for it.
   *
   * It is OK to call this with a size of zero. This is commonly done when the handle will be used
   * as a parameter to a function that will size it appropriately.
   *
   * @param handleSize number of bytes to allocate
   * @param outHandle output handle
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4NewHandle(u32 handleSize, MP4Handle *outHandle);
  /**
   * @brief Sets the logical size of the handle to requestedSize bytes. If this is larger than the
   * number of bytes allocated for this handle, the handle will be grown accordingly. If the new
   * size is smaller than the allocated size the memory is not freed. The only way to free this
   * memory is to dispose of the handle.
   *
   * @param theHandle input handle
   * @param requestedSize new size in bytes
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4SetHandleSize(MP4Handle theHandle, u32 handleSize);
  /**
   * @brief Frees the memory that was allocated for a handle.
   *
   * @param theHandle input handle to kill
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4DisposeHandle(MP4Handle theHandle);
  /**
   * @brief Use this to determine the present logical size (in bytes) of a handle.
   *
   * @param theHandle input handle
   * @param outSize output size in bytes
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetHandleSize(MP4Handle theHandle, u32 *outSize);
  /**
   * @brief Appends the data contained in theSrcHandle to data contained in theDstHandle by
   * reallocating, if necessary, the number of bytes allocated for theDstHandle.
   *
   * @param theDstHandle destination handle
   * @param theSrcHandle source handle
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4HandleCat(MP4Handle theDstHandle, MP4Handle theSrcHandle); /* FB_RESO 09/02 */
  /**
   * @brief Sets the handle so that subsequent de-references of it refer to the data starting at the
   * given byte offset. This can be used to cause data to be read from or written to locations after
   * the beginning, leaving room (for example) for an encryption header.
   *
   * @param theHandle input handle
   * @param offset byte offset
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4SetHandleOffset(MP4Handle theHandle, u32 offset);

  /**
   * @}
   * \defgroup Movie Movie related functions
   *
   * Functions to operate with ISOMovie/MP4Movie objects.
   * @{
   */

  /**
   * @brief This function releases any resources owned by the Movie.
   * @note You should not make any further reference to the Movie after calling this.
   *
   * @param theMovie Movie object to kill
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4DisposeMovie(MP4Movie theMovie);

  /**
   * @brief This calculates and returns the movie duration as recorded in the movie header.
   *
   * It is the maximum track duration, expressed in units of the movie’s timescale.
   * Get the timescale using the MP4GetMovieTimeScale().
   *
   * @param theMovie input movie object
   * @param outDuration output duration in movie's timescale.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMovieDuration(MP4Movie theMovie, u64 *outDuration);

  /**
   * @brief Retrieves the initial object descriptor from a Movie and places it into a Handle.
   *
   * @param theMovie input movie object
   * @param outDescriptorH output handle with the data
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMovieInitialObjectDescriptor(MP4Movie theMovie, MP4Handle outDescriptorH);

  /**
   * @brief Retrieves the initial object descriptor from a Movie.
   *
   * Similar to MP4GetMovieInitialObjectDescriptor() but with additional MP4SLConfig parameter.
   *
   * @param theMovie input movie object
   * @param slconfig input MP4SLConfig configuration object
   * @param outDescriptorH output handle with the data
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMovieInitialObjectDescriptorUsingSLConfig(MP4Movie theMovie, MP4SLConfig slconfig,
                                                  MP4Handle outDescriptorH);

  /**
   * @brief Queries the setting of includeInlineProfileLevelFlag from the movie's initial object
   * descriptor.
   *
   * @param theMovie input movie object
   * @param outFlag output flags
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMovieIODInlineProfileFlag(MP4Movie theMovie, u8 *outFlag);

  /**
   * @brief Use this to obtain the profiles and levels from a Movie's initial object descriptor.
   *
   * You may pass in null for parameters that don't care to read.
   *
   * @note This call will return an error if the movie does not contain an initial object descriptor
   *
   * @param theMovie input movie object
   * @param outOD output object descriptor profile and level
   * @param outScene output scene profile and level
   * @param outAudio output audio profile and level
   * @param outVisual output visual profile and level
   * @param outGraphics output graphics profile and level
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMovieProfilesAndLevels(MP4Movie theMovie, u8 *outOD, u8 *outScene, u8 *outAudio,
                               u8 *outVisual, u8 *outGraphics);

  /**
   * @brief Use this to obtain the time scale of a Movie.
   *
   * @param theMovie input movie object
   * @param outTimeScale output value for timescale
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMovieTimeScale(MP4Movie theMovie, u32 *outTimeScale);

  /**
   * @brief Use this to obtain a track given its track ID.
   *
   * @param theMovie input movie object
   * @param trackID track ID
   * @param outTrack output track object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMovieTrack(MP4Movie theMovie, u32 trackID, MP4Track *outTrack);

  /**
   * @brief Get the movie level user data 'udta'.
   * @ingroup UserData
   * @note If no 'udta' is found, the default one is created in theMovie and returned.
   *
   * @param theMovie input movie object
   * @param outUserData output pointer to user data
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMovieUserData(MP4Movie theMovie, MP4UserData *outUserData);

  /**
   * @brief Adds the given atom to the movie.
   *
   * @see MP4NewForeignAtom, MP4NewUUIDAtom
   *
   * @param theMovie input movie object
   * @param the_atom new atom to add to the movie object
   */
  MP4_EXTERN(MP4Err) MP4AddAtomToMovie(MP4Movie theMovie, MP4GenericAtom the_atom);

  /**
   * @brief Creates a new empty Movie in memory.
   *
   * You may assign an initial object descriptor ID and profile and level indications that describe
   * the presentation you are making. The profile and level definitions are detailed in the MPEG-4
   * Systems specification.
   *
   * If you provide zero as the initialODID, then no object descriptor is created. Such files can be
   * used as the target of an ES URL, or standalone. Similarly if you provide zero for all the
   * profiles, then the IOD atom will contain an object descriptor, not an initial object
   * descriptor; these files can also be useful on occasion.
   *
   * @attention Do not attempt to add tracks to the object descriptor if you have asked that one not
   * be created.
   *
   * @param outMovie output movie object
   * @param initialODID initial object descriptor. If 0, then no object descriptor is created.
   * @param OD_profileAndLevel Object descriptor profile and level
   * @param scene_profileAndLevel scene profile and level
   * @param audio_profileAndLevel audio profile and level
   * @param visual_profileAndLevel visual profile and level
   * @param graphics_profileAndLevel graphics profile and level
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4NewMovie(MP4Movie *outMovie, u32 initialODID, u8 OD_profileAndLevel, u8 scene_profileAndLevel,
              u8 audio_profileAndLevel, u8 visual_profileAndLevel, u8 graphics_profileAndLevel);

  /**
   * @brief Parses a movie ‘file’ in the handle and creates a Movie in memory from this file.
   *
   * @param outMovie output movie object
   * @param movieH input movie handle
   * @param newMovieFlags is ordinarily set to MP4OpenMovieNormal, but if you set it to
   * MP4OpenMovieDebug a parse tree will be printed to standard output while the movie is parsed
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4NewMovieFromHandle(MP4Movie *outMovie, MP4Handle movieH, u32 newMovieFlags);

  /**
   * @brief Creates a new empty MPEG-21 ‘movie’ in memory
   *
   * Actually it creates an MPEG-21 file with a meta-atom but no movie atom. You will probably want
   * to get the meta-atom reference ISOGetFileMeta to add items etc.
   *
   * @note If you want a dual-function MP4/MPEG-21 file, create an MP4 file and then add a
   * file-level meta atom to that, instead of using this function.
   *
   * @param outMovie output movie object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) NewMPEG21(MP4Movie *outMovie);

  /**
   * @brief Like NewMPEG21() but allows you to specify the meta handlertype, and the major brand and
   * minor version of that brand.
   *
   * @param outMovie output movie object
   * @param handlertype Handler type
   * @param brand Brand
   * @param minorversion Minor version
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISONewMetaMovie(MP4Movie *outMovie, u32 handlertype, u32 brand, u32 minorversion);

  /**
   * @brief Indicates that the given track is to be considered as the initial BIFS track.
   *
   * This information is placed into the Movie's initial object descriptor.
   *
   * @param theMovie
   * @param theBIFSTrack
   * @return MP4Err error code
   */
  /* MP4_EXTERN(MP4Err) MP4SetMovieInitialBIFSTrack( MP4Movie theMovie, MP4Track theBIFSTrack ); */

  /**
   * @brief Indicates that the given track is to be considered as the initial Object Descriptor
   * stream track.
   *
   * This information is placed into the Movie's initial object descriptor.
   *
   * @param theMovie
   * @param theODTrack
   * @return MP4Err error code
   */
  /* MP4_EXTERN(MP4Err) MP4SetMovieInitialODTrack( MP4Movie theMovie, MP4Track theODTrack ); */

  /**
   * @brief Sets the includeInlineProfileLevelFlag in the movie's initial object descriptor.
   *
   * @param theMovie input movie object
   * @param theFlag profile and level flag
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4SetMovieIODInlineProfileFlag(MP4Movie theMovie, u8 theFlag);

  /**
   * @brief Sets the Movie’s time scale.
   *
   * @param theMovie input movie object
   * @param timeScale timescale value
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4SetMovieTimeScale(MP4Movie theMovie, u32 timeScale);

  /* Dealing with Movie files */

  /**
   * @brief Opens a movie file and creates a Movie in memory from this file.
   *
   * @param theMovie output movie object
   * @param movieURL pathname or a file URL.
   * @param openMovieFlags is ordinarily set to MP4OpenMovieNormal, but if you set it to
   * MP4OpenMovieDebug a parse tree will be printed to standard output while the movie is parsed
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4OpenMovieFile(MP4Movie *theMovie, const char *movieURL, int openMovieFlags);

  /**
   * @brief Places the movie and its media samples into a handle.
   *
   * @param theMovie input movie object
   * @param movieH output handle with the data from theMovie
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4PutMovieIntoHandle(MP4Movie theMovie, MP4Handle movieH);

  /**
   * @brief See ISOWriteMovieToFile()
   */
  MP4_EXTERN(MP4Err) MP4WriteMovieToFile(MP4Movie theMovie, const char *filename);

  /**
   * @}
   * \defgroup Track Track related functions
   *
   * Functions to operate with ISOTrack/MP4Track objects.
   * @{
   */

  /**
   * @brief Track reference types
   */
  enum
  {
    MP4HintTrackReferenceType = MP4_FOUR_CHAR_CODE('h', 'i', 'n', 't'),        /**< for hint tracks,
    to point to the media track they depend on */
    MP4StreamDependencyReferenceType = MP4_FOUR_CHAR_CODE('d', 'p', 'n', 'd'), /**< for elementary
    stream tracks, to indicate other elementary stream tracks they depend on (an enhancement layer
    points to a base layer, for example) */
    MP4ODTrackReferenceType = MP4_FOUR_CHAR_CODE('m', 'p', 'o', 'd'),          /**< for object
    descriptor tracks, to point to the elementary stream whose metadata is being updated. */
    /* JLF 12/00: added "sync" type for OCR_ES_ID (was broken before) */
    MP4SyncTrackReferenceType = MP4_FOUR_CHAR_CODE('s', 'y', 'n', 'c'),
    MP4DescTrackReferenceType = MP4_FOUR_CHAR_CODE('c', 'd', 's', 'c')
  };

  /**
   * @brief Indicate that there exists a dependency between two tracks.
   *
   * @param theTrack track object to which the track reference will be added
   * @param dependsOn track object on which theTrack depends
   * @param referenceType refenrece type fourcc. Could be one of the following values, or any other
   * reference type set with MP4_FOUR_CHAR_CODE():
   *
   * @param outReferenceIndex output reference number. This is only useful for OD streams because
   * the other reference types only allow one reference per track.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4AddTrackReference(MP4Track theTrack, MP4Track dependsOn, u32 referenceType,
                       u32 *outReferenceIndex);

  /**
   * @brief Indicate that there exists a dependency between two tracks.
   *
   * Similar to MP4AddTrackReference() but allows you to set the reference using a track ID
   * rather than a track.
   *
   * @param theTrack track object to which the track reference will be added
   * @param dependsOnID track ID on which theTrack depends
   * @param dependencyType same as in MP4AddTrackReference()
   * @param outReferenceIndex output reference number
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4AddTrackReferenceWithID(MP4Track theTrack, u32 dependsOnID, u32 dependencyType,
                             u32 *outReferenceIndex);
  /**
   * @brief
   *
   * @param theTrack input track object
   * @param subs
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4AddSubSampleInformationToTrack(MP4Track theTrack, MP4GenericAtom *subs);
  /**
   * @brief
   *
   * @param subsample
   * @param flags
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4SetSubSampleInformationFlags(MP4GenericAtom subsample, u32 flags);
  /**
   * @brief
   *
   * @param theTrack input track object
   * @param flags
   * @param entry_count
   * @param sample_delta
   * @param subsample_count
   * @param subsample_size_array
   * @param subsample_priority_array
   * @param subsample_discardable_array
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetSubSampleInformationEntryFromTrack(MP4Track theTrack, u32 *flags, u32 *entry_count,
                                           u32 **sample_delta, u32 **subsample_count,
                                           u32 ***subsample_size_array,
                                           u32 ***subsample_priority_array,
                                           u32 ***subsample_discardable_array);
  /**
   * @brief
   *
   * @param subsample
   * @param sample_delta
   * @param subsample_count
   * @param subsample_size_array
   * @param subsample_priority_array
   * @param subsample_discardable_array
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4AddSubSampleInformationEntry(MP4GenericAtom subsample, u32 sample_delta, u32 subsample_count,
                                  MP4Handle subsample_size_array,
                                  MP4Handle subsample_priority_array,
                                  MP4Handle subsample_discardable_array);
  /**
   * @brief Same as MP4AddSubSampleInformationEntry but also allows to add the codec specific
   * parameters
   *
   * @param subsample
   * @param sample_delta
   * @param subsample_count
   * @param subsample_size_array
   * @param subsample_priority_array
   * @param subsample_discardable_array
   * @param codec_specific_parameters_array
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4AddSubSampleInformationEntry2(MP4GenericAtom subsample, u32 sample_delta, u32 subsample_count,
                                   MP4Handle subsample_size_array,
                                   MP4Handle subsample_priority_array,
                                   MP4Handle subsample_discardable_array,
                                   MP4Handle codec_specific_parameters_array);
  /**
   * @brief Add track to a track group ID.
   *
   * If no track group with the dependency type is found in the track one is created and added to
   * theTrack.
   *
   * @param theTrack input track object
   * @param groupID track_group_id in the TrackGroupTypeBox
   * @param dependencyType track_group_type of the TrackGroupTypeBox
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4AddTrackGroup(MP4Track theTrack, u32 groupID, u32 dependencyType);
  /**
   * @brief Add track's ES_Descriptor to its movie's initial object descriptor.
   *
   * This should be called for any tracks that need to appear in the initial object descriptor other
   * than the initial BIFS track or the OD track.
   *
   * @param theTrack input track object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4AddTrackToMovieIOD(MP4Track theTrack);
  /**
   * @brief Get track using the track index.
   *
   * Use this to sequence through tracks in the Movie without regard to the trackID.
   *
   * @param theMovie input movie object
   * @param trackIndex index of the track ranges between 1 and the number of tracks in theMovie.
   * @param outTrack output track object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMovieIndTrack(MP4Movie theMovie, u32 trackIndex, MP4Track *outTrack);
  /**
   * @brief Get sample entry type of a track.
   *
   * @note This function only returns the first sample entry type.
   * @param theMovie input movie object
   * @param idx index of the track ranges between 1 and the number of tracks in theMovie.
   * @param SEType [out] sample entry type (4CC)
   */
  MP4_EXTERN(MP4Err) MP4GetMovieIndTrackSampleEntryType(MP4Movie theMovie, u32 idx, u32 *SEType);

  /*
  MP4_EXTERN ( MP4Err )
  MP4GetMovieInitialBIFSTrack( MP4Movie theMovie, MP4Track *outBIFSTrack );
  */

  /**
   * @brief This function allows you to determine the number of Tracks in a Movie.
   *
   * @param theMovie input movie object
   * @param outTrackCount output number of tracks.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMovieTrackCount(MP4Movie theMovie, u32 *outTrackCount);

  /**
   * @brief Calculates and returns the duration of the track.
   *
   * If there is an edit list, it is the sum of the durations of the edits. In the absence of an
   * edit list, it is the track’s cumulative sample durations.
   *
   * @param theTrack input track object
   * @param outDuration output duration of the track in the movie's timescale.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackDuration(MP4Track theTrack, u64 *outDuration);

  /**
   * @brief This returns a non-zero value in outEnabled if the track is enabled.
   *
   * @param theTrack input track object
   * @param outEnabled output enabled flag is a non-zero value if the track is enabled
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackEnabled(MP4Track theTrack, u32 *outEnabled);

  /**
   * @brief Get the elementary stream ID for a given Track.
   *
   * @param theTrack input track object
   * @param outTrackID output track ID
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackID(MP4Track theTrack, u32 *outTrackID);

  /**
   * @brief Returns the Media for a given Track. An error is returned if the track contains no media
   *
   * @param theTrack input track object
   * @param outMedia output pointer to a track media object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackMedia(MP4Track theTrack, MP4Media *outMedia);

  /**
   * @brief Get the Movie associated with a Track.
   *
   * @param theTrack input track object
   * @param outMovie output pointer to a movie object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackMovie(MP4Track theTrack, MP4Movie *outMovie);

  /**
   * @brief Get the track's offset (the length of its initial empty edit).
   *
   * @param track input track object
   * @param outMovieOffsetTime output movie offset value in the Movie's time scale.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackOffset(MP4Track track, u32 *outMovieOffsetTime);

  /**
   * @brief Obtain a specific track reference of the specified type.
   *
   * @param theTrack input track object
   * @param referenceType The type of track reference you wish to index
   * @param referenceIndex The particular reference of this type that you wish to retrieve.
   * This is usually one, since most track references only allow one entry.
   * @param outReferencedTrack output pointer to a track object that is referenced.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetTrackReference(MP4Track theTrack, u32 referenceType, u32 referenceIndex,
                       MP4Track *outReferencedTrack);

  /**
   * @brief Determine the number of a particular type of track references that are contained in a
   * track.
   *
   * @param theTrack input track object
   * @param referenceType input reference type
   * @param outReferenceCount number of track references of a specific type. If the track doesn't
   * contain any references of the specified type the return value will be zero.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetTrackReferenceCount(MP4Track theTrack, u32 referenceType, u32 *outReferenceCount);

  /**
   * @brief Get the track group of a track
   *
   * @param theTrack input track object
   * @param groupType group type
   * @param outGroupId output group ID
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackGroup(MP4Track theTrack, u32 groupType, u32 *outGroupId);

  /**
   * @brief Get the track level user data 'udta'.
   * @ingroup UserData
   * @note If no 'udta' is found, the default one is created in theTrack and returned.
   *
   * @param theTrack input track object
   * @param outUserData output pointer to user data
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackUserData(MP4Track theTrack, MP4UserData *outUserData);

  /**
   * @brief Adds the given atom to the track.
   *
   * @see MP4NewForeignAtom, MP4NewUUIDAtom
   *
   * @param theTrack input track object
   * @param the_atom atom to add
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4AddAtomToTrack(MP4Track theTrack, MP4GenericAtom the_atom);

  /**
   * @brief Adds a reference to the specified segment of media into a Track.
   *
   * @note This may create an edit list entry for this track.
   *
   * @param trak input track object
   * @param trackStartTime time (in the movie’s time scale) at which the media is to be inserted in
   * to the track
   * @param mediaStartTime time (in the media’s time scale) at which the desired media segment
   * begins
   * @param segmentDuration duration (in the media’s time scale) of the segment
   * @param mediaRate The desired playback rate of the media. A value of 1 indicates normal rate.
   * The only other permitted value is -1 which indicates a ’dwell’ should occur.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4InsertMediaIntoTrack(MP4Track trak, s32 trackStartTime, s32 mediaStartTime,
                          u64 segmentDuration, s32 mediaRate);

  /**
   * @brief Creates a new track for the movie.
   *
   * @param theMovie input movie object
   * @param newTrackFlags track flags can be zero or one of:
   * Flags value            | Description
   * -----------------------|-----------------------------------------------------------
   * ISONewTrackIsVisual    | if the track will contain visual media
   * ISONewTrackIsAudio     | if the track will contain audio media
   * ISONewTrackIsPrivate   | if the track will not contain a media type known to MPEG-4
   * ISONewTrackIsMetadata  | if the track will be a meta-data track
   * @param outTrack output track object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4NewMovieTrack(MP4Movie theMovie, u32 newTrackFlags, MP4Track *outTrack);

  /**
   * @brief Creates a new track for the movie with a specified track ID.
   *
   * @param theMovie input movie object
   * @param newTrackFlags track flags as in MP4NewMovieTrack()
   * @param newTrackID input track ID to set for a new track
   * @param outTrack output track object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4NewMovieTrackWithID(MP4Movie theMovie, u32 newTrackFlags, u32 newTrackID, MP4Track *outTrack);

  /**
   * @brief Creates the media container for a track.
   *
   * @param theTrack input track object
   * @param outMedia output pointer to a media object
   * @param handlerType Describes the stream type at a high level.
   * Use one of the handler types from ISOMovies.h. Examples are ISOVisualHandlerType,
   * ISOAudioHandlerType and ISOHintHandlerType.
   * @param timeScale The media time scale. This is the time coordinate system for the media.
   * You express durations as a multiple of this value, so typical values are the sampling rate for
   * audio, or frame rate for video.
   * @param dataReference A URL that indicates the location of the media samples. This must either
   * be NULL to indicate that the samples are included in the movie’s file, or a "file://" URL
   * encoded in UTF-8. This data reference is given index 1 in the Media. You can add others using
   * MP4AddMediaDataReference()
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4NewTrackMedia(MP4Track theTrack, MP4Media *outMedia, u32 handlerType, u32 timeScale,
                   MP4Handle dataReference);

  /**
   * @brief Enables or disables the track.
   *
   * @param theTrack input track object
   * @param enabled Set enabled to a nonzero value to enable the track, or zero to disable the track
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4SetTrackEnabled(MP4Track theTrack, u32 enabled);

  /**
   * @brief Modifies the duration of the empty space that lies at the beginning of the track,
   * thus changing the duration of the entire track.
   *
   * @note Call this function after you have constructed your track - it relies on being able to
   * find the duration of the media in the track.
   *
   * @param track input track object
   * @param movieOffsetTime time offset as a time value in the movie’s time scale
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4SetTrackOffset(MP4Track track, u32 movieOffsetTime);

  /**
   * @brief Convert from a time expressed in the movie time scale to a time expressed in the media
   * time scale.
   *
   * @param theTrack input track object
   * @param inTrackTime input track timestamp
   * @param outMediaTime output media timestamp (in the media time scale)
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4TrackTimeToMediaTime(MP4Track theTrack, u64 inTrackTime, s64 *outMediaTime);

  /**
   * @}
   * \defgroup Media Media related functions
   *
   * Functions to operate with ISOMedia/MP4Media objects.
   * @{
   */

  /**
   * @brief Use this to add a data reference to a media container.
   *
   * The data reference specifies a location for media samples. The outReferenceIndex parameter is
   * updated to contain the index for the new reference. (You will need to use this index to create
   * a sample description that will point to this new data reference.)
   *
   * @param theMedia input media object
   * @param outReferenceIndex output the index for the new reference
   * @param urlHandle UTF-8 encoded file:// URL that indicates the file containing the samples. Set
   * this to NULL to indicate that the samples are in the movie’s file.
   * @param urnHandle UTF-8 encoded URN. This may be used in addition to urlHandle to indicate the
   * uniform resource name associated with this media file. If no URN is used, set urnHandle to NULL
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4AddMediaDataReference(MP4Media theMedia, u32 *outReferenceIndex, MP4Handle urlHandle,
                           MP4Handle urnHandle);
  /**
   * @brief Use this function to add samples to a media by reference.
   *
   * This is usually used when the samples are already present in some file, for example when you
   * are making reference to samples contained in an external file.
   *
   * @param media input media object
   * @param dataOffset The byte offset into the file containing the samples. All samples added in
   * this function are assumed to be contiguous in the file starting at this offset.
   * @param sampleCount The number of samples to be added in this call.
   * @param durationsH A handle containing u32[] durations for each sample to be added. If the
   * durations differ you must supply a duration for each sample. If the durations are constant you
   * should only place one entry in this handle.
   * @param sizesH A handle containing u32[] sizes (in bytes) for each sample to be added. If the
   * sizes differ you must indicate a size for each sample. If the sizes are constant you should
   * only place one entry in this handle.
   * @param sampleEntryH Contains a sample entry that describes the samples to be added. For MPEG-4,
   * this entry can be created using MP4CreateSampleDescriptionAtom(), or for both MJ2 and MP4 you
   * can supply your own as long as it is formatted according to the appropriate standard.
   * If this handle is set to NULL the previous non-null sample entry will be used.
   * @param decodingOffsetsH If the media contains separate decoding and composition timestamps you
   * must use this handle to indicate the offset between the composition time and the decoding time
   * for each sample in a u32[]. Note that samples must be added in decoding order.
   * @param syncSamplesH If all the samples added are sync samples you can set this to NULL.
   * Otherwise, place the one-based indexes of each sync sample into this handle as a u32[]. If none
   * of the samples you are adding is a sync sample this handle will have a size of zero.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4AddMediaSampleReference(MP4Media media, u64 dataOffset, u32 sampleCount, MP4Handle durationsH,
                             MP4Handle sizesH, MP4Handle sampleEntryH, MP4Handle decodingOffsetsH,
                             MP4Handle syncSamplesH);
  /**
   * @brief Use this function to samples to the media.
   *
   * @param sampleH contains the data for all the samples
   * @note Other parameters are exactly the same as in MP4AddMediaSampleReference()
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4AddMediaSamples(MP4Media media, MP4Handle sampleH, u32 sampleCount, MP4Handle durationsH,
                     MP4Handle sizesH, MP4Handle sampleEntryH, MP4Handle decodingOffsetsH,
                     MP4Handle syncSamplesH);
  /**
   * @brief Add media samples by reference with padding bits
   *
   * @param padsH padding bits. May be NULL, indicating no pad, or may be a handle to an array of
   * padding values (which are 8 bits each). If the array has only one value, then it is used for
   * all the samples. Otherwise, the array should be sampleCount long.
   * @note Other parameters are exactly the same as in MP4AddMediaSampleReference()
   */
  MP4_EXTERN(MP4Err)
  MP4AddMediaSampleReferencePad(MP4Media media, u64 dataOffset, u32 sampleCount,
                                MP4Handle durationsH, MP4Handle sizesH, MP4Handle sampleEntryH,
                                MP4Handle decodingOffsetsH, MP4Handle syncSamplesH,
                                MP4Handle padsH);
  /**
   * @brief Add media samples with padding bits
   *
   * @param padsH padding bits as in MP4AddMediaSampleReferencePad()
   * @note Other parameters are exactly the same as in MP4AddMediaSampleReference()
   */
  MP4_EXTERN(MP4Err)
  MP4AddMediaSamplesPad(MP4Media media, MP4Handle sampleH, u32 sampleCount, MP4Handle durationsH,
                        MP4Handle sizesH, MP4Handle sampleEntryH, MP4Handle decodingOffsetsH,
                        MP4Handle syncSamplesH, MP4Handle padsH);
  /**
   * @brief Adds a Sample Group Description to the indicated media.
   *
   * @param media input media object
   * @param groupType grouping_type of the SampleGroupDescriptionBox
   * @param description SampleGroupDescriptionEntry pre-serialized (big-endian) group description
   * @param index output index of the added group
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOAddGroupDescription(MP4Media media, u32 groupType, MP4Handle description, u32 *index);
  /**
   * @brief Returns in the handle ‘description’ the group description associated with the given
   * group index of the given group type.
   *
   * @param media media object where to look for the specific group description
   * @param groupType grouping_type
   * @param index index starting with 1
   * @param description output handle holding the SampleGroupDescriptionEntry
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOGetGroupDescription(MP4Media media, u32 groupType, u32 index, MP4Handle description);
  /**
   * @brief Returns the entry_count of the desired SampleGroupDescriptionBox of the given type
   *
   * @param media media object where to look for the specific group description
   * @param groupType grouping_type of 'sgpd'
   * @param outEntryCount entry_count output
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOGetGroupDescriptionEntryCount(MP4Media media, u32 groupType, u32 *outEntryCount);
  /**
   * @brief Set the SampleToGroup mapping type
   *
   * @note Calling this function will also change the mapping type in the current movie or fragment.
   *
   * @param media media object
   * @param sampleToGroupType choose between SAMPLE_GROUP_NORMAL ('sbgp' default),
   * SAMPLE_GROUP_COMPACT ('csgp') or SAMPLE_GROUP_AUTO (decide automatically, smaller size wins)
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOSetSamplestoGroupType(MP4Media media, sampleToGroupType_t sampleToGroupType);
  /**
   * @brief Maps a set of samples to a group of a given type
   *
   * sample_index can also be negative. For example, a sample_index of -2 and a count of 2 sets the
   * mapping for the last two samples. If movie fragments are in use, the sample index is relative
   * to the current fragment (and cannot be outside it), not the whole movie.
   *
   * @param media input media object
   * @param groupType grouping_type of the sample group
   * @param group_index either 0 (to map samples to no group of this type) or an index value
   * obtained using ISOAddGroupDescription()
   * @param sample_index the first sample index to map. If it is zero or positive, it is the sample
   * at that offset from the first sample in the media. If it is negative, it is at that offset from
   * the last sample in the media.
   * @param count the number of samples for which to set the mapping
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOMapSamplestoGroup(MP4Media media, u32 groupType, u32 group_index, s32 sample_index, u32 count);
  /**
   * @brief Returns in group_index the group index associated with the given sample number of the
   * given group type.
   *
   * @param media input media object
   * @param groupType grouping_type of the sample group
   * @param sample_number input sample number
   * @param group_index output group description index
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOGetSampletoGroupMap(MP4Media media, u32 groupType, u32 sample_number, u32 *group_index);
  /**
   * @brief Get all sample numbers associated with the given group type and index of the description
   *
   * @param media input media object
   * @param groupType grouping_type of the sample group
   * @param group_index group description index
   * @param outSampleNubers output array containing all sample numbers
   * @param outSampleCnt output number of samples (size of outSampleNubers)
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOGetSampleGroupSampleNumbers(MP4Media media, u32 groupType, u32 group_index,
                                 u32 **outSampleNubers, u32 *outSampleCnt);
  /**
   * @brief Sets the sample dependency of a set of samples, after having added the samples.
   *
   * @note The number of samples for which to set the mapping is specified by the size of the
   * dependencies handle.
   *
   * @param media input media object
   * @param sample_index the first same index as in ISOMapSamplestoGroup()
   * @param dependencies The handle contains a set of one-byte values each made from OR-ing together
   * the at most one from each pair of the sample dependency constants (does_depend_on,
   * does_not_depend_on, is_depended_on, is_not_depended_on, has_redundancy, etc.)
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOSetSampleDependency(MP4Media media, s32 sample_index, MP4Handle dependencies);
  /**
   * @brief Returns in dependency the sample dependency associated with the given sample number.
   *
   * @param media input media object
   * @param sample_index index of the sample
   * @param dependency output dependency byte as defined in SampleDependencyTypeBox
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) ISOGetSampleDependency(MP4Media media, s32 sample_index, u8 *dependency);
  /**
   * @brief Use this function to prepare the media before adding samples.
   * @todo finish the implementation
   */
  MP4_EXTERN(MP4Err) MP4BeginMediaEdits(MP4Media theMedia);
  /**
   * @brief Tests all data references in the media to ensure that the references exist and are
   * readable.
   *
   * @param theMedia input media object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4CheckMediaDataReferences(MP4Media theMedia);
  /**
   * @brief Call this function when you have finished adding samples to the media.
   */
  MP4_EXTERN(MP4Err) MP4EndMediaEdits(MP4Media theMedia);
  /**
   * @brief Returns a sample with padding bits from the media, given the sample's index.
   *
   * @note If the track has no recorded padding table (no padding information) then the padding
   * value returned will be 0xF8. This is not a legal value for actual padding and should not be
   * supplied to calls which add samples.
   *
   * @param theMedia input media object
   * @param sampleNumber The index of the desired sample within its Media. This index runs from one
   * to the total number of samples in the Media.
   * @param outSample The data for the desired sample. This is the raw access unit without headers
   * @param outSize The size of the returned sample in bytes
   * @param outDTS The decoding time for the sample using the media time scale
   * @param outCTSOffset The composition time offset for the sample using the media time scale
   * @param outDuration The sample's duration in units of media time scale
   * @param outSampleFlags Flags that indicate the nature of this sample. Values are combinations of
   * MP4MediaSampleNotSync if the sample is not a sync sample and MP4MediaSampleHasCompositionOffset
   * if the sample's DTS differs from its CTS.
   * @param outSampleDescIndex The index of the sample description that corresponds to this sample
   * @param outPad output padding bits. Returns dummy 0xF8 value if no padding information is found
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetIndMediaSampleWithPad(MP4Media theMedia, u32 sampleNumber, MP4Handle outSample,
                              u32 *outSize, u64 *outDTS, s32 *outCTSOffset, u64 *outDuration,
                              u32 *outSampleFlags, u32 *outSampleDescIndex, u8 *outPad);
  /**
   * @brief Returns a sample from the media, given the sample's index.
   *
   * Parameters are the same as in MP4GetIndMediaSampleWithPad()
   */
  MP4_EXTERN(MP4Err)
  MP4GetIndMediaSample(MP4Media theMedia, u32 sampleNumber, MP4Handle outSample, u32 *outSize,
                       u64 *outDTS, s32 *outCTSOffset, u64 *outDuration, u32 *outSampleFlags,
                       u32 *outSampleDescIndex);
  /**
   * @brief Returns a reference (offset and size) for the sample from the media, given the sample's
   * index.
   *
   * Other parameters are similar to MP4GetIndMediaSampleWithPad()
   *
   * @param outOffset The offset of the returned sample in bytes
   * @param sampleDesc The sample description that corresponds to this sample in the media. Set this
   * to NULL if you do not want this information to be returned.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetIndMediaSampleReference(MP4Media theMedia, u32 sampleNumber, u32 *outOffset, u32 *outSize,
                                u32 *outDuration, u32 *outSampleFlags, u32 *outSampleDescIndex,
                                MP4Handle sampleDesc);
  /**
   * @brief Returns the number of data references.
   *
   * @param theMedia input media object
   * @param outCount output number of data references
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMediaDataRefCount(MP4Media theMedia, u32 *outCount);
  /**
   * @brief Sets the version of the MP4CompositionOffsetAtom to 1.
   *
   * This means that negative offsets for the composition time are enabled. Call this function
   * before adding samples to the media. This function also adds the composition to decode atom to
   * the sample table.
   *
   * @note before you start a new movie fragment you have to call
   * ISOSetCompositonToDecodePropertiesForFragments() to set the composition to decode fields
   * manually, because they cannot be automatically computed for fragments.
   *
   * @param media input media object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4UseSignedCompositionTimeOffsets(MP4Media media);

  /**
   * @brief Data reference attributes
   * @ingroup Types
   * @see MP4GetMediaDataReference()
   */
  enum
  {
    MP4DataRefSelfReferenceMask = (1 << 0) /**< samples are in the same file as the movie */
  };

  /**
   * @brief Data reference types
   * @ingroup Types
   * @see MP4GetMediaDataReference()
   */
  enum
  {
    MP4URLDataReferenceType = MP4_FOUR_CHAR_CODE('u', 'r', 'l', ' '), /**< URL */
    MP4URNDataReferenceType = MP4_FOUR_CHAR_CODE('u', 'r', 'n', ' ')  /**< URN */
  };

  /**
   * @brief Use this to get information about a specific data reference to a media container.
   *
   * The data reference specifies a location for media samples. Note that data references to samples
   * contained in the movie's file are treated as URL reference types with no name and a special
   * attribute bit set.
   *
   * @param theMedia input media object
   * @param index The index of the desired data reference. This number ranges from one to the number
   * of data references in the media.
   * @param referenceURL Will be set to a UTF-8 encoded file:// URL that indicates the file
   * containing the samples. If the data reference doesn't contain a URL the handle size will be set
   * to zero. Set referenceURL to NULL if you do not want this information.
   * @param referenceURN UTF-8 encoded URN. This may be used in addition to urlHandle to indicate
   * the uniform resource name associated with this media file. If the data reference does not
   * contain a URN the handle size will be set to zero. Set referenceURN to NULL if you do not want
   * this information.
   * @param outReferenceType This will be set to MP4URLDataReferenceType or MP4URNDataReferenceType
   * to indicate the type of data reference.
   * @param outReferenceAttributes Returns the attributes of the data reference as a bit set. The
   * only bit currently defined is MP4DataRefSelfReferenceMask which will be set if the media's
   * samples are contained in the same file as the movie.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaDataReference(MP4Media theMedia, u32 index, MP4Handle referenceURL,
                           MP4Handle referenceURN, u32 *outReferenceType,
                           u32 *outReferenceAttributes);
  /**
   * @brief Get the total duration of the media, expressed in the media’s time scale.
   *
   * @param theMedia input media object
   * @param outDuration output duration of the media in media's time scale
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMediaDuration(MP4Media theMedia, u64 *outDuration);
  /**
   * @brief This function can be used to obtain the media type.
   *
   * The handler type and the decoder config information, can be used to obtain enough information
   * to instantiate a decoder.
   *
   * @param theMedia input media object
   * @param outType contains the media handler (ISOVisualHandlerType, ISOAudioHandlerType, etc.)
   * @param outName If you want to retrieve the name for the handler you must supply a handle for
   * outName, otherwise set outName to NULL.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaHandlerDescription(MP4Media theMedia, u32 *outType, MP4Handle *outName);
  /**
   * @brief Returns the ISO 639-2/T three character language code associated with the media.
   *
   * @param theMedia input media object
   * @param outThreeCharCode output ISO 639-2/T three character language code
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMediaLanguage(MP4Media theMedia, char *outThreeCharCode);
  /**
   * @brief Returns the extended language, based on RFC 4646 (Best Common Practices – BCP – 47)
   * industry standard.
   *
   * @param theMedia input media object
   * @param extended_language Allocates memory for extended_language string. extended_language
   * string will be set to NULL if there is no extended language tag.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMediaExtendedLanguageTag(MP4Media theMedia, char **extended_language);

  /**
   * @brief Flags for NextInterestingTime
   * @ingroup Types
   * @see MP4GetMediaNextInterestingTime()
   */
  enum
  {
    MP4NextTimeSearchForward  = 0,        /**< Forward search */
    MP4NextTimeSearchBackward = -1,       /**< Backward search */
    MP4NextTimeMediaSample    = (1 << 0), /**< Media sample time */
    MP4NextTimeMediaEdit      = (1 << 1), /**< Media edit time */
    MP4NextTimeTrackEdit      = (1 << 2), /**< Track edit time */
    MP4NextTimeSyncSample     = (1 << 3), /**< Sync sample time */
    MP4NextTimeEdgeOK         = (1 << 4)  /**< Time edge is ok */
  };

  /**
   * @brief Search for interesging time
   *
   * @bug NB: This ignores any edit list present in the Media's Track
   *
   * @param theMedia input media object
   * @param interestingTimeFlags eg: MP4NextTimeMediaSample
   * @param searchFromTime in Media time scale
   * @param searchDirection eg: MP4NextTimeSearchForward
   * @param outInterestingTime output interesting time in Media time scale
   * @param outInterestingDuration output interesting duration in Media's time coordinate system
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaNextInterestingTime(MP4Media theMedia, u32 interestingTimeFlags, u64 searchFromTime,
                                 u32 searchDirection, u64 *outInterestingTime,
                                 u64 *outInterestingDuration);

  /**
   * @brief flags for MP4GetMediaSample(), MP4GetIndMediaSampleWithPad(), MP4GetIndMediaSample() ...
   * @ingroup Types
   */
  enum
  {
    MP4MediaSampleNotSync      = (1 << 0), /**< the sample is not a sync sample */
    MP4MediaSampleHasCTSOffset = (1 << 1)  /**< the sample’s DTS differs from its CTS */
  };

  /**
   * @brief Returns a closest sample from the media, given the desired decoding time.
   *
   * @param theMedia input media object
   * @param outSample The data for the desired sample. This is the raw access unit without headers.
   * @param outSize The size of the returned sample in bytes.
   * @param desiredDecodingTime The decoding time of the sample to be retrieved. You must specify
   * this value in the media’s time scale.
   * @param outDecodingTime The decoding time for the sample using the media time scale. This time
   * may differ from the value specified in the desiredDecodingTime parameter if that parameter's
   * value is not at a sample time boundary.
   * @param outCompositionTime The composition time for the sample expressed in the media time scale
   * @param outDuration The sample's duration in units of media time scale.
   * @param outSampleDescription The sample description that corresponds to this sample in the
   * media. Set this to NULL if you do not want this information to be returned.
   * @param outSampleDescriptionIndex The index of the sample description that corresponds to this
   * sample in the media.
   * @param outSampleFlags Flags that indicate the nature of this sample. Same as in
   * MP4GetIndMediaSampleWithPad()
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaSample(MP4Media theMedia, MP4Handle outSample, u32 *outSize, u64 desiredDecodingTime,
                    u64 *outDecodingTime, u64 *outCompositionTime, u64 *outDuration,
                    MP4Handle outSampleDescription, u32 *outSampleDescriptionIndex,
                    u32 *outSampleFlags);
  /**
   * @brief Returns a closest sample with padding bits, given the desired decoding time.
   *
   * @note If the track has no recorded padding table (no padding information) then the padding
   * value returned will be 0xF8. This is not a legal value for actual padding and should not be
   * supplied to calls which add samples.
   *
   * @param outPad output padding bits of the sample.
   *
   * other parameters are the same as in MP4GetMediaSample()
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaSampleWithPad(MP4Media theMedia, MP4Handle outSample, u32 *outSize,
                           u64 desiredDecodingTime, u64 *outDecodingTime, u64 *outCompositionTime,
                           u64 *outDuration, MP4Handle outSampleDescription,
                           u32 *outSampleDescriptionIndex, u32 *outSampleFlags, u8 *outPad);
  /**
   * @brief Use this to determine the total number of samples contained in the media.
   *
   * @param theMedia input media object
   * @param outCount output number of samples in the media
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMediaSampleCount(MP4Media theMedia, u32 *outCount);
  /**
   * @brief This returns the sample description at the given index, with the data reference index
   * that is associated with it
   * @ingroup SampleDescr
   *
   * @param theMedia input media object
   * @param index description index starting with 1
   * @param outDescriptionH output description handle
   * @param outDataReferenceIndex
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaSampleDescription(MP4Media theMedia, u32 index, MP4Handle outDescriptionH,
                               u32 *outDataReferenceIndex);
  /**
   * @brief Returns the time scale associated with the media.
   *
   * @param theMedia input media object
   * @param outTimeScale output time scale
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMediaTimeScale(MP4Media theMedia, u32 *outTimeScale);
  /**
   * @brief Returns the track that is associated with this media.
   *
   * @param theMedia input media object
   * @param outTrack output track object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetMediaTrack(MP4Media theMedia, MP4Track *outTrack);
  /**
   * @brief Use this to obtain the sample number for a sample with the given decoding time.
   *
   * @note The time may differ from the mediaTime if the desired time is not on a sample boundary.
   *
   * @param theMedia input media object
   * @param mediaTime The decoding time of the desired sample. In the media's time scale.
   * @param outSampleNum The sample number index for the desired sample.
   * @param outSampleCTS The composition time for the desired sample.
   * @param outSampleDTS The decoding time for the desired sample.
   * @param outSampleDuration The duration of the sample, expressed in the media time scale.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4MediaTimeToSampleNum(MP4Media theMedia, u64 mediaTime, u32 *outSampleNum, u64 *outSampleCTS,
                          u64 *outSampleDTS, s32 *outSampleDuration);
  /**
   * @brief Use this to obtain the decoding and composition times for a particular media sample.
   *
   * @param theMedia input media object
   * @param sampleNum The sample number index for the desired sample.
   * @param outSampleCTS The composition time for the desired sample.
   * @param outSampleDTS The decoding time for the desired sample.
   * @param outSampleDuration The duration of the sample, expressed in the media time scale.
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4SampleNumToMediaTime(MP4Media theMedia, u32 sampleNum, u64 *outSampleCTS, u64 *outSampleDTS,
                          s32 *outSampleDuration);
  /**
   * @brief Use this to indicate the ISO 639-2/T three character language code that is to be
   * associated with the media.
   */
  MP4_EXTERN(MP4Err) MP4SetMediaLanguage(MP4Media theMedia, char *threeCharCode);
  /**
   * @brief The extended language tag box represents media language information, based on RFC 4646
   * (Best Common Practices – BCP – 47) industry standard.
   */
  MP4_EXTERN(MP4Err) MP4SetMediaExtendedLanguageTag(MP4Media theMedia, char *extended_language);
  /**
   * @brief Use this to set the size of samplesize entries in the sample size table.
   *
   * The default value is 32, which gives the ‘old’ sample size table. Values of 4, 8, or 16 (the
   * size in bits of the field) may be given, to request the ‘compact’ sample size table.
   */
  MP4_EXTERN(MP4Err) ISOSetSampleSizeField(MP4Media theMedia, u32 fieldsize);

  /* Sync Layer media access routines */

  MP4_EXTERN(MP4Err)
  MP4GetElementaryStreamPacket(MP4Media theMedia, MP4Handle outSample, u32 *outSize,
                               u32 sequenceNumber, u64 desiredTime, u64 *outActualTime,
                               u64 *outDuration);
  /**
   * @brief Use this to obtain the decoder configuration descriptor for a given sample description.
   *
   * @param sampleDescIndex sample description index (starts with 1)
   * @param decoderConfigH output decoder confiburation handle
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaDecoderConfig(MP4Media theMedia, u32 sampleDescIndex, MP4Handle decoderConfigH);
  /**
   * @brief Obtain the complete decoder configuration and specific info without having to parse the
   * decoder config descriptor.
   *
   * @param sampleDescIndex Index of the sample description you desire (>0)
   * @param outObjectType Will contain the object type for this decoder config.
   * @param outStreamType Will contain the stream type for this decoder config.
   * @param outBufferSize Will contain the decoder buffer size.
   * @param outUpstream Will contain the value of the upstream flag.
   * @param outMaxBitrate Will contain the maximum bitrate for this decoder config.
   * @param outAvgBitrate Will contain the average bitrate for this decoder config.
   * @param specificInfoH Handle that will contain the specific info for this decoder config The tag
   * and length fields are stripped from the descriptor so only the configuration data remains. Set
   * specificInfoH to NULL if you do not want this information.
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaDecoderInformation(MP4Media theMedia, u32 sampleDescIndex, u32 *outObjectType,
                                u32 *outStreamType, u32 *outBufferSize, u32 *outUpstream,
                                u32 *outMaxBitrate, u32 *outAvgBitrate, MP4Handle specificInfoH);
  /**
   * @brief Use this to obtain the decoder configuration and specific info without having to parse
   * the decoder config descriptor.
   *
   * See MP4GetMediaDecoderConfig() if you need all the configuration information.
   *
   * @param sampleDescIndex Index of the sample description you desire (>0)
   * @param outObjectType Will contain the object type for this decoder config.
   * @param outStreamType Will contain the stream type for this decoder config.
   * @param outBufferSize Will contain the decoder buffer size.
   * @param specificInfoH Handle that will contain the specific info for this decoder config The tag
   * and length fields are stripped from the descriptor so only the configuration data remains. Set
   * specificInfoH to NULL if you do not want this information.
   */
  MP4_EXTERN(MP4Err)
  MP4GetMediaDecoderType(MP4Media theMedia, u32 sampleDescIndex, u32 *outObjectType,
                         u32 *outStreamType, u32 *outBufferSize, MP4Handle specificInfoH);

  /**
   * @}
   * \defgroup SyncLayer Sync layer functions
   *
   * MPEG sync layer functions
   * @{
   */

  /**
   * @brief Create a basic sample description that can be used for calls to MP4AddMediaSamples().
   *
   * This will create the proper kind of sample entry atom for the track type, and a basic
   * elementary stream descriptor that contains the information you provide in the parameters.
   *
   * @param sampleDescriptionH The handle that will contain the new sample description.
   * @param dataReferenceIndex The index of the data reference that describes the media samples.
   * If you haven’t called MP4AddMediaDataReference() this parameter should be set to 1.
   * Otherwise set it to the proper reference index for these samples.
   * @param objectTypeIndication Set this properly according to the table in the Systems spec.
   * @param streamType Set this properly according to the table in the Systems specification.
   * @param decoderBufferSize Set this to the size (in bytes) needed for the decoder input buffer.
   * @param maxBitrate Set this to the maximum number of bits per second needed to transmit media.
   * @param avgBitrate Set this to the average bitrate for this media.
   * @param decoderSpecificInfoH A handle that must contain the decoder specific info needed for a
   * decoder for this media type. This must be a properly formed descriptor, including tag and
   * length fields.
   */
  MP4_EXTERN(MP4Err)
  MP4NewSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH, u32 dataReferenceIndex,
                          u32 objectTypeIndication, u32 streamType, u32 decoderBufferSize,
                          u32 maxBitrate, u32 avgBitrate, MP4Handle decoderSpecificInfoH);
  /**
   * @brief Create a sample description with clock reference.
   *
   * This function is a version of MP4NewSampleDescription() that permits the caller to indicate an
   * association of the sample description with a clock reference. Parameters are the same as
   * MP4NewSampleDescription(), with the addition of:
   *
   * @param theOCRESID Set this to the ESID of the clock reference stream. A value of zero is
   * permitted, and indicates that the stream has no explicit OCR association.
   */
  MP4_EXTERN(MP4Err)
  MP4NewSampleDescriptionWithOCRAssociation(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                            u32 dataReferenceIndex, u32 objectTypeIndication,
                                            u32 streamType, u32 decoderBufferSize, u32 maxBitrate,
                                            u32 avgBitrate, MP4Handle decoderSpecificInfoH,
                                            u32 theOCRESID);

  /**
   * @}
   * \defgroup SampleDescr Sample Description functions
   *
   * Sample Description functions, General, AVC, Metadata, etc.
   * @{
   */

  /**
   * @brief Create a basic sample description that can be used for calls to MP4AddMediaSamples().
   *
   * This will create the proper kind of sample entry atom for the track type.
   *
   * @param theTrack input track
   * @param sampleDescriptionH The handle that will contain the new sample description.
   * @param dataReferenceIndex The index of the data reference that describes the media samples. If
   * you haven’t called MP4AddMediaDataReference() this parameter should be set to 1. Otherwise set
   * it to the proper reference index for these samples.
   * @param sampleEntryType The four-character-code for the type of sample entry. Use a pre-defined
   * type or the macro MP4_FOUR_CHAR_CODE(), which takes four single character arguments.
   * @param extensionAtom You can supply an extra atom to be placed into the sample entry here.
   */
  MP4_EXTERN(MP4Err)
  ISONewGeneralSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                 u32 dataReferenceIndex, u32 sampleEntryType,
                                 MP4GenericAtom extensionAtom);
  /**
   * @brief Add an arbitrary atom to a sample entry (description).
   *
   * @param sampleEntryH sample entry handle
   * @param extensionAtom atom to add to the sample entry
   */
  MP4_EXTERN(MP4Err)
  ISOAddAtomToSampleDescription(MP4Handle sampleEntryH, MP4GenericAtom extensionAtom);
  /**
   * @brief Find the atom of the given type and return it.
   *
   * @param sampleEntryH sample entry to look into
   * @param atomType atom type to search for
   * @param outAtom output atom of a given type
   */
  MP4_EXTERN(MP4Err)
  ISOGetAtomFromSampleDescription(MP4Handle sampleEntryH, u32 atomType, MP4GenericAtom *outAtom);
  /**
   * @brief Make a sample description for a timed XML meta-data track.
   *
   * @param theTrack
   * @param sampleDescriptionH
   * @param dataReferenceIndex
   * @param content_encoding
   * @param xml_namespace
   * @param schema_location
   */
  MP4_EXTERN(MP4Err)
  ISONewXMLMetaDataSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                     u32 dataReferenceIndex, char *content_encoding,
                                     char *xml_namespace, char *schema_location);
  /**
   * @brief Make a sample description for a timed text meta-data track.
   *
   * @param theTrack
   * @param sampleDescriptionH
   * @param dataReferenceIndex
   * @param content_encoding
   * @param mime_format
   */
  MP4_EXTERN(MP4Err)
  ISONewTextMetaDataSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                      u32 dataReferenceIndex, char *content_encoding,
                                      char *mime_format);

  /**
   * @}
   * \defgroup TrackReader Track reader functions
   *
   * Track readers are objects that allow you to easily read sequential samples or access units from
   * a movie track.
   * @{
   */

  /**
   * @brief Create a track reader for a movie track.
   */
  MP4_EXTERN(MP4Err) MP4CreateTrackReader(MP4Track theTrack, MP4TrackReader *outReader);
  /**
   * @brief Select local_key for reading. Demux mebx track.
   */
  MP4_EXTERN(MP4Err) MP4SetMebxTrackReader(MP4TrackReader theReader, u32 local_key);
  /**
   * @brief Frees up resources associated with a track reader.
   */
  MP4_EXTERN(MP4Err) MP4DisposeTrackReader(MP4TrackReader theReader);
  /**
   * @brief Places the appropriate decoder config descriptor into the handle provided.
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetCurrentDecoderConfig(MP4TrackReader theReader, MP4Handle decoderConfigH);
  /**
   * @brief Get the sample description associated with the current read-point.
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetCurrentSampleDescription(MP4TrackReader theReader, MP4Handle sampleEntryH);
  /**
   * @brief Get the index of the current sample description.
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetCurrentSampleDescriptionIndex(MP4TrackReader theReader, u32 *index);
  /**
   * @brief Use this to get the next access unit (MPEG-4), or sample, from the track.
   *
   * @note This function can also be called as MJ2TrackReaderGetNextSample.
   *
   * @param theReader input track reader object
   * @param outAccessUnit A handler that the sample/AU will be placed into
   * @param outSize The size of the returned sample/AU in bytes
   * @param outSampleFlags Contains information about the media sample that specifies this
   * sample/AU. Flag values are combinations of MP4MediaSampleNotSync if the sample is not a sync
   * sample and MP4MediaSampleHasCompositionOffset if the sample’s DTS differs from its CTS.
   * @param outCTS The composition time stamp for this sample/AU. This is measured in the movie
   * time, but in media time-units. If the DTS is negative (see below) and the CTS offset not that
   * large (or not present, and hence zero), this may also be negative. NB in previous releases this
   * parameter was unsigned.
   * @param outDTS The decoding time stamp for this sample/AU. This is measured in the movie time,
   * but in media time-units. It may be negative, or overlap the end of the previously decoded
   * access unit, indicating that not all the normal duration of this AU should be presented, and
   * some material trimmed from the beginning (the amount the time-stamp is negative or overlaps the
   * previous timestamp plus previous duration). NB in previous releases this parameter was unsigned
   * @return MP4Err Error code. This function returns ISOEOF if no more samples are available.
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetNextAccessUnit(MP4TrackReader theReader, MP4Handle outAccessUnit, u32 *outSize,
                                  u32 *outSampleFlags, s32 *outCTS, s32 *outDTS);
  /**
   * @brief Get the next access unit (MPEG-4), or sample, from the track with duration of the sample
   *
   * The duration may be shorter than the natural duration of the AU, in the case where the end of
   * an edit falls within the AU. Some material may need to be trimmed from the end of the AU.
   * @see MP4TrackReaderGetNextAccessUnit
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetNextAccessUnitWithDuration(MP4TrackReader theReader, MP4Handle outAccessUnit,
                                              u32 *outSize, u32 *outSampleFlags, s32 *outCTS,
                                              s32 *outDTS, u32 *outDuration);
  /**
   * @brief Get the next access unit, or sample, from the track with the padding bits for the sample
   *
   * If the track has no recorded padding table (no padding information) then the padding value
   * returned will be 0xF8. This is not a legal value for actual padding and should not be supplied
   * to calls which add samples.
   * @see MP4TrackReaderGetNextAccessUnit
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetNextAccessUnitWithPad(MP4TrackReader theReader, MP4Handle outAccessUnit,
                                         u32 *outSize, u32 *outSampleFlags, s32 *outCTS,
                                         s32 *outDTS, u8 *outPad);
  /**
   * @brief Use this to read the next SL-packet from the track.
   *
   * The packet is placed into the outPacket handle, and its size is indicated in outSize.
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetNextPacket(MP4TrackReader theReader, MP4Handle outPacket, u32 *outSize);
  /**
   * @brief Set the SLConfig of the trackreader
   */
  MP4_EXTERN(MP4Err) MP4TrackReaderSetSLConfig(MP4TrackReader theReader, MP4SLConfig slConfig);
  /**
   * @brief Get the sample number of the last returned access unit.
   *
   * After a track reader has returned an access unit (one of the above functions), this function
   * can be called to find the sample number of that access unit. This may be useful when (for
   * example), sample group association information is wanted for that sample.
   */
  MP4_EXTERN(MP4Err)
  MP4TrackReaderGetCurrentSampleNumber(MP4TrackReader theReader, u32 *sampleNumber);

  /**
   * @}
   * \defgroup SampleAux Sample Auxiliary Information
   *
   * MPEG sample auxiliary information functions
   * @{
   */

  /**
   * @brief Setup and initialize sample auxiliary information for a track.
   *
   * It must be called before adding sample auxiliary data. It can be called multiple times with
   * different type, parameter combinations to setup multiple sample auxiliary information for a
   * single track. To add sample auxiliary data for a sample use MP4AddSampleAuxiliaryInformation()
   *
   * @param isUsingAuxInfoPropertiesFlag Indicates whether aux_info_type and aux_info_type_parameter
   * are used for setting up auxiliary information.
   * @param aux_info_type Set this properly according to the specification.
   * @param aux_info_type_parameter Set this properly according to the specification.
   * @param default_sample_info_size If not equal to zero, all data for sample auxiliary information
   * must be the given size.
   */
  MP4_EXTERN(MP4Err)
  MP4SetupSampleAuxiliaryInformation(MP4Media theMedia, u8 isUsingAuxInfoPropertiesFlag,
                                     u32 aux_info_type, u32 aux_info_type_parameter,
                                     u8 default_sample_info_size);
  /**
   * @brief Add sample auxiliary information data for a range of samples.
   *
   * The first three parameters are used to identify the type of sample auxiliary information.
   * @note before adding data, MP4SetupSampleAuxiliaryInformation() has to be called with the same
   * parameters.
   */
  MP4_EXTERN(MP4Err)
  MP4AddSampleAuxiliaryInformation(MP4Media theMedia, u8 isUsingAuxInfoPropertiesFlag,
                                   u32 aux_info_type, u32 aux_info_type_parameter, MP4Handle dataH,
                                   u32 sampleCount, MP4Handle sizesH);
  /**
   * @brief Get information about all sample auxiliary information for a track.
   *
   * The MP4Handles will contain an array of u8 and u32, which will represent the description of
   * each instance of sample auxiliary information inside a track.
   *
   * @param outCount the number of different sample auxiliary information.
   * @param isUsingAuxInfoPropertiesFlags
   * @param aux_info_types
   * @param aux_info_type_parameters
   */
  MP4_EXTERN(MP4Err)
  MP4GetSampleAuxiliaryInformation(MP4Media theMedia, u32 *outCount,
                                   MP4Handle isUsingAuxInfoPropertiesFlags,
                                   MP4Handle aux_info_types, MP4Handle aux_info_type_parameters);
  /**
   * @brief Get the sample auxiliary information data for a specific sample and type.
   */
  MP4_EXTERN(MP4Err)
  MP4GetSampleAuxiliaryInformationForSample(MP4Media theMedia, u8 isUsingAuxInfoPropertiesFlag,
                                            u32 aux_info_type, u32 aux_info_type_parameter,
                                            u32 sampleNr, MP4Handle outDataH, u32 *outSize);
  /** @}*/

  /* User Data */
  /**
   * \defgroup UserData User data functions
   *
   * These functions allow you to access and manipulate track and movie user data.
   * @{
   */

  /**
   * @brief Adds an entry to the user data list.
   *
   * @param theUserData The user data list you are modifying
   * @param dataH An ISOHandle containing the data you are adding
   * @param userDataType The atom type that will identify this data
   * @param outIndex Returns the index (of entries of userDataType) that corresponds to this entry.
   * You will need this to retrieve this particular entry.
   */
  MP4_EXTERN(MP4Err)
  MP4AddUserData(MP4UserData theUserData, MP4Handle dataH, u32 userDataType, u32 *outIndex);
  /**
   * @brief Queries an indexed type of user data in the user data list.
   *
   * @param theUserData The user data list you are querying
   * @param typeIndex The index for the atom type you are querying. This should be one based.
   * @param outType Returns the atom type corresponding to the given index.
   */
  MP4_EXTERN(MP4Err) MP4GetIndUserDataType(MP4UserData theUserData, u32 typeIndex, u32 *outType);
  /**
   * @brief Returns the count of user data atoms of the specified type.
   */
  MP4_EXTERN(MP4Err)
  MP4GetUserDataEntryCount(MP4UserData theUserData, u32 userDataType, u32 *outCount);
  /**
   * @brief Returns the contents of the requested user data item.
   *
   * @param theUserData The user data list you are querying
   * @param dataH A handle to contain the returned data
   * @param userDataType The type of user data
   * @param itemIndex The index for the atom type you are querying. This should be one based
   */
  MP4_EXTERN(MP4Err)
  MP4GetUserDataItem(MP4UserData theUserData, MP4Handle dataH, u32 userDataType, u32 itemIndex);
  /**
   * @brief Returns the contents of the requested user data item.
   *
   * @param theUserData The user data list you are querying
   * @param outAtom A generic atom, which will contain the requested atom
   * @param userDataType The type of user data
   * @param itemIndex The index for the atom type you are querying. This should be one based
   */
  MP4_EXTERN(MP4Err)
  MP4GetAtomFromUserData(MP4UserData theUserData, MP4GenericAtom *outAtom, u32 userDataType,
                         u32 itemIndex);
  /**
   * @brief Returns the count of user data types.
   */
  MP4_EXTERN(MP4Err) MP4GetUserDataTypeCount(MP4UserData theUserData, u32 *outCount);
  /**
   * @brief Deletes a user-data item, by index.
   */
  MP4_EXTERN(MP4Err)
  MP4DeleteUserDataItem(MP4UserData theUserData, u32 userDataType, u32 itemIndex);
  /**
   * @brief Creates a new user data list.
   */
  MP4_EXTERN(MP4Err) MP4NewUserData(MP4UserData *outUserData);
  /**
   * @brief Construct a new custom atom object
   *
   * @param outAtom output new atom object
   * @param atomType atom type fourcc
   * @param atomPayload atom payload data handle
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4NewForeignAtom(MP4GenericAtom *outAtom, u32 atomType, MP4Handle atomPayload);
  /**
   * @brief Construct a new custom atom object with UUID type
   *
   * @param outAtom output new atom object
   * @param the_uuid atom UUID
   * @param atomPayload atom payload data handle
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4NewUUIDAtom(MP4GenericAtom *outAtom, u8 the_uuid[16], MP4Handle atomPayload);
  /**
   * @brief Returns the type and contents of the given foreign atom, and, if it is a UUID atom, its
   * UUID also.
   */
  MP4_EXTERN(MP4Err)
  MP4GetForeignAtom(MP4GenericAtom atom, u32 *atomType, u8 the_uuid[16], MP4Handle atomPayload);

  /**
   * @}
   * \defgroup SLConfig SLConfig functions
   *
   * MPEG SLConfig functions
   * @{
   */

  /**
   * @brief SL configuration settings record
   */
  typedef struct MP4SLConfigSettingsRecord
  {
    u32 predefined;
    u32 useAccessUnitStartFlag;
    u32 useAccessUnitEndFlag;
    u32 useRandomAccessPointFlag;
    u32 useRandomAccessUnitsOnlyFlag;
    u32 usePaddingFlag;
    u32 useTimestampsFlag;
    u32 useIdleFlag;
    u32 durationFlag;
    u32 timestampResolution;
    u32 OCRResolution;
    u32 timestampLength;
    u32 OCRLength;
    u32 AULength;
    u32 instantBitrateLength;
    u32 degradationPriorityLength;
    u32 AUSeqNumLength;
    u32 packetSeqNumLength;
    u32 timeScale;
    u32 AUDuration;
    u32 CUDuration;
    u64 startDTS;
    u64 startCTS;
    u32 OCRESID;
  } MP4SLConfigSettings, *MP4SLConfigSettingsPtr;

  /**
   * @brief Create new SLConfig object
   *
   * @param settings input settings
   * @param outSLConfig output SLConfig object
   */
  MP4_EXTERN(MP4Err) MP4NewSLConfig(MP4SLConfigSettingsPtr settings, MP4SLConfig *outSLConfig);
  /**
   * @brief Get settings from SLConfig object
   *
   * @param config input config
   * @param outSettings output settings
   */
  MP4_EXTERN(MP4Err) MP4GetSLConfigSettings(MP4SLConfig config, MP4SLConfigSettingsPtr outSettings);
  /**
   * @brief Set settings of SLConfig object
   *
   * @param config input config
   * @param settings input settings
   */
  MP4_EXTERN(MP4Err) MP4SetSLConfigSettings(MP4SLConfig config, MP4SLConfigSettingsPtr settings);
  /** @} */

#ifndef INCLUDED_ISOMOVIE_H
#include "ISOMovies.h"
#endif

  /* JLF 12/00: added support for URL and for exchange files */
  MP4_EXTERN(MP4Err)
  MP4NewMovieExt(MP4Movie *outMovie, u32 initialODID, u8 OD_profileAndLevel,
                 u8 scene_profileAndLevel, u8 audio_profileAndLevel, u8 visual_profileAndLevel,
                 u8 graphics_profileAndLevel, char *url, u8 IsExchangeFile);

  /* JLF 12/00: added support for stream priority */
  /**
   * @brief This function sets the MPEG-4 elementary stream priority of the associated stream (in
   * the elementary stream descriptor).
   * @ingroup SyncLayer
   */
  MP4_EXTERN(MP4Err) MP4SetSampleDescriptionPriority(MP4Handle sampleEntryH, u32 priority);

  /**
   * \defgroup IPMPX MPEG 4 IPMPX related functions
   *
   * MPEG 4 IPMPX related functions
   * @{
   */

  /* JLF 12/00: added support for descriptors in the ESD */
  /**
   * @brief Can be used to add an IPMP Descriptor Pointer into an existing sampleDescription.
   */
  MP4_EXTERN(MP4Err) MP4AddDescToSampleDescription(MP4Handle sampleEntryH, MP4Handle descriptorH);

  /* JLF 12/00: added support for descriptors in the OD/IOD */
  /**
   * @brief Adds a descriptor to the movie IOD. The descriptor can be an IPMP Tool List Descriptor.
   */
  MP4_EXTERN(MP4Err) MP4AddDescToMovieIOD(MP4Movie theMovie, MP4Handle descriptorH);

  /* JLF 12/00: checking of a specific data entry. */
  /**
   * @brief Tests a specific data reference in the media to ensure that the reference exists and is
   * readable.
   * @ingroup Media
   */
  MP4_EXTERN(MP4Err) MP4CheckMediaDataRef(MP4Media theMedia, u32 dataEntryIndex);
  /**
   * @brief This function sets the visual width and height of a sample description.
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err)
  ISOSetSampleDescriptionDimensions(MP4Handle sampleEntryH, u16 width, u16 height);
  /**
   * @brief (Re)set the type of a sample description.
   *
   * @attention do not set the type away from, and back to, an MPEG-4 type, or the elementary stream
   * descriptor atom will be lost.
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err) ISOSetSampleDescriptionType(MP4Handle sampleEntryH, u32 type);
  /**
   * @brief This function gets the visual width and height of a sample description.
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err)
  ISOGetSampleDescriptionDimensions(MP4Handle sampleEntryH, u16 *width, u16 *height);
  /**
   * @brief This function returns the type of a sample description.
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err) ISOGetSampleDescriptionType(MP4Handle sampleEntryH, u32 *type);

  /**
   * @brief This starts a new movie fragment.
   *
   * Before calling this, you should create a normal movie, up to the point you would write it to
   * the file, and you should set the track fragment defaults on the tracks to which you want to add
   * samples. After calling this, you should only add samples, and write the file out.
   * Each call to this function starts a new fragment. For example, you might call this function
   * after ISOEndMediaEdits, and then insert some more samples into the track(s).
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) ISOStartMovieFragment(MP4Movie theMovie);
  /**
   * @brief Add delay to track fragment decode time.
   *
   * Setting the delay parameter to other than zero, a delay will be introduced, which will add a
   * time offset to the track fragment decode time.
   *
   * @note Must be called right after ISOStartMovieFragment().
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param delay delay value
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) ISOAddDelayToTrackFragmentDecodeTime(MP4Movie theMovie, u32 delay);
  /**
   * @brief Sets the default sample duration, size, sync-flag, and padding bits for samples added
   * into movie fragments, for this track.
   *
   * @note This function must be called for each track to which fragment  samples will be added,
   * before the first fragment is started.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param duration default sample duration
   * @param size default sample size
   * @param is_sync default sample sync-flag
   * @param pad default sample padding bits
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOSetTrackFragmentDefaults(MP4Track theTrack, u32 duration, u32 size, u32 is_sync, u8 pad);
  /**
   * @brief Sets the compositon to decode parameters for a specific track.
   *
   * They have to be calculated and added before calling the first ISOStartMovieFragment() and
   * cannot be changed later.
   *
   * @note Must be called before the first ISOStartMovieFragment().
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param trackID input track ID
   * @param compositionToDTSShift composition to decoding time shift
   * @param leastDecodeToDisplayDelta least decode to display delta
   * @param greatestDecodeToDisplayDelta greatest decode to display delta
   * @param compositionStartTime composition start time
   * @param compositionEndTime composition end time
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  ISOSetCompositonToDecodePropertiesForFragments(MP4Movie theMovie, u32 trackID,
                                                 s32 compositionToDTSShift,
                                                 s32 leastDecodeToDisplayDelta,
                                                 s32 greatestDecodeToDisplayDelta,
                                                 s32 compositionStartTime, s32 compositionEndTime);

  /**
   * @brief Creates a new IPMPTool structure.
   *
   * @param ipmpToolH Handle containing the output IPMPTool structure
   * @param ipmpToolIdLowerPart First 64 bits of the 128bits IPMP Tool ID
   * @param ipmpToolIdUpperPart Last 64 bits of the 128bits IPMP Tool ID
   * @param altGroupInfoH Can be set to NULL if the IPMPTool has no alternate. Handle built as an
   * u64[] array containing for example:
   * - alternateTool1LowerPart
   * - alternateTool1UpperPart
   * - alternateTool2LowerPart
   * - alternateTool2UpperPart
   * - ...
   * @param parametricInfoH Handle containing the parametric information of the IPMPTool. Can be set
   * to NULL if the tool is not parametric.
   */
  MP4_EXTERN(MP4Err)
  MP4NewIPMPTool(MP4Handle ipmpToolH, u64 ipmpToolIdLowerPart, u64 ipmpToolIdUpperPart,
                 MP4Handle altGroupInfoH, MP4Handle parametricInfoH);
  /**
   * @brief Add a URL from which one or more tools specified in IPMPTool can be downloaded.
   */
  MP4_EXTERN(MP4Err) MP4AddUrlToIPMPTool(MP4Handle ipmpToolH, MP4Handle urlH);
  /**
   * @brief Creates a new IPMPToolListDescriptor.
   */
  MP4_EXTERN(MP4Err) MP4NewIPMPToolListDescriptor(MP4Handle ipmpToolListDescrH);
  /**
   * @brief Description TBD
   */
  MP4_EXTERN(MP4Err) MP4AddToolToIPMPToolList(MP4Handle ipmpToolListDescrH, MP4Handle ipmpToolH);
  /**
   * @brief Creates a new IPMPDescriptorPointer.
   *
   * @param ipmpDescPtrH Handle containing the output IPMP Descriptor Pointer
   * @param ipmpDescriptorId ID of the output IPMP Descriptor Pointer
   * @param ipmpToolDescrId ID of the IPMP Tool Descriptor pointed by the output IPMP Descriptor
   * Pointer
   */
  MP4_EXTERN(MP4Err)
  MP4NewIPMPDescriptorPointer(MP4Handle ipmpDescPtrH, u8 ipmpDescriptorId, u16 ipmpToolDescrId);
  /**
   * @brief Creates a new IPMP Tool Descriptor.
   *
   * @param ipmpToolDescH Handle containing the ouptut IPMP Tool Descriptor
   * @param ipmpToolDescrId ID of the output IPMP Tool Descriptor
   * @param ipmpToolIdLowerPart First 64 bits of the 128bits IPMP Tool ID
   * @param ipmpToolIdUpperPart Last 64 bits of the 128bits IPMP Tool ID
   * @param ipmpInitializeH Handle Containing the IPMPInitialize structure. Can be set to NULL if no
   * IPMPInitialize structure is present.
   */
  MP4_EXTERN(MP4Err)
  MP4NewIPMPToolDescriptor(MP4Handle ipmpToolDescH, u16 ipmpToolDescrId, u64 ipmpToolIdLowerPart,
                           u64 ipmpToolIdUpperPart, MP4Handle ipmpInitializeH);
  /**
   * @brief Creates a new IPMPInitialize structure
   *
   * @param ipmpInitializeH Handle containing the output IPMPInitialize structure
   * @param controlPoint The control Point Code can be one of the following:
   * - MP4IPMP_NoControlPoint
   * - MP4IPMP_DB_Decoder_ControlPoint
   * - MP4IPMP_Decoder_CB_ControlPoint
   * - MP4IPMP_CB_Compositor_ControlPoint
   * - MP4IPMP_BIFSTree_ControlPoint
   * @param sequenceCode The higher the sequence code, the higher the sequencing priority of the
   * IPMP tool instance at the given control point.
   */
  MP4_EXTERN(MP4Err)
  MP4NewIPMPInitialize(MP4Handle ipmpInitializeH, u8 controlPoint, u8 sequenceCode);
  /**
   * @brief Adds IPMP data to the IPMPInitialize structure.
   *
   * @param ipmpInitializeH Handle containing the IPMPInitialize structure. Can be created with the
   * MP4NewIPMPInitialize() function.
   * @param ipmpDataH Handle containing the IPMP Data to insert in the IPMPInitalize structure.
   */
  MP4_EXTERN(MP4Err) MP4AddIPMPDataToIPMPInitialize(MP4Handle ipmpInitializeH, MP4Handle ipmpDataH);
  /**
   * @brief Adds IPMP data to an IPMP Tool Descriptor.
   *
   * @param ipmpToolDescrH Handle containing the IPMP Tool Descriptor. Can be created with the
   * MP4NewIPMPToolDescriptor() function.
   * @param ipmpDataH Handle containing the IPMP Data to insert in the IPMP Tool Descriptor.
   */
  MP4_EXTERN(MP4Err)
  MP4AddIPMPDataToIPMPToolDescriptor(MP4Handle ipmpToolDescrH, MP4Handle ipmpDataH);
  /**
   * @brief Creates a new IPMPToolDescriptorUpdate command.
   */
  MP4_EXTERN(MP4Err) MP4NewIPMPToolDescriptorUpdate(MP4Handle ipmpToolDescrUpdateH);
  /**
   * @brief Adds an IPMP Tool Descriptor to IPMPToolDescriptorUpdate command.
   *
   * @param ipmpToolDescrUpdateH Handle containing the IPMPToolDescriptorUpdate command. Can be
   * created with the MP4NewIPMPToolDescriptorUpdate() function.
   * @param ipmpToolDescrH Handle containing the IPMPToolDescriptor to insert in the command. Can be
   * created with the MP4NewIPMPToolDescriptor() function.
   */
  MP4_EXTERN(MP4Err)
  MP4AddIPMPToolDescriptorToUpdate(MP4Handle ipmpToolDescrUpdateH, MP4Handle ipmpToolDescrH);
  /** @} */

#ifdef ISMACrypt
  /**
   * \defgroup ISMA ISMACrypt Support
   *
   * ISMACrypt Support
   * @{
   */

  /**
   * @brief ‘Transforms’ a sample entry into the format expected by ISMA for a sample entry for
   * protected (encrypted) content.
   *
   * The flags are the same as for new tracks (audio or video track).
   * @note it is possible to build without ISMACrypt support using an ifdef.
   */
  MP4_EXTERN(MP4Err)
  ISMATransformSampleEntry(u32 newTrackFlags, MP4Handle insampleEntryH, u8 selective_encryption,
                           u8 key_indicator_length, u8 IV_length, char *kms_URL,
                           MP4Handle outsampleEntryH);
  /**
   * @brief ‘Untransforms’ a sample entry, from the encrypted form to the standard form.
   *
   * Returns the encryption parameter information previously supplied to the previous function.
   */
  MP4_EXTERN(MP4Err)
  ISMAUnTransformSampleEntry(MP4Handle insampleEntryH, u8 *selective_encryption,
                             u8 *key_indicator_length, u8 *IV_length, char **kms_URL,
                             MP4Handle outsampleEntryH);
  /**
   * @brief ‘Transforms’ a sample entry into the format expected by ISMA for a sample entry for
   * protected (encrypted) content.
   *
   * The flags are the same as for new tracks (audio or video track). The salt is an initial salt
   * value which will be stored in the file if non-zero.
   * @note it is possible to build without ISMACrypt support using an ifdef.
   */
  MP4_EXTERN(MP4Err)
  ISMATransformSampleEntrySalt(u32 newTrackFlags, MP4Handle insampleEntryH, u8 selective_encryption,
                               u8 key_indicator_length, u8 IV_length, char *kms_URL, u64 salt,
                               MP4Handle outsampleEntryH);
  /**
   * @brief ‘Untransforms’ a sample entry, from the encrypted form to the standard form.
   *
   * Returns the encryption parameter information previously supplied to the previous function,
   * filling in salt if it was supplied in the file.
   */
  MP4_EXTERN(MP4Err)
  ISMAUnTransformSampleEntrySalt(MP4Handle insampleEntryH, u8 *selective_encryption,
                                 u8 *key_indicator_length, u8 *IV_length, char **kms_URL, u64 *salt,
                                 MP4Handle outsampleEntryH);

#define ISMA_selective_encrypt 0x80
  /** @} */
#endif

  /**
   * @brief This returns the number of existent Edit list entries.
   *
   * Returns the number of existent Edit list entries in entryCount if the Edit list information
   * is present, otherwise 0.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param entryCount output number of Edit list entries
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err) MP4GetTrackEditlistEntryCount(MP4Track theTrack, u32 *entryCount);
  /**
   * @brief This returns a non-zero value in outSegmentDuration and outMediaTime if the Edit list
   * information is present and valid.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param outSegmentDuration output segment duration
   * @param outMediaTime output media time
   * @param entryIndex defines the one-based index of the entry to return
   * @return MP4Err error code
   */
  MP4_EXTERN(MP4Err)
  MP4GetTrackEditlist(MP4Track theTrack, u64 *outSegmentDuration, s64 *outMediaTime,
                      u32 entryIndex);

#ifdef __cplusplus
}
#endif
#ifdef PRAGMA_EXPORT
#pragma export off
#endif

#endif
