# $Id: Makefile,v 1.2 2002/09/27 13:41:22 julien Exp $


VPATH = .. ../../src

override CFLAGS += -I.. -I../../src -O -g -Wall -W -ansi -pedantic -Wno-long-long -Wmissing-prototypes -DLINUX

CC = gcc

sources = \
	AudioSampleEntryAtom.c \
	BaseDescriptor.c \
	ChunkOffsetAtom.c \
	ClockReferenceMediaHeader.c \
	CompatibleSchemeTypeAtom.c \
	CopyrightAtom.c \
	DataEntryURLAtom.c \
	DataEntryURNAtom.c \
	DataInformationAtom.c \
	DataReferenceAtom.c \
	DecodingOffsetAtom.c \
	DegradationPriorityAtom.c \
	ESDAtom.c \
	ESUpdateCommand.c \
	EditAtom.c \
	EditListAtom.c \
	FileMappingDataHandler.c \
	FreeSpaceAtom.c \
	GenericSampleEntryAtom.c \
	HandlerAtom.c \
	HEVCConfigAtom.c \
	HintMediaHeaderAtom.c \
	IPMPToolUpdateCommand.c \
	MemoryFileMappingObject.c \
	MdatDataHandler.c \
	MJ2BitsPerComponentAtom.c \
	MJ2ColorSpecificationAtom.c \
	MJ2FileTypeAtom.c \
	MJ2HeaderAtom.c \
	MJ2ImageHeaderAtom.c \
	MJ2Movies.c \
	MJ2SignatureAtom.c \
	MP4Atoms.c \
	MP4Commands.c \
	MP4DataHandler.c \
	MP4DecoderConfigDescriptor.c \
	MP4DefaultCommand.c \
	MP4DefaultDescriptor.c \
	MP4Descriptors.c \
	MP4ESDescriptor.c \
	MP4ES_ID_IncDescriptor.c \
	MP4ES_ID_RefDescriptor.c \
	MP4FileMappingInputStream.c \
	MP4Handle.c \
	MP4InitialObjectDescriptor.c \
	MP4InputStream.c \
	MP4IPMPDescriptorPointer.c \
	MP4IPMPInitialize.c \
	MP4IPMPTool.c \
	MP4IPMPToolDescriptor.c \
	MP4IPMPToolList.c \
	MP4IPMPX.c \
	MP4IPMPXData.c \
	MP4IPMPXDefaultData.c \
	MP4LinkedList.c \
	MP4Media.c \
	MP4MemoryInputStream.c \
	MP4MovieFile.c \
	MP4Movies.c \
	MP4ODTrackReader.c \
	MP4ObjectDescriptor.c \
	MP4OrdinaryTrackReader.c \
	MP4SLPacket.c \
	MP4TrackReader.c \
	MP4UserData.c \
	MPEGMediaHeaderAtom.c \
	MPEGSampleEntryAtom.c \
	MediaAtom.c \
	MediaDataAtom.c \
	MediaHeaderAtom.c \
	MediaInformationAtom.c \
	MovieAtom.c \
	MovieHeaderAtom.c \
	MovieTracks.c \
	ODUpdateCommand.c \
	ObjectDescriptorAtom.c \
	ObjectDescriptorMediaHeader.c \
	ProducerReferenceTimeAtom.c \
	PCMConfigAtom.c \
	QTMovies.c \
	RestrictedSchemeInfoAtom.c \
	RestrictedVideoSampleEntry.c \
	SLConfigDescriptor.c \
	SampleDescriptionAtom.c \
	SampleSizeAtom.c \
	SampleTableAtom.c \
	SampleToChunkAtom.c \
	SceneDescriptionMediaHeader.c \
	SchemeTypeAtom.c \
	SegmentIndexAtom.c \
	SegmentTypeAtom.c \
	SubsegmentIndexAtom.c \
	ShadowSyncAtom.c \
	SimpleFileMappingObject.c \
	SoundMediaHeaderAtom.c \
	StereoVideoAtom.c \
	StereoVideoGroupAtom.c \
	SubSampleInformationAtom.c \
	SyncSampleAtom.c \
	TimeToSampleAtom.c \
	TrackAtom.c \
	TrackHeaderAtom.c \
	TrackReferenceAtom.c \
	TrackReferenceTypeAtom.c \
	TrackTypeAtom.c \
	TrackGroupAtom.c \
	TrackGroupTypeAtom.c \
	UnknownAtom.c \
	UserDataAtom.c \
	VideoMediaHeaderAtom.c \
	VisualSampleEntryAtom.c \
	TrackFragmentAtom.c \
	MovieFragmentAtom.c \
	TrackFragmentHeaderAtom.c \
	MovieExtendsAtom.c \
	TrackFragmentRunAtom.c \
	TrackExtendsAtom.c \
	MovieFragmentHeaderAtom.c \
	PaddingBitsAtom.c \
	SecuritySchemeAtom.c \
	SecurityInfoAtom.c \
	OriginalFormatAtom.c \
	SchemeInfoAtom.c \
	EncVisualSampleEntryAtom.c \
	EncAudioSampleEntryAtom.c \
	ISMASampleFormatAtom.c \
	ISMAKMSAtom.c \
	ISMASaltAtom.c \
	ISMASecurity.c \
	SampleGroupDescriptionAtom.c \
	SampleToGroupAtom.c \
	ISOMeta.c \
	ItemInfoAtom.c \
	ItemInfoEntryAtom.c \
	ItemLocationAtom.c \
	ItemProtectionAtom.c \
	MetaAtom.c \
	PrimaryItemAtom.c \
	ISOSampleDescriptions.c \
	VCConfigAtom.c \
	TextMetaSampleEntry.c \
	XMLMetaSampleEntry.c \
	BitRateAtom.c \
	AMRWPSpecificInfoAtom.c \
	AMRSpecificInfoAtom.c \
	H263SpecificInfoAtom.c \
	SampleDependencyAtom.c \
	TrackFragmentDecodeTimeAtom.c \
	ExtendedLanguageTag.c \
	ChannelLayoutAtom.c \
	DownMixInstructionsAtom.c \
	LoudnessBaseAtom.c \
	LoudnessAtom.c \
	CompositionToDecodeAtom.c \
	TrackExtensionPropertiesAtom.c \
	SampleAuxiliaryInformationSizesAtom.c \
	SampleAuxiliaryInformationOffsetsAtom.c \
	AdditionalMetaDataContainerAtom.c \
	MetaboxRelationAtom.c \
	ItemReferenceAtom.c \
	ItemDataAtom.c \
	SingleItemTypeReferenceAtom.c \
	ItemPropertiesAtom.c \
	ItemPropertyAssociationAtom.c \
	ItemPropertyContainerAtom.c



objects := $(patsubst %.c,%.o,$(sources))

libisomediafile.a : $(objects)
	ar r libisomediafile.a $(objects)
	-ranlib libisomediafile.a

clean:
	-rm $(objects) $(sources:.c=.d) libisomediafile.a

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

TAGS: $(sources)
	etags $^

#
# Include all dependency files
#
-include $(sources:.c=.d)
