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
 * @file TrackTypeAtom.c
 * @author Ahmed Hamza <Ahmed.Hamza@InterDigital.com>
 * @date
 * @brief Implements functions for reading and writing TrackTypeAtom instances.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4TrackTypeAtomPtr self = (MP4TrackTypeAtomPtr)s;
  if(self == NULL) return;
  if(self->compatibilityList)
  {
    free(self->compatibilityList);
    self->compatibilityList = NULL;
  }
  if(self->super) self->super->destroy(s);
}

static ISOErr serialize(struct MP4Atom *s, char *buffer)
{
  ISOErr err;
  u32 i;
  MP4TrackTypeAtomPtr self = (MP4TrackTypeAtomPtr)s;

  err = ISONoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtom *)s, buffer);
  if(err) goto bail; /* Full Atom */
  buffer += self->bytesWritten;

  PUT32(majorBrand);
  PUT32(minorVersion);

  for(i = 0; i < self->itemCount; i++)
  {
    PUT32_V((self->compatibilityList[i]));
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static ISOErr calculateSize(struct MP4Atom *s)
{
  ISOErr err;
  MP4TrackTypeAtomPtr self = (MP4TrackTypeAtomPtr)s;
  err                      = ISONoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;                           /* Full Atom */
  self->size += 2 * sizeof(u32);               /* brand and minorVersion */
  self->size += self->itemCount * sizeof(u32); /* compatibilityList */
bail:
  TEST_RETURN(err);

  return err;
}

static ISOErr getBrand(struct MP4TrackTypeAtom *self, u32 *standard, u32 *minorversion)
{
  *standard     = self->majorBrand;
  *minorversion = self->minorVersion;

  return MP4NoErr;
}

static u32 getStandard(struct MP4TrackTypeAtom *self, u32 standard)
{
  u32 i;
  u32 outval;

  outval = 0;

  for(i = 0; i < self->itemCount; i++)
  {
    if(self->compatibilityList[i] == standard)
    {
      outval = standard;
      break;
    }
  }
  return outval;
}

/* add a track type to the compatibility list */
static ISOErr addStandard(struct MP4TrackTypeAtom *self, u32 standard)
{
  ISOErr err;
  err = ISONoErr;

  if(!getStandard(self, standard))
  {
    self->itemCount++;
    self->compatibilityList =
      (u32 *)realloc(self->compatibilityList, self->itemCount * sizeof(u32));
    TESTMALLOC(self->compatibilityList);
    self->compatibilityList[self->itemCount - 1] = (u32)standard;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static ISOErr setBrand(struct MP4TrackTypeAtom *self, u32 standard, u32 minorversion)
{
  u32 oldstandard;
  MP4Err err;
  oldstandard = self->majorBrand;

  self->majorBrand   = standard;
  self->minorVersion = minorversion;

  /* in the compatibility list are also the new major brand, and the old one, if any */
  if(oldstandard)
  {
    err = addStandard(self, oldstandard);
    if(err) return err;
  }
  err = addStandard(self, standard);
  if(err) return err;

  return err;
}

static ISOErr createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  ISOErr err;
  u32 items = 0;
  long bytesToRead;
  char typeString[8];
  char msgString[80];

  MP4TrackTypeAtomPtr self = (MP4TrackTypeAtomPtr)s;

  err = ISONoErr;
  if(self == NULL) BAILWITHERROR(ISOBadParamErr);

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET32(majorBrand);
  MP4TypeToString(self->majorBrand, typeString);
  sprintf(msgString, " major brand is '%s'", typeString);
  inputStream->msg(inputStream, msgString);

  GET32(minorVersion);

  bytesToRead = self->size - self->bytesRead;
  if(bytesToRead <
     ((long)sizeof(u32))) /* there must be at least one item in the compatibility list */
    BAILWITHERROR(ISOBadDataErr);

  if(self->compatibilityList) free(self->compatibilityList);

  self->compatibilityList = (u32 *)calloc(1, bytesToRead);
  TESTMALLOC(self->compatibilityList);

  while(bytesToRead > 0)
  {
    if(bytesToRead < ((long)sizeof(u32))) /* we need to read a full u32 */
      BAILWITHERROR(ISOBadDataErr);

    GET32(compatibilityList[items]);
    MP4TypeToString(self->compatibilityList[items], typeString);
    sprintf(msgString, " minor brand is '%s'", typeString);
    inputStream->msg(inputStream, msgString);
    items++;
    bytesToRead = self->size - self->bytesRead;
  }

  self->itemCount = items;
bail:
  TEST_RETURN(err);

  return err;
}

ISOErr MP4CreateTrackTypeAtom(MP4TrackTypeAtomPtr *outAtom)
{
  ISOErr err;
  MP4TrackTypeAtomPtr self;

  self = (MP4TrackTypeAtomPtr)calloc(1, sizeof(MP4TrackTypeAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self); /* Full atom */
  if(err) goto bail;

  self->type                  = MP4TrackTypeAtomType;
  self->name                  = "TrackTypeBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addStandard           = addStandard;
  self->setBrand              = setBrand;
  self->getBrand              = getBrand;
  self->getStandard           = getStandard;

  self->majorBrand   = 0; /* was ISOISOBrand */
  self->minorVersion = (u32)0;

  self->compatibilityList = (u32 *)calloc(1, sizeof(u32));
  TESTMALLOC(self->compatibilityList);

  /* self->compatibilityList[0]	= ISOISOBrand; */
  /* self->compatibilityList[1]	= ISOISOBrand; */
  /* No, MPEG-21 and meta movies are not ISOM branded */
  self->itemCount = (u32)0;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
