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
 * or derivative works. Copyright (c) 2014.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  ISOSingleItemTypeReferenceAtomPtr self;
  self = (ISOSingleItemTypeReferenceAtomPtr)s;
  if(self == NULL) return;
  if(self->to_item_IDs) free(self->to_item_IDs);
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  ISOSingleItemTypeReferenceAtomPtr self = (ISOSingleItemTypeReferenceAtomPtr)s;
  u32 i;
  err = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  if(self->isLarge == 0)
  {
    PUT16(from_item_ID);
    PUT16(reference_count);
  }
  else
  {
    PUT32(from_item_ID);
    PUT16(reference_count);
  }

  for(i = 0; i < self->reference_count; i++)
  {
    if(self->isLarge == 0)
    {
      PUT16_V(self->to_item_IDs[i]);
    }
    else
    {
      PUT32_V(self->to_item_IDs[i]);
    }
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOSingleItemTypeReferenceAtomPtr self = (ISOSingleItemTypeReferenceAtomPtr)s;
  err                                    = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;

  if(self->isLarge == 0)
  {
    self->size += 4;
    self->size += 2 * self->reference_count;
  }
  else
  {
    self->size += 6;
    self->size += 4 * self->reference_count;
  }
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i;
  char typeString[8];
  char msgString[80];

  void MP4TypeToString(u32 inType, char *ioStr);

  ISOSingleItemTypeReferenceAtomPtr self = (ISOSingleItemTypeReferenceAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);

  self->bytesRead = 0;

  GET32(size);
  GET32(type);
  MP4TypeToString(self->type, typeString);
  sprintf(msgString, "type is '%s'", typeString);
  inputStream->msg(inputStream, msgString);

  if(self->isLarge == 0)
  {
    GET16(from_item_ID);
    GET16(reference_count);
    self->to_item_IDs = calloc(self->reference_count, sizeof(u16));
  }
  else
  {
    GET32(from_item_ID);
    GET16(reference_count);
    self->to_item_IDs = calloc(self->reference_count, sizeof(u32));
  }

  for(i = 0; i < self->reference_count; i++)
  {
    if(self->isLarge == 0)
    {
      GET16_V(self->to_item_IDs[i]);
    }
    else
    {
      GET32_V(self->to_item_IDs[i]);
    }
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

MP4Err ISOCreateSingleItemTypeReferenceAtom(ISOSingleItemTypeReferenceAtomPtr *outAtom, u32 type,
                                            u8 isLarge)
{
  MP4Err err;
  ISOSingleItemTypeReferenceAtomPtr self;

  self = (ISOSingleItemTypeReferenceAtomPtr)calloc(1, sizeof(ISOSingleItemTypeReferenceAtom));
  TESTMALLOC(self)

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = type;
  self->name                  = "single item type reference";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->isLarge               = isLarge == 1;

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
