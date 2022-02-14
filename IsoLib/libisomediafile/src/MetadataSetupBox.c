/* This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4.
 * Those intending to use this software module in hardware or software products are advised that its
 * use may infringe existing patents. The original developer of this software module and his/her
 * company, the subsequent editors and their companies, and ISO/IEC have no liability for use of
 * this software module or modifications thereof in an implementation. Copyright is not released for
 * non MPEG-4 conforming products. Apple Computer, Inc. retains full right to use the code for its
 * own purpose, assign or donate the code to a third party and to inhibit third parties from using
 * the code for non MPEG-4 conforming products. This copyright notice must be included in all copies
 * or derivative works. Copyright (c) 2022.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  u32 i;
  MP4MetadataSetupBoxPtr self = (MP4MetadataSetupBoxPtr)s;
  if(self == NULL) return;

  DESTROY_ATOM_LIST;
  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);
  return;
}

static MP4Err addAtom(MP4MetadataSetupBoxPtr self, MP4AtomPtr atom)
{
  MP4Err err = MP4NoErr;

  if(self == 0) BAILWITHERROR(MP4BadParamErr);
  err = MP4AddListEntry(atom, self->atomList);

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err                  = MP4NoErr;
  MP4MetadataSetupBoxPtr self = (MP4MetadataSetupBoxPtr)s;

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
  MP4MetadataSetupBoxPtr self = (MP4MetadataSetupBoxPtr)s;
  err                         = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  ADD_ATOM_LIST_SIZE(atomList);

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  PARSE_ATOM_LIST(MP4MetadataSetupBox);
bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateMetadataSetupBox(MP4MetadataSetupBoxPtr *outAtom)
{
  MP4Err err;
  MP4MetadataSetupBoxPtr self;

  self = (MP4MetadataSetupBoxPtr)calloc(1, sizeof(MP4MetadataSetupBox));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4MetadataSetupBoxType;
  self->name                  = "MetadataSetupBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addAtom               = addAtom;
  err                         = MP4MakeLinkedList(&self->atomList);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
