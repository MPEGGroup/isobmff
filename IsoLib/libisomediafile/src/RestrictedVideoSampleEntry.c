/*
 * This software module was originally developed by InterDigital, Inc.
 * in the course of development of MPEG-4.
 * This software module is an implementation of a part of one or
 * more MPEG-4 tools as specified by MPEG-4.
 * ISO/IEC gives users of MPEG-4 free license to this
 * software module or modifications thereof for use in hardware
 * or software products claiming conformance to MPEG-4 only for evaluation and testing purposes.
 * Those intending to use this software module in hardware or software
 * products are advised that its use may infringe existing patents.
 * The original developer of this software module and his/her company,
 * the subsequent editors and their companies, and ISO/IEC have no
 * liability for use of this software module or modifications thereof
 * in an implementation.
 *
 * Copyright is not released for non MPEG-4 conforming
 * products. InterDigital, Inc. retains full right to use the code for its own
 * purpose, assign or donate the code to a third party and to
 * inhibit third parties from using the code for non
 * MPEG-4 conforming products.
 *
 * This copyright notice must be included in all copies or
 * derivative works.
 */

/**
 * @file RestrictedVideoSampleEntry.c
 * @author Ahmed Hamza <Ahmed.Hamza@InterDigital.com>
 * @date January 19, 2018
 * @brief Implements functions for reading and writing RestrictedVideoSampleEntryAtom instances.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4RestrictedVideoSampleEntryAtomPtr self;
  err  = MP4NoErr;
  self = (MP4RestrictedVideoSampleEntryAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  DESTROY_ATOM_LIST_F(ExtensionAtomList)

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;
  err                                       = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUTBYTES(self->reserved1, 6);
  PUT16(dataReferenceIndex);
  PUTBYTES(self->reserved2, 16);
  PUT16(width);
  PUT16(height);
  /* PUT32( reserved3 ); */
  PUT32(reserved4);
  PUT32(reserved5);
  PUT32(reserved6);
  PUT16(reserved7);
  PUT8(nameLength);
  PUTBYTES(self->name31, 31);
  PUT16(reserved8);
  PUT16(reserved9);

  SERIALIZE_ATOM_LIST(ExtensionAtomList);

  assert(self->bytesWritten == self->size);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;
  err                                       = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  self->size +=
    (6 + 16 + 31 + (4 * 2) + (1 * 1) + (4 * 4)); /* TODO this probably includes restriction_type */
  ADD_ATOM_LIST_SIZE(ExtensionAtomList);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addAtom(MP4RestrictedVideoSampleEntryAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  if(atom == NULL) BAILWITHERROR(MP4BadParamErr);
  err = MP4AddListEntry(atom, self->ExtensionAtomList);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GETBYTES(6, reserved1);
  GET16(dataReferenceIndex);
  GETBYTES(16, reserved2);
  GET16(width);
  GET16(height);
  /* GET32( reserved3 ); */
  GET32(reserved4);
  GET32(reserved5);
  GET32(reserved6);
  GET16(reserved7);
  GET8(nameLength);
  GETBYTES(31, name31);
  GET16(reserved8);
  GET16(reserved9);
  GETATOM_LIST(ExtensionAtomList);

  if(self->bytesRead != self->size) BAILWITHERROR(MP4BadDataErr);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err transform(struct MP4Atom *s, u32 sch_type, u32 sch_version, char *sch_url)
{
  MP4Err err;

  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;

  MP4RestrictedSchemeInfoAtomPtr rinf;
  MP4OriginalFormatAtomPtr frma;
  MP4SchemeTypeAtomPtr schm = NULL;
  MP4SchemeInfoAtomPtr schi = NULL;
  char *sch_url_copy        = NULL;

  err = MP4CreateOriginalFormatAtom(&frma);
  if(err) goto bail;
  frma->original_format = self->type;

  err = MP4CreateSchemeInfoAtom(&schi);
  if(err) goto bail;

  err = MP4CreateSchemeTypeAtom(&schm);
  if(err) goto bail;
  schm->scheme_type    = sch_type;
  schm->scheme_version = sch_version;

  if(sch_url)
  {
    sch_url_copy = (char *)calloc(1, strlen(sch_url) + 1);
    TESTMALLOC(sch_url_copy);
    memcpy(sch_url_copy, sch_url, strlen(sch_url) + 1);
    schm->scheme_url = sch_url_copy;
    sch_url_copy     = NULL;
  }
  else
    schm->scheme_url = NULL;

  /* create 'rinf' atom */
  err = MP4CreateRestrictedSchemeInfoAtom(&rinf);
  if(err) goto bail;

  /* assign */
  rinf->addAtom(rinf, (MP4AtomPtr)frma);
  frma = NULL;
  rinf->addAtom(rinf, (MP4AtomPtr)schm);
  schm = NULL;
  rinf->addAtom(rinf, (MP4AtomPtr)schi);
  schi = NULL;

  self->type = self->restriction_type;

  /* set */
  self->addAtom(self, (MP4AtomPtr)rinf);

bail:
  if(frma) frma->destroy((MP4AtomPtr)frma);
  if(schm) schm->destroy((MP4AtomPtr)schm);
  if(schi) schi->destroy((MP4AtomPtr)schi);
  if(sch_url_copy) free(sch_url_copy);

  TEST_RETURN(err);

  return err;
}

static MP4Err untransform(struct MP4Atom *s)
{
  MP4Err err;
  u32 atomListSize;
  u32 i, index = 0;
  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;
  MP4OriginalFormatAtomPtr fmt;
  MP4RestrictedSchemeInfoAtomPtr rinf;

  err  = MP4NoErr;
  rinf = NULL;

  err = MP4GetListEntryCount(self->ExtensionAtomList, &atomListSize);
  if(err) goto bail;
  for(i = 0; i < atomListSize; i++)
  {
    MP4AtomPtr a;
    err = MP4GetListEntry(self->ExtensionAtomList, i, (char **)&a);
    if(err) goto bail;
    if(a->type == MP4RestrictedSchemeInfoAtomType)
    {
      index = i;
      rinf  = (MP4RestrictedSchemeInfoAtomPtr)a;
      break;
    }
  }

  if(!rinf)
  {
    err = MP4BadParamErr;
    goto bail;
  }

  fmt = (MP4OriginalFormatAtomPtr)rinf->MP4OriginalFormat;
  if(!fmt)
  {
    err = MP4BadDataErr;
    goto bail;
  }

  self->type = fmt->original_format;
  rinf->destroy((MP4AtomPtr)rinf);

  err = MP4DeleteListEntry(self->ExtensionAtomList, index);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSchemeInfoAtom(struct MP4Atom *s, struct MP4Atom *theAtom)
{
  MP4Err err;

  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;
  MP4SchemeInfoAtomPtr schi;
  MP4RestrictedSchemeInfoAtomPtr rinf;

  err = self->getRinf((MP4AtomPtr)self, (MP4AtomPtr *)&rinf);
  if(err) goto bail;
  if(!rinf)
  {
    err = MP4BadParamErr;
    goto bail;
  }

  schi = (MP4SchemeInfoAtomPtr)rinf->MP4SchemeInfo;
  if(!schi)
  {
    err = MP4BadDataErr;
    goto bail;
  }

  err = schi->addAtom(schi, theAtom);
  if(err) goto bail;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSchemeInfoAtom(struct MP4Atom *s, u32 theType, struct MP4Atom **theAtom)
{
  MP4Err err;

  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;
  MP4SchemeInfoAtomPtr schi;
  MP4RestrictedSchemeInfoAtomPtr rinf;

  err = MP4NoErr;

  err = self->getRinf((MP4AtomPtr)self, (MP4AtomPtr *)&rinf);
  if(err) goto bail;
  if(!rinf)
  {
    err = MP4BadParamErr;
    goto bail;
  }

  schi = (MP4SchemeInfoAtomPtr)rinf->MP4SchemeInfo;
  if(!schi)
  {
    err = MP4BadDataErr;
    goto bail;
  }

  err      = MP4BadParamErr;
  *theAtom = NULL;

  if(schi->atomList)
  {
    u32 count;
    u32 i;
    struct MP4Atom *desc;
    err = MP4GetListEntryCount(schi->atomList, &count);
    if(err) goto bail;
    for(i = 0; i < count; i++)
    {
      err = MP4GetListEntry(schi->atomList, i, (char **)&desc);
      if(err) goto bail;
      if(desc && (desc->type == theType))
      {
        *theAtom = desc;
        err      = MP4NoErr;
        break;
      }
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getRinf(struct MP4Atom *s, struct MP4Atom **theAtom)
{
  MP4Err err;
  u32 atomListSize;
  u32 i;
  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;

  err = MP4GetListEntryCount(self->ExtensionAtomList, &atomListSize);
  if(err) goto bail;
  for(i = 0; i < atomListSize; i++)
  {
    MP4AtomPtr a;
    err = MP4GetListEntry(self->ExtensionAtomList, i, (char **)&a);
    if(err) goto bail;
    if(a->type == MP4RestrictedSchemeInfoAtomType)
    {
      *theAtom = a;
      return MP4NoErr;
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err getScheme(struct MP4Atom *s, u32 *sch_type, u32 *sch_version, char **sch_url)
{
  MP4Err err;

  MP4RestrictedVideoSampleEntryAtomPtr self = (MP4RestrictedVideoSampleEntryAtomPtr)s;
  MP4SchemeTypeAtomPtr schm;
  MP4RestrictedSchemeInfoAtomPtr rinf;
  char *sch_url_copy;

  err = MP4NoErr;

  err = self->getRinf((MP4AtomPtr)self, (MP4AtomPtr *)&rinf);
  if(err) goto bail;
  if(!rinf)
  {
    err = MP4BadParamErr;
    goto bail;
  }

  schm = (MP4SchemeTypeAtomPtr)rinf->MP4SchemeType;
  if(!schm)
  {
    err = MP4BadDataErr;
    goto bail;
  }

  *sch_type    = schm->scheme_type;
  *sch_version = schm->scheme_version;

  if(sch_url)
  {
    sch_url_copy = (char *)calloc(1, strlen(schm->scheme_url) + 1);
    TESTMALLOC(sch_url_copy);
    memcpy(sch_url_copy, schm->scheme_url, strlen(schm->scheme_url) + 1);
    *sch_url = sch_url_copy;
  }

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateRestrictedVideoSampleEntryAtom(MP4RestrictedVideoSampleEntryAtomPtr *outAtom)
{
  MP4Err err;
  MP4RestrictedVideoSampleEntryAtomPtr self;

  self = (MP4RestrictedVideoSampleEntryAtomPtr)calloc(1, sizeof(MP4RestrictedVideoSampleEntryAtom));
  TESTMALLOC(self);

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->untransform       = untransform;
  self->addSchemeInfoAtom = addSchemeInfoAtom;
  self->getSchemeInfoAtom = getSchemeInfoAtom;
  self->getRinf           = getRinf;
  self->getScheme         = getScheme;
  self->transform         = transform;
  self->addAtom           = addAtom;

  err = MP4MakeLinkedList(&self->ExtensionAtomList);
  if(err) goto bail;

  self->type                  = MP4RestrictedVideoSampleEntryAtomType;
  self->name                  = "Restricted Video Sample Entry";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->restriction_type      = MP4RestrictedVideoSampleEntryAtomType;

  self->width     = 0x140;
  self->height    = 0xf0;
  self->reserved4 = 0x00480000;
  self->reserved5 = 0x00480000;
  self->reserved7 = 1;
  self->reserved8 = 0x18;
  self->reserved9 = -1;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
