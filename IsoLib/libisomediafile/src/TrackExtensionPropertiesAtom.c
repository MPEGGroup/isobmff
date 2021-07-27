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
  $Id: TrackExtensionPropertiesAtom.c,v 1.1.1.1 2014/10/22 08:10:00 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4TrackExtensionPropertiesAtomPtr self = (MP4TrackExtensionPropertiesAtomPtr)s;
  if(self == NULL) return;
  if(self->super) self->super->destroy(s);
}

static MP4Err getAtom(MP4TrackExtensionPropertiesAtomPtr self, u32 atomType, MP4AtomPtr *outAtom)
{
  u32 i;
  MP4Err err;
  u32 count;
  err = MP4NoErr;

  MP4GetListEntryCount(self->atomList, &count);
  for(i = 0; i < count; i++)
  {
    MP4AtomPtr atom;

    err = MP4GetListEntry(self->atomList, i, (char **)&atom);
    if(err) goto bail;

    if(atom->type == atomType)
    {
      *outAtom = atom;
      break;
    }
  }
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err addAtom(MP4TrackExtensionPropertiesAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  err = MP4AddListEntry(atom, self->atomList);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4TrackExtensionPropertiesAtomPtr self = (MP4TrackExtensionPropertiesAtomPtr)s;
  err                                     = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUT32(track_id);
  SERIALIZE_ATOM_LIST(atomList);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4TrackExtensionPropertiesAtomPtr self = (MP4TrackExtensionPropertiesAtomPtr)s;
  err                                     = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  self->size += 4;

  ADD_ATOM_LIST_SIZE(atomList);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4TrackExtensionPropertiesAtomPtr self = (MP4TrackExtensionPropertiesAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;
  GET32(track_id);
  GETATOM_LIST(atomList);

  assert(self->bytesRead == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateTrackExtensionPropertiesAtom(MP4TrackExtensionPropertiesAtomPtr *outAtom)
{
  MP4Err err;
  MP4TrackExtensionPropertiesAtomPtr self;

  self = (MP4TrackExtensionPropertiesAtomPtr)calloc(1, sizeof(MP4TrackExtensionPropertiesAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4TrackExtensionPropertiesAtomType;
  self->name                  = "track extension properties";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->track_id              = 0;
  self->addAtom               = addAtom;
  self->getAtom               = getAtom;

  err = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
