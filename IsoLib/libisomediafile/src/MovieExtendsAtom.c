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
  $Id: MovieExtendsAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4MovieExtendsAtomPtr self;

  self = (MP4MovieExtendsAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = MP4DeleteLinkedList(self->trackExtendsList);
  if(err) goto bail;
  err = MP4DeleteLinkedList(self->trackExtensionPropertiesList);
  if(err) goto bail;

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4MovieExtendsAtomPtr self = (MP4MovieExtendsAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  SERIALIZE_ATOM_LIST(trackExtendsList);
  SERIALIZE_ATOM_LIST(trackExtensionPropertiesList);

  assert(self->bytesWritten == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4MovieExtendsAtomPtr self = (MP4MovieExtendsAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  ADD_ATOM_LIST_SIZE(trackExtendsList);
  ADD_ATOM_LIST_SIZE(trackExtensionPropertiesList);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addAtom(MP4MovieExtendsAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);

  switch(atom->type)
  {
  case MP4TrackExtendsAtomType:
    err = MP4AddListEntry(atom, self->trackExtendsList);
    break;
  case MP4TrackExtensionPropertiesAtomType:
    err = MP4AddListEntry(atom, self->trackExtensionPropertiesList);
    break;
  default:
    BAILWITHERROR(MP4BadDataErr)
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err setCompositionToDecodeProperties(struct MP4MovieExtendsAtom *self, u32 trackID,
                                               s32 compositionToDTSShift,
                                               s32 leastDecodeToDisplayDelta,
                                               s32 greatestDecodeToDisplayDelta,
                                               s32 compositionStartTime, s32 compositionEndTime)
{
  MP4Err err;
  MP4TrackExtensionPropertiesAtomPtr trep;
  MP4CompositionToDecodeAtomPtr cslg;
  u8 createCslg;

  err        = MP4NoErr;
  createCslg = 0;
  cslg       = NULL;

  trep = NULL;
  self->getTrackExtensionPropertiesAtom(self, trackID, (MP4AtomPtr *)&trep);

  if(trep != NULL)
  {
    cslg = NULL;
    trep->getAtom(trep, MP4CompositionToDecodeAtomType, (MP4AtomPtr *)&cslg);
    if(cslg == NULL) createCslg = 1;
  }
  else
  {
    err = MP4CreateTrackExtensionPropertiesAtom(&trep);
    if(err) goto bail;
    trep->track_id = trackID;
    err            = self->addAtom(self, (MP4AtomPtr)trep);
    if(err) goto bail;
    createCslg = 1;
  }

  if(createCslg == 1)
  {
    err = MP4CreateCompositionToDecodeAtom(&cslg);
    if(err) goto bail;
    trep->addAtom(trep, (MP4AtomPtr)cslg);
  }

  if(cslg)
  {
    cslg->compositionToDTSShift        = compositionToDTSShift;
    cslg->leastDecodeToDisplayDelta    = leastDecodeToDisplayDelta;
    cslg->greatestDecodeToDisplayDelta = greatestDecodeToDisplayDelta;
    cslg->compositionStartTime         = compositionStartTime;
    cslg->compositionEndTime           = compositionEndTime;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err maketrackfragments(struct MP4MovieExtendsAtom *self, MP4MovieFragmentAtomPtr moof,
                                 MP4MovieAtomPtr moov, MP4MediaDataAtomPtr mdat)
{
  u32 i;
  MP4Err err;
  u32 trackCount;
  err = MP4NoErr;

  MP4GetListEntryCount(self->trackExtendsList, &trackCount);
  for(i = 0; i < trackCount; i++)
  {
    MP4TrackExtendsAtomPtr trex;
    MP4TrackFragmentAtomPtr traf;
    MP4TrackFragmentHeaderAtomPtr tfhd;
    MP4TrackFragmentDecodeTimeAtomPtr tfdt;

    err = MP4GetListEntry(self->trackExtendsList, i, (char **)&trex);
    if(err) goto bail;

    err = MP4CreateTrackFragmentAtom(&traf);
    if(err) goto bail;
    err = MP4CreateTrackFragmentHeaderAtom(&tfhd);
    if(err) goto bail;
    err = MP4CreateTrackFragmentDecodeTimeAtom(&tfdt);
    if(err) goto bail;

    traf->tfhd = (MP4AtomPtr)tfhd;
    traf->tfdt = (MP4AtomPtr)tfdt;
    traf->trex = (MP4AtomPtr)trex;
    traf->mdat = mdat;

    traf->default_sample_description_index = trex->default_sample_description_index;
    traf->default_sample_duration          = trex->default_sample_duration;
    traf->default_sample_size              = trex->default_sample_size;
    traf->default_sample_flags             = trex->default_sample_flags;

    if(trex->isInitialMediaDecodeTimeAdded == 0)
    {
      MP4MediaAtomPtr mdia;
      MP4Track trak;
      u64 initialMediaDuration;

      err = moov->getTrackMedia(moov, trex->trackID, (MP4AtomPtr *)&mdia);
      if(err) goto bail;
      err = MP4GetMediaTrack((MP4Media)mdia, &trak);
      if(err) goto bail;
      err = MP4GetMediaDuration((MP4Media)mdia, &initialMediaDuration);
      if(err) goto bail;

      trex->baseMediaDecodeTime           = (u32)initialMediaDuration;
      trex->isInitialMediaDecodeTimeAdded = 1;
    }

    tfdt->baseMediaDecodeTime = trex->baseMediaDecodeTime;

    tfhd->trackID                  = trex->trackID;
    tfhd->sample_description_index = trex->default_sample_description_index;
    /* if we ever allow flipping of sample descriptions, this should be copied from the
       last fragment, and only the movie for the first one */

    err = moov->settrackfragment(moov, trex->trackID, (MP4AtomPtr)traf);
    if(err) goto bail;
    err = moof->addAtom(moof, (MP4AtomPtr)traf);
    if(err) goto bail;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getTrackExtendsAtom(struct MP4MovieExtendsAtom *self, u32 trackID,
                                  MP4AtomPtr *outTrack)
{
  u32 i;
  MP4Err err;
  u32 trackCount;
  err = MP4NoErr;

  MP4GetListEntryCount(self->trackExtendsList, &trackCount);
  for(i = 0; i < trackCount; i++)
  {
    MP4TrackExtendsAtomPtr trex;

    err = MP4GetListEntry(self->trackExtendsList, i, (char **)&trex);
    if(err) goto bail;

    if((trex->type == MP4TrackExtendsAtomType) && (trex->trackID == trackID))
    {
      *outTrack = (MP4AtomPtr)trex;
      break;
    }
  }
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err getTrackExtensionPropertiesAtom(struct MP4MovieExtendsAtom *self, u32 trackID,
                                              MP4AtomPtr *outProbs)
{
  u32 i;
  MP4Err err;
  u32 probCount;
  err = MP4NoErr;

  MP4GetListEntryCount(self->trackExtensionPropertiesList, &probCount);
  for(i = 0; i < probCount; i++)
  {
    MP4TrackExtensionPropertiesAtomPtr trep;

    err = MP4GetListEntry(self->trackExtendsList, i, (char **)&trep);
    if(err) goto bail;

    if((trep->type == MP4TrackExtensionPropertiesAtomType) && (trep->track_id == trackID))
    {
      *outProbs = (MP4AtomPtr)trep;
      break;
    }
  }
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err setSampleDescriptionIndexes(struct MP4MovieExtendsAtom *self, MP4AtomPtr moov)
{
  u32 i;
  MP4Err err;
  u32 trackCount;
  err = MP4NoErr;

  MP4GetListEntryCount(self->trackExtendsList, &trackCount);
  for(i = 0; i < trackCount; i++)
  {
    MP4TrackExtendsAtomPtr trex;
    u32 sd_index;

    err = MP4GetListEntry(self->trackExtendsList, i, (char **)&trex);
    if(err) goto bail;

    err = ((MP4MovieAtomPtr)moov)
            ->getSampleDescriptionIndex((MP4MovieAtom *)moov, trex->trackID, &sd_index);
    if(err) goto bail;
    trex->default_sample_description_index = sd_index;
  }
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4MovieExtendsAtomPtr self = (MP4MovieExtendsAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GETATOM_LIST(trackExtendsList);
  GETATOM_LIST(trackExtensionPropertiesList);

  assert(self->bytesRead == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateMovieExtendsAtom(MP4MovieExtendsAtomPtr *outAtom)
{
  MP4Err err;
  MP4MovieExtendsAtomPtr self;

  self = (MP4MovieExtendsAtomPtr)calloc(1, sizeof(MP4MovieExtendsAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                             = MP4MovieExtendsAtomType;
  self->name                             = "movie extends";
  self->createFromInputStream            = (cisfunc)createFromInputStream;
  self->destroy                          = destroy;
  self->calculateSize                    = calculateSize;
  self->serialize                        = serialize;
  self->addAtom                          = addAtom;
  self->maketrackfragments               = maketrackfragments;
  self->getTrackExtendsAtom              = getTrackExtendsAtom;
  self->getTrackExtensionPropertiesAtom  = getTrackExtensionPropertiesAtom;
  self->setSampleDescriptionIndexes      = setSampleDescriptionIndexes;
  self->setCompositionToDecodeProperties = setCompositionToDecodeProperties;

  err = MP4MakeLinkedList(&self->trackExtendsList);
  if(err) goto bail;
  err = MP4MakeLinkedList(&self->trackExtensionPropertiesList);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
