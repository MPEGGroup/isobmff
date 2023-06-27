#---------------------------------------------------------------------------------------------------
# libisomediafile top-level list
#---------------------------------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.9.2)
#---------------------------------------------------------------------------------------------------
project(libisomediafile)
#---------------------------------------------------------------------------------------------------

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_library(
    ${PROJECT_NAME}

    #-----------------------------------------------------------------------------------------------
    # Common header files
    #-----------------------------------------------------------------------------------------------
    src/FileMappingDataHandler.h
    src/FileMappingObject.h
    src/ISOMovies.h
    src/MdatDataHandler.h
    src/MJ2Atoms.h
    src/MovieTracks.h
    src/MP4Atoms.h
    src/MP4DataHandler.h
    src/MP4Descriptors.h
    src/MP4Impl.h
    src/MP4InputStream.h
    src/MP4IPMPXData.h
    src/MP4LinkedList.h
    src/MP4Movies.h
    src/MP4TrackReader.h

    #-----------------------------------------------------------------------------------------------
    # Common sources
    #-----------------------------------------------------------------------------------------------
    src/AdditionalMetaDataContainerAtom.c
    src/AMRSpecificInfoAtom.c
    src/AMRWPSpecificInfoAtom.c
    src/AudioSampleEntryAtom.c
    src/BaseDescriptor.c
    src/BitRateAtom.c
    src/ChannelLayoutAtom.c
    src/ChunkOffsetAtom.c
    src/ClockReferenceMediaHeader.c
    src/CompatibleSchemeTypeAtom.c
    src/CompositionToDecodeAtom.c
    src/CopyrightAtom.c
    src/DataEntryURLAtom.c
    src/DataEntryURNAtom.c
    src/DataInformationAtom.c
    src/DataReferenceAtom.c
    src/DecodingOffsetAtom.c
    src/DegradationPriorityAtom.c
    src/DownMixInstructionsAtom.c
    src/EditAtom.c
    src/EditListAtom.c
    src/EncAudioSampleEntryAtom.c
    src/EncVisualSampleEntryAtom.c
    src/ESDAtom.c
    src/ESUpdateCommand.c
    src/ExtendedLanguageTag.c
    src/FileMappingDataHandler.c
    src/FreeSpaceAtom.c
    src/GenericSampleEntryAtom.c
    src/H263SpecificInfoAtom.c
    src/HandlerAtom.c
    src/HEVCConfigAtom.c
    src/VVCConfigAtom.c
    src/VVCNALUConfigAtom.c
    src/HintMediaHeaderAtom.c
    src/IPMPToolUpdateCommand.c
    src/ISMAKMSAtom.c
    src/ISMASaltAtom.c
    src/ISMASampleFormatAtom.c
    src/ISMASecurity.c
    src/ISOMeta.c
    src/ISOSampleDescriptions.c
    src/ItemDataAtom.c
    src/ItemInfoAtom.c
    src/ItemInfoEntryAtom.c
    src/ItemLocationAtom.c
    src/ItemPropertiesAtom.c
    src/ItemPropertyAssociationAtom.c
    src/ItemPropertyContainerAtom.c
    src/ItemProtectionAtom.c
    src/ItemReferenceAtom.c
    src/LoudnessAtom.c
    src/LoudnessBaseAtom.c
    src/MdatDataHandler.c
    src/MediaAtom.c
    src/MediaDataAtom.c
    src/MediaHeaderAtom.c
    src/MediaInformationAtom.c
    src/MemoryFileMappingObject.c
    src/MetaAtom.c
    src/MetaboxRelationAtom.c
    src/MJ2BitsPerComponentAtom.c
    src/MJ2ColorSpecificationAtom.c
    src/MJ2FileTypeAtom.c
    src/MJ2HeaderAtom.c
    src/MJ2ImageHeaderAtom.c
    src/MJ2Movies.c
    src/MJ2SignatureAtom.c
    src/MovieAtom.c
    src/MovieExtendsAtom.c
    src/MovieFragmentAtom.c
    src/MovieFragmentHeaderAtom.c
    src/MovieHeaderAtom.c
    src/MovieTracks.c
    src/MP4Atoms.c
    src/MP4Commands.c
    src/MP4DataHandler.c
    src/MP4DecoderConfigDescriptor.c
    src/MP4DefaultCommand.c
    src/MP4DefaultDescriptor.c
    src/MP4Descriptors.c
    src/MP4ESDescriptor.c
    src/MP4ES_ID_IncDescriptor.c
    src/MP4ES_ID_RefDescriptor.c
    src/MP4FileMappingInputStream.c
    src/MP4Handle.c
    src/MP4InitialObjectDescriptor.c
    src/MP4InputStream.c
    src/MP4IPMPDescriptorPointer.c
    src/MP4IPMPInitialize.c
    src/MP4IPMPTool.c
    src/MP4IPMPToolDescriptor.c
    src/MP4IPMPToolList.c
    src/MP4IPMPX.c
    src/MP4IPMPXData.c
    src/MP4IPMPXDefaultData.c
    src/MP4LinkedList.c
    src/MP4Media.c
    src/MP4MemoryInputStream.c
    src/MP4MovieFile.c
    src/MP4Movies.c
    src/MP4ObjectDescriptor.c
    src/MP4MebxTrackReader.c
    src/MP4ODTrackReader.c
    src/MP4OrdinaryTrackReader.c
    src/MP4SLPacket.c
    src/MP4TrackReader.c
    src/MP4UserData.c
    src/MPEGMediaHeaderAtom.c
    src/MPEGSampleEntryAtom.c
    src/ObjectDescriptorAtom.c
    src/ObjectDescriptorMediaHeader.c
    src/ODUpdateCommand.c
    src/OriginalFormatAtom.c
    src/PaddingBitsAtom.c
    src/PCMConfigAtom.c
    src/PrimaryItemAtom.c
    src/ProducerReferenceTimeAtom.c
    src/QTMovies.c
    src/RestrictedSchemeInfoAtom.c
    src/RestrictedVideoSampleEntry.c
    src/SampleAuxiliaryInformationOffsetsAtom.c
    src/SampleAuxiliaryInformationSizesAtom.c
    src/SampleDependencyAtom.c
    src/SampleDescriptionAtom.c
    src/SampleGroupDescriptionAtom.c
    src/SampleSizeAtom.c
    src/SampleTableAtom.c
    src/SampleToChunkAtom.c
    src/SampleToGroupAtom.c
    src/SceneDescriptionMediaHeader.c
    src/SchemeInfoAtom.c
    src/SchemeTypeAtom.c
    src/SecurityInfoAtom.c
    src/SecuritySchemeAtom.c
    src/SegmentIndexAtom.c
    src/SegmentTypeAtom.c
    src/ShadowSyncAtom.c
    src/SingleItemTypeReferenceAtom.c
    src/SLConfigDescriptor.c
    src/SoundMediaHeaderAtom.c
    src/StereoVideoAtom.c
    src/StereoVideoGroupAtom.c
    src/SubSampleInformationAtom.c
    src/SubsegmentIndexAtom.c
    src/SyncSampleAtom.c
    src/TextMetaSampleEntry.c
    src/TimeToSampleAtom.c
    src/TrackAtom.c
    src/TrackExtendsAtom.c
    src/TrackExtensionPropertiesAtom.c
    src/TrackFragmentAtom.c
    src/TrackFragmentDecodeTimeAtom.c
    src/TrackFragmentHeaderAtom.c
    src/TrackFragmentRunAtom.c
    src/TrackGroupAtom.c
    src/TrackGroupTypeAtom.c
    src/TrackHeaderAtom.c
    src/TrackReferenceAtom.c
    src/TrackReferenceTypeAtom.c
    src/TrackTypeAtom.c
    src/UnknownAtom.c
    src/UserDataAtom.c
    src/VCConfigAtom.c
    src/VideoMediaHeaderAtom.c
    src/VisualSampleEntryAtom.c
    src/VolumetricVisualMediaHeaderAtom.c
    src/VolumetricVisualSampleEntryAtom.c
    src/HapticSampleEntryAtom.c
    src/XMLMetaSampleEntry.c
    src/BoxedMetadataSampleEntry.c
    src/MetadataKeyDeclarationBox.c
    src/MetadataKeyTableBox.c
    src/MetadataKeyBox.c
    src/MetadataLocaleBox.c
    src/MetadataSetupBox.c
    src/GroupsListBox.c
    src/EntityToGroupBox.c

    #-----------------------------------------------------------------------------------------------
    # Platform specific header
    #-----------------------------------------------------------------------------------------------

    # Linux
    $<$<PLATFORM_ID:Linux>:linux/MP4OSMacros.h>

    # Windows
    $<$<PLATFORM_ID:Windows>:w32/MP4OSMacros.h>

    # Mac
    $<$<PLATFORM_ID:Darwin>:macosx/MP4OSMacros.h>


    #-----------------------------------------------------------------------------------------------
    # Platform specific sources
    #-----------------------------------------------------------------------------------------------

    # Linux
    $<$<PLATFORM_ID:Linux>:linux/SimpleFileMappingObject.c linux/SimpleFileMappingObject.h>

    # Windows
    $<$<PLATFORM_ID:Windows>:w32/W32FileMappingObject.c w32/W32FileMappingObject.h>

    # Mac
    $<$<PLATFORM_ID:Darwin>:macosx/SimpleFileMappingObject.c macosx/SimpleFileMappingObject.h>

    ) # add_library


#---------------------------------------------------------------------------------------------------

target_include_directories(
    ${PROJECT_NAME}

    PUBLIC

    ${CMAKE_CURRENT_LIST_DIR}/src # Must be public because of ISOMovies.h

    # Linux
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_LIST_DIR}/linux>

    # Windows
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_LIST_DIR}/w32>

    # Mac
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_LIST_DIR}/macosx>

    )

#---------------------------------------------------------------------------------------------------

target_compile_options(
    ${PROJECT_NAME}

    PRIVATE

    # GNU compiler
    $<$<C_COMPILER_ID:GNU>:-Wall -W -ansi -pedantic -Wno-long-long -Wmissing-prototypes>

    # Visual Studio run time library
    $<$<AND:$<C_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/MTd>
    $<$<AND:$<C_COMPILER_ID:MSVC>,$<NOT:$<CONFIG:Debug>>>:/MT>
    )

#---------------------------------------------------------------------------------------------------

target_compile_definitions(
    ${PROJECT_NAME}

    PRIVATE

    # Linux
    $<$<PLATFORM_ID:Linux>:LINUX> # for strings.h

    # Windows
    $<$<PLATFORM_ID:Windows>:ISOMP4DLLAPI _CRT_SECURE_NO_WARNINGS>

    )

#---------------------------------------------------------------------------------------------------
