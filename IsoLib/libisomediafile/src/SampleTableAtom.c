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
  $Id: SampleTableAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4SampleTableAtomPtr self;
  u32 i;
  err  = MP4NoErr;
  self = (MP4SampleTableAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  DESTROY_ATOM_LIST

  err = MP4DeleteLinkedList(self->groupDescriptionList);
  if(err) goto bail;
  err = MP4DeleteLinkedList(self->sampletoGroupList);
  if(err) goto bail;
  err = MP4DeleteLinkedList(self->SampleAuxiliaryInformationSizes);
  if(err) goto bail;
  err = MP4DeleteLinkedList(self->SampleAuxiliaryInformationOffsets);
  if(err) goto bail;

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

MP4Err MP4FindGroupAtom(MP4LinkedList theList, u32 type, MP4AtomPtr *theAtom)
{
  u32 count, i;
  MP4Err err;

  *theAtom = NULL;

  err = MP4GetListEntryCount(theList, &count);
  if(err) goto bail;
  for(i = 0; i < count; i++)
  {
    MP4AtomPtr desc;
    err = MP4GetListEntry(theList, i, (char **)&desc);
    if(err) goto bail;
    if(desc)
    {
      MP4SampletoGroupAtomPtr grp = (MP4SampletoGroupAtomPtr)desc;
      if(grp->grouping_type == type)
      {
        *theAtom = desc;
        return err;
      }
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

#define ADDCASE(atomName)                           \
  case MP4##atomName##AtomType:                     \
    if(self->atomName) BAILWITHERROR(MP4BadDataErr) \
    self->atomName = atom;                          \
    break

static MP4Err addAtom(MP4SampleTableAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  err = MP4AddListEntry(atom, self->atomList);
  if(err) goto bail;
  switch(atom->type)
  {
    ADDCASE(TimeToSample);
    ADDCASE(CompositionOffset);
    ADDCASE(CompositionToDecode);
    ADDCASE(SyncSample);
    ADDCASE(SampleDescription);

  case MP4CompactSampleSizeAtomType:

    ADDCASE(SampleSize);
    ADDCASE(SampleToChunk);

  case MP4ChunkLargeOffsetAtomType:
    ADDCASE(ChunkOffset);

    ADDCASE(ShadowSync);
    ADDCASE(DegradationPriority);
    ADDCASE(PaddingBits);

    ADDCASE(SampleDependency);

  case MP4SampleAuxiliaryInformationSizesAtomType:
    err = MP4AddListEntry((void *)atom, self->SampleAuxiliaryInformationSizes);
    break;

  case MP4SampleAuxiliaryInformationOffsetsAtomType:
    err = MP4AddListEntry((void *)atom, self->SampleAuxiliaryInformationOffsets);
    break;

  case MP4SampleGroupDescriptionAtomType:
    err = MP4AddListEntry((void *)atom, self->groupDescriptionList);
    break;

  case MP4SampletoGroupAtomType:
  case MP4CompactSampletoGroupAtomType:
    err = MP4AddListEntry((void *)atom, self->sampletoGroupList);
    break;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateDuration(struct MP4SampleTableAtom *self, u32 *outDuration)
{
  MP4Err err;
  MP4TimeToSampleAtomPtr stts;

  err = MP4NoErr;
  if(outDuration == NULL) BAILWITHERROR(MP4BadParamErr)

  stts = (MP4TimeToSampleAtomPtr)self->TimeToSample;
  if(stts == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  err = stts->getTotalDuration(stts, outDuration);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setSampleEntry(struct MP4SampleTableAtom *self, MP4AtomPtr entry)
{
  MP4Err err;
  MP4SampleDescriptionAtomPtr stsd;

  err = MP4NoErr;
  if(entry == NULL) BAILWITHERROR(MP4BadParamErr)

  stsd = (MP4SampleDescriptionAtomPtr)self->SampleDescription;
  if(stsd == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  err = stsd->addEntry(stsd, entry);
  if(err) goto bail;
  self->currentSampleEntry      = entry;
  self->currentSampleEntryIndex = stsd->getEntryCount(stsd);
bail:
  TEST_RETURN(err);

  return err;
}

static u32 getCurrentSampleEntryIndex(struct MP4SampleTableAtom *self)
{
  return self->currentSampleEntryIndex;
}

static MP4Err setDefaultSampleEntry(struct MP4SampleTableAtom *self, u32 index)
{
  MP4SampleDescriptionAtomPtr stsd;
  MP4Err err;

  err = MP4NoErr;

  stsd = (MP4SampleDescriptionAtomPtr)self->SampleDescription;
  if(stsd == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  err = stsd->getEntry(stsd, index, (GenericSampleEntryAtomPtr *)&(self->currentSampleEntry));
  self->currentSampleEntryIndex = index;

bail:
  return err;
}

static MP4Err setfieldsize(struct MP4SampleTableAtom *self, u32 fieldsize)
{
  MP4Err err;
  MP4SampleSizeAtomPtr stsz;

  err  = MP4NoErr;
  stsz = (MP4SampleSizeAtomPtr)self->SampleSize;
  assert(stsz);
  assert(stsz->setfieldsize);
  err = stsz->setfieldsize(stsz, fieldsize);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getCurrentDataReferenceIndex(struct MP4SampleTableAtom *self,
                                           u32 *outDataReferenceIndex)
{
  GenericSampleEntryAtomPtr entry;
  MP4Err err;

  err   = MP4NoErr;
  entry = (GenericSampleEntryAtomPtr)self->currentSampleEntry;
  if(entry == NULL) BAILWITHERROR(MP4InvalidMediaErr)
  *outDataReferenceIndex = entry->dataReferenceIndex;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err extendLastSampleDuration(struct MP4SampleTableAtom *self, u32 duration)
{
  MP4Err err;
  MP4TimeToSampleAtomPtr stts;

  err  = MP4NoErr;
  stts = (MP4TimeToSampleAtomPtr)self->TimeToSample;

  if(stts == NULL) BAILWITHERROR(MP4InvalidMediaErr);

  stts->extendLastSampleDuration(stts, duration);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSamples(struct MP4SampleTableAtom *self, u32 sampleCount, u64 sampleOffset,
                         MP4Handle durationsH, MP4Handle sizesH, MP4Handle compositionOffsetsH,
                         MP4Handle syncSamplesH, MP4Handle padsH)
{
  u32 beginningSampleNumber;
  u32 sampleDescriptionIndex;
  MP4SampleDescriptionAtomPtr stsd;
  MP4TimeToSampleAtomPtr stts;
  MP4SampleSizeAtomPtr stsz;
  MP4SampleToChunkAtomPtr stsc;
  MP4ChunkOffsetAtomPtr stco;
  MP4CompositionOffsetAtomPtr ctts;
  MP4SyncSampleAtomPtr stss;
  MP4PaddingBitsAtomPtr padb;
  MP4CompositionToDecodeAtomPtr cslg;
  u32 groupCount;
  u32 i;
  MP4Err err;

  err  = MP4NoErr;
  stsd = (MP4SampleDescriptionAtomPtr)self->SampleDescription;
  if(stsd == NULL)
  {
    BAILWITHERROR(MP4InvalidMediaErr);
  }
  stts = (MP4TimeToSampleAtomPtr)self->TimeToSample;
  if(stts == NULL)
  {
    BAILWITHERROR(MP4InvalidMediaErr);
  }
  stsz = (MP4SampleSizeAtomPtr)self->SampleSize;
  if(stsz == NULL)
  {
    BAILWITHERROR(MP4InvalidMediaErr);
  }
  stsc = (MP4SampleToChunkAtomPtr)self->SampleToChunk;
  if(stsc == NULL)
  {
    BAILWITHERROR(MP4InvalidMediaErr);
  }
  stco = (MP4ChunkOffsetAtomPtr)self->ChunkOffset;
  if(stco == NULL)
  {
    BAILWITHERROR(MP4InvalidMediaErr);
  }
  ctts = NULL;
  if(compositionOffsetsH != NULL)
  {
    if(self->CompositionOffset == NULL)
    {
      MP4Err MP4CreateCompositionOffsetAtom(MP4CompositionOffsetAtomPtr * outAtom);
      err = MP4CreateCompositionOffsetAtom(&ctts);
      if(err) goto bail;
      if(self->useSignedCompositionTimeOffsets == 1)
      {
        MP4Err MP4CreateCompositionToDecodeAtom(MP4CompositionToDecodeAtomPtr * outAtom);
        ctts->version = 1;
        err           = MP4CreateCompositionToDecodeAtom(&cslg);
        if(err) goto bail;
        err = addAtom(self, (MP4AtomPtr)cslg);
        if(err) goto bail;
      }
      err = addAtom(self, (MP4AtomPtr)ctts);
      if(err) goto bail;
    }
    ctts = (MP4CompositionOffsetAtomPtr)self->CompositionOffset;
  }
  stss = NULL;
  if(syncSamplesH != NULL)
  {
    if(self->SyncSample == NULL)
    {
      /*
        create a sync sample atom when we get the first
        indication that all samples are not sync samples
       */
      u32 samplesInHandle;
      err = MP4GetHandleSize(syncSamplesH, &samplesInHandle);
      if(err) goto bail;
      samplesInHandle /= 4;
      if(samplesInHandle != sampleCount)
      {
        MP4Err MP4CreateSyncSampleAtom(MP4SyncSampleAtomPtr * outAtom);
        err = MP4CreateSyncSampleAtom(&stss);
        if(err) goto bail;
        err = addAtom(self, (MP4AtomPtr)stss);
        if(err) goto bail;
      }
    }
    stss = (MP4SyncSampleAtomPtr)self->SyncSample;
  }
  beginningSampleNumber = stsz->sampleCount + 1;
  err                   = stts->addSamples(stts, sampleCount, durationsH);
  if(err) goto bail;
  if(ctts)
  {
    err = ctts->addSamples(ctts, beginningSampleNumber, sampleCount, compositionOffsetsH);
    if(err) goto bail;
    if(self->useSignedCompositionTimeOffsets == 1)
    {
      cslg = (MP4CompositionToDecodeAtomPtr)self->CompositionToDecode;
      cslg->updateFields((MP4AtomPtr)cslg, sampleCount, durationsH, compositionOffsetsH);
      if(err) goto bail;
    }
  }
  if(stss)
  {
    err = stss->addSamples(stss, beginningSampleNumber, sampleCount, syncSamplesH);
    if(err) goto bail;
  }
  sampleDescriptionIndex = stsd->getEntryCount(stsd);
  if(sampleDescriptionIndex == 0)
  {
    BAILWITHERROR(MP4InvalidMediaErr);
  }

  padb = (MP4PaddingBitsAtomPtr)self->PaddingBits;
  if(padsH != NULL)
  {
    if(padb == NULL)
    {
      MP4Err MP4CreatePaddingBitsAtom(MP4PaddingBitsAtomPtr * outAtom);
      err = MP4CreatePaddingBitsAtom(&padb);
      if(err) goto bail;
      err = addAtom(self, (MP4AtomPtr)padb);
      if(err) goto bail;
      if(stsz->sampleCount > 0)
      {
        err = padb->addSamplePads(padb, stsz->sampleCount, NULL);
        if(err) goto bail;
      }
    }
    err = padb->addSamplePads(padb, sampleCount, padsH);
    if(err) goto bail;
  }
  else if(padb != NULL)
  {
    err = padb->addSamplePads(padb, sampleCount, NULL);
    if(err) goto bail;
  }

  err = stsz->addSamples(stsz, sampleCount, sizesH);
  if(err) goto bail;
  err = stco->addOffset(stco, sampleOffset);
  if(err) goto bail;
  err = stsc->setEntry(stsc, stco->getChunkCount((MP4AtomPtr)stco), sampleCount,
                       sampleDescriptionIndex);
  if(err) goto bail;

  err = MP4GetListEntryCount(self->sampletoGroupList, &groupCount);
  if(err) goto bail;
  for(i = 0; i < groupCount; i++)
  {
    MP4SampletoGroupAtomPtr desc;
    err = MP4GetListEntry(self->sampletoGroupList, i, (char **)&desc);
    if(err) goto bail;
    if(desc)
    {
      err = desc->addSamples(desc, sampleCount);
      if(err) goto bail;
    }
  }

  if(self->SampleDependency)
  {
    MP4SampleDependencyAtomPtr sdtp;
    sdtp = (MP4SampleDependencyAtomPtr)self->SampleDependency;
    err  = sdtp->addSamples(sdtp, sampleCount);
    if(err) goto bail;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4SampleTableAtomPtr self = (MP4SampleTableAtomPtr)s;
  err                        = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  SERIALIZE_ATOM_LIST(atomList);
  SERIALIZE_ATOM_LIST(SampleAuxiliaryInformationSizes);
  SERIALIZE_ATOM_LIST(SampleAuxiliaryInformationOffsets);
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4SampleTableAtomPtr self = (MP4SampleTableAtomPtr)s;
  err                        = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  ADD_ATOM_LIST_SIZE(atomList);
  ADD_ATOM_LIST_SIZE(SampleAuxiliaryInformationSizes);
  ADD_ATOM_LIST_SIZE(SampleAuxiliaryInformationOffsets);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setupNew(struct MP4SampleTableAtom *self)
{
  MP4Err MP4CreateTimeToSampleAtom(MP4TimeToSampleAtomPtr * outAtom);
  MP4Err MP4CreateSampleDescriptionAtom(MP4SampleDescriptionAtomPtr * outAtom);
  MP4Err MP4CreateSampleSizeAtom(MP4SampleSizeAtomPtr * outAtom);
  MP4Err MP4CreateSampleToChunkAtom(MP4SampleToChunkAtomPtr * outAtom);
  MP4Err MP4CreateChunkOffsetAtom(MP4ChunkOffsetAtomPtr * outAtom);

  MP4Err err;
  MP4TimeToSampleAtomPtr ctts;
  MP4SampleDescriptionAtomPtr stsd;
  MP4SampleSizeAtomPtr stsz;
  MP4SampleToChunkAtomPtr stsc;
  MP4ChunkOffsetAtomPtr stco;

  err = MP4NoErr;

  if(!(self->TimeToSample))
  {
    err = MP4CreateTimeToSampleAtom(&ctts);
    if(err) goto bail;
    err = addAtom(self, (MP4AtomPtr)ctts);
    if(err) goto bail;
  }
  if(!(self->SampleDescription))
  {
    err = MP4CreateSampleDescriptionAtom(&stsd);
    if(err) goto bail;
    err = addAtom(self, (MP4AtomPtr)stsd);
    if(err) goto bail;
  }
  if(!(self->SampleSize))
  {
    err = MP4CreateSampleSizeAtom(&stsz);
    if(err) goto bail;
    err = addAtom(self, (MP4AtomPtr)stsz);
    if(err) goto bail;
  }
  if(!(self->SampleToChunk))
  {
    err = MP4CreateSampleToChunkAtom(&stsc);
    if(err) goto bail;
    err = addAtom(self, (MP4AtomPtr)stsc);
    if(err) goto bail;
  }
  if(!(self->ChunkOffset))
  {
    err = MP4CreateChunkOffsetAtom(&stco);
    if(err) goto bail;
    err = addAtom(self, (MP4AtomPtr)stco);
    if(err) goto bail;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addGroupDescription(struct MP4SampleTableAtom *self, u32 theType,
                                  MP4Handle theDescription, u32 *index)
{
  MP4Err err;
  MP4SampleGroupDescriptionAtomPtr theGroup;

  err = MP4FindGroupAtom(self->groupDescriptionList, theType, (MP4AtomPtr *)&theGroup);
  if(!theGroup)
  {
    err = MP4CreateSampleGroupDescriptionAtom(&theGroup);
    if(err) goto bail;
    theGroup->grouping_type = theType;
    err                     = addAtom(self, (MP4AtomPtr)theGroup);
    if(err) goto bail;
  }
  err = theGroup->addGroupDescription(theGroup, theDescription, index);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getGroupDescription(struct MP4SampleTableAtom *self, u32 theType, u32 index,
                                  MP4Handle theDescription)
{
  MP4Err err;
  MP4SampleGroupDescriptionAtomPtr theGroup;

  err = MP4FindGroupAtom(self->groupDescriptionList, theType, (MP4AtomPtr *)&theGroup);
  if(!theGroup) BAILWITHERROR(MP4BadParamErr);
  err = theGroup->getGroupDescription(theGroup, index, theDescription);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mergeSampleGroupDescriptions(struct MP4SampleTableAtom *self,
                                           MP4AtomPtr otherSampleGroupDescr)
{
  MP4Err err, errTemp;
  MP4SampleGroupDescriptionAtomPtr stblGroup;
  MP4SampleGroupDescriptionAtomPtr other;
  u32 idx, dummy;
  MP4Handle descrH;

  if(otherSampleGroupDescr == NULL) BAILWITHERROR(MP4BadParamErr);

  err = errTemp = MP4NoErr;
  other         = (MP4SampleGroupDescriptionAtomPtr)otherSampleGroupDescr;
  idx           = 1;

  err =
    MP4FindGroupAtom(self->groupDescriptionList, other->grouping_type, (MP4AtomPtr *)&stblGroup);

  if(err == MP4NoErr && stblGroup != NULL)
  {
    /* group description of the same type is already present in stbl, add only entries which are not
     * yet present */
    while(errTemp == MP4NoErr)
    {
      MP4NewHandle(0, &descrH);
      errTemp = other->getGroupDescription(other, idx++, descrH);
      if(errTemp == MP4NoErr)
      {
        err = stblGroup->findGroupDescriptionIdx(stblGroup, descrH, &dummy);
        if(err == MP4NotFoundErr)
          err = addGroupDescription(self, other->grouping_type, descrH, &dummy);
      }
      MP4DisposeHandle(descrH);
    }
  }
  else
  {
    /* no such group description, iterate over all entries and add them */
    while(errTemp == MP4NoErr)
    {
      MP4NewHandle(0, &descrH);
      errTemp = other->getGroupDescription(other, idx++, descrH);
      if(errTemp == MP4NoErr) err = addGroupDescription(self, other->grouping_type, descrH, &dummy);
      MP4DisposeHandle(descrH);
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err changeSamplestoGroupType(struct MP4SampleTableAtom *self,
                                       sampleToGroupType_t sampleToGroupType)
{
  MP4Err err;
  u32 i, count;

  err = MP4GetListEntryCount(self->sampletoGroupList, &count);
  if(err) goto bail;

  for(i = 0; i < count; i++)
  {
    MP4AtomPtr desc;
    err = MP4GetListEntry(self->sampletoGroupList, i, (char **)&desc);
    if(err) goto bail;
    if(desc)
    {
      MP4SampletoGroupAtomPtr grp = (MP4SampletoGroupAtomPtr)desc;
      grp->changeSamplestoGroupType(grp, sampleToGroupType);
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mapSamplestoGroup(struct MP4SampleTableAtom *self, u32 groupType, u32 group_index,
                                s32 sample_index, u32 count, sampleToGroupType_t sampleToGroupType)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr theGroup;
  MP4SampleGroupDescriptionAtomPtr theDesc;

  err = MP4FindGroupAtom(self->groupDescriptionList, groupType, (MP4AtomPtr *)&theDesc);
  if(!theDesc) BAILWITHERROR(MP4BadParamErr);
  if(group_index > theDesc->groupCount) BAILWITHERROR(MP4BadParamErr);

  /* create one if the box doesn't extist */
  err = MP4FindGroupAtom(self->sampletoGroupList, groupType, (MP4AtomPtr *)&theGroup);
  if(!theGroup)
  {
    MP4SampleSizeAtomPtr stsz;

    stsz = (MP4SampleSizeAtomPtr)self->SampleSize;
    if(stsz == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
    err = MP4CreateSampletoGroupAtom(&theGroup, sampleToGroupType);
    if(err) goto bail;
    theGroup->grouping_type     = groupType;
    theGroup->groupIsInFragment = 0;
    err                         = addAtom(self, (MP4AtomPtr)theGroup);
    if(err) goto bail;
    err = theGroup->addSamples(theGroup, stsz->sampleCount);
    if(err) goto bail;
  }

  err = theGroup->mapSamplestoGroup(theGroup, group_index, sample_index, count);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupMap(struct MP4SampleTableAtom *self, u32 groupType, u32 sample_number,
                                u32 *group_index)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr theGroup;

  err = MP4FindGroupAtom(self->sampletoGroupList, groupType, (MP4AtomPtr *)&theGroup);
  if(theGroup)
  {
    err = theGroup->getSampleGroupMap(theGroup, sample_number, group_index);
    if(err) goto bail;
  }
  else
    BAILWITHERROR(MP4BadParamErr);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupSampleNumbers(struct MP4SampleTableAtom *self, u32 groupType,
                                          u32 groupIndex, u32 **outSampleNumbers, u32 *outSampleCnt)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr theGroup;
  u32 i, cur_index;
  *outSampleCnt = 0;

  err = MP4FindGroupAtom(self->sampletoGroupList, groupType, (MP4AtomPtr *)&theGroup);
  if(theGroup)
  {
    *outSampleNumbers = (u32 *)malloc((theGroup->sampleCount) * sizeof(u32));
    TESTMALLOC(*outSampleNumbers);
    cur_index = (theGroup->group_index)[0];
    for(i = 1; i < theGroup->sampleCount; i++)
    {
      if((theGroup->group_index)[i - 1] != (theGroup->group_index)[i])
      {
        if(cur_index == groupIndex)
        {
          (*outSampleNumbers)[(*outSampleCnt)++] = i;
        }
        cur_index = (theGroup->group_index)[i];
      }
    }
    if(cur_index == groupIndex)
    {
      (*outSampleNumbers)[(*outSampleCnt)++] = i;
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err setSampleDependency(struct MP4SampleTableAtom *self, s32 sample_index,
                                  MP4Handle dependencies)
{
  MP4Err err;
  MP4SampleDependencyAtomPtr sdtp;

  err  = MP4NoErr;
  sdtp = (MP4SampleDependencyAtomPtr)self->SampleDependency;
  if(!sdtp)
  {
    MP4Err MP4CreateSampleDependencyAtom(MP4SampleDependencyAtomPtr * outAtom);
    MP4SampleSizeAtomPtr stsz;

    stsz = (MP4SampleSizeAtomPtr)self->SampleSize;
    if(stsz == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
    err = MP4CreateSampleDependencyAtom(&sdtp);
    if(err) goto bail;
    err = addAtom(self, (MP4AtomPtr)sdtp);
    if(err) goto bail;
    if(stsz->sampleCount > 0)
    {
      err = sdtp->addSamples(sdtp, stsz->sampleCount);
      if(err) goto bail;
    }
  }
  assert(sdtp->setSampleDependency);
  err = sdtp->setSampleDependency(sdtp, sample_index, dependencies);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleDependency(struct MP4SampleTableAtom *self, u32 sampleNumber, u8 *dependency)
{
  MP4Err err;
  MP4SampleDependencyAtomPtr sdtp;

  err  = MP4NoErr;
  sdtp = (MP4SampleDependencyAtomPtr)self->SampleDependency;
  if(sdtp)
  {
    assert(sdtp->getSampleDependency);
    err = sdtp->getSampleDependency(sdtp, sampleNumber, dependency);
    if(err) goto bail;
  }
  else
  {
    *dependency = 0;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleAuxiliaryInformation(struct MP4SampleTableAtom *self,
                                            u8 isUsingAuxInfoPropertiesFlag, u32 aux_info_type,
                                            u32 aux_info_type_parameter,
                                            MP4SampleAuxiliaryInformationSizesAtomPtr *saizOut,
                                            MP4SampleAuxiliaryInformationOffsetsAtomPtr *saioOut)
{
  MP4Err err;
  u32 i;
  err = MP4NoErr;

  *saizOut = NULL;
  *saioOut = NULL;

  for(i = 0; i < self->SampleAuxiliaryInformationSizes->entryCount; i++)
  {
    MP4SampleAuxiliaryInformationSizesAtomPtr saizExisting;
    MP4SampleAuxiliaryInformationOffsetsAtomPtr saioExisting;
    err = MP4GetListEntry(self->SampleAuxiliaryInformationSizes, i, (char **)&saizExisting);
    err = MP4GetListEntry(self->SampleAuxiliaryInformationOffsets, i, (char **)&saioExisting);

    if(((saizExisting->flags & 1) == isUsingAuxInfoPropertiesFlag) &&
       (saizExisting->aux_info_type == aux_info_type) &&
       (saizExisting->aux_info_type_parameter == aux_info_type_parameter))
    {
      *saizOut = saizExisting;
    }

    if(((saioExisting->flags & 1) == isUsingAuxInfoPropertiesFlag) &&
       (saioExisting->aux_info_type == aux_info_type) &&
       (saioExisting->aux_info_type_parameter == aux_info_type_parameter))
    {
      *saioOut = saioExisting;
    }
  }

  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  PARSE_ATOM_LIST(MP4SampleTableAtom)

  err = self->setupNew((MP4SampleTableAtomPtr)s);
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSampleTableAtom(MP4SampleTableAtomPtr *outAtom)
{
  MP4Err err;
  MP4SampleTableAtomPtr self;

  self = (MP4SampleTableAtomPtr)calloc(1, sizeof(MP4SampleTableAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type = MP4SampleTableAtomType;
  self->name = "sample table";
  err        = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;
  self->createFromInputStream        = (cisfunc)createFromInputStream;
  self->destroy                      = destroy;
  self->calculateSize                = calculateSize;
  self->calculateDuration            = calculateDuration;
  self->setSampleEntry               = setSampleEntry;
  self->serialize                    = serialize;
  self->getCurrentDataReferenceIndex = getCurrentDataReferenceIndex;
  self->extendLastSampleDuration     = extendLastSampleDuration;
  self->addSamples                   = addSamples;
  self->setupNew                     = setupNew;
  self->setfieldsize                 = setfieldsize;
  self->getCurrentSampleEntryIndex   = getCurrentSampleEntryIndex;
  self->setDefaultSampleEntry        = setDefaultSampleEntry;

  self->addGroupDescription         = addGroupDescription;
  self->changeSamplestoGroupType    = changeSamplestoGroupType;
  self->mapSamplestoGroup           = mapSamplestoGroup;
  self->getSampleGroupMap           = getSampleGroupMap;
  self->getSampleGroupSampleNumbers = getSampleGroupSampleNumbers;
  self->getGroupDescription         = getGroupDescription;

  self->setSampleDependency = setSampleDependency;
  self->getSampleDependency = getSampleDependency;

  self->mergeSampleGroupDescriptions = mergeSampleGroupDescriptions;

  err = MP4MakeLinkedList(&(self->groupDescriptionList));
  if(err) goto bail;
  err = MP4MakeLinkedList(&(self->sampletoGroupList));
  if(err) goto bail;

  self->useSignedCompositionTimeOffsets = 0;
  self->getSampleAuxiliaryInformation   = getSampleAuxiliaryInformation;

  err = MP4MakeLinkedList(&(self->SampleAuxiliaryInformationSizes));
  if(err) goto bail;
  err = MP4MakeLinkedList(&(self->SampleAuxiliaryInformationOffsets));
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
