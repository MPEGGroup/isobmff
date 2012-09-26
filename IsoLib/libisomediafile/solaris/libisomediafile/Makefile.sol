# $Id: Makefile.sol,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $


VPATH = ..:../../src

CFLAGS = -I.. -I../../src -O -g -Wall

CC = cc

OBJS = \
	AudioSampleEntryAtom.o \
	BaseDescriptor.o \
	ChunkOffsetAtom.o \
	ClockReferenceMediaHeader.o \
	CopyrightAtom.o \
	DataEntryURLAtom.o \
	DataEntryURNAtom.o \
	DataInformationAtom.o \
	DataReferenceAtom.o \
	DecodingOffsetAtom.o \
	DegradationPriorityAtom.o \
	ESDAtom.o \
	ESUpdateCommand.o \
	EditAtom.o \
	EditListAtom.o \
	FileMappingDataHandler.o \
	FreeSpaceAtom.o \
	GenericSampleEntryAtom.o \
	HandlerAtom.o \
	HintMediaHeaderAtom.o \
	MemoryFileMappingObject.o \
	MdatDataHandler.o \
	MJ2BitsPerComponentAtom.o \
	MJ2ColorSpecificationAtom.o \
	MJ2FileTypeAtom.o \
	MJ2HeaderAtom.o \
	MJ2ImageHeaderAtom.o \
	MJ2Movies.o \
	MJ2SignatureAtom.o \
	MP4Atoms.o \
	MP4Commands.o \
	MP4DataHandler.o \
	MP4DecoderConfigDescriptor.o \
	MP4DefaultCommand.o \
	MP4DefaultDescriptor.o \
	MP4Descriptors.o \
	MP4ESDescriptor.o \
	MP4ES_ID_IncDescriptor.o \
	MP4ES_ID_RefDescriptor.o \
	MP4FileMappingInputStream.o \
	MP4Handle.o \
	MP4InitialObjectDescriptor.o \
	MP4InputStream.o \
	MP4LinkedList.o \
	MP4Media.o \
	MP4MemoryInputStream.o \
	MP4MovieFile.o \
	MP4Movies.o \
	MP4ODTrackReader.o \
	MP4ObjectDescriptor.o \
	MP4OrdinaryTrackReader.o \
	MP4SLPacket.o \
	MP4TrackReader.o \
	MP4UserData.o \
	MPEGMediaHeaderAtom.o \
	MPEGSampleEntryAtom.o \
	MediaAtom.o \
	MediaDataAtom.o \
	MediaHeaderAtom.o \
	MediaInformationAtom.o \
	MovieAtom.o \
	MovieHeaderAtom.o \
	MovieTracks.o \
	ODUpdateCommand.o \
	ObjectDescriptorAtom.o \
	ObjectDescriptorMediaHeader.o \
	QTMovies.o \
	SLConfigDescriptor.o \
	SampleDescriptionAtom.o \
	SampleSizeAtom.o \
	SampleTableAtom.o \
	SampleToChunkAtom.o \
	SceneDescriptionMediaHeader.o \
	ShadowSyncAtom.o \
	SimpleFileMappingObject.o \
	SoundMediaHeaderAtom.o \
	SyncSampleAtom.o \
	TimeToSampleAtom.o \
	TrackAtom.o \
	TrackHeaderAtom.o \
	TrackReferenceAtom.o \
	TrackReferenceTypeAtom.o \
	UnknownAtom.o \
	UserDataAtom.o \
	VideoMediaHeaderAtom.o \
	VisualSampleEntryAtom.o \
	TrackFragmentAtom.o \
	MovieFragmentAtom.o \
	TrackFragmentHeaderAtom.o \
	MovieExtendsAtom.o \
	TrackFragmentRunAtom.o \
	TrackExtendsAtom.o \
	MovieFragmentHeaderAtom.o \
	PaddingBitsAtom.o \
	IPMPToolUpdateCommand.o \
	MP4IPMPInitialize.o \
	MP4IPMPDescriptorPointer.o \
	MP4IPMPXData.o \
	MP4IPMPX.o \
	MP4IPMPXDefaultData.o \
	MP4IPMPToolList.o \
	MP4IPMPToolDescriptor.o \
	MP4IPMPTool.o \
	SecuritySchemeAtom.o \
	SecurityInfoAtom.o \
	OriginalFormatAtom.o \
	SchemeInfoAtom.o \
	EncVisualSampleEntryAtom.o \
	EncAudioSampleEntryAtom.o \
	ISMASampleFormatAtom.o \
	ISMASaltAtom.o \
	ISMAKMSAtom.o \
	ISMASecurity.o \
	SampleGroupDescriptionAtom.o \
	SampleToGroupAtom.o \
	ISOMeta.o \
	ItemInfoAtom.o \
	ItemInfoEntryAtom.o \
	ItemLocationAtom.o \
	ItemProtectionAtom.o \
	MetaAtom.o \
	PrimaryItemAtom.o \
	ISOSampleDescriptions.o \
	VCConfigAtom.o \
	TextMetaSampleEntry.o \
	XMLMetaSampleEntry.o \
	BitRateAtom.o \
	AMRWPSpecificInfoAtom.o \
	AMRSpecificInfoAtom.o \
	H263SpecificInfoAtom.o





libisomediafile.a : $(OBJS)
	ar r libisomediafile.a $(OBJS)
	-ranlib libisomediafile.a

$(OBJS) : $(@F:.o=.c)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	-rm $(OBJS) libisomediafile.a

