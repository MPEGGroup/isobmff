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
  $Id: MovieFragmentAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4MovieFragmentAtomPtr self;

  self = (MP4MovieFragmentAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  DESTROY_ATOM_LIST_F(atomList);
  (self->mfhd)->destroy((MP4AtomPtr)(self->mfhd));
  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);

  return;
}

static MP4Err mdatMoved(struct MP4MovieAtom *s, u64 mdatBase, u64 mdatEnd, s32 mdatOffset)
{
  u32 trackCount;
  u32 i;
  MP4Err err;
  MP4MovieFragmentAtomPtr self;

  self = (MP4MovieFragmentAtomPtr)s;

  err = MP4NoErr;

  MP4GetListEntryCount(self->atomList, &trackCount);

  for(i = 0; i < trackCount; i++)
  {
    MP4TrackFragmentAtomPtr traf;
    err = MP4GetListEntry(self->atomList, i, (char **)&traf);
    if(err) goto bail;
    if(traf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
    err = traf->mdatMoved((struct MP4MediaInformationAtom *)traf, mdatBase, mdatEnd, mdatOffset);
    if(err) goto bail;
  }
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err mergeFragments(struct MP4MovieFragmentAtom *self, MP4MovieAtomPtr moov)
{
  u32 trackCount;
  u32 i;
  MP4Err err;

  u64 base_offset;
  u32 traf_data_end;

  err = MP4NoErr;

  MP4GetListEntryCount(self->atomList, &trackCount);

  base_offset = self->streamOffset;

  for(i = 0; i < trackCount; i++)
  {
    MP4TrackFragmentAtomPtr traf;
    MP4TrackFragmentHeaderAtomPtr tfhd;
    MP4TrackExtendsAtomPtr trex;
    MP4MediaAtomPtr mdia;
    MP4Track trak;
    u64 mediaDuration, initialMediaDuration;
    u32 tfhd_flags;
    u32 groupDescriptionCount;
    ISOHandle desc;
    u32 n;
    MP4SampleGroupDescriptionAtomPtr groupDescriptionTraf;
    MP4MediaInformationAtomPtr minf;
    MP4SampleTableAtomPtr stbl;

    err = MP4GetListEntry(self->atomList, i, (char **)&traf);
    if(err) goto bail;
    if(traf == NULL) BAILWITHERROR(MP4InvalidMediaErr);
    tfhd = (MP4TrackFragmentHeaderAtomPtr)traf->tfhd;

    err = moov->getTrackExtendsAtom(moov, tfhd->trackID, (MP4AtomPtr *)&trex);
    if(err) goto bail;

    traf->default_sample_description_index = trex->default_sample_description_index;
    traf->default_sample_duration          = trex->default_sample_duration;
    traf->default_sample_size              = trex->default_sample_size;
    traf->default_sample_flags             = trex->default_sample_flags;

    tfhd_flags = tfhd->flags;

    if((tfhd_flags & tfhd_sample_description_index_present) == 0)
      tfhd->sample_description_index = trex->default_sample_description_index;
    if((tfhd_flags & tfhd_default_sample_duration_present) == 0)
      tfhd->default_sample_duration = trex->default_sample_duration;
    if((tfhd_flags & tfhd_default_sample_size_present) == 0)
      tfhd->default_sample_size = trex->default_sample_size;
    if((tfhd_flags & tfhd_default_sample_flags_present) == 0)
      tfhd->default_sample_flags = trex->default_sample_flags;
    if((tfhd_flags & tfhd_base_data_offset_present) == 0)
      tfhd->base_data_offset =
        (tfhd_flags & tfhd_default_base_is_moof ? self->streamOffset : base_offset);

    err = traf->calculateDataEnd(traf, &traf_data_end);
    if(err) goto bail;
    base_offset = traf_data_end;

    err = moov->getTrackMedia(moov, tfhd->trackID, (MP4AtomPtr *)&mdia);
    if(err) goto bail;
    err = MP4GetMediaTrack((MP4Media)mdia, &trak);
    if(err) goto bail;

    /* merge sample group descriptions */
    minf = (MP4MediaInformationAtomPtr)mdia->information;
    stbl = (MP4SampleTableAtomPtr)minf->sampleTable;

    ISONewHandle(0, &desc);
    MP4GetListEntryCount(traf->groupDescriptionList, &groupDescriptionCount);
    for(n = 0; n < groupDescriptionCount; n++)
    {
      MP4GetListEntry(traf->groupDescriptionList, n, (char **)&groupDescriptionTraf);
      assert(groupDescriptionTraf != NULL);
      err = stbl->mergeSampleGroupDescriptions(stbl, (MP4AtomPtr)groupDescriptionTraf);
    }

    err = MP4GetMediaDuration((MP4Media)mdia, &initialMediaDuration);
    if(err) goto bail;

    if(traf->tfdt)
    {
      MP4TrackFragmentDecodeTimeAtomPtr tfdt;
      tfdt = (MP4TrackFragmentDecodeTimeAtomPtr)traf->tfdt;

      if(tfdt->baseMediaDecodeTime < initialMediaDuration) BAILWITHERROR(MP4InvalidMediaErr);

      if(tfdt->baseMediaDecodeTime > initialMediaDuration)
      {
        u32 duration = (tfdt->baseMediaDecodeTime - initialMediaDuration) & 0xffffffff;
        err          = mdia->extendLastSampleDuration(mdia, duration);
      }
      if(err) goto bail;
    }

    if((tfhd_flags & tfhd_duration_is_empty)) /* BAILWITHERROR( MP4NotImplementedErr ) */
    {
      /* if there is not already an edit list, and there is media, insert an edit covering the
       * current track duration */
      if((initialMediaDuration > 0) && (((MP4TrackAtomPtr)trak)->trackEditAtom == 0))
      {
        err = MP4InsertMediaIntoTrack(trak, -1, 0, initialMediaDuration, 1);
        if(err) goto bail;
      }
      err = MP4InsertMediaIntoTrack(trak, -1, -1, tfhd->default_sample_duration, 1);
      if(err) goto bail;
    }
    else
    {
      err = MP4BeginMediaEdits((MP4Media)mdia);
      if(err) goto bail;
      err = traf->mergeRuns(traf, mdia);

      err = traf->mergeSampleAuxiliaryInformation(traf, mdia);

      err = MP4EndMediaEdits((MP4Media)mdia);
      if(err) goto bail;

      err = MP4GetMediaDuration((MP4Media)mdia, &mediaDuration);
      if(err) goto bail;
      if(((MP4TrackAtomPtr)trak)->trackEditAtom)
      {
        err = MP4InsertMediaIntoTrack(trak, -1, (s32)initialMediaDuration,
                                      mediaDuration - initialMediaDuration, 1);
        if(err) goto bail;
      }
    }
  }
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4MovieFragmentAtomPtr self = (MP4MovieFragmentAtomPtr)s;
  err                          = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  SERIALIZE_ATOM(mfhd);
  SERIALIZE_ATOM_LIST(atomList);
  assert(self->bytesWritten == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4MovieFragmentAtomPtr self = (MP4MovieFragmentAtomPtr)s;
  err                          = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  ADD_ATOM_SIZE(mfhd);
  ADD_ATOM_LIST_SIZE(atomList);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateDuration(struct MP4MovieAtom *self)
{
  MP4Err err;
  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addAtom(MP4MovieFragmentAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);
  switch(atom->type)
  {
  case MP4MovieFragmentHeaderAtomType:
    self->mfhd = atom;
    break;
  case MP4TrackFragmentAtomType:
  default:
    err = MP4AddListEntry(atom, self->atomList);
    break;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  PARSE_ATOM_LIST(MP4MovieFragmentAtom)
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateMovieFragmentAtom(MP4MovieFragmentAtomPtr *outAtom)
{
  MP4Err err;
  MP4MovieFragmentAtomPtr self;

  self = (MP4MovieFragmentAtomPtr)calloc(1, sizeof(MP4MovieFragmentAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4MovieFragmentAtomType;
  self->name                  = "movie fragment";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  err                         = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;
  self->calculateSize     = calculateSize;
  self->serialize         = serialize;
  self->mdatMoved         = mdatMoved;
  self->calculateDuration = calculateDuration;
  self->addAtom           = addAtom;
  self->mergeFragments    = mergeFragments;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
