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
/*
  $Id: LoudnessAtom.c,v 1.1.1.1 2014/09/19 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

#ifdef ISMACrypt

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  u32 i;
  MP4LoudnessAtomPtr self = (MP4LoudnessAtomPtr)s;
  err                     = MP4NoErr;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  DESTROY_ATOM_LIST_V(self->albumLoudnessInfoList);
  DESTROY_ATOM_LIST_V(self->trackLoudnessInfoList);

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err addAtom(MP4LoudnessAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  if(atom->type == MP4TrackLoudnessInfoAtomType)
    err = MP4AddListEntry(atom, self->trackLoudnessInfoList);
  if(err) goto bail;
  if(atom->type == MP4AlbumLoudnessInfoAtomType)
    err = MP4AddListEntry(atom, self->albumLoudnessInfoList);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4LoudnessAtomPtr self = (MP4LoudnessAtomPtr)s;
  err                     = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields((MP4AtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  SERIALIZE_ATOM_LIST(trackLoudnessInfoList);
  SERIALIZE_ATOM_LIST(albumLoudnessInfoList);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serializeData(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4LoudnessAtomPtr self = (MP4LoudnessAtomPtr)s;
  err                     = MP4NoErr;

  SERIALIZE_ATOM_LIST(trackLoudnessInfoList);
  SERIALIZE_ATOM_LIST(albumLoudnessInfoList);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getDataSize(struct MP4Atom *s, u32 *dataSizeOut)
{
  MP4Err err;
  u32 baseAtomFieldSize;
  MP4LoudnessAtomPtr self;

  self       = (MP4LoudnessAtomPtr)s;
  self->size = 0;
  err        = MP4NoErr;
  err        = MP4CalculateBaseAtomFieldSize((MP4AtomPtr)s);
  if(err) goto bail;

  baseAtomFieldSize = self->size;

  ADD_ATOM_LIST_SIZE(trackLoudnessInfoList);
  ADD_ATOM_LIST_SIZE(albumLoudnessInfoList);

  *dataSizeOut = self->size - baseAtomFieldSize;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4LoudnessAtomPtr self;

  self = (MP4LoudnessAtomPtr)s;
  err  = MP4NoErr;
  err  = MP4CalculateBaseAtomFieldSize((MP4AtomPtr)s);
  if(err) goto bail;

  ADD_ATOM_LIST_SIZE(trackLoudnessInfoList);
  ADD_ATOM_LIST_SIZE(albumLoudnessInfoList);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4LoudnessAtomPtr self = (MP4LoudnessAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  while(self->bytesRead < self->size)
  {
    MP4AtomPtr atom;
    err = MP4ParseAtom(inputStream, &atom);
    if(err) goto bail;
    self->bytesRead += atom->size;
    err = addAtom(self, atom);
    if(err) goto bail;
  }

  assert(self->bytesRead == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateLoudnessAtom(MP4LoudnessAtomPtr *outAtom)
{
  MP4Err err;
  MP4LoudnessAtomPtr self;

  self = (MP4LoudnessAtomPtr)calloc(1, sizeof(MP4LoudnessAtom));
  TESTMALLOC(self);

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  err = MP4MakeLinkedList(&self->trackLoudnessInfoList);
  if(err) goto bail;
  err = MP4MakeLinkedList(&self->albumLoudnessInfoList);
  if(err) goto bail;

  self->type                  = MP4LoudnessAtomType;
  self->name                  = "Loudness";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addAtom               = addAtom;
  self->serializeData         = serializeData;
  self->getDataSize           = getDataSize;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}

#endif
