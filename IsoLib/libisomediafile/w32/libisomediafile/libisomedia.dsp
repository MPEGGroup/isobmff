# Microsoft Developer Studio Project File - Name="libisomedia" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=libisomedia - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libisomedia.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libisomedia.mak" CFG="libisomedia - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libisomedia - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "libisomedia - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libisomedia - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\w32" /I "..\..\w32" /I "..\..\src" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "ISOMP4DLLAPI" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "libisomedia - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug\obj"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\w32" /I "..\..\src" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "ISOMP4DLLAPI" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "libisomedia - Win32 Release"
# Name "libisomedia - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter ".c"
# Begin Source File

SOURCE=..\..\src\AudioSampleEntryAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\BaseDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ChunkOffsetAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ClockReferenceMediaHeader.c
# End Source File
# Begin Source File

SOURCE=..\..\src\CopyrightAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\DataEntryURLAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\DataEntryURNAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\DataInformationAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\DataReferenceAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\DecodingOffsetAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\DegradationPriorityAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\EditAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\EditListAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ESDAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ESUpdateCommand.c
# End Source File
# Begin Source File

SOURCE=..\..\src\FileMappingDataHandler.c
# End Source File
# Begin Source File

SOURCE=..\..\src\FreeSpaceAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\GenericSampleEntryAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\HandlerAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\HEVCConfigAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\HintMediaHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\IPMPToolUpdateCommand.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MdatDataHandler.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MediaAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MediaDataAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MediaHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MediaInformationAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MemoryFileMappingObject.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2BitsPerComponentAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2ColorSpecificationAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2FileTypeAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2HeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2ImageHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2Movies.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2SignatureAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieExtendsAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieFragmentAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieFragmentHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieTracks.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Atoms.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Commands.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4DataHandler.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4DecoderConfigDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4DefaultCommand.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4DefaultDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Descriptors.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4ES_ID_IncDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4ES_ID_RefDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4ESDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4FileMappingInputStream.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Handle.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4InitialObjectDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4InputStream.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPDescriptorPointer.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPInitialize.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPTool.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPToolDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPToolList.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPX.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPXData.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPXDefaultData.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4LinkedList.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Media.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4MemoryInputStream.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4MovieFile.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Movies.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4ObjectDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4ODTrackReader.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4OrdinaryTrackReader.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4SLPacket.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4TrackReader.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4UserData.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MPEGMediaHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MPEGSampleEntryAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ObjectDescriptorAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ObjectDescriptorMediaHeader.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ODUpdateCommand.c
# End Source File
# Begin Source File

SOURCE=..\..\src\PaddingBitsAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\QTMovies.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SampleDescriptionAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SampleSizeAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SampleTableAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SampleToChunkAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\SampleGroupDescriptionAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SampleToGroupAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\SceneDescriptionMediaHeader.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ShadowSyncAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SLConfigDescriptor.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SoundMediaHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\SyncSampleAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TimeToSampleAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackExtendsAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackFragmentAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackFragmentHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackFragmentRunAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackReferenceAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackReferenceTypeAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackGroupAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\TrackGroupTypeAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\UnknownAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\UserDataAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\VideoMediaHeaderAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\VisualSampleEntryAtom.c
# End Source File
# Begin Source File

SOURCE=..\W32FileMappingObject.c
# End Source File

# Begin Source File

SOURCE=..\..\src\SecuritySchemeAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\SecurityInfoAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\OriginalFormatAtom.c

# End Source File

# Begin Source File

SOURCE=..\..\src\SchemeInfoAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\EncVisualSampleEntryAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\EncAudioSampleEntryAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\ISMASampleFormatAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\ISMAKMSAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\ISMASaltAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\ISMASecurity.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ISOMeta.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ItemInfoAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ItemInfoEntryAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ItemLocationAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ItemProtectionAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\MetaAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\PrimaryItemAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\ISOSampleDescriptions.c
# End Source File
# Begin Source File

SOURCE=..\..\src\VCConfigAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\TextMetaSampleEntry.c
# End Source File
# Begin Source File

SOURCE=..\..\src\XMLMetaSampleEntry.c
# End Source File

# Begin Source File

SOURCE=..\..\src\BitRateAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\AMRWPSpecificInfoAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\AMRSpecificInfoAtom.c
# End Source File
# Begin Source File

SOURCE=..\..\src\H263SpecificInfoAtom.c
# End Source File

# Begin Source File

SOURCE=..\..\src\SampleDependencyAtom.c
# End Source File

# End Group
# Begin Group "Header Files"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=..\..\src\FileMappingDataHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\FileMappingObject.h
# End Source File
# Begin Source File

SOURCE=..\..\src\ISOMovies.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MdatDataHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MJ2Atoms.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MovieTracks.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Atoms.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4DataHandler.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Descriptors.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Impl.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4InputStream.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4IPMPXData.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4LinkedList.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4Movies.h
# End Source File
# Begin Source File

SOURCE=..\MP4OSMacros.h
# End Source File
# Begin Source File

SOURCE=..\..\src\MP4TrackReader.h
# End Source File
# Begin Source File

SOURCE=..\W32FileMappingObject.h
# End Source File

# End Group
# End Target
# End Project
