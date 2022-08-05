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
 * or derivative works. Copyright (c) 2021.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  u32 i, count;
  MP4Err err               = MP4NoErr;
  EntityToGroupBoxPtr self = (EntityToGroupBoxPtr)s;

  if(self == NULL) return;

  err = MP4GetListEntryCount(self->entity_ids, &count);
  assert(self->num_entities_in_group == count);
  if(err) goto bail;

  for(i = 0; i < count; i++)
  {
    char *p;
    err = MP4GetListEntry(self->entity_ids, i, &p);
    if(err) goto bail;
    if(p) free(p);
  }
  err = MP4DeleteLinkedList(self->entity_ids);
  if(err) goto bail;
  self->entity_ids            = NULL;
  self->num_entities_in_group = 0;
  self->group_id              = 0;

  if(self->remainingData)
  {
    free(self->remainingData);
    self->remainingData     = NULL;
    self->remainingDataSize = 0;
  }

  if(self->super) self->super->destroy(s);

bail:
  TEST_RETURN(err);
  return;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err               = MP4NoErr;
  EntityToGroupBoxPtr self = (EntityToGroupBoxPtr)s;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr);
  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  /* group_id + num_entities_in_group */
  self->size += (4 + 4);

  if(self->entity_ids->entryCount != self->num_entities_in_group) BAILWITHERROR(MP4BadDataErr);
  self->size += (self->num_entities_in_group * 4);

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  u32 i;
  u32 entity_id;
  MP4Err err               = MP4NoErr;
  EntityToGroupBoxPtr self = (EntityToGroupBoxPtr)s;
  if(self == NULL || buffer == NULL) BAILWITHERROR(MP4BadParamErr);

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUT32(group_id);

  if(self->entity_ids->entryCount != self->num_entities_in_group) BAILWITHERROR(MP4BadDataErr);
  PUT32(num_entities_in_group);

  for(i = 0; i < self->num_entities_in_group; i++)
  {
    err = self->getEntityId(self, &entity_id, i);
    if(err) goto bail;
    PUT32_V(entity_id);
  }

  PUTBYTES(self->remainingData, self->remainingDataSize);
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  u32 bytesToRead;
  u32 i, count, entity_id;
  MP4Err err               = MP4NoErr;
  EntityToGroupBoxPtr self = (EntityToGroupBoxPtr)s;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr);
  if(self->entity_ids == NULL) BAILWITHERROR(MP4BadParamErr);

  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET32(group_id);
  GET32_V(count);

  for(i = 0; i < count; i++)
  {
    GET32_V(entity_id);
    err = self->addEntityId(self, entity_id);
    if(err) goto bail;
  }
  assert(self->num_entities_in_group == count);

  bytesToRead = self->size - self->bytesRead;
  if(bytesToRead > 0)
  {
    self->remainingData = (char *)calloc(1, bytesToRead);
    TESTMALLOC(self->remainingData)
    GETBYTES_MSG(bytesToRead, remainingData, "unknown EntityToGroupBox data");
    self->remainingDataSize = bytesToRead;
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  if(err && self->remainingData)
  {
    free(self->remainingData);
  }
  return err;
}

static MP4Err addEntityId(struct EntityToGroupBox *s, u32 entity_id)
{
  u32 *p;
  MP4Err err               = MP4NoErr;
  EntityToGroupBoxPtr self = (EntityToGroupBoxPtr)s;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  p  = (u32 *)malloc(sizeof(u32) * 1);
  *p = entity_id;

  err = MP4AddListEntry(p, self->entity_ids);

  if(err)
  {
    free(p);
    goto bail;
  }

  self->num_entities_in_group += 1;

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err getEntityId(struct EntityToGroupBox *s, u32 *entity_id, u32 index)
{
  u32 *p;
  MP4Err err               = MP4NoErr;
  u32 count                = 0;
  EntityToGroupBoxPtr self = (EntityToGroupBoxPtr)s;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntryCount(self->entity_ids, &count);
  if(err) return err;
  if(index >= count) BAILWITHERROR(MP4BadParamErr);

  err = MP4GetListEntry(self->entity_ids, index, (char **)&p);
  if(err) goto bail;
  *entity_id = *p;

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateEntityToGroupBox(EntityToGroupBoxPtr *pOut, u32 type)
{
  MP4Err err;
  EntityToGroupBoxPtr self;

  self = (EntityToGroupBoxPtr)calloc(1, sizeof(EntityToGroupBox));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;

  /* functions */
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addEntityId           = addEntityId;
  self->getEntityId           = getEntityId;
  self->remainingData         = NULL;
  self->remainingDataSize     = 0;

  /* members */
  self->type    = type;
  self->name    = "EntityToGroupBox";
  self->version = 0;
  self->flags   = 0;

  self->group_id              = 0;
  self->num_entities_in_group = 0;

  err = MP4MakeLinkedList(&self->entity_ids);
  if(err) goto bail;

  *pOut = self;
bail:
  TEST_RETURN(err);
  return err;
}
