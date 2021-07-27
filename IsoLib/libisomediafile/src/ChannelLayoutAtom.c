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
  $Id: ChannelLayoutAtom.c,v 1.1.1.1 2014/09/18 08:10:00 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

#define SPEAKER_POSITION_EXPLICIT 126
#define STREAM_STRUCTURE_CHANNELS 0x1
#define STREAM_STRUCTURE_OBJECTS 0x2

static void destroy(MP4AtomPtr s)
{
  MP4ChannelLayoutAtomPtr self = (MP4ChannelLayoutAtomPtr)s;

  if(self == NULL) return;

  if(self->definedLayouts != NULL)
  {
    while(self->definedLayouts->entryCount > 0)
    {
      MP4ChannelLayoutDefinedLayout *definedLayout;
      MP4GetListEntry(self->definedLayouts, 0, (char **)&definedLayout);
      free(definedLayout);
      MP4DeleteListEntry(self->definedLayouts, 0);
    }
    MP4DeleteLinkedList(self->definedLayouts);
  }
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  u16 i;
  MP4Err err;
  u8 tmp8;
  MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
  MP4ChannelLayoutAtomPtr self = (MP4ChannelLayoutAtomPtr)s;
  err                          = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  if(self->version == 0)
  {
    PUT8(stream_structure);
    if(self->stream_structure & STREAM_STRUCTURE_CHANNELS)
    {
      PUT8(definedLayout);
      if(self->definedLayout == 0)
      {
        for(i = 0; i < self->channelCount; i++)
        {
          err = MP4GetListEntry(self->definedLayouts, i, (char **)&definedLayoutStruct);
          if(err) goto bail;
          PUT8_V(definedLayoutStruct->speaker_position);
          if(definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
          {
            PUT16_V(definedLayoutStruct->azimuth);
            PUT8_V(definedLayoutStruct->elevation);
          }
        }
      }
      else
      {
        PUT64(omittedChannelsMap);
      }
    }
    if(self->stream_structure & STREAM_STRUCTURE_OBJECTS)
    {
      PUT8(object_count);
    }
  }
  else
  { /* version > 0 */
    tmp8 = (self->stream_structure << 4) + self->formatOrdering;
    PUT8_V(tmp8);
    PUT8(baseChannelCount);
    if(self->stream_structure & STREAM_STRUCTURE_CHANNELS)
    {
      PUT8(definedLayout);
      if(self->definedLayout == 0)
      {
        PUT8(layoutChannelCount);
        for(i = 0; i < self->layoutChannelCount; i++)
        {
          err = MP4GetListEntry(self->definedLayouts, i, (char **)&definedLayoutStruct);
          if(err) goto bail;
          PUT8_V(definedLayoutStruct->speaker_position);
          if(definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
          {
            PUT16_V(definedLayoutStruct->azimuth);
            PUT8_V(definedLayoutStruct->elevation);
          }
        }
      }
      else
      {
        tmp8 = (self->channelOrderDefinition << 1) + self->omittedChannelsPresent;
        PUT8_V(tmp8);
        if(self->omittedChannelsPresent)
        {
          PUT64(omittedChannelsMap);
        }
      }
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
  u16 i;
  MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
  MP4ChannelLayoutAtomPtr self = (MP4ChannelLayoutAtomPtr)s;
  err                          = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  if(self->version == 0)
  {
    self->size += 1;
    if(self->stream_structure & STREAM_STRUCTURE_CHANNELS)
    {
      self->size += 1;
      if(self->definedLayout == 0)
      {
        for(i = 0; i < self->channelCount; i++)
        {
          MP4GetListEntry(self->definedLayouts, i, (char **)&definedLayoutStruct);
          self->size += 1;
          if(definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
          {
            self->size += 3;
          }
        }
      }
      else
      {
        self->size += 8;
      }
    }
    if(self->stream_structure & STREAM_STRUCTURE_OBJECTS)
    {
      self->size += 1;
    }
  }
  else
  { /* version > 0 */
    self->size += 2;
    if(self->stream_structure & STREAM_STRUCTURE_CHANNELS)
    {
      self->size += 1;
      if(self->definedLayout == 0)
      {
        self->size += 1;
        for(i = 0; i < self->layoutChannelCount; i++)
        {
          MP4GetListEntry(self->definedLayouts, i, (char **)&definedLayoutStruct);
          self->size += 1;
          if(definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
          {
            self->size += 3;
          }
        }
      }
      else
      {
        self->size += 1;
        if(self->omittedChannelsPresent)
        {
          self->size += 8;
        }
      }
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  u32 tmp8, i;
  MP4Err err;
  MP4ChannelLayoutAtomPtr self = (MP4ChannelLayoutAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  if(self->version == 0)
  {
    GET8(stream_structure);
    if(self->stream_structure & STREAM_STRUCTURE_CHANNELS)
    {
      GET8(definedLayout);
      if(self->definedLayout == 0)
      {
        while(self->bytesRead < self->size)
        {
          MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
          definedLayoutStruct = calloc(1, sizeof(MP4ChannelLayoutDefinedLayout));
          GET8_V(definedLayoutStruct->speaker_position);
          if(definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
          {
            GET16_V(definedLayoutStruct->azimuth);
            GET8_V(definedLayoutStruct->elevation);
          }
          else
          {
            definedLayoutStruct->azimuth   = 0;
            definedLayoutStruct->elevation = 0;
          }
          err = MP4AddListEntry(definedLayoutStruct, self->definedLayouts);
          if(err) goto bail;
        }
      }
      else
      {
        GET64(omittedChannelsMap);
      }
    }
    if(self->stream_structure & STREAM_STRUCTURE_OBJECTS)
    {
      GET8(object_count);
    }
  }
  else
  { /* version > 0 */
    GET8_V(tmp8);
    self->stream_structure = (u8)(tmp8 >> 4);
    self->formatOrdering   = tmp8 & 0xF;
    GET8(baseChannelCount);
    if(self->stream_structure & STREAM_STRUCTURE_CHANNELS)
    {
      GET8(definedLayout);
      if(self->definedLayout == 0)
      {
        GET8(layoutChannelCount);
        for(i = 0; i < self->layoutChannelCount; i++)
        {
          MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
          definedLayoutStruct = calloc(1, sizeof(MP4ChannelLayoutDefinedLayout));
          GET8_V(definedLayoutStruct->speaker_position);
          if(definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
          {
            GET16_V(definedLayoutStruct->azimuth);
            GET8_V(definedLayoutStruct->elevation);
          }
          else
          {
            definedLayoutStruct->azimuth   = 0;
            definedLayoutStruct->elevation = 0;
          }
          err = MP4AddListEntry(definedLayoutStruct, self->definedLayouts);
          if(err) goto bail;
        }
      }
      else
      {
        GET8_V(tmp8);
        self->channelOrderDefinition = (tmp8 >> 1) & 0x7;
        self->omittedChannelsPresent = tmp8 & 0x1;
        if(self->omittedChannelsPresent)
        {
          GET64(omittedChannelsMap);
        }
      }
    }
  }
  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateChannelLayoutAtom(MP4ChannelLayoutAtomPtr *outAtom)
{
  MP4Err err;
  MP4ChannelLayoutAtomPtr self;

  self = (MP4ChannelLayoutAtomPtr)calloc(1, sizeof(MP4ChannelLayoutAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4ChannelLayoutAtomType;
  self->name                  = "channel layout";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  err = MP4MakeLinkedList(&self->definedLayouts);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
