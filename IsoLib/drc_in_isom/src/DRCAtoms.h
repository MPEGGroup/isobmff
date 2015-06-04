/*
This software module was originally developed by Apple Computer, Inc.
in the course of development of MPEG-4. 
This software module is an implementation of a part of one or 
more MPEG-4 tools as specified by MPEG-4. 
ISO/IEC gives users of MPEG-4 free license to this
software module or modifications thereof for use in hardware 
or software products claiming conformance to MPEG-4.
Those intending to use this software module in hardware or software
products are advised that its use may infringe existing patents.
The original developer of this software module and his/her company,
the subsequent editors and their companies, and ISO/IEC have no
liability for use of this software module or modifications thereof
in an implementation.
Copyright is not released for non MPEG-4 conforming
products. Apple Computer, Inc. retains full right to use the code for its own
purpose, assign or donate the code to a third party and to
inhibit third parties from using the code for non 
MPEG-4 conforming products.
This copyright notice must be included in all copies or
derivative works. Copyright (c) 2014.
*/

/*!
 @header DRCAtoms
 DRCAtoms defines DRC specific atoms for the file format version of the DRC bitstream as defined
 in ISO/IEC DIS 23003-4.
 @copyright Apple
 @updated 2014-09-19
 @author Armin Trattnig
 @version 1.0
 */

#ifndef DRCAtoms_h
#define DRCAtoms_h

#include "MP4Atoms.h"

/*!
 @enum DRC Atoms
 @abstract Extends the list of atom types from MP4Atoms.h with atoms used with drc data
 @constant DRCUniDrcSampleEntryAtomType Type for DRCUniDrcSampleEntry. "unid"
 @constant DRCCoefficientBasicAtomType Type for DRCCoefficientBasic. "udc1"
 @constant DRCInstructionsBasicAtomType Type for DRCInstructionsBasic. "udi1"
 @constant DRCCoefficientUniDRCAtomType Type for DRCCoefficientUniDRC. "udc2"
 @constant DRCInstructionsUniDRCAtomType Type for DRCInstructionsUniDRC. "udi2"
 */
enum
{
    DRCTrackReferenceAtomType               = MP4_FOUR_CHAR_CODE( 'a', 'd', 'r', 'c' ),
    DRCUniDrcSampleEntryAtomType  			= MP4_FOUR_CHAR_CODE( 'u', 'n', 'i', 'd' ),
	DRCCoefficientBasicAtomType  			= MP4_FOUR_CHAR_CODE( 'u', 'd', 'c', '1' ),
	DRCInstructionsBasicAtomType			= MP4_FOUR_CHAR_CODE( 'u', 'd', 'i', '1' ),
    DRCCoefficientUniDRCAtomType 			= MP4_FOUR_CHAR_CODE( 'u', 'd', 'c', '2' ),
    DRCInstructionsUniDRCAtomType          	= MP4_FOUR_CHAR_CODE( 'u', 'd', 'i', '2' )
};

/*!
 * @brief Represents a DRCUniDrcSampleEntryAtom.
 * @field MP4_BASE_ATOM MP4Atoms.h macro to add all fields needed by a base atom
 * @field COMMON_SAMPLE_ENTRY_FIELDS Fields used by a generic sample entry
 */
typedef struct DRCUniDrcSampleEntryAtom
{
    MP4_BASE_ATOM
    COMMON_SAMPLE_ENTRY_FIELDS
} DRCUniDrcSampleEntryAtom;

/*!
 * @brief Represents a DRCCoefficientBasicAtom.
 * @field MP4_FULL_ATOM MP4Atoms.h macro to add all fields needed by a full atom
 * @field reserved bit(4) reserved = 0;
 * @field DRC_location signed int(5) DRC_location;
 * @field DRC_characteristic unsigned int(7) DRC_characteristic;
 */
typedef struct DRCCoefficientBasicAtom
{
	MP4_FULL_ATOM
	u8      reserved;               // bit(4) = 0
    s8      DRC_location;           // int(5)
    u8      DRC_characteristic;     // uint(7)
} DRCCoefficientBasicAtom;

/*!
 * @brief Part of DRCCoefficientUniDRCSequence
 * @field reserved bit(1) reserved = 0;
 * @field DRC_characteristic unsigned int(7) DRC_characteristic;
 */
typedef struct DRCCoefficientUniDRCSequenceBandCharacteristic
{
	u8      reserved;               // bit(1) = 0
    u8      DRC_characteristic;     // uint(7)
} DRCCoefficientUniDRCSequenceBandCharacteristic;

/*!
 * @brief Part of DRCCoefficientUniDRCSequence
 * @field reserved bit(4) reserved = 0;
 * @field crossover_freq_index unsigned int(4) crossover_freq_index;
 * @field reserved bit(6) reserved = 0;
 * @field start_sub_band_index unsigned int(10) start_sub_band_index;
 */
typedef struct DRCCoefficientUniDRCSequenceBandIndex
{
	u8      reserved1;               // bit(4) = 0
    u8      crossover_freq_index;    // uint(4)
    u8      reserved2;               // bit(6) = 0
    u16     start_sub_band_index;    // uint(10)
} DRCCoefficientUniDRCSequenceBandIndex;

/*!
 * @brief Part of DRCCoefficientUniDRCAtom
 * @field reserved1 bit(2) reserved = 0;
 * @field gain_coding_profile unsigned int(2) gain_coding_profile;
 * @field gain_interpolation_type unsigned int(1) gain_interpolation_type;
 * @field full_frame unsigned int(1) full_frame;
 * @field time_alignment unsigned int(1) time_alignment;
 * @field time_delta_min_present bit(1) time_delta_min_present;
 * @field reserved2 bit(5) reserved = 0;
 * @field bs_time_delta_min unsigned int(11) bs_time_delta_min;
 * @field reserved3 bit(3) reserved = 0;
 * @field band_count unsigned int(4) band_count; // must be >= 1
 * @field drc_band_type unsigned int(1) drc_band_type;
 * @field bandCharacteristics List of DRCCoefficientUniDRCSequenceBandCharacteristics
 * @field bandIndexes List of DRCCoefficientUniDRCSequenceBandIndex
 */
typedef struct DRCCoefficientUniDRCSequence
{
	u8              reserved1;                  // bit(2) = 0
    u8              gain_coding_profile;        // uint(2)
    u8              gain_interpolation_type;    // uint(1)
    u8              full_frame;                 // uint(1)
    u8              time_alignment;             // uint(1)
    u8              time_delta_min_present;     // bit(1)
    u8              reserved2;                  // bit(5) = 0
    u16             bs_time_delta_min;          // uint(11)
    u8              reserved3;                  // bit(3) = 0
    u8              band_count;                 // uint(4)
    u8              drc_band_type;              // uint(1)
    MP4LinkedList   bandCharacteristics;        // List of DRCCoefficientUniDRCSequenceBandCharacteristics
    MP4LinkedList   bandIndexes;                // List of DRCCoefficientUniDRCSequenceBandIndex
} DRCCoefficientUniDRCSequence;

/*!
 * @brief Represents a DRCCoefficientUniDRCAtom
 * @field MP4_FULL_ATOM MP4Atoms.h macro to add all fields needed by a full atom
 * @field reserved1 bit(2) reserved = 0;
 * @field DRC_location signed int(5) DRC_location;
 * @field drc_frame_size_present bit(1) drc_frame_size_present;
 * @field reserved2 bit(1) reserved = 0;
 * @field bs_drc_frame_size unsigned int(15) bs_drc_frame_size;
 * @field reserved3 bit(2) reserved = 0;
 * @field sequence_count unsigned int(6) sequence_count;
 * @field sequences List of DRCCoefficientUniDRCSequences
 */
typedef struct DRCCoefficientUniDRCAtom
{
	MP4_FULL_ATOM
	u8              reserved1;                  // bit(2) = 0
    s8              DRC_location;               // int(5)
    u8              drc_frame_size_present;     // bit(1)
    u8              reserved2;                  // bit(1)
    u16             bs_drc_frame_size;          // uint(15)
    u8              reserved3;                  // bit(1) = 0
    u8              delayMode;                  // bit(1)
    u8              sequence_count;             // uint(6)
    MP4LinkedList   sequences;                  // List of DRCCoefficientUniDRCSequences
} DRCCoefficientUniDRCAtom;

/*!
 * @brief Part of DRCInstructionsBasicAtom and DRCInstructionsUniDRCAtom
 * @field reserved bit(1) reserved = 0;
 * @field additional_dowmix_ID unsigned int(7) additional_dowmix_ID;
 */
typedef struct DRCInstructionsAdditionalDownMixID
{
	u8      reserved;               //  bit(1) = 0
    u8      additional_dowmix_ID;   //  uint(7)
} DRCInstructionsAdditionalDownMixID;

/*!
 * @brief Represents a DRCInstructionsBasicAtom
 * @field MP4_FULL_ATOM MP4Atoms.h macro to add all fields needed by a full atom
 * @field reserved1 bit(3) reserved = 0;
 * @field DRC_set_ID unsigned int(6) DRC_set_ID;
 * @field DRC_location signed int(5) DRC_location;
 * @field downmix_ID unsigned int(7) downmix_ID;
 * @field additional_dowmix_ID_count unsigned int(3) additional_downmix_ID_count;
 * @field additionalDownMixIDs List of DRCInstructionsAdditionalDownMixIDs
 * @field DRC_set_effect bit(16) DRC_set_effect;
 * @field reserved2 bit(7) reserved = 0;
 * @field limiter_peak_target_present bit(1) limiter_peak_target_present;
 * @field bs_limiter_peak_target unsigned int(8) bs_limiter_peak_target;
 * @field reserved3 bit(7) reserved = 0;
 * @field DRC_set_target_loudness_present bit(1) DRC_set_target_loudness_present;
 * @field reserved4 bit(4) reserved = 0;
 * @field bs_DRC_set_target_loudness_value_upper unsigned int(6) bs_DRC_set_target_loudness_value_upper;
 * @field bs_DRC_set_target_loudness_value_lower unsigned int(6) bs_DRC_set_target_loudness_value_lower;
 */
typedef struct DRCInstructionsBasicAtom
{
	MP4_FULL_ATOM
	u8              reserved1;                                  // bit(3) = 0
    u8              DRC_set_ID;                                 // uint(6)
    s8              DRC_location;                               // int(5)
    u8              downmix_ID;                                 // uint(7)
    u8              additional_dowmix_ID_count;                 // uint(3)
    MP4LinkedList   additionalDownMixIDs;                       // List of DRCInstructionsAdditionalDownMixIDs
    u16             DRC_set_effect;                             // bit(16)
    u8              reserved2;                                  // bit(7) = 0
    u8              limiter_peak_target_present;                // bit(1)
    u8              bs_limiter_peak_target;                     // uint(8)
    u8              reserved3;                                  // bit(7) = 0
    u8              DRC_set_target_loudness_present;            // bit(1)
    u8              reserved4;                                  // bit(4) = 0
    u8              bs_DRC_set_target_loudness_value_upper;     // uint(6)
    u8              bs_DRC_set_target_loudness_value_lower;     // uint(6)
} DRCInstructionsBasicAtom;

/*!
 * @brief Part of DRCInstructionsUniDRCAtom
 * @field reserved bit(2) reserved = 0;
 * @field bs_sequence_index unsigned int(6) bs_sequence_index;
 */
typedef struct DRCInstructionsGroupIndexPerChannel
{
    u8      channel_group_index;    //  uint(8)
} DRCInstructionsGroupIndexPerChannel;

/*!
 * @brief Part of DRCInstructionsUniDRCAtom
 * @field reserved bit(2) reserved = 0;
 * @field bs_sequence_index unsigned int(6) bs_sequence_index;
 */
typedef struct DRCInstructionsSequenceIndexOfChannelGroup
{
    u8      reserved;               //  bit(2) = 0
    u8      bs_sequence_index;      //  uint(6)
} DRCInstructionsSequenceIndexOfChannelGroup;

/*!
 * @brief Part of DRCInstructionsUniDRCAtom
 * @field reserved1 bit(7) reserved = 0;
 * @field ducking_scaling_present bit(1) ducking_scaling_present;
 * @field reserved2 bit(4) reserved = 0;
 * @field bs_ducking_scaling bit(4) bs_ducking_scaling;
 */
typedef struct DRCInstructionsChannelGroupDuckingScaling
{
	u8      reserved1;                      //  bit(7) = 0
    u8      ducking_scaling_present;        //  bit(1)
    u8      reserved2;                      //  bit(4) = 0
    u8      bs_ducking_scaling;             //  bit(4)
} DRCInstructionsChannelGroupDuckingScaling;

/*!
 * @brief Part of DRCInstructionsUniDRCAtom
 * @field reserved1 bit(7) reserved = 0;
 * @field gain_scaling_present bit(1) gain_scaling_present;
 * @field bs_attenuation_scaling uint(4) bs_attenuation_scaling = 0;
 * @field bs_amplification_scaling uint(4) bs_amplification_scaling;
 * @field reserved2 bit(7) reserved = 0;
 * @field gain_offset_present bit(1) gain_offset_present;
 * @field reserved3 bit(2) reserved = 0;
 * @field bs_gain_offset bit(6) bs_gain_offset;
 */
typedef struct DRCInstructionsChannelGroupGainScaling
{
	u8      reserved1;                      //  bit(7) = 0
    u8      gain_scaling_present;           //  bit(1)
    u8      bs_attenuation_scaling;         //  uint(4)
    u8      bs_amplification_scaling;       //  uint(4)
    u8      reserved2;                      //  bit(7) = 0
    u8      gain_offset_present;            //  bit(1)
    u8      reserved3;                      //  bit(2)
    u8      bs_gain_offset;                 //  bit(6)
} DRCInstructionsChannelGroupGainScaling;

/*!
 * @brief Represents a DRCInstructionsUniDRCAtom
 * @field MP4_FULL_ATOM MP4Atoms.h macro to add all fields needed by a full atom
 * @field reserved1 bit(3) reserved = 0;
 * @field DRC_set_ID unsigned int(6) DRC_set_ID;
 * @field DRC_location signed int(5) DRC_location;
 * @field downmix_ID unsigned int(7) downmix_ID;
 * @field additional_dowmix_ID_count unsigned int(3) additional_downmix_ID_count;
 * @field additionalDownMixIDs List of DRCInstructionsAdditionalDownMixIDs
 * @field DRC_set_effect bit(16) DRC_set_effect;
 * @field reserved2 bit(7) reserved = 0;
 * @field limiter_peak_target_present bit(1) limiter_peak_target_present;
 * @field bs_limiter_peak_target unsigned int(8) bs_limiter_peak_target;
 * @field reserved3 bit(7) reserved = 0;
 * @field DRC_set_target_loudness_present bit(1) DRC_set_target_loudness_present;
 * @field reserved4 bit(4) reserved = 0;
 * @field bs_DRC_set_target_loudness_value_upper unsigned int(6) bs_DRC_set_target_loudness_value_upper;
 * @field bs_DRC_set_target_loudness_value_lower unsigned int(6) bs_DRC_set_target_loudness_value_lower;
 * @field reserved5 bit(1) reserved = 0;
 * @field depends_on_DRC_set unsigned int(6) depends_on_DRC_set;
 * @field no_independent_use bit(1) no_independent_use;
 * @field reserved6 bit(1) reserved = 0;
 * @field channel_count unsigned int(8) channel_count;
 * @field groupIndexesPerChannels List of DRCInstructionsGroupIndexPerChannel
 * @field sequenceIndexesOfChannelGroups List of DRCInstructionsSequenceIndexOfChannelGroup
 * @field channel_group_count unsigned int(8) channel_group_count;
 * @field channelGroupDuckingScalings List of DRCInstructionsChannelGroupDuckingScalings
 * @field channelGroupGainScalings List of DRCInstructionsChannelGroupGainScaling
 */
typedef struct DRCInstructionsUniDRCAtom
{
	MP4_FULL_ATOM
	u8              reserved1;                                  // bit(3) = 0
    u8              DRC_set_ID;                                 // uint(6)
    s8              DRC_location;                               // int(5)
    u8              downmix_ID;                                 // uint(7)
    u8              additional_dowmix_ID_count;                 // uint(3)
    MP4LinkedList   additionalDownMixIDs;                       // List of DRCInstructionsAdditionalDownMixIDs
    u16             DRC_set_effect;                             // bit(16)
    u8              reserved2;                                  // bit(7) = 0
    u8              limiter_peak_target_present;                // bit(1)
    u8              bs_limiter_peak_target;                     // uint(8)
    u8              reserved3;                                  // bit(7) = 0
    u8              DRC_set_target_loudness_present;            // bit(1)
    u8              reserved4;                                  // bit(4) = 0
    u8              bs_DRC_set_target_loudness_value_upper;     // uint(6)
    u8              bs_DRC_set_target_loudness_value_lower;     // uint(6)
    u8              reserved5;                                  // bit(1) = 0
    u8              depends_on_DRC_set;                         // uint(3)
    u8              no_independent_use;                         // bit(1)
    u8              reserved6;                                  // bit(1) = 0
    u8              channel_count;                              // uint(8)
    MP4LinkedList   groupIndexesPerChannels;                    // List of DRCInstructionsGroupIndexPerChannel
    MP4LinkedList   sequenceIndexesOfChannelGroups;             // List of DRCInstructionsSequenceIndexOfChannelGroup
    MP4LinkedList   channelGroupDuckingScalings;                // List of DRCInstructionsChannelGroupDuckingScalings
    MP4LinkedList   channelGroupGainScalings;                   // List of DRCInstructionsChannelGroupGainScaling
} DRCInstructionsUniDRCAtom;


/*!
 * @brief Points to a DRCUniDrcSampleEntryAtom structure
 */
typedef struct DRCUniDrcSampleEntryAtom      *DRCUniDrcSampleEntryAtomPtr;
/*!
 * @brief Points to a DRCCoefficientBasicAtom structure
 */
typedef struct DRCCoefficientBasicAtom      *DRCCoefficientBasicAtomPtr;

/*!
 * @brief Points to a DRCCoefficientUniDRCAtom structure
 */
typedef struct DRCCoefficientUniDRCAtom     *DRCCoefficientUniDRCAtomPtr;

/*!
 * @brief Points to a DRCInstructionsBasicAtom structure
 */
typedef struct DRCInstructionsBasicAtom     *DRCInstructionsBasicAtomPtr;

/*!
 * @brief Points to a DRCInstructionsUniDRCAtom structure
 */
typedef struct DRCInstructionsUniDRCAtom    *DRCInstructionsUniDRCAtomPtr;


/*!
 * @discussion Creates and initializes a atom structure with a certain type, memory will be allocated
 * @param atomType Type of the atom, which will be created
 * @param outAtom pointer to a structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreateDRCAtom                ( u32 atomType, MP4AtomPtr *outAtom );

/*!
 * @discussion Reads an atom from inputStream. If type of atom not in protolist it fails.
 * @param inputStream MP4InputStreamPtr (can be found in libisomediafile.a). Data will be read from this stream.
 * @param protoList List of atoms, which are valid for being parsed. Can be NULL.
 * @param defaultAtom Prototype of parsed atom. Can be NULL.
 * @param outAtom pointer to a structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4ParseDRCAtomUsingProtoList   ( MP4InputStreamPtr inputStream, u32* protoList, u32 defaultAtom, MP4AtomPtr *outAtom );

/*!
 * @discussion Reads an atom from inputStream.
 * @param inputStream MP4InputStreamPtr (can be found in libisomediafile.a). Data will be read from this stream.
 * @param outAtom pointer to a structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4ParseDRCAtom                 ( MP4InputStreamPtr inputStream, MP4AtomPtr *outAtom );


/*!
 * @discussion Creates and initializes a MP4AudioSampleEntryAtom structure, memory will be allocated
 * @param outAtom pointer to MP4AudioSampleEntryAtom structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreateDrcAudioSampleEntryAtom     ( MP4AudioSampleEntryAtomPtr            *outAtom );
/*!
 * @discussion Creates and initializes a DRCUniDrcSampleEntryAtom structure, memory will be allocated
 * @param outAtom pointer to DRCUniDrcSampleEntryAtom structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreateDRCUniDrcSampleEntryAtom    ( DRCUniDrcSampleEntryAtomPtr           *outAtom );
/*!
 * @discussion Creates and initializes a DRCCoefficientBasicAtom structure, memory will be allocated
 * @param outAtom pointer to DRCCoefficientBasicAtom structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreateDRCCoefficientBasicAtom     ( DRCCoefficientBasicAtomPtr            *outAtom );
/*!
 * @discussion Creates and initializes a DRCCoefficientUniDRCAtom structure, memory will be allocated
 * @param outAtom pointer to DRCCoefficientUniDRCAtom structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreateDRCCoefficientUniDRCAtom    ( DRCCoefficientUniDRCAtomPtr           *outAtom );
/*!
 * @discussion Creates and initializes a DRCInstructionsBasicAtom structure, memory will be allocated
 * @param outAtom pointer to DRCInstructionsBasicAtom structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreateDRCInstructionsBasicAtom    ( DRCInstructionsBasicAtomPtr           *outAtom );
/*!
 * @discussion Creates and initializes a DRCInstructionsUniDRCAtom structure, memory will be allocated
 * @param outAtom pointer to DRCInstructionsUniDRCAtom structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreateDRCInstructionsUniDRCAtom   ( DRCInstructionsUniDRCAtomPtr          *outAtom );

#endif