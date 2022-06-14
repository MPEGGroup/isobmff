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
        $Id: MediaInformationAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include "MP4DataHandler.h"
#include <stdlib.h>
#include <string.h>

extern u32 MP4SampleEntryProtos[];

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4MediaInformationAtomPtr self;
  u32 i;
  err  = MP4NoErr;
  self = (MP4MediaInformationAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  if(self->atomList)
  {
    u32 atomListSize;
    err = MP4GetListEntryCount(self->atomList, &atomListSize);
    if(err) goto bail;

    for(i = 0; i < atomListSize; i++)
    {
      MP4AtomPtr a;
      err = MP4GetListEntry(self->atomList, i, (char **)&a);
      if(err) goto bail;
      if(a) a->destroy(a);
    }
    err = MP4DeleteLinkedList(self->atomList);
    if(err) goto bail;
    self->atomList = NULL;
  }

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err addAtom(MP4MediaInformationAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  err = MP4AddListEntry(atom, self->atomList);
  if(err) goto bail;
  switch(atom->type)
  {
  case MP4MediaHeaderAtomType:
  case MP4VideoMediaHeaderAtomType:
  case MP4SoundMediaHeaderAtomType:
  case MP4HintMediaHeaderAtomType:
    if(self->mediaHeader) BAILWITHERROR(MP4BadDataErr)
    self->mediaHeader = atom;
    break;

  case MP4DataInformationAtomType:
    if(self->dataInformation) BAILWITHERROR(MP4BadDataErr)
    self->dataInformation = atom;
    break;

  case MP4SampleTableAtomType:
    if(self->sampleTable) BAILWITHERROR(MP4BadDataErr)
    self->sampleTable = atom;
    break;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err testDataEntry(struct MP4MediaInformationAtom *self, u32 dataEntryIndex)
{
  MP4Err err;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryAtomPtr dataEntryAtom;

  err = MP4NoErr;
  if((self == NULL) || (dataEntryIndex == 0)) BAILWITHERROR(MP4BadParamErr)
  if(self->dataHandler && (self->dataEntryIndex == dataEntryIndex))
  {
    /* already open */
  }
  else
  {
    dinf = (MP4DataInformationAtomPtr)self->dataInformation;
    if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr)
    dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
    if(dref == NULL) BAILWITHERROR(MP4InvalidMediaErr)
    if(dataEntryIndex > dref->getEntryCount(dref)) BAILWITHERROR(MP4BadParamErr)
    err = dref->getEntry(dref, dataEntryIndex, &dataEntryAtom);
    if(err) goto bail;
    if(dataEntryAtom == NULL) BAILWITHERROR(MP4InvalidMediaErr)
    err = MP4PreflightDataHandler(self->inputStream, dataEntryAtom);
    if(err) goto bail;
  }
bail:
  TEST_RETURN(err);

  return err;
}

/* Guido : inserted to clean-up resources */
static MP4Err closeDataHandler(MP4AtomPtr s)
{
  MP4Err err;
  MP4MediaInformationAtomPtr self;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryAtomPtr dataEntryAtom;

  err = MP4NoErr;
  if(s == NULL) BAILWITHERROR(MP4BadParamErr)
  self = (MP4MediaInformationAtomPtr)s;
  dinf = (MP4DataInformationAtomPtr)self->dataInformation;
  if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
  dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
  if(dref == NULL) BAILWITHERROR(MP4InvalidMediaErr);
  /* If no Access Unit was requested, dataEntryIndex was not initialized */
  /*
  if (self->dataEntryIndex > 0)
  {
          err = dref->getEntry( dref, self->dataEntryIndex, &dataEntryAtom ); if (err) goto bail;
  }
  if ( dataEntryAtom == NULL )
          BAILWITHERROR( MP4InvalidMediaErr )
  if ( self->dataHandler )
  {
          err = MP4DisposeDataHandler( (MP4DataHandlerPtr) self->dataHandler, dataEntryAtom ); if
  (err) goto bail; self->dataHandler = NULL;
  }
  self->dataEntryIndex = 0;
  */

  /* GUIDO: if no Access Unit was requested, or openDataHandler failed,
     dataHandler is NULL and dataEntryIndex is 0 */
  if(self->dataHandler)
  {
    if(self->dataEntryIndex == 0) BAILWITHERROR(MP4InvalidMediaErr)
    err = dref->getEntry(dref, self->dataEntryIndex, &dataEntryAtom);
    if(err) goto bail;
    if(dataEntryAtom == NULL) BAILWITHERROR(MP4InvalidMediaErr)
    err = MP4DisposeDataHandler((MP4DataHandlerPtr)self->dataHandler, dataEntryAtom);
    if(err) goto bail;
    self->dataHandler    = NULL;
    self->dataEntryIndex = 0;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err openDataHandler(MP4AtomPtr s, u32 dataEntryIndex)
{
  MP4Err err;
  MP4MediaInformationAtomPtr self;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryAtomPtr dataEntryAtom;

  err = MP4NoErr;
  if((s == NULL) || (dataEntryIndex == 0)) BAILWITHERROR(MP4BadParamErr)
  self = (MP4MediaInformationAtomPtr)s;
  if(self->dataHandler && (self->dataEntryIndex == dataEntryIndex))
  {
    /* desired one is already open */
  }
  else
  {
    if(self->dataHandler)
    {
      /* close the current one */
      /* Guido : changed because I have now a function to close cleanly */
      err = closeDataHandler((MP4AtomPtr)self);
      if(err) goto bail;
      /*			MP4DataHandlerPtr dh = (MP4DataHandlerPtr) self->dataHandler;
                      if ( dh->close )
                      {
                          dh->close( dh );
                                      self->dataHandler = NULL;
                      }
      */
    }
    dinf = (MP4DataInformationAtomPtr)self->dataInformation;
    if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
    dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
    if(dref == NULL) BAILWITHERROR(MP4InvalidMediaErr);
    if(dataEntryIndex > dref->getEntryCount(dref)) BAILWITHERROR(MP4BadParamErr)
    err = dref->getEntry(dref, dataEntryIndex, &dataEntryAtom);
    if(err) goto bail;
    if(dataEntryAtom == NULL) BAILWITHERROR(MP4InvalidMediaErr)

    err = MP4CreateDataHandler(self->inputStream, dataEntryAtom,
                               (struct MP4DataHandler **)&self->dataHandler);
    if(err) goto bail;
    self->dataEntryIndex = dataEntryIndex;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4MediaInformationAtomPtr self = (MP4MediaInformationAtomPtr)s;
  err                             = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  SERIALIZE_ATOM_LIST(atomList);
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mdatArrived(struct MP4MediaInformationAtom *self, MP4AtomPtr mdat)
{

  MP4Err err;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryAtomPtr dataEntryAtom;
  u32 count;
  u32 i;

  err = MP4NoErr;

  dinf = (MP4DataInformationAtomPtr)self->dataInformation;
  if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr)

  dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
  if(dref == NULL) BAILWITHERROR(MP4InvalidMediaErr)

  count = dref->getEntryCount(dref);
  for(i = 0; i < count; i++)
  {
    err = dref->getEntry(dref, i + 1, &dataEntryAtom);
    if(err) goto bail;

    if(dataEntryAtom->flags == 1) dataEntryAtom->mdat = mdat;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setupNewMedia(struct MP4MediaInformationAtom *self, u32 handlerType, MP4Handle dataH,
                            MP4AtomPtr mdat)
{
  MP4Err MP4CreateDataInformationAtom(MP4DataInformationAtomPtr * outAtom);
  MP4Err MP4CreateSampleTableAtom(MP4SampleTableAtomPtr * outAtom);
  MP4Err MP4CreateVideoMediaHeaderAtom(MP4VideoMediaHeaderAtomPtr * outAtom);
  MP4Err MP4CreateSoundMediaHeaderAtom(MP4SoundMediaHeaderAtomPtr * outAtom);
  MP4Err MP4CreateHintMediaHeaderAtom(MP4HintMediaHeaderAtomPtr * outAtom);
  MP4Err MP4CreateMPEGMediaHeaderAtom(MP4MPEGMediaHeaderAtomPtr * outAtom);
  MP4Err MP4CreateDataReferenceAtom(MP4DataReferenceAtomPtr * outAtom);
  MP4Err MP4CreateDataEntryURLAtom(MP4DataEntryURLAtomPtr * outAtom);

  MP4Err err;
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4DataEntryURLAtomPtr dataEntryAtom;
  MP4SampleTableAtomPtr stbl;
  MP4AtomPtr mdhd;
  err = MP4NoErr;

  /* media header */
  mdhd = NULL;
  switch(handlerType)
  {
  case MP4VisualHandlerType:
    err = MP4CreateVideoMediaHeaderAtom((MP4VideoMediaHeaderAtomPtr *)&mdhd);
    if(err) goto bail;
    break;

  case MP4AudioHandlerType:
    err = MP4CreateSoundMediaHeaderAtom((MP4SoundMediaHeaderAtomPtr *)&mdhd);
    if(err) goto bail;
    break;

  case MP4HintHandlerType:
    err = MP4CreateHintMediaHeaderAtom((MP4HintMediaHeaderAtomPtr *)&mdhd);
    if(err) goto bail;
    break;

  case MP4VolumetricHandlerType:
    err = MP4CreateVisualMediaHeaderAtom((MP4VolumetricVisualMediaHeaderAtomPtr *)&mdhd);
    if(err) goto bail;
    break;

  /* Note that MP4TextHandlerType uses the MP4MPEGMediaHeaderAtom, i.e. the default is right */
  default:
    err = MP4CreateMPEGMediaHeaderAtom((MP4MPEGMediaHeaderAtomPtr *)&mdhd);
    if(err) goto bail;
    break;
  }
  if(mdhd == NULL) BAILWITHERROR(MP4InvalidMediaErr)

  err = addAtom(self, mdhd);
  if(err) goto bail;

  err = MP4CreateDataInformationAtom(&dinf);
  if(err) goto bail;
  err = addAtom(self, (MP4AtomPtr)dinf);
  if(err) goto bail;
  err = MP4CreateSampleTableAtom(&stbl);
  if(err) goto bail;
  err = addAtom(self, (MP4AtomPtr)stbl);
  if(err) goto bail;
  err = stbl->setupNew(stbl);
  if(err) goto bail;

  /* data information */
  err = MP4CreateDataReferenceAtom(&dref);
  if(err) goto bail;
  err = MP4CreateDataEntryURLAtom(&dataEntryAtom);
  if(err) goto bail;
  if(dataH == NULL)
  {
    dataEntryAtom->flags |= 1;
    dataEntryAtom->mdat = mdat;
  }
  else
  {
    u32 sz;
    err = MP4GetHandleSize(dataH, &sz);
    if(err) goto bail;
    dataEntryAtom->locationLength = (u32)sz;
    dataEntryAtom->location       = (char *)calloc(1, sz);
    TESTMALLOC(dataEntryAtom->location)
    memcpy(dataEntryAtom->location, *dataH, sz);
  }
  err = dref->addDataEntry(dref, (MP4AtomPtr)dataEntryAtom);
  if(err) goto bail;
  err = dinf->addAtom(dinf, (MP4AtomPtr)dref);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSampleReference(struct MP4MediaInformationAtom *self, u64 dataOffset,
                                 u32 sampleCount, MP4Handle durationsH, MP4Handle sizesH,
                                 MP4Handle sampleEntryH, MP4Handle decodingOffsetsH,
                                 MP4Handle syncSamplesH, MP4Handle padsH)
{
  MP4Err err;
  u32 dataReferenceIndex;
  MP4SampleTableAtomPtr stbl;
  MP4DataInformationAtomPtr dinf;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  if(stbl == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  dinf = (MP4DataInformationAtomPtr)self->dataInformation;
  if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  if(sampleEntryH != NULL)
  {
    MP4Err MP4CreateMemoryInputStream(char *base, u32 size, MP4InputStreamPtr *outStream);
    /*		MP4Err MP4ParseAtom( MP4InputStreamPtr inputStream, MP4AtomPtr *outAtom  ); */
    MP4Err MP4ParseAtomUsingProtoList(MP4InputStreamPtr inputStream, u32 * protoList,
                                      u32 defaultAtom, MP4AtomPtr * outAtom);

    MP4InputStreamPtr is;
    u32 size;
    MP4AtomPtr entry;
    err = MP4GetHandleSize(sampleEntryH, &size);
    if(err) goto bail;
    err = MP4CreateMemoryInputStream(*sampleEntryH, size, &is);
    if(err) goto bail;
    is->debugging = 0;
    /*		err = MP4ParseAtom( is, &entry ); if (err) goto bail; */
    err =
      MP4ParseAtomUsingProtoList(is, MP4SampleEntryProtos, MP4GenericSampleEntryAtomType, &entry);
    if(err) goto bail;

    err = stbl->setSampleEntry(stbl, entry);
    if(err) goto bail;

    if(is)
    {
      is->destroy(is);
      is = NULL;
    }
  }
  err = stbl->getCurrentDataReferenceIndex(stbl, &dataReferenceIndex);
  if(err) goto bail;

  err = stbl->addSamples(stbl, sampleCount, dataOffset, durationsH, sizesH, decodingOffsetsH,
                         syncSamplesH, padsH);
  if(err) goto bail;
  err = dinf->addSampleReference(dinf, sampleCount, dataReferenceIndex, dataOffset, sizesH);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setfieldsize(struct MP4MediaInformationAtom *self, u32 fieldsize)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err  = MP4NoErr;
  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->setfieldsize);
  err = stbl->setfieldsize(stbl, fieldsize);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err extendLastSampleDuration(struct MP4MediaInformationAtom *self, u32 duration)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  if(stbl == NULL) BAILWITHERROR(MP4InvalidMediaErr);

  err = stbl->extendLastSampleDuration(stbl, duration);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSamples(struct MP4MediaInformationAtom *self, MP4Handle sampleH, u32 sampleCount,
                         MP4Handle durationsH, MP4Handle sizesH, MP4Handle sampleEntryH,
                         MP4Handle decodingOffsetsH, MP4Handle syncSamplesH, MP4Handle padsH)
{
  MP4Err err;
  u64 sampleOffset;
  u32 dataReferenceIndex;
  MP4SampleTableAtomPtr stbl;
  MP4DataInformationAtomPtr dinf;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  if(stbl == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  dinf = (MP4DataInformationAtomPtr)self->dataInformation;
  if(dinf == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  if(sampleEntryH != NULL)
  {
    MP4Err MP4CreateMemoryInputStream(char *base, u32 size, MP4InputStreamPtr *outStream);
    MP4Err MP4ParseAtomUsingProtoList(MP4InputStreamPtr inputStream, u32 * protoList,
                                      u32 defaultAtom, MP4AtomPtr * outAtom);

    MP4InputStreamPtr is;
    u32 size;
    MP4AtomPtr entry;
    err = MP4GetHandleSize(sampleEntryH, &size);
    if(err) goto bail;
    err = MP4CreateMemoryInputStream(*sampleEntryH, size, &is);
    if(err) goto bail;
    is->debugging = 0;
    err =
      MP4ParseAtomUsingProtoList(is, MP4SampleEntryProtos, MP4GenericSampleEntryAtomType, &entry);
    if(err) goto bail;
    err = stbl->setSampleEntry(stbl, entry);
    if(err) goto bail;

    if(is)
    {
      is->destroy(is);
      is = NULL;
    }
  }
  if(sampleCount > 0)
  {
    err = stbl->getCurrentDataReferenceIndex(stbl, &dataReferenceIndex);
    if(err) goto bail;
    err = dinf->getOffset(dinf, dataReferenceIndex, &sampleOffset);
    if(err) goto bail;
    err = stbl->addSamples(stbl, sampleCount, sampleOffset, durationsH, sizesH, decodingOffsetsH,
                           syncSamplesH, padsH);
    if(err) goto bail;
    err = dinf->addSamples(dinf, sampleCount, dataReferenceIndex, sampleH);
    if(err) goto bail;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addGroupDescription(struct MP4MediaInformationAtom *self, u32 groupType,
                                  MP4Handle description, u32 *index)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->addGroupDescription);
  err = stbl->addGroupDescription(stbl, groupType, description, index);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getGroupDescription(struct MP4MediaInformationAtom *self, u32 groupType, u32 index,
                                  MP4Handle description)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->getGroupDescription);
  err = stbl->getGroupDescription(stbl, groupType, index, description);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err changeSamplestoGroupType(struct MP4MediaInformationAtom *self,
                                       sampleToGroupType_t sampleToGroupType)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->changeSamplestoGroupType);
  err = stbl->changeSamplestoGroupType(stbl, sampleToGroupType);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mapSamplestoGroup(struct MP4MediaInformationAtom *self, u32 groupType,
                                u32 group_index, s32 sample_index, u32 count,
                                sampleToGroupType_t sampleToGroupType)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->mapSamplestoGroup);
  err =
    stbl->mapSamplestoGroup(stbl, groupType, group_index, sample_index, count, sampleToGroupType);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupMap(struct MP4MediaInformationAtom *self, u32 groupType,
                                u32 sample_number, u32 *group_index)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->getSampleGroupMap);
  err = stbl->getSampleGroupMap(stbl, groupType, sample_number, group_index);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupSampleNumbers(struct MP4MediaInformationAtom *self, u32 groupType,
                                          u32 groupIndex, u32 **outSampleNumbers, u32 *outSampleCnt)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->getSampleGroupSampleNumbers);
  err =
    stbl->getSampleGroupSampleNumbers(stbl, groupType, groupIndex, outSampleNumbers, outSampleCnt);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setSampleDependency(struct MP4MediaInformationAtom *self, s32 sample_index,
                                  MP4Handle dependencies)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->setSampleDependency);
  err = stbl->setSampleDependency(stbl, sample_index, dependencies);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleDependency(struct MP4MediaInformationAtom *self, u32 sampleNumber,
                                  u8 *dependency)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->getSampleDependency);
  err = stbl->getSampleDependency(stbl, sampleNumber, dependency);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setSampleEntry(struct MP4MediaInformationAtom *self, MP4AtomPtr entry)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;

  err = MP4NoErr;

  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  assert(stbl);
  assert(stbl->setSampleEntry);
  err = stbl->setSampleEntry(stbl, entry);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mdatMoved(struct MP4MediaInformationAtom *self, u64 mdatBase, u64 mdatEnd,
                        s32 mdatOffset)
{
  MP4DataInformationAtomPtr dinf;
  MP4DataReferenceAtomPtr dref;
  MP4SampleTableAtomPtr stbl;
  MP4SampleToChunkAtomPtr stsc;
  MP4ChunkOffsetAtomPtr stco;
  MP4SampleDescriptionAtomPtr stsd;
  MP4Err err;
  u32 i;

  err  = MP4NoErr;
  dinf = (MP4DataInformationAtomPtr)self->dataInformation;
  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  if((dinf == NULL) || (stbl == NULL)) BAILWITHERROR(MP4InvalidMediaErr)
  dref = (MP4DataReferenceAtomPtr)dinf->dataReference;
  if(dref == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  stsc = (MP4SampleToChunkAtomPtr)stbl->SampleToChunk;
  stco = (MP4ChunkOffsetAtomPtr)stbl->ChunkOffset;
  stsd = (MP4SampleDescriptionAtomPtr)stbl->SampleDescription;
  if((stsc == NULL) || (stco == NULL) || (stsd == NULL)) BAILWITHERROR(MP4InvalidMediaErr)

  err = stsc->mdatMoved(stsc, mdatBase, mdatEnd, mdatOffset, stsd, dref, stco);
  if(err) goto bail;

  for(i = 0; i < stbl->SampleAuxiliaryInformationOffsets->entryCount; i++)
  {
    MP4SampleAuxiliaryInformationOffsetsAtomPtr saio;
    err = MP4GetListEntry(stbl->SampleAuxiliaryInformationOffsets, i, (char **)&saio);
    if(err) goto bail;
    err = saio->mdatMoved((MP4AtomPtr)saio, mdatBase, mdatEnd, mdatOffset);
    if(err) goto bail;
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err getMediaDuration(struct MP4MediaInformationAtom *self, u32 *outDuration)
{
  MP4Err err;
  MP4SampleTableAtomPtr stbl;
  err = MP4NoErr;

  if(self->sampleTable == NULL) BAILWITHERROR(MP4InvalidMediaErr);
  stbl = (MP4SampleTableAtomPtr)self->sampleTable;
  err  = stbl->calculateDuration(stbl, outDuration);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4MediaInformationAtomPtr self = (MP4MediaInformationAtomPtr)s;
  err                             = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  ADD_ATOM_LIST_SIZE(atomList);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  PARSE_ATOM_LIST(MP4MediaInformationAtom)
  self->inputStream = inputStream;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateMediaInformationAtom(MP4MediaInformationAtomPtr *outAtom)
{
  MP4Err err;
  MP4MediaInformationAtomPtr self;

  self = (MP4MediaInformationAtomPtr)calloc(1, sizeof(MP4MediaInformationAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4MediaInformationAtomType;
  self->name                  = "media information";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->closeDataHandler      = closeDataHandler;
  self->openDataHandler       = openDataHandler;
  self->setupNewMedia         = setupNewMedia;
  self->getMediaDuration      = getMediaDuration;
  err                         = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;
  self->calculateSize               = calculateSize;
  self->serialize                   = serialize;
  self->addSamples                  = addSamples;
  self->setfieldsize                = setfieldsize;
  self->mdatMoved                   = mdatMoved;
  self->mdatArrived                 = mdatArrived;
  self->addSampleReference          = addSampleReference;
  self->testDataEntry               = testDataEntry;
  self->addGroupDescription         = addGroupDescription;
  self->changeSamplestoGroupType    = changeSamplestoGroupType;
  self->mapSamplestoGroup           = mapSamplestoGroup;
  self->getSampleGroupMap           = getSampleGroupMap;
  self->getSampleGroupSampleNumbers = getSampleGroupSampleNumbers;
  self->getGroupDescription         = getGroupDescription;
  self->getSampleDependency         = getSampleDependency;
  self->setSampleDependency         = setSampleDependency;
  self->extendLastSampleDuration    = extendLastSampleDuration;
  self->setSampleEntry              = setSampleEntry;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
