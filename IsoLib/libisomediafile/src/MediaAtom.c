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
derivative works. Copyright (c) 1999.
*/
/*
  $Id: MediaAtom.c,v 1.2 2002/10/01 12:49:19 julien Exp $
*/

#include "MP4Atoms.h"
#include "MP4Descriptors.h"
#include <stdlib.h>
#include <string.h>

MP4Err MP4ParseODFrame(struct MP4MediaAtom *self, MP4Handle sampleH, MP4Handle sizesH);

MP4Err storeIPMPDescriptorPointers(MP4DescriptorPtr desc, MP4TrackReferenceTypeAtomPtr mpod);

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4MediaAtomPtr self;
  u32 i;
  err  = MP4NoErr;
  self = (MP4MediaAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  DESTROY_ATOM_LIST

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err calculateDuration(struct MP4MediaAtom *self)
{
  MP4Err err;
  u32 mediaDuration;
  MP4MediaInformationAtomPtr minf;
  MP4MediaHeaderAtomPtr mdhd;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  if(minf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
  mdhd = (MP4MediaHeaderAtomPtr)self->mediaHeader;
  if(mdhd == NULL) BAILWITHERROR(MP4InvalidMediaErr);
  err = minf->getMediaDuration(minf, &mediaDuration);
  if(err) goto bail;
  mdhd->duration = mediaDuration;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mdatMoved(struct MP4MediaAtom *self, u64 mdatBase, u64 mdatEnd, s32 mdatOffset)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;
  err = MP4NoErr;

  minf = (MP4MediaInformationAtomPtr)self->information;
  if(minf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
  err = minf->mdatMoved(minf, mdatBase, mdatEnd, mdatOffset);
  if(err) goto bail;
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err addAtom(MP4MediaAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  if(atom == NULL) BAILWITHERROR(MP4BadParamErr);
  err = MP4AddListEntry(atom, self->atomList);
  if(err) goto bail;
  switch(atom->type)
  {
  case MP4MediaHeaderAtomType:
    if(self->mediaHeader) BAILWITHERROR(MP4BadDataErr)
    self->mediaHeader = atom;
    break;

  case MP4ExtendedLanguageTagAtomType:
    if(self->extendedLanguageTag) BAILWITHERROR(MP4BadDataErr)
    self->extendedLanguageTag = atom;
    break;

  case MP4HandlerAtomType:
    if(self->handler) BAILWITHERROR(MP4BadDataErr)
    self->handler = atom;
    break;

  case MP4MediaInformationAtomType:
    if(self->information) BAILWITHERROR(MP4BadDataErr)
    self->information = atom;
    break;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setupNew(struct MP4MediaAtom *self, MP4AtomPtr track, u32 mediaType, u32 timeScale,
                       MP4Handle dataHandlerH)
{
  MP4Err MP4CreateMediaHeaderAtom(MP4MediaHeaderAtomPtr * outAtom);
  MP4Err MP4CreateMediaInformationAtom(MP4MediaInformationAtomPtr * outAtom);
  MP4Err MP4CreateHandlerAtom(MP4HandlerAtomPtr * outAtom);
  void MP4TypeToString(u32 inType, char *ioStr);

  char name[8];
  MP4Err err;
  u64 currentTime;
  MP4MediaHeaderAtomPtr mdhd;
  MP4MediaInformationAtomPtr minf;
  MP4HandlerAtomPtr hdlr;
  MP4AtomPtr mdat;

  err              = MP4NoErr;
  self->mediaTrack = track;
  mdat             = ((MP4TrackAtomPtr)track)->mdat;
  err              = MP4CreateMediaHeaderAtom(&mdhd);
  if(err) goto bail;
  err = MP4CreateMediaInformationAtom(&minf);
  if(err) goto bail;
  err = MP4CreateHandlerAtom(&hdlr);
  if(err) goto bail;

  err = MP4GetCurrentTime(&currentTime);
  if(err) goto bail;
  mdhd->timeScale        = timeScale;
  mdhd->creationTime     = currentTime;
  mdhd->modificationTime = currentTime;
  mdhd->packedLanguage   = 21956;

  hdlr->handlerType = mediaType;
  MP4TypeToString(mediaType, name);
  hdlr->setName((MP4AtomPtr)hdlr, name, hdlr->is_qt);

  err = minf->setupNewMedia(minf, mediaType, dataHandlerH, mdat);

  err = addAtom(self, (MP4AtomPtr)mdhd);
  if(err) goto bail;
  err = addAtom(self, (MP4AtomPtr)hdlr);
  if(err) goto bail;
  err = addAtom(self, (MP4AtomPtr)minf);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSampleReference(struct MP4MediaAtom *self, u64 dataOffset, u32 sampleCount,
                                 MP4Handle durationsH, MP4Handle sizesH, MP4Handle sampleEntryH,
                                 MP4Handle decodingOffsetsH, MP4Handle syncSamplesH,
                                 MP4Handle padsH)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->addSampleReference);
  err = minf->addSampleReference(minf, dataOffset, sampleCount, durationsH, sizesH, sampleEntryH,
                                 decodingOffsetsH, syncSamplesH, padsH);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setfieldsize(struct MP4MediaAtom *self, u32 fieldsize)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->setfieldsize);
  err = minf->setfieldsize(minf, fieldsize);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err settrackfragment(struct MP4MediaAtom *self, MP4AtomPtr fragment)
{
  MP4MediaInformationAtomPtr minf;
  MP4SampleTableAtomPtr stbl;
  MP4TrackFragmentAtomPtr traf;

  if(self->true_minf == NULL) self->true_minf = self->information;
  self->information = fragment;

  minf = (MP4MediaInformationAtomPtr)self->true_minf;
  stbl = (MP4SampleTableAtomPtr)minf->sampleTable;
  traf = (MP4TrackFragmentAtomPtr)fragment;

  traf->useSignedCompositionTimeOffsets = stbl->useSignedCompositionTimeOffsets;

  if(stbl->SampleAuxiliaryInformationSizes->entryCount != 0)
  {
    u32 i;
    MP4SampleAuxiliaryInformationSizesAtomPtr saizOld;
    MP4SampleAuxiliaryInformationSizesAtomPtr saizNew;
    MP4SampleAuxiliaryInformationOffsetsAtomPtr saioOld;
    MP4SampleAuxiliaryInformationOffsetsAtomPtr saioNew;

    for(i = 0; i < stbl->SampleAuxiliaryInformationSizes->entryCount; i++)
    {
      MP4GetListEntry(stbl->SampleAuxiliaryInformationSizes, i, (char **)&saizOld);
      MP4GetListEntry(stbl->SampleAuxiliaryInformationOffsets, i, (char **)&saioOld);

      MP4CreateSampleAuxiliaryInformationSizesAtom(&saizNew);
      saizNew->aux_info_type            = saizOld->aux_info_type;
      saizNew->aux_info_type_parameter  = saizOld->aux_info_type_parameter;
      saizNew->default_sample_info_size = saizOld->default_sample_info_size;
      saizNew->flags                    = saizOld->flags;

      MP4CreateSampleAuxiliaryInformationOffsetsAtom(&saioNew);
      saioNew->aux_info_type           = saioOld->aux_info_type;
      saioNew->aux_info_type_parameter = saioOld->aux_info_type_parameter;
      saioNew->flags                   = saioOld->flags;

      MP4AddListEntry(saizNew, traf->saizList);
      MP4AddListEntry(saioNew, traf->saioList);
    }
  }

  return MP4NoErr;
}

static MP4Err addSamples(struct MP4MediaAtom *self, MP4Handle sampleH, u32 sampleCount,
                         MP4Handle durationsH, MP4Handle sizesH, MP4Handle sampleEntryH,
                         MP4Handle decodingOffsetsH, MP4Handle syncSamplesH, MP4Handle padsH)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->addSamples);

  /*JLF 06/00 (jlefeuvre@e-vue.com)
               Check for the sample type, if it's an OD command frame, rewrite it in case
               it contains ESDescriptors instead of ES_ID_RefDescriptors
  */
  if((sampleCount > 0) &&
     (((MP4HandlerAtomPtr)self->handler)->handlerType == MP4ObjectDescriptorHandlerType))
  {

    /* Warning, only one OD-AU can be handled within each call.*/
    if(sampleCount != 1)
    {
      BAILWITHERROR(MP4NotImplementedErr)
    }
    /* rewrite the OD Access Unit (don't forget the size !!) */
    err = MP4ParseODFrame(self, sampleH, sizesH);
    if(err) goto bail;
  }

  err = minf->addSamples(minf, sampleH, sampleCount, durationsH, sizesH, sampleEntryH,
                         decodingOffsetsH, syncSamplesH, padsH);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err extendLastSampleDuration(struct MP4MediaAtom *self, u32 duration)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->extendLastSampleDuration);

  err = minf->extendLastSampleDuration(minf, duration);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addGroupDescription(struct MP4MediaAtom *self, u32 groupType, MP4Handle description,
                                  u32 *index)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information; /* this can be 'traf' if fragmented */
  assert(minf);
  assert(minf->addGroupDescription);

  err = minf->addGroupDescription(minf, groupType, description, index);
  if(err) goto bail;

  if(minf->type == MP4TrackFragmentAtomType)
  {
    *index += 0x10000;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getGroupDescription(struct MP4MediaAtom *self, u32 groupType, u32 index,
                                  MP4Handle description)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->getGroupDescription);
  err = minf->getGroupDescription(minf, groupType, index, description);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err changeSamplestoGroupType(struct MP4MediaAtom *self,
                                       sampleToGroupType_t sampleToGroupType)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information; /* this can be 'traf' if fragmented */
  assert(minf);
  assert(minf->changeSamplestoGroupType);
  err = minf->changeSamplestoGroupType(minf, sampleToGroupType);
  if(err) goto bail;
  self->sampleToGroupType = sampleToGroupType;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mapSamplestoGroup(struct MP4MediaAtom *self, u32 groupType, u32 group_index,
                                s32 sample_index, u32 count)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information; /* this can be 'traf' if fragmented */
  assert(minf);
  assert(minf->mapSamplestoGroup);
  err = minf->mapSamplestoGroup(minf, groupType, group_index, sample_index, count,
                                self->sampleToGroupType);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupMap(struct MP4MediaAtom *self, u32 groupType, u32 sample_number,
                                u32 *group_index)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->getSampleGroupMap);
  err = minf->getSampleGroupMap(minf, groupType, sample_number, group_index);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupSampleNumbers(struct MP4MediaAtom *self, u32 groupType, u32 groupIndex,
                                          u32 **outSampleNumbers, u32 *outSampleCnt)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;
  err = MP4NoErr;

  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->getSampleGroupSampleNumbers);
  err =
    minf->getSampleGroupSampleNumbers(minf, groupType, groupIndex, outSampleNumbers, outSampleCnt);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setSampleDependency(struct MP4MediaAtom *self, s32 sample_index,
                                  MP4Handle dependencies)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->setSampleDependency);
  err = minf->setSampleDependency(minf, sample_index, dependencies);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleDependency(struct MP4MediaAtom *self, u32 sampleNumber, u8 *dependency)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->getSampleDependency);
  err = minf->getSampleDependency(minf, sampleNumber, dependency);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setSampleEntry(struct MP4MediaAtom *self, MP4AtomPtr entry)
{
  MP4Err err;
  MP4MediaInformationAtomPtr minf;

  err  = MP4NoErr;
  minf = (MP4MediaInformationAtomPtr)self->information;
  assert(minf);
  assert(minf->setSampleEntry);
  err = minf->setSampleEntry(minf, entry);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setExtendedLanguageTag(struct MP4MediaAtom *self, MP4AtomPtr elng)
{
  MP4Err err;

  err = MP4NoErr;
  err = addAtom(self, elng);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4MediaAtomPtr self = (MP4MediaAtomPtr)s;
  err                  = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  SERIALIZE_ATOM_LIST(atomList);
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4MediaAtomPtr self = (MP4MediaAtomPtr)s;
  err                  = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  ADD_ATOM_LIST_SIZE(atomList);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  PARSE_ATOM_LIST(MP4MediaAtom)
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateMediaAtom(MP4MediaAtomPtr *outAtom)
{
  MP4Err err;
  MP4MediaAtomPtr self;

  self = (MP4MediaAtomPtr)calloc(1, sizeof(MP4MediaAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4MediaAtomType;
  self->name                  = "media";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  err                         = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;
  self->calculateSize               = calculateSize;
  self->serialize                   = serialize;
  self->addSamples                  = addSamples;
  self->calculateDuration           = calculateDuration;
  self->setfieldsize                = setfieldsize;
  self->setupNew                    = setupNew;
  self->mdatMoved                   = mdatMoved;
  self->addSampleReference          = addSampleReference;
  self->settrackfragment            = settrackfragment;
  self->addGroupDescription         = addGroupDescription;
  self->changeSamplestoGroupType    = changeSamplestoGroupType;
  self->mapSamplestoGroup           = mapSamplestoGroup;
  self->getSampleGroupMap           = getSampleGroupMap;
  self->getSampleGroupSampleNumbers = getSampleGroupSampleNumbers;
  self->getGroupDescription         = getGroupDescription;

  self->setSampleDependency = setSampleDependency;
  self->getSampleDependency = getSampleDependency;

  self->extendLastSampleDuration = extendLastSampleDuration;
  self->setSampleEntry           = setSampleEntry;
  self->setExtendedLanguageTag   = setExtendedLanguageTag;
  self->extendedLanguageTag      = NULL;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}

/*JLF 06/00 (jlefeuvre@e-vue.com) Rewriting the OD frame and
  replace ESDescriptors by ES_ID_RefDescriptors */
/* JB_RESO 09/02 also take care of the IPMPDecriptorPointers */
MP4Err MP4ParseODFrame(struct MP4MediaAtom *self, MP4Handle sampleH, MP4Handle sizesH)
{
  MP4Err err = MP4NoErr;

  u32 commandSize   = 0;
  u32 numCmds       = 0;
  char *encodedCmds = NULL;
  char *buffer;
  u32 i;
  u32 j;
  u32 frameSize;
  MP4InputStreamPtr is;
  MP4LinkedList descList;
  MP4TrackReferenceAtomPtr tref;
  MP4TrackGroupAtomPtr trgr;
  MP4TrackAtomPtr trak;
  MP4TrackReferenceTypeAtomPtr mpod;
  MP4TrackGroupTypeAtomPtr msrc;

  MP4Err MP4CreateTrackReferenceAtom(MP4TrackReferenceAtomPtr * outAtom);
  MP4Err MP4CreateTrackReferenceTypeAtom(u32 atomType, MP4TrackReferenceTypeAtomPtr * outAtom);
  MP4Err MP4CreateTrackGroupAtom(MP4TrackGroupAtomPtr * outAtom);
  MP4Err MP4CreateTrackGroupTypeAtom(u32 atomType, MP4TrackGroupTypeAtomPtr * outAtom);
  MP4Err MP4ParseCommand(MP4InputStreamPtr inputStream, MP4DescriptorPtr * outDesc);
  MP4Err MP4CreateES_ID_RefDescriptor(u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr * outDesc);

  descList = NULL;
  is       = NULL;

  err = MP4GetHandleSize(sampleH, &frameSize);
  if(err) goto bail;
  err = MP4CreateMemoryInputStream(*sampleH, frameSize, &is);
  if(err) goto bail;
  err = MP4MakeLinkedList(&descList);
  if(err) goto bail;

  trak = (MP4TrackAtomPtr)self->mediaTrack;
  tref = (MP4TrackReferenceAtom *)trak->trackReferences;
  if(tref == NULL)
  {
    err = MP4CreateTrackReferenceAtom(&tref);
    if(err) goto bail;
    trak->addAtom(trak, (MP4AtomPtr)tref);
    if(err) goto bail;
  }

  err = tref->findAtomOfType(tref, MP4ODTrackReferenceAtomType, (MP4AtomPtr *)&mpod);
  if(err) goto bail;
  if(mpod == NULL)
  {
    err = MP4CreateTrackReferenceTypeAtom(MP4ODTrackReferenceAtomType, &mpod);
    tref->addAtom(tref, (MP4AtomPtr)mpod);
  }

  /* Track Group */
  trgr = (MP4TrackGroupAtom *)trak->trackGroups;
  if(trgr == NULL)
  {
    err = MP4CreateTrackGroupAtom(&trgr);
    if(err) goto bail;
    trak->addAtom(trak, (MP4AtomPtr)trgr);
    if(err) goto bail;
  }

  err = trgr->findAtomOfType(trgr, MP4_FOUR_CHAR_CODE('m', 's', 'r', 'c'), (MP4AtomPtr *)&msrc);
  if(err) goto bail;
  if(msrc == NULL)
  {
    err = MP4CreateTrackGroupTypeAtom(MP4_FOUR_CHAR_CODE('m', 's', 'r', 'c'), &msrc);
    trgr->addAtom(trgr, (MP4AtomPtr)msrc);
  }

  while(is->available > 0)
  {
    MP4DescriptorPtr desc;
    MP4ObjectDescriptorUpdatePtr odUpdate;
    MP4ESDescriptorUpdatePtr esUpdate;
    u32 odCount;
    u32 esCount;

    err = MP4ParseCommand(is, &desc);
    if(err) goto bail;
    /*		if (!desc) goto bail;*/
    numCmds += 1;

    switch(desc->tag)
    {
      /* ObjectDescriptor UPDATE COMMAND */
    case MP4ObjectDescriptorUpdateTag:
      odUpdate = (MP4ObjectDescriptorUpdatePtr)desc;
      err      = MP4GetListEntryCount(odUpdate->objectDescriptors, &odCount);
      if(err) goto bail;
      if(err) goto bail;

      for(i = 0; i < odCount; i++)
      {
        MP4ObjectDescriptorPtr od;
        err = MP4GetListEntry(odUpdate->objectDescriptors, i, (char **)&od);
        if(err) goto bail;
        err = MP4GetListEntryCount(od->ESDescriptors, &esCount);
        if(err) goto bail;
        /* JB_RESO rewrite IPMPDescriptorPointers */
        err = storeIPMPDescriptorPointers((MP4DescriptorPtr)od, mpod);
        if(err) goto bail;

        /* Guido 11/10/00 : also rewrite the OD TAG */
        od->tag = MP4_OD_Tag;

        for(j = 0; j < esCount; j++)
        {
          MP4ES_ID_RefDescriptorPtr ref;
          MP4ES_DescriptorPtr esd;

          err = MP4GetListEntry(od->ESDescriptors, j, (char **)&esd);
          if(err) goto bail;
          if(err) goto bail;

          /* TO DO in libisomp4 : Check if this ES is in the movie
             ES_ID == TrackID */
          err = mpod->addTrackID(mpod, esd->ESID);
          if(err) goto bail;

          /* JB_RESO rewrite IPMPDescriptorPointers */
          err = storeIPMPDescriptorPointers((MP4DescriptorPtr)esd, mpod);
          if(err) goto bail;

          /* The ref_index == mpod->TrackIdCount after adding the ref (NON NULL) */
          err =
            MP4CreateES_ID_RefDescriptor(MP4ES_ID_RefDescriptorTag, 0, 0, (MP4DescriptorPtr *)&ref);
          if(err) goto bail;
          assert(ref);
          ref->refIndex = mpod->trackIDCount;
          err           = od->addDescriptor((MP4DescriptorPtr)od, (MP4DescriptorPtr)ref);
          if(err) goto bail;
        }
        /* Remove the ESDescriptor List from each OD present in the ObjectDescriptor !! */
        if(od->ESDescriptors)
        {
          u32 listSize;
          u32 i1;
          err = MP4GetListEntryCount(od->ESDescriptors, &listSize);
          if(err) goto bail;
          for(i1 = 0; i1 < listSize; i1++)
          {
            MP4DescriptorPtr a;
            err = MP4GetListEntry(od->ESDescriptors, i1, (char **)&a);
            if(err) goto bail;
            if(a) a->destroy(a);
          }
          err = MP4DeleteLinkedList(od->ESDescriptors);
          if(err) goto bail;

          od->ESDescriptors = NULL;
        }
      }

      break;

      /* ElementaryStreamDescriptor UPDATE COMMAND */
    case MP4ESDescriptorUpdateTag:
      esUpdate = (MP4ESDescriptorUpdatePtr)desc;
      err      = MP4GetListEntryCount(esUpdate->ESDescriptors, &esCount);
      if(err) goto bail;
      for(j = 0; j < esCount; j++)
      {
        MP4ES_ID_RefDescriptorPtr ref;
        MP4ES_DescriptorPtr esd;

        err = MP4GetListEntry(esUpdate->ESDescriptors, j, (char **)&esd);
        if(err) goto bail;

        /* TO DO in libisomp4 : Check if this ES is in the movie */
        /* ES_ID == TrackID */
        err = mpod->addTrackID(mpod, esd->ESID);
        if(err) goto bail;

        /* JB_RESO rewrite IPMPDescriptorPointers */
        err = storeIPMPDescriptorPointers((MP4DescriptorPtr)esd, mpod);
        if(err) goto bail;

        err =
          MP4CreateES_ID_RefDescriptor(MP4ES_ID_RefDescriptorTag, 0, 0, (MP4DescriptorPtr *)&ref);
        if(err) goto bail;
        assert(ref);
        ref->refIndex = mpod->trackIDCount;
        err           = esUpdate->addDescriptor((MP4DescriptorPtr)esUpdate, (MP4DescriptorPtr)ref);
        if(err) goto bail;
      }

      /* Remove the ESDescriptors from the ES Listr present in this update!! */
      if(esUpdate->ESDescriptors)
      {
        u32 listSize;
        u32 i1;
        err = MP4GetListEntryCount(esUpdate->ESDescriptors, &listSize);
        if(err) goto bail;
        for(i1 = 0; i1 < listSize; i1++)
        {
          MP4DescriptorPtr a;
          err = MP4GetListEntry(esUpdate->ESDescriptors, i1, (char **)&a);
          if(err) goto bail;
          if(a) a->destroy(a);
        }
        err = MP4DeleteLinkedList(esUpdate->ESDescriptors);
        if(err) goto bail;

        esUpdate->ESDescriptors = NULL;
      }

      break;

    default:
      break;
    }
    err = MP4AddListEntry(desc, descList);
    if(err) goto bail;
    err = desc->calculateSize(desc);
    if(err) goto bail;
    commandSize += desc->size;
  }

  encodedCmds = (char *)calloc(1, commandSize);
  buffer      = encodedCmds;
  TESTMALLOC(encodedCmds)

  for(i = 0; i < numCmds; i++)
  {
    MP4DescriptorPtr desc;
    err = MP4GetListEntry(descList, i, (char **)&desc);
    if(err) goto bail;
    err = desc->serialize(desc, buffer);
    if(err) goto bail;
    buffer += desc->size;
  }
  err = MP4SetHandleSize(sampleH, commandSize);
  if(err) goto bail;
  memcpy(*sampleH, encodedCmds, commandSize);
  free(encodedCmds);

  *(u32 *)*sizesH = commandSize;

bail:
  if(is) is->destroy(is);
  if(descList)
  {
    u32 listSize;
    u32 i1;
    err = MP4GetListEntryCount(descList, &listSize);
    if(err) goto bail;
    for(i1 = 0; i1 < listSize; i1++)
    {
      MP4DescriptorPtr a;
      err = MP4GetListEntry(descList, i1, (char **)&a);
      if(err) goto bail;
      if(a) a->destroy(a);
    }
    err = MP4DeleteLinkedList(descList);
    if(err) goto bail;
  }
  TEST_RETURN(err);
  return err;
}

MP4Err storeIPMPDescriptorPointers(MP4DescriptorPtr desc, MP4TrackReferenceTypeAtomPtr mpod)
{
  u32 k;
  u32 ipmpDescPointersCount;
  MP4Err err;
  MP4LinkedList ipmpDescPointersList;

  err = MP4NoErr;
  /* if ( desc->name == "MP4ES_Descriptor" ) fb_reso */
  if(desc->tag == MP4ES_DescriptorTag) /* fb_reso */
  {
    ipmpDescPointersList = ((MP4ES_DescriptorPtr)desc)->IPMPDescriptorPointers;
  }
  /* else if ( desc->name = "MP4ObjectDescriptor" ) fb_reso */
  else if(desc->tag == MP4ObjectDescriptorTag) /* fb_reso */
  {
    ipmpDescPointersList = ((MP4ObjectDescriptorPtr)desc)->IPMPDescriptorPointers;
  }
  else
    BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntryCount(ipmpDescPointersList, &ipmpDescPointersCount);
  if(err) goto bail;
  for(k = 0; k < ipmpDescPointersCount; k++)
  {
    MP4IPMPDescriptorPointerPtr ipmpDescPtr;
    err = MP4GetListEntry(ipmpDescPointersList, k, (char **)&ipmpDescPtr);
    if(err) goto bail;
    if((ipmpDescPtr->ipmpEsId != 0) && (ipmpDescPtr->ipmpEsId != 0xFFFF))
    {
      /* TO DO in libisomp4 : Check if this ES is in the movie
                     ES_ID == TrackID */
      err = mpod->addTrackID(mpod, ipmpDescPtr->ipmpEsId);
      if(err) goto bail;
      /* change the id */
      ipmpDescPtr->ipmpEsId = (u16)mpod->trackIDCount;
    }
  }

bail:
  return err;
}
