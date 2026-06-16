/**
 * @file MP4ColourInformationAtom.c
 * @brief ISOBMFF Colour Information Box
 * @version 0.1
 *
 * @copyright This software module was originally developed by Apple Computer, Inc. in the course of
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
 * or derivative works. Copyright (c) 1999.
 *
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4ColorInformationAtomPtr self = (MP4ColorInformationAtomPtr)s;
  if(self == NULL) return;
  if(self->profile)
  {
    free(self->profile);
    self->profile = NULL;
  }
  if(self->super) self->super->destroy(s);
}

static ISOErr serialize(struct MP4Atom *s, char *buffer)
{
  ISOErr err;
  MP4ColorInformationAtomPtr self = (MP4ColorInformationAtomPtr)s;

  err = ISONoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUT32(colour_type);

  if(self->colour_type == MP4ColorParameterTypeNCLX)
  {
    PUT16(colour_primaries);
    PUT16(transfer_characteristics);
    PUT16(matrix_coefficients);
    PUT8(full_range_flag);
  }
  else if(self->colour_type == QTColorParameterTypeNCLC)
  {
    PUT16(colour_primaries);
    PUT16(transfer_characteristics);
    PUT16(matrix_coefficients);
  }
  else if(self->colour_type == MP4ColorParameterTypeRICC ||
          self->colour_type == MP4ColorParameterTypePROF)
  {
    PUTBYTES(self->profile, self->profileSize);
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

static ISOErr calculateSize(struct MP4Atom *s)
{
  ISOErr err;
  MP4ColorInformationAtomPtr self = (MP4ColorInformationAtomPtr)s;
  err                             = ISONoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  self->size += 4; /* colour_type */
  if(self->colour_type == MP4ColorParameterTypeNCLX)
  {
    self->size += 2; /* colour_primaries */
    self->size += 2; /* transfer_characteristics */
    self->size += 2; /* matrix_coefficients */
    self->size += 1; /* full_range_flag */
  }
  else if(self->colour_type == QTColorParameterTypeNCLC)
  {
    self->size += 2; /* colour_primaries */
    self->size += 2; /* transfer_characteristics */
    self->size += 2; /* matrix_coefficients */
  }
  else if(self->colour_type == MP4ColorParameterTypeRICC ||
          self->colour_type == MP4ColorParameterTypePROF)
  {
    self->size += self->profileSize;
  }

bail:
  TEST_RETURN(err);
  return err;
}

static ISOErr createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  ISOErr err;
  MP4ColorInformationAtomPtr self = (MP4ColorInformationAtomPtr)s;
  u32 temp;
  char typeString[8];

  err = ISONoErr;
  if(self == NULL) BAILWITHERROR(ISOBadParamErr)

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET32_V_NOMSG(temp);
  MP4TypeToString(temp, typeString);
  DEBUG_SPRINTF("colour_type = '%s'", typeString);
  self->colour_type = temp;

  if(self->colour_type == MP4ColorParameterTypeNCLX ||
     self->colour_type == QTColorParameterTypeNCLC)
  {
    GET16(colour_primaries);
    GET16(transfer_characteristics);
    GET16(matrix_coefficients);
    if(self->colour_type == MP4ColorParameterTypeNCLX)
    {
      GET8_V_NOMSG(temp);
      self->full_range_flag = (temp & 0x80) >> 7;
      DEBUG_SPRINTF("full_range_flag = %d", self->full_range_flag);
    }
  }
  else if(self->colour_type == MP4ColorParameterTypeRICC ||
          self->colour_type == MP4ColorParameterTypePROF)
  {
    self->profileSize = self->size - self->bytesRead;
    self->profile     = (char *)malloc(self->profileSize);
    TESTMALLOC(self->profile);
    GETBYTES(self->profileSize, profile);
  }

bail:
  TEST_RETURN(err);
  return err;
}

ISOErr MP4CreateColorInformationAtom(MP4ColorInformationAtomPtr *outAtom)
{
  ISOErr err;
  MP4ColorInformationAtomPtr self;

  self = (MP4ColorInformationAtomPtr)calloc(1, sizeof(MP4ColorInformationAtom));
  TESTMALLOC(self);

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;

  self->type                  = MP4ColorInformationAtomType;
  self->name                  = "ColourInformationBox";
  self->destroy               = destroy;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  *outAtom                    = self;
bail:
  TEST_RETURN(err);
  return err;
}
