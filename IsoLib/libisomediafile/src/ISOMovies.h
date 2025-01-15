/**
 * @file ISOMovies.h
 * @brief A wrapper for the code defined in the MPEG-4 library.
 * @version 0.1
 *
 * @copyright This header file may be freely copied and distributed.
 *
 */

#ifndef INCLUDED_ISOMOVIE_H
#define INCLUDED_ISOMOVIE_H

#ifndef INCLUDED_MP4MOVIE_H
#include "MP4Movies.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* These data types are common to both MPEG-4 and JPEG-2; ideally the "ISO" names should be used. */
#define ISOHandle MP4Handle
#define ISOErr MP4Err
#define ISOMovie MP4Movie
#define ISOTrack MP4Track
#define ISOMedia MP4Media
#define ISOTrackReader MP4TrackReader
#define ISOUserData MP4UserData
#define ISOAtomPtr MP4AtomPtr
#define ISOLinkedList MP4LinkedList

#define ISO_EXTERN MP4_EXTERN

/* These constants are common to both MPEG-4 and JPEG-2; ideally the "ISO" names should be used. */
#define ISOEOF MP4EOF
#define ISONoErr MP4NoErr
#define ISOFileNotFoundErr MP4FileNotFoundErr
#define ISOBadParamErr MP4BadParamErr
#define ISONoMemoryErr MP4NoMemoryErr
#define ISOIOErr MP4IOErr
#define ISONoLargeAtomSupportErr MP4NoLargeAtomSupportErr
#define ISOBadDataErr MP4BadDataErr
#define ISOVersionNotSupportedErr MP4VersionNotSupportedErr
#define ISOInvalidMediaErr MP4InvalidMediaErr
#define ISODataEntryTypeNotSupportedErr MP4DataEntryTypeNotSupportedErr
#define ISONoQTAtomErr MP4NoQTAtomErr
#define ISONotImplementedErr MP4NotImplementedErr

#define ISONewTrackIsVisual MP4NewTrackIsVisual
#define ISONewTrackIsAudio MP4NewTrackIsAudio
#define ISONewTrackIsPrivate MP4NewTrackIsPrivate
#define ISONewTrackIsMetadata MP4NewTrackIsMetadata

#define ISOVisualHandlerType MP4VisualHandlerType
#define ISOAudioHandlerType MP4AudioHandlerType
#define ISOHintHandlerType MP4HintHandlerType
#define ISOVolumetricHandlerType MP4VolumetricHandlerType
#define ISOHapticHandlerType MP4HapticHandlerType

#define ISOOpenMovieNormal MP4OpenMovieNormal
#define ISOOpenMovieDebug MP4OpenMovieDebug
#define ISOOpenMovieInPlace MP4OpenMovieInPlace

  struct MP4BoxedMetadataSampleEntry;

  /**
   * @brief constants for the graphics modes (e.g. for MJ2SetMediaGraphicsMode)
   */
  enum
  {
    ISOGraphicsModeSrcCopy     = 0x00000000,
    ISOGraphicsModeTransparent = 0x00000024,
    ISOGraphicsModeAlpha       = 0x00000100,
    ISOGraphicsModeWhiteAlpha  = 0x00000101,
    ISOGraphicsModeBlackAlpha  = 0x00000102
  };

  /**
   * @brief These data types are specific to JPEG-2; however, they have "ISO" names.
   */
  struct ISOMatrixRecord
  {
    u32 data[9];
  };
  typedef struct ISOMatrixRecord ISOMatrixRecord;
  typedef ISOMatrixRecord *ISOMatrix;

  /**
   * @brief RGB color type
   */
  struct ISORGBColor
  {
    u16 red;
    u16 green;
    u16 blue;
  };
  typedef struct ISORGBColor ISORGBColor;
  typedef ISORGBColor *ISORGBColorPtr;

  /**
   * @brief constants for the fileType field of the MP4PrivateMovieRecord structure
   */
  enum
  {
    ISOUnknownFileType        = (u32)0,
    ISOMPEG4FileType          = (u32)1,
    ISOStillJPEG2000FileType  = (u32)2,
    ISOMotionJPEG2000FileType = (u32)3,
    ISOQuickTimeFileType      = (u32)4,
    ISO3GPPFileType           = (u32)5,
    ISOMPEG21FileType         = (u32)6
  };

  /**
   * @brief constants for the qt_componentType field of the MP4HandlerAtom structure
   */
  enum
  {
    ISOMediaHandlerType = MP4_FOUR_CHAR_CODE('m', 'h', 'l', 'r'),
    ISODataHandlerType  = MP4_FOUR_CHAR_CODE('d', 'h', 'l', 'r')
  };

  /**
   * @brief Brands
   * @ingroup Types
   */
  enum
  {
    /** brand for JPEG-2000 */
    JP2JPEG2000Brand = MP4_FOUR_CHAR_CODE('j', 'p', '2', ' '),
    /** brand for Motion JPEG-2000 */
    MJ2JPEG2000Brand = MP4_FOUR_CHAR_CODE('m', 'j', 'p', '2'),
    /** brand for QuickTime */
    ISOQuickTimeBrand = MP4_FOUR_CHAR_CODE('q', 't', ' ', ' '),
    /** brand for MPEG-4 version 1 */
    ISOMpeg4V1Brand = MP4_FOUR_CHAR_CODE('m', 'p', '4', '1'),
    /** brand for MPEG-4 version 2 */
    ISOMpeg4V2Brand = MP4_FOUR_CHAR_CODE('m', 'p', '4', '2'),
    /** conforming brand for all files */
    ISOISOBrand = MP4_FOUR_CHAR_CODE('i', 's', 'o', 'm'),
    /** conforming brand for all files */
    ISOISO2Brand = MP4_FOUR_CHAR_CODE('i', 's', 'o', '2'),
    /** 3GPP Release 4 */
    ISO3GP4Brand = MP4_FOUR_CHAR_CODE('3', 'g', 'p', '4'),
    /** 3GPP Release 5 */
    ISO3GP5Brand = MP4_FOUR_CHAR_CODE('3', 'g', 'p', '5'),
    /** 3GPP Release 6 */
    ISO3GP6Brand = MP4_FOUR_CHAR_CODE('3', 'g', 'p', '6'),
    /** MPEG-21 */
    ISOMPEG21Brand = MP4_FOUR_CHAR_CODE('m', 'p', '2', '1'),
    /** default 'brand' */
    ISOUnknownBrand = MP4_FOUR_CHAR_CODE(' ', ' ', ' ', ' '),
    /** DASH (ISO/IEC 23009-1) */
    ISO_DASH_Brand = MP4_FOUR_CHAR_CODE('d', 'a', 's', 'h'),
    /** DASH self-initializing media seg. */
    ISO_DASH_DSMS_Brand = MP4_FOUR_CHAR_CODE('d', 's', 'm', 's'),
    /** DASH general format media segment */
    ISO_DASH_MSDH_Brand = MP4_FOUR_CHAR_CODE('m', 's', 'd', 'h'),
    /** DASH indexed media segment */
    ISO_DASH_MSIX_Brand = MP4_FOUR_CHAR_CODE('m', 's', 'd', 'h'),
    /** DASH representation index segment */
    ISO_DASH_RISX_Brand = MP4_FOUR_CHAR_CODE('r', 'i', 's', 'x'),
    /** DASH last media segment indicator */
    ISO_DASH_LMSG_Brand = MP4_FOUR_CHAR_CODE('l', 'm', 's', 'g'),
    /** DASH single index segment */
    ISO_DASH_SISX_Brand = MP4_FOUR_CHAR_CODE('s', 'i', 's', 'x'),
    /** DASH subsegment index segment */
    ISO_DASH_SSSS_Brand = MP4_FOUR_CHAR_CODE('s', 's', 's', 's')
  };

  /**
   * @brief AVC/SVC/HEVC Parameter Set places
   * @ingroup Types
   * @see ISOAddVCSampleDescriptionPS, ISOGetVCSampleDescriptionPS, ISOGetRESVSampleDescriptionPS,
   * ISOGetHEVCSampleDescriptionPS
   */
  enum
  {
    AVCsps    = 1,    /**< AVC Sequence parameter set */
    AVCpps    = 2,    /**< AVC Picture parameter set */
    AVCspsext = 3,    /**< AVC Sequence parameter set extension */
    SVCsps    = 0x11, /**< SVC Sequence parameter set */
    SVCpps    = 0x12, /**< SVC Picture parameter set */
    SVCspsext = 0x13, /**< SVC SPS extension */
    HEVCvps   = 0x20, /**< HEVC Video Parameter Set */
    HEVCsps   = 0x21, /**< HEVC Sequence Parameter Set */
    HEVCpps   = 0x22  /**< HEVC Picture Parameter Set */
  };

  /**
   * @brief VVC Parameter Set places
   * @ingroup Types
   * @see ISOAddVVCSampleDescriptionPS, ISOGetVVCSampleDescriptionPS,
   */
  enum
  {
    VVCsps         = 15,
    VVCpps         = 16,
    VVCvps         = 14,
    VVCopi         = 12,
    VVCdci         = 13,
    VVC_prefix_aps = 17,
    VVC_prefix_sei = 23
  };

  /**
   * @brief Meta-data records
   */
  struct ISOMetaRecord
  {
    void *data;
  };
  typedef struct ISOMetaRecord ISOMetaRecord;
  /**
   * @brief This is an opaque handle that contains a reference to rich meta-data.
   * @ingroup Types
   */
  typedef ISOMetaRecord *ISOMeta;

  /**
   * @brief Meta-item records
   */
  struct ISOMetaItemRecord
  {
    void *data;
  };
  typedef struct ISOMetaItemRecord ISOMetaItemRecord;
  /**
   * @brief This is an opaque handle that contains a reference to a rich meta-data item.
   * @ingroup Types
   */
  typedef ISOMetaItemRecord *ISOMetaItem;

  /**
   * @brief Structure which contanis all the common parameters of an EntityGroup
   * @ingroup Types
   */
  typedef struct EntityGroupEntry
  {
    u32 grouping_type;
    u32 group_id;
    u32 num_entities_in_group;
    u32 *entity_ids;
  } EntityGroupEntry, *EntityGroupEntryPtr;

/* These functions are general movie-handling functions and are common to both MPEG-4 and JPEG-2;
   ideally the "ISO" names should be used. */
#define ISODisposeMovie MP4DisposeMovie
#define ISOGetMovieTimeScale MP4GetMovieTimeScale
#define ISOGetMovieTrack MP4GetMovieTrack
#define ISOOpenMovieFile MP4OpenMovieFile
#define ISOPutMovieIntoHandle MP4PutMovieIntoHandle
#define ISOSetMovieTimeScale MP4SetMovieTimeScale
#define ISOAddTrackReference MP4AddTrackReference
#define ISOAddSubSampleInformationToTrack MP4AddSubSampleInformationToTrack
#define ISOSetSubSampleInformationFlags MP4SetSubSampleInformationFlags
#define ISOGetSubSampleInformationEntryFromTrack MP4GetSubSampleInformationEntryFromTrack
#define ISOAddSubSampleInformationEntry MP4AddSubSampleInformationEntry
#define ISOAddTrackGroup MP4AddTrackGroup
#define ISOAddTrackReferenceWithID MP4AddTrackReferenceWithID
#define ISOGetMovieIndTrack MP4GetMovieIndTrack
#define ISOGetMovieTrackCount MP4GetMovieTrackCount
#define ISOGetTrackEnabled MP4GetTrackEnabled
#define ISOGetTrackID MP4GetTrackID
#define ISOGetTrackMedia MP4GetTrackMedia
#define ISOGetTrackMovie MP4GetTrackMovie
#define ISOGetTrackOffset MP4GetTrackOffset
#define ISOGetTrackReference MP4GetTrackReference
#define ISOGetTrackReferenceCount MP4GetTrackReferenceCount
#define ISOGetTrackGroup MP4GetTrackGroup
#define ISOInsertMediaIntoTrack MP4InsertMediaIntoTrack
#define ISONewMovieTrack MP4NewMovieTrack
#define ISONewMovieTrackWithID MP4NewMovieTrackWithID
#define ISONewTrackMedia MP4NewTrackMedia
#define ISOSetTrackEnabled MP4SetTrackEnabled
#define ISOSetTrackOffset MP4SetTrackOffset
#define ISOTrackTimeToMediaTime MP4TrackTimeToMediaTime
#define ISOAddMediaDataReference MP4AddMediaDataReference
#define ISOAddMediaSampleReference MP4AddMediaSampleReference
#define ISOAddMediaSamples MP4AddMediaSamples
#define ISOAddMediaSamplesPad MP4AddMediaSamplesPad
#define ISOBeginMediaEdits MP4BeginMediaEdits
#define ISOCheckMediaDataReferences MP4CheckMediaDataReferences
#define ISOEndMediaEdits MP4EndMediaEdits
#define ISOGetIndMediaSample MP4GetIndMediaSample
#define ISOGetIndMediaSampleWithPad MP4GetIndMediaSampleWithPad
#define ISOGetMediaDataReference MP4GetMediaDataReference
#define ISOGetMovieDuration MP4GetMovieDuration
#define ISOGetTrackDuration MP4GetTrackDuration
#define ISOGetMediaDuration MP4GetMediaDuration
#define ISOGetMediaHandlerDescription MP4GetMediaHandlerDescription
#define ISOGetMediaLanguage MP4GetMediaLanguage
#define ISOGetMediaSample MP4GetMediaSample
#define ISOGetMediaSampleWithPad MP4GetMediaSampleWithPad
#define ISOGetMediaSampleCount MP4GetMediaSampleCount
#define ISOGetMediaTimeScale MP4GetMediaTimeScale
#define ISOGetMediaTrack MP4GetMediaTrack
#define ISOMediaTimeToSampleNum MP4MediaTimeToSampleNum
#define ISOSampleNumToMediaTime MP4SampleNumToMediaTime
#define ISOSetMediaLanguage MP4SetMediaLanguage
#define ISOSetMediaExtendedLanguageTag MP4SetMediaExtendedLanguageTag
#define ISOAddUserData MP4AddUserData
#define ISOGetIndUserDataType MP4GetIndUserDataType
#define ISOGetMovieUserData MP4GetMovieUserData
#define ISOGetTrackUserData MP4GetTrackUserData
#define ISOGetUserDataEntryCount MP4GetUserDataEntryCount
#define ISOGetUserDataItem MP4GetUserDataItem
#define ISOGetAtomFromUserData MP4GetAtomFromUserData
#define ISODeleteUserDataItem MP4DeleteUserDataItem
#define ISOGetUserDataTypeCount MP4GetUserDataTypeCount
#define ISONewUserData MP4NewUserData
#define ISOCreateTrackReader MP4CreateTrackReader
#define ISOSetMebxTrackReader MP4SetMebxTrackReader
#define ISODisposeTrackReader MP4DisposeTrackReader
#define ISONewHandle MP4NewHandle
#define ISOSetHandleSize MP4SetHandleSize
#define ISODisposeHandle MP4DisposeHandle
#define ISOGetHandleSize MP4GetHandleSize
#define ISOSetHandleOffset MP4SetHandleOffset
#define ISOUseSignedCompositionTimeOffsets MP4UseSignedCompositionTimeOffsets

#define QTPutMovieIntoHandle MP4PutMovieIntoHandle
#define MJ2PutMovieIntoHandle MP4PutMovieIntoHandle
#define ISOPutMovieIntoHandle MP4PutMovieIntoHandle

#define QTWriteMovieToFile MP4WriteMovieToFile
#define MJ2WriteMovieToFile MP4WriteMovieToFile
#define ISOWriteMovieToFile MP4WriteMovieToFile

#define ISOAddAtomToMovie MP4AddAtomToMovie
#define ISONewForeignAtom MP4NewForeignAtom
#define ISOGetForeignAtom MP4GetForeignAtom
#define ISONewUUIDAtom MP4NewUUIDAtom
#define ISOAddAtomToTrack MP4AddAtomToTrack

#define ISOGetTrackEditlistEntryCount MP4GetTrackEditlistEntryCount
#define ISOGetTrackEditlist MP4GetTrackEditlist

#define ISOGenericAtom MP4GenericAtom

#define MJ2TrackReaderGetNextSample MP4TrackReaderGetNextAccessUnit

  /*************************************************************************************************
   * These functions are specific to Motion JPEG-2; they have only the "MJ2" names.
   ************************************************************************************************/

  /**
   * @brief This sets the matrix of the overall movie.
   *
   * The default matrix is the unity matrix, if none has been set.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param matrix input matrix
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2SetMovieMatrix(ISOMovie theMovie, u32 matrix[9]);
  /**
   * @brief This returns the overall transformation matrix for the movie.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param matrix output matrix
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2GetMovieMatrix(ISOMovie theMovie, u32 outMatrix[9]);
  /**
   * @brief This sets the rate of the movie (the normal and default rate is 1.0).
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param rate input rate represented as a 16.16 fixed-point number
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2SetMoviePreferredRate(ISOMovie theMovie, u32 rate);
  /**
   * @brief This returns the currently set movie preferred rate.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param outRate output preferred rate
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2GetMoviePreferredRate(ISOMovie theMovie, u32 *outRate);
  /**
   * @brief This sets the normal volume of the movie.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param volume input volume value. The normal, default, value is 1.0. The volume is expressed as
   * an 8.8 fixed-point number.
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2SetMoviePreferredVolume(ISOMovie theMovie, s16 volume);
  /**
   * @brief This returns the movie volume setting.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param outVolume output volume
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2GetMoviePreferredVolume(ISOMovie theMovie, s16 *outVolume);
  /**
   * @brief This sets the overall transformation matrix for the movie as a whole.
   *
   * The matrix allows for 2D transformations; see the MJ2 specification for the details of how it
   * is applied. The default matrix is the unitary transform.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param matrix input matrix
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2SetTrackMatrix(ISOTrack theTrack, u32 matrix[9]);
  /**
   * @brief This returns the current matrix.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param outMatrix output matrix
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2GetTrackMatrix(ISOTrack theTrack, u32 outMatrix[9]);
  /**
   * @brief This sets the ordering of the visual tracks.
   *
   * It should be set if there is more than one visual track. Smaller numbers are closer to the
   * front.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param layer layer value
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2SetTrackLayer(ISOTrack theTrack, s16 layer);
  /**
   * @brief This returns the currently set track layer.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param outLayer output layer value
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2GetTrackLayer(ISOTrack theTrack, s16 *outLayer);
  /**
   * @brief This sets the width and height of a track.
   *
   * Note that this may be different from the width and height of the media; scaling may occur.
   * All MJ2 visual tracks should have this set explicitly.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param width input width
   * @param height input height
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2SetTrackDimensions(ISOTrack theTrack, u32 width, u32 height);
  /**
   * @brief This returns the currently set dimensions.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param outWidth output width
   * @param outHeight output height
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2GetTrackDimensions(ISOTrack theTrack, u32 *outWidth, u32 *outHeight);
  /**
   * @brief This sets the normal volume of the track.
   *
   * The normal, default, value is 1.0. The volume is expressed as an 8.8 fixed-point number.
   * Different audio tracks may have different volume settings; they are mixed for playback.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param volume input volume
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2SetTrackVolume(ISOTrack theTrack, s16 volume);
  /**
   * @brief Returns the currently set track volume.
   *
   * @ingroup Track
   * @param theTrack input track object
   * @param outVolume output volume
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2GetTrackVolume(ISOTrack theTrack, s16 *outVolume);
  /**
   * @brief Sets the graphics mode for this track.
   *
   * The mode should be chosen from the following list. The default mode is ISOGraphicsModeSrcCopy.
   *
   * ISOGraphicsModeSrcCopy: This video image will be copied over the top of the layers below it.
   * This is the default value, and should be used for the backmost track.
   *
   * ISOGraphicsModeTransparent: The color ISORGBColor in this video image will be treated as
   * transparent, allowing layers behind to be seen.
   *
   * ISOGraphicsModeAlpha: This video image includes an alpha plane to define its transparency.
   *
   * ISOGraphicsModeWhiteAlpha: This video image includes an alpha plane, which has been
   * premultiplied with white, to define its transparency.
   *
   * ISOGraphicsModeBlackAlpha: This video image includes an alpha plane, which has been
   * premultiplied with black, to define its transparency.
   *
   */
  ISO_EXTERN(ISOErr)
  MJ2SetMediaGraphicsMode(ISOMedia theMedia, u32 mode, const ISORGBColor *opColor);
  /**
   * @brief Returns the currently set graphics mode.
   */
  ISO_EXTERN(ISOErr)
  MJ2GetMediaGraphicsMode(ISOMedia theMedia, u32 *outMode, ISORGBColor *outOpColor);
  /**
   * @brief Sets the left-right balance of an audio track (normally a mono track).
   *
   * Balance values are represented as 16-bit, fixed-point numbers that range from -1.0 to +1.0. The
   * high-order 8 bits contain the integer portion of the value; the low-order 8 bits contain the
   * fractional part. Negative values weight the balance toward the left speaker; positive values
   * emphasize the right channel. Setting the balance to 0 (the default) corresponds to a neutral
   * setting.
   */
  ISO_EXTERN(ISOErr) MJ2SetMediaSoundBalance(ISOMedia theMedia, s16 balance);
  /**
   * @brief Returns the currently set balance value.
   */
  ISO_EXTERN(ISOErr) MJ2GetMediaSoundBalance(ISOMedia theMedia, s16 *outBalance);
  /**
   * @brief Creates a new empty Motion JPEG 2000 Movie in memory.
   * @ingroup Movie
   *
   * @param outMovie output movie object
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) MJ2NewMovie(ISOMovie *outMovie);

  /*************************************************************************************************
   * These functions are specific to 3GPP; they have only the "3GPP" names.
   ************************************************************************************************/

  /**
   * @brief Creates a new empty 3GPP Movie in memory, and sets the brand to the indicated release
   * (4, 5 or 6).
   * @ingroup Movie
   *
   * @param outMovie output movie object
   * @param release release brand (4, 5 or 6).
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) New3GPPMovie(ISOMovie *outMovie, u16 release);

  /*************************************************************************************************
   * These functions are specific to QuickTime; they have only the "QT" names.
   ************************************************************************************************/

  /**
   * @brief Creaets a new empty QT Movie in memory.
   */
  ISO_EXTERN(ISOErr) QTNewMovie(ISOMovie *outMovie);

  /*************************************************************************************************
   * These functions are general.
   ************************************************************************************************/

  /**
   * @brief Writes the in-memory Movie to a file.
   *
   * The file given by filename is created, written, and closed.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param filename file name
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) ISOWriteMovieToFile(ISOMovie theMovie, const char *filename);
  /**
   * @brief Sets the Movie’s major brand.
   *
   * Also inserts the major brand into the compatible brands list. This function is not normally
   * needed; the brand is set by the appropriate movie-creation function.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param brand brand value to set
   * @param minorversion minor version
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) ISOSetMovieBrand(ISOMovie theMovie, u32 brand, u32 minorversion);
  /**
   * @brief Adds a minor brand into the compatible brands list of the Movie.
   *
   * The following brands have defined constants in the headers, though of course you may use
   * MP4_FOUR_CHAR_CODE() also.
   *
   * Brand             | Description
   * ----------------- | ------------------------------
   * JP2JPEG2000Brand  | brand for JPEG-2000
   * MJ2JPEG2000Brand  | brand for Motion JPEG-2000
   * ISOQuickTimeBrand | brand for QuickTime
   * ISOMpeg4V1Brand   | brand for MPEG-4 version 1
   * ISOMpeg4V2Brand   | brand for MPEG-4 version 2
   * ISOISOBrand       | conforming brand for all files
   * ISOISO2Brand      | conforming brand for all files
   * ISO3GP4Brand      | 3GPP Release 4
   * ISO3GP5Brand      | 3GPP Release 5
   * ISO3GP6Brand      | 3GPP Release 6
   * ISOMPEG21Brand    | MPEG-21
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param brand brand value to set
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) ISOSetMovieCompatibleBrand(ISOMovie theMovie, u32 brand);
  /**
   * @brief Returns the Movie’s major brand and minor version.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param brand output brand
   * @param minorversion output minor version
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) ISOGetMovieBrand(ISOMovie theMovie, u32 *brand, u32 *minorversion);
  /**
   * @brief If the brand is a compatible brand of the movie, this returns MP4NoErr,
   * otherwise it returns MP4NotFoundErr.
   *
   * @ingroup Movie
   * @param theMovie input movie object
   * @param brand brand value to check
   * @return ISOErr error code
   */
  ISO_EXTERN(ISOErr) ISOIsMovieCompatibleBrand(ISOMovie theMovie, u32 brand);

  /*************************************************************************************************
   * AVC Sample descriptions
   ************************************************************************************************/

  /**
   * @brief Create a new AVC sample entry.
   * @ingroup SampleDescr
   *
   * @note You almost certainly will need to set the sample description width and height, after
   * calling this function.
   * @param length_size the size of the NAL Unit length field (and must be 1, 2 or 4). The value of
   * length_size = lengthSizeMinusOne + 1
   * @param first_sps The sequence parameter set (MUST be passed) used to get the profile, level,
   * etc. It will only be added to the configuration if a picture parameter set is also present.
   * @param first_pps picture parameter set. Can be NULL.
   * @param first_spsext sequence extension parameter set, only used if an appropriate profile is
   * used, of course. Can be NULL.
   */
  ISO_EXTERN(ISOErr)
  ISONewAVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                             u32 dataReferenceIndex, u32 length_size, MP4Handle first_sps,
                             MP4Handle first_pps, MP4Handle first_spsext);
  /**
   * @brief This adds another parameter set (which is not, in fact, inspected), to the
   * configuration.
   * @ingroup SampleDescr
   *
   * @param where can be AVCsps, AVCpps or AVCspsext.
   */
  ISO_EXTERN(ISOErr) ISOAddVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where);
  /**
   * @brief Gets the basic parameters of the AVC sample entry.
   * @ingroup SampleDescr
   */
  ISO_EXTERN(ISOErr)
  ISOGetAVCSampleDescription(MP4Handle sampleEntryH, u32 *dataReferenceIndex, u32 *length_size,
                             u32 *sps_count, u32 *pss_count, u32 *spsext_count);
  /**
   * @brief Gets an AVC parameter set, placing it in the given handle
   * @ingroup SampleDescr
   *
   * @param sampleEntryH input sample entry handle
   * @param ps output handle which is holding the parameter set.
   * @param where can be AVCsps, AVCpps or AVCspsext.
   * @param index the indexes start at 1 (1 is the first parameter set in the indicated array).
   */
  ISO_EXTERN(ISOErr)
  ISOGetVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index);
  /**
   * @brief Gets a HEVC parameter set, placing it in the given handle
   * @ingroup SampleDescr
   *
   * @param sampleEntryH input sample entry handle
   * @param ps output handle which is holding the parameter set.
   * @param where can be HEVCvps, HEVCsps or HEVCpps
   * @param index the indexes start at 1 (1 is the first parameter set in the indicated array).
   */
  ISO_EXTERN(ISOErr)
  ISOGetHEVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index);
  /**
   * @brief Gets a restricted video parameter set (AVC or HEVC), placing it in the given handle
   * @ingroup SampleDescr
   *
   * @param sampleEntryH input sample entry handle
   * @param ps output handle which is holding the parameter set.
   * @param where can be AVCsps, AVCpps, AVCspsext, HEVCvps, HEVCsps or HEVCpps
   * @param index the indexes start at 1 (1 is the first parameter set in the indicated array).
   */
  ISO_EXTERN(ISOErr)
  ISOGetRESVSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index);
  /**
   * @brief Get the NALUnitLength size in bytes
   * @ingroup SampleDescr
   */
  ISO_EXTERN(ISOErr) ISOGetNALUnitLength(MP4Handle sampleEntryH, u32 *out);
  /**
   * @brief Get the four character code of the original un-transformed sample entry
   * @ingroup SampleDescr
   * @todo rename the function to ISOGetOriginalFormat and parse the cinf and sinf as well
   */
  ISO_EXTERN(ISOErr) ISOGetRESVOriginalFormat(MP4Handle sampleEntryH, u32 *outOrigFmt);
  /**
   * @brief Get scheme_type and scheme_version from the SchemeTypeBox in resv
   * @ingroup SampleDescr
   * @param sampleEntryH resv sample entry data
   * @param schemeType [out] scheme_type 4CC
   * @param schemeVersion [out] scheme_version
   * @param schemeURI [out] scheme_uri
   */
  ISO_EXTERN(ISOErr)
  ISOGetRESVSchemeType(MP4Handle sampleEntryH, u32 *schemeType, u32 *schemeVersion,
                       char **schemeURI);
  /**
   * @brief Get the box from the SchemeInformationBox in resv
   * @ingroup SampleDescr
   * @param sampleEntryH resv sample entry data
   * @param atomType type of the atom inside SchemeInformationBox
   * @param outAtom [out] data of the found box
   */
  ISO_EXTERN(ISOErr)
  ISOGetRESVSchemeInfoAtom(MP4Handle sampleEntryH, u32 atomType, MP4Handle outAtom);
  /**
   * @brief Create a new HEVC sample entry.
   * @ingroup SampleDescr
   *
   * @param length_size the size of the NAL Unit length field (and must be 1, 2 or 4). The value of
   * length_size = lengthSizeMinusOne + 1
   * @param first_sps The sequence parameter set (MUST be passed) used to get the profile, level,
   * etc.
   * @param first_pps picture parameter set.
   * @param first_vps video parameter set.
   */
  ISO_EXTERN(ISOErr)
  ISONewHEVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                              u32 dataReferenceIndex, u32 length_size, MP4Handle first_sps,
                              MP4Handle first_pps, MP4Handle first_vps);

  /**
   * @brief Construct a new mebx sample entry
   * @ingroup SampleDescr
   *
   * @param outSE output sample entry object
   * @param dataReferenceIndex data reference index
   */
  ISO_EXTERN(ISOErr)
  ISONewMebxSampleDescription(struct MP4BoxedMetadataSampleEntry **outSE, u32 dataReferenceIndex);

  /**
   * @brief Add a new metadata type to mebx sample entry
   *
   * @param mebxSE input mebx sample entry
   * @param desired_local_key_id local key ID which you would like to have
   * @param out_local_key_id assigned local key ID. If available it will be the same as desired.
   * @param key_namespace Metadata key declaration box namespace
   * @param key_value Metadata key declaration box key value. Can be 0 if
   * key_namespace=MP4KeyNamespace_me4c
   * @param locale_string Metadata locale box string. If 0 then no 'loca' box is present.
   * @param setupInfo Metadata setup box data. If 0 then no 'setu' box is present.
   */
  ISO_EXTERN(ISOErr)
  ISOAddMebxMetadataToSampleEntry(struct MP4BoxedMetadataSampleEntry *mebxSE,
                                  u32 desired_local_key_id, u32 *out_local_key_id,
                                  u32 key_namespace, MP4Handle key_value, char *locale_string,
                                  MP4Handle setupInfo);

  ISO_EXTERN(ISOErr)
  ISOGetMebxHandle(struct MP4BoxedMetadataSampleEntry *mebxSE, MP4Handle sampleDescriptionH);

  /**
   * @brief Get the number of entries in the mebx sample entry
   *
   * @param sampleEntryH input sample entry of the mebx track
   * @param key_cnt number of local_key_id's
   */
  ISO_EXTERN(ISOErr)
  ISOGetMebxMetadataCount(MP4Handle sampleEntryH, u32 *key_cnt);

  ISO_EXTERN(ISOErr)
  ISOGetMebxMetadataConfig(MP4Handle sampleEntryH, u32 cnt, u32 *local_key_id, u32 *key_namespace,
                           MP4Handle key_value, char **locale_string, MP4Handle setupInfo);

  /*************************************************************************************************
   * VVC Sample descriptions
   ************************************************************************************************/
  /**
   * @brief Create a new VVC sample entry.
   * @ingroup SampleDescr
   *
   * @note implement me
   * @param length_size the size of the NAL Unit length field (and must be 1, 2 or 4). The value of
   * length_size = LengthSizeMinusOne + 1
   * @param first_sps The sequence parameter set (MUST be passed) used to get the profile, level,
   * etc. It will only be added to the configuration if a picture parameter set is also present.
   * @param other ps implement me
   */
  ISO_EXTERN(ISOErr)
  ISONewVVCSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                             u32 dataReferenceIndex, u32 length_size, MP4Handle first_sps,
                             MP4Handle first_pps);
  /**
   * @brief Gets the basic parameters of the VVC sample entry.
   * @ingroup SampleDescr
   *
   * @param sampleEntryH input sample entry handle
   * @param dataReferenceIndex output dataReferenceIndex
   * @param length_size output length size: the size of the NAL Unit length field (and must be 1, 2
   * or 4). The value of length_size = LengthSizeMinusOne + 1
   * @param naluType input NAL unit type (must be one of 12, 13, 14, 15, 16, 17, 23)
   * @param count output the number of nalus with type = @param naluType, if there is no nalu,
   * output 0
   */
  MP4_EXTERN(ISOErr)
  ISOGetVVCSampleDescription(MP4Handle sampleEntryH, u32 *dataReferenceIndex, u32 *length_size,
                             u32 naluType, u32 *count);
  /**
   * @brief Gets a VVC parameter set, placing it in the given handle
   * @ingroup SampleDescr
   *
   * @param sampleEntryH input sample entry handle
   * @param where can be VVC vps, sps, pps, dci, opi, prefix APS, prefix SEI.
   * @param num_nalus output the number of nalus in the corresponding parameter set.
   */
  ISO_EXTERN(ISOErr)
  ISOGetVVCNaluNums(MP4Handle sampleEntryH, u32 where, u32 *num_nalus);
  /**
   * @brief Gets a VVC parameter set, placing it in the given handle
   * @ingroup SampleDescr
   *
   * @param sampleEntryH input sample entry handle
   * @param ps output handle which is holding the parameter set.
   * @param where can be VVC vps, sps, pps, dci, opi, prefix APS, prefix SEI.
   * @param index the indexes start at 1 (1 is the first parameter set in the indicated array).
   */
  ISO_EXTERN(ISOErr)
  ISOGetVVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index);
  /**
   * @brief This adds another parameter set (which is not, in fact, inspected), to the
   * configuration.
   * @ingroup SampleDescr
   *
   * @param ps input handle which is saving the parameter set.
   * @param where can be VVC vps, sps, pps, dci, opi, prefix APS, prefix SEI.
   */
  MP4_EXTERN(ISOErr)
  ISOAddVVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where);
  /**
   * @brief Create a new VVC subpicture (vvs1) sample entry.
   * including a NALUconfigAtom (vvnC)
   * @ingroup SampleDescr
   *
   * @param width indicate the width of subpicture
   * @param height indicate the height of subpicture
   * @param length_size the size of the NAL Unit length field (and must be 1, 2 or 4). The value of
   * length_size = LengthSizeMinusOne + 1
   */
  MP4_EXTERN(ISOErr)
  ISONewVVCSubpicSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                                   u32 dataReferenceIndex, u32 width, u32 height, u32 length_size);
  /**
   * @brief Gets the length size of the VVC subpicture sample entry.
   * @ingroup SampleDescr
   *
   * @param sampleEntryH input sample entry handle
   * @param dataReferenceIndex output dataReferenceIndex
   * @param length_size output length size: the size of the NAL Unit length field (and must be 1, 2
   * or 4). The value of length_size = LengthSizeMinusOne + 1
   */
  MP4_EXTERN(ISOErr)
  ISOGetVVCSubpicSampleDescription(MP4Handle sampleEntryH, u32 *dataReferenceIndex,
                                   u32 *length_size);

  /*************************************************************************************************
   * 3GPP media
   ************************************************************************************************/
  /**
   * @brief Add a bitrate atom to a sample entry (description).
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err)
  ISOAddBitrateToSampleDescription(MP4Handle sampleEntryH, u8 is_3GPP, u32 buffersizeDB,
                                   u32 maxBitrate, u32 avgBitrate);
  /**
   * @brief Creates a new H.263 video sample description according to the 3GPP specifications.
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err)
  ISONewH263SampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                              u32 dataReferenceIndex, u32 vendor, u8 decoder_version, u8 H263_level,
                              u8 H263_profile);
  /**
   * @brief Creates a new AMR audio sample description according to the 3GPP specifications.
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err)
  ISONewAMRSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                             u32 dataReferenceIndex, u8 is_WB, u32 vendor, u8 decoder_version,
                             u16 mode_set, u8 mode_change_period, u8 frames_per_sample);
  /**
   * @brief Creates a new AMR wideband plus audio sample description according to the 3GPP spec.
   * @ingroup SampleDescr
   */
  MP4_EXTERN(MP4Err)
  ISONewAMRWPSampleDescription(MP4Track theTrack, MP4Handle sampleDescriptionH,
                               u32 dataReferenceIndex, u32 vendor, u8 decoder_version);

  /*************************************************************************************************
   * Meta Data
   ************************************************************************************************/
  /**
   * \defgroup Meta Meta data functions
   *
   * These functions allow you to manage meta boxes (atoms) at file, movie, and track level, and the
   * items they contain. The meta-type in the following functions can be a four-character code, or
   * one of the pre-defined codes:
   *
   * Type | four CC
   * ------|---------
   * MP7TextHandlerType | mp7t
   * 7BinaryHandlerType | mp7b
   * MP21HandlerType    | mp21
   * MP4NullHandlerType | null
   *
   * @{
   */
  /**
   * @brief Creates a new meta box (atom), with the indicated ‘handler’ type, at the file level.
   *
   * If there is already a meta box present, a additional meta box atom will be created and the new
   * meta box will be added inside.
   *
   * @param theMovie input movie object
   * @param metaType metadata type fourcc
   * @param meta output meta object
   */
  ISO_EXTERN(ISOErr) ISONewFileMeta(ISOMovie theMovie, u32 metaType, ISOMeta *meta);
  /**
   * @brief Creates a new meta box (atom), with the indicated ‘handler’ type, in the movie box
   * (atom).
   *
   * If there is already a meta box present, a additional meta box atom will be created and the new
   * meta box will be added inside.
   *
   * @param theMovie input movie object
   * @param metaType metadata type fourcc
   * @param meta output meta object
   */
  ISO_EXTERN(ISOErr) ISONewMovieMeta(ISOMovie theMovie, u32 metaType, ISOMeta *meta);
  /**
   * @brief Creates a new meta box (atom), with the indicated ‘handler’ type, in the indicated track
   * box (atom).
   *
   * If there is already a meta box present, a additional meta box atom will be created and the new
   * meta box will be added inside
   *
   * @param theTrack input track object
   * @param metaType metadata type fourcc
   * @param meta output meta object
   */
  ISO_EXTERN(ISOErr) ISONewTrackMeta(ISOTrack theTrack, u32 metaType, ISOMeta *meta);
  /**
   * @brief Adds a meta box relation box to the additional meta data box, related to first and
   * second meta box.
   *
   * @note Both meta boxes must live on the same level (file, movie, track).
   * @deprecated This box has been deprecated and is no longer defined in ISOBMFF
   */
  ISO_EXTERN(ISOErr)
  ISOAddMetaBoxRelation(ISOMeta first_meta, ISOMeta second_meta, u8 relation_type);
  /**
   * @brief Get the reletation type.
   *
   * @param relation_type will contain the type found in a meta box relation box or 0.
   */
  ISO_EXTERN(ISOErr)
  ISOGetMetaBoxRelation(ISOMeta first_meta, ISOMeta second_meta, u8 *relation_type);
  /**
   * @brief Adds a data reference to the given meta-data.
   *
   * The returned reference can later be passed to the ISOAddMetaItem().
   */
  ISO_EXTERN(ISOErr)
  ISOAddMetaDataReference(ISOMeta meta, u16 *out_ref, ISOHandle urlHandle, ISOHandle urnHandle);
  /**
   * @brief Creates a new meta-data item, and returns an opaque pointer to it.
   *
   * @param meta input meta object
   * @param outItem output pointer to created meta item object
   * @param base_offset should generally be zero. The software will increment this to reflect the
   * actual base offset of the item’s first extent in the file
   * @param data_ref_index can be zero (for items to be in the same file) or as returned by
   * ISOAddMetaDataReference().
   */
  ISO_EXTERN(ISOErr)
  ISOAddMetaItem(ISOMeta meta, ISOMetaItem *outItem, u64 base_offset, u16 data_ref_index);
  /**
   * @brief Creates a new meta-data item with specific ID, and returns an opaque pointer to it.
   * @see ISOAddMetaItem
   */
  ISO_EXTERN(ISOErr)
  ISOAddMetaItemWithID(ISOMeta meta, ISOMetaItem *outItem, u64 base_offset, u16 data_ref_index,
                       u16 item_ID);
  /**
   * @brief Adds some data to the given item, as its first (perhaps only) extent.
   *
   * Should generally be called only when the data reference for the item was supplied as zero. The
   * data will be added to the related mdat box.
   * @note If there are multiple extents for one item, their data must be added using the same
   * ISOAddItemExtent() function.
   */
  ISO_EXTERN(ISOErr) ISOAddItemExtent(ISOMetaItem item, MP4Handle data);
  /**
   * @brief Adds a reference to the data.
   *
   * Should generally be called only when a non-zero data reference was supplied for the item.
   * @note If there are multiple extents for one item, their data must be added using the same
   * ISOAddItemExtent() function.
   */
  ISO_EXTERN(ISOErr) ISOAddItemExtentReference(ISOMetaItem item, u64 offset, u64 length);
  /**
   * @brief Adds some data to the given item, as its first (perhaps only) extent.
   *
   * The data will be added to an item data box, which is contained in the meta data box.
   * @note If there are multiple extents for one item, their data must be added using the same
   * ISOAddItemExtent() function.
   */
  ISO_EXTERN(ISOErr) ISOAddItemExtentUsingItemData(ISOMetaItem item, MP4Handle data);
  /**
   * @brief Indicates that the data for this extent is located at another item’s extends.
   *
   * @note If there are multiple extents for one item, their data must be added using the same
   * ISOAddItemExtent() function.
   */
  ISO_EXTERN(ISOErr)
  ISOAddItemExtentItem(ISOMetaItem item, ISOMetaItem extent_item, u32 offset, u32 length);
  /**
   * @brief This function adds an item reference to the item reference box.
   *
   * If there is no existing item reference box, it will be created.
   *
   * @param outIndex will contain the position of the to_item_ID in all to_item_IDs of this item and
   * reference_type combination.
   */
  ISO_EXTERN(ISOErr)
  ISOAddItemReference(ISOMetaItem item, u32 reference_type, u32 to_item_ID, u32 *outIndex);
  /**
   * @brief This function adds multiple item references to the item reference box.
   *
   * If there is no existing item reference box, it will be created.
   * @note If there are already existing references of this item / reference_type combination, they
   * will be overwritten.
   */
  ISO_EXTERN(ISOErr)
  ISOAddItemReferences(ISOMetaItem item, u32 reference_type, u16 reference_count,
                       MP4Handle to_item_IDs);
  /**
   * @param fromItem The relation points from this item to the toItem
   * @param toItem The relation points to this item from the fromItem
   * @param relationType The 4cc relation type
   */
  MP4_EXTERN(MP4Err)
  ISOAddItemRelation(ISOMetaItem fromItem, ISOMetaItem toItem, u32 relationType);
  /**
   * @brief This function collects all item references of the item / reference_type combination.
   */
  ISO_EXTERN(ISOErr)
  ISOGetItemReferences(ISOMetaItem item, u32 reference_type, u16 *reference_count,
                       MP4Handle to_item_IDs);
  /**
   * @brief Get an item by reference type and index
   */
  ISO_EXTERN(ISOErr)
  ISOGetItemReference(ISOMetaItem item, u32 reference_type, u16 reference_index,
                      ISOMetaItem *outItem);
  /**
   * @brief Places the indicated data inside a box in the meta-box, with the indicated box type
   * (e.g. ISOXMLAtomType or ISOBinaryXMLAtomType).
   *
   * @param is_full_atom This flag can be used to insert the four bytes of version and flags
   * required in a full atom.
   */
  ISO_EXTERN(ISOErr) ISOAddPrimaryData(ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom);
  /**
   * @brief Finds the box of the given type inside the meta box, and returns its contents.
   *
   * @param is_full_atom can be used to skip the four bytes of version and flags of a full atom.
   */
  ISO_EXTERN(ISOErr) ISOGetPrimaryData(ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom);
  /**
   * @brief Finds the box of the given type inside the meta box, and returns its contents.
   *
   * Like ISOGetPrimaryData(), but probably more useful, as if a box of the given type does not
   * exist, finds the primary item and returns its data. Gives an error indication if neither the
   * box nor the primary item exist.
   */
  ISO_EXTERN(ISOErr)
  ISOGetPrimaryItemData(ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom);
  /**
   * @brief Sets the item of the indicated ID as the primary item (using a primary item box).
   */
  ISO_EXTERN(ISOErr) ISOSetPrimaryItem(ISOMeta meta, ISOMetaItem item);
  /**
   * @brief Finds the ID of the primary item.
   */
  ISO_EXTERN(ISOErr) ISOGetPrimaryItemID(ISOMeta meta, u16 *ID);
  /**
   * @brief Gets the ID of the indicated item.
   */
  ISO_EXTERN(ISOErr) ISOGetItemID(ISOMetaItem item, u16 *ID);
  /**
   * @brief Sets the information for the indicated item.
   *
   * @param protection_index is returned by ISONewMetaProtection()
   */
  ISO_EXTERN(ISOErr)
  ISOSetItemInfo(ISOMetaItem item, u16 protection_index, char *name, char *content_type,
                 char *content_encoding);
  /**
   * @brief Adds an item info extension to an existing item info entry.
   */
  ISO_EXTERN(ISOErr)
  ISOSetItemInfoExtension(ISOMetaItem item, MP4Handle extension, u32 extension_type);
  /**
   * @brief Gets an item info extension and its extension type from an item.
   */
  ISO_EXTERN(ISOErr)
  ISOGetItemInfoExtension(ISOMetaItem item, MP4Handle extension, u32 *extension_type);
  /**
   * @brief Adds an item info item type to an existing item info entry.
   * @param item_uri_type can be NULL, if the item_type is not ‘uri ‘
   */
  ISO_EXTERN(ISOErr) ISOSetItemInfoItemType(ISOMetaItem item, u32 item_type, char *item_uri_type);
  /**
   * @brief Hide item by setting the (flags & 1) = 1
   * @param item Item to hide
   */
  ISO_EXTERN(ISOErr) ISOHideItem(ISOMetaItem item);
  /**
   * @brief Check if item is hiden
   * @param item Item to check
   * @return MP4NoErr if item is hidden, MP4NotFoundErr if not, MP4InvalidMediaErr otherwise
   */
  ISO_EXTERN(ISOErr) ISOIsItemHidden(ISOMetaItem item);
  /**
   * @brief Gets an item info item type from an existing item info entry.
   * @param item_uri_type could be NULL, if the item_type is not ‘uri ‘.
   */
  ISO_EXTERN(ISOErr) ISOGetItemInfoItemType(ISOMetaItem item, u32 *item_type, char **item_uri_type);
  /**
   * @brief Gets a reference to the file-level meta data.
   *
   * The input parameter meta-type, if non-zero, can be used to check that the associated meta-data
   * is of the given type. The output always supplies the type.
   */
  ISO_EXTERN(ISOErr)
  ISOGetFileMeta(ISOMovie theMovie, ISOMeta *meta, u32 inMetaType, u32 *outMetaType);
  /**
   * @brief Gets a reference to the movie-level meta data.
   *
   * The input parameter meta-type, if non-zero, can be used to check that the associated meta-data
   * is of the given type. The output always supplies the type.
   */
  ISO_EXTERN(ISOErr)
  ISOGetMovieMeta(ISOMovie theMovie, ISOMeta *meta, u32 inMetaType, u32 *outMetaType);
  /**
   * @brief Gets a reference to the track-level meta data.
   *
   * The input parameter meta-type, if non-zero, can be used to check that the associated meta-data
   * is of the given type. The output always supplies the type.
   */
  ISO_EXTERN(ISOErr)
  ISOGetTrackMeta(ISOTrack theTrack, ISOMeta *meta, u32 inMetaType, u32 *outMetaType);
  /**
   * @brief Collects all items of a given type and presents the result in form of an array.
   */
  ISO_EXTERN(ISOErr)
  ISOGetAllItemsWithType(ISOMeta meta, u32 type, ISOMetaItem **items, u32 *numberOfItemsFound);
  /**
   * @brief Finds an item that has the associated name in its item information.
   */
  ISO_EXTERN(ISOErr) ISOFindItemByName(ISOMeta meta, ISOMetaItem *item, char *name, u8 exact_case);
  /**
   * @brief Gets a reference to an item by its ID.
   */
  ISO_EXTERN(ISOErr) ISOFindItemByID(ISOMeta meta, ISOMetaItem *item, u16 ID);
  /**
   * @brief Reads and returns the data associated with all the extents of the given item,
   * concatenating them.
   *
   * The returned base offset will be adjusted to represent the offset in the resulting handle
   * (which will generally match the value passed in to ISOAddMetaItem(), typically zero).
   */
  ISO_EXTERN(ISOErr) ISOGetItemData(ISOMetaItem item, MP4Handle data, u64 *base_offset);
  /**
   * @brief Returns the information for a given item.
   *
   * @note It is your responsibility to make sure the buffers are large enough for the strings.
   */
  ISO_EXTERN(ISOErr)
  ISOGetItemInfo(ISOMetaItem item, u16 *protection_index, char *name, char *content_type,
                 char *content_encoding);
  /**
   * @brief Checks that all the data references used by the meta-data are accessible.
   */
  ISO_EXTERN(MP4Err) ISOCheckMetaDataReferences(ISOMeta meta);
  /**
   * @brief Creates a new item protection information inside the given meta-data box.
   */
  ISO_EXTERN(MP4Err)
  ISONewMetaProtection(ISOMeta meta, u32 sch_type, u32 sch_version, char *sch_url,
                       u16 *protection_index);
  /**
   * @brief Adds an arbitrary atom to the scheme information of the associated protection
   * information.
   */
  ISO_EXTERN(MP4Err)
  ISOAddMetaProtectionInfo(ISOMeta meta, u16 protection_index, MP4GenericAtom schi_atom);
  /**
   * @brief Gets the scheme type, version, and URL associated with the protection information of the
   * given index.
   */
  ISO_EXTERN(MP4Err)
  ISOGetMetaProtection(ISOMeta meta, u16 protection_index, u32 *sch_type, u32 *sch_version,
                       char *sch_url);
  /**
   * @brief Finds and returns the atom of the given type inside the scheme information of the
   * protection scheme at the given index.
   */
  ISO_EXTERN(MP4Err)
  ISOGetMetaProtectionInfo(ISOMeta meta, u16 protection_index, u32 atom_type,
                           MP4GenericAtom *schi_atom);
  /**
   * @brief Adds an item property, which can be any MP4GenericAtom, to an ISOMetaItem.
   *
   * @param essential The essential flag can be set with 1 and unset with 0.
   */
  ISO_EXTERN(MP4Err)
  ISOAddMetaItemProperty(ISOMetaItem item, MP4GenericAtom *itemProperty, u8 essential);
  /**
   * @brief Retruns an array of MP4GenericAtom pointers, which represent the item properties found
   * for the item given.
   *
   * @param propertiesFound indicates the amount of items found and the size of the array, that is
   * returned.
   */
  ISO_EXTERN(MP4Err)
  ISOGetProperitesOfMetaItem(ISOMetaItem item, MP4GenericAtom **properties, u32 *propertiesFound);
  /**
   * @brief Add new EntityToGroupBox (creates grpl if needed)
   *
   * @param meta MetaBox where the new EntityToGroupBox should be added to
   * @param grouping_type grouping type
   * @param group_id group ID
   */
  ISO_EXTERN(ISOErr) ISONewEntityGroup(ISOMeta meta, u32 grouping_type, u32 group_id);
  /**
   * @brief Add entity_id to EntityToGroupBox
   *
   * @param meta MetaBox on which we are operating
   * @param group_id unique group ID to which we want to add the entity_id
   * @param entity_id entity ID value
   */
  ISO_EXTERN(ISOErr) ISOAddEntityIDToGroup(ISOMeta meta, u32 group_id, u32 entity_id);
  /**
   * @brief Get number of entries in the EntityToGroupBox
   *
   * @param meta MetaBox on which we are operating
   * @param group_id group ID
   * @param num_entities_in_group [out] number of entities in group
   */
  ISO_EXTERN(ISOErr) ISOGetEntityIDCnt(ISOMeta meta, u32 group_id, u32 *num_entities_in_group);
  /**
   * @brief Get common data for all EntityToGroup entries.
   *
   * @note Each call of this function allocates memory in pEntries and you are responsible to clean.
   *
   * @param meta MetaBox on which we are operating
   * @param pEntries [out] An array of EntityGroup entries with common data (id's, count, etc.)
   * @param cnt [out] Number of EntityGroup entries (size of EntityGroupEntryPtr array)
   */
  ISO_EXTERN(ISOErr)
  ISOGetEntityGroupEntries(ISOMeta meta, EntityGroupEntryPtr *pEntries, u32 *cnt);

  /** @}*/

#ifdef PRAGMA_EXPORT
#pragma export on
#endif

#ifdef __cplusplus
}
#endif
#ifdef PRAGMA_EXPORT
#pragma export off
#endif

#endif
