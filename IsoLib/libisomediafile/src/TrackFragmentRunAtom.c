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
  $Id: TrackFragmentRunAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy(MP4AtomPtr s)
{
  MP4TrackRunAtomPtr self = (MP4TrackRunAtomPtr)s;
  if(self == NULL) return;
  if(self->entries)
  {
    free(self->entries);
    self->entries = NULL;
  }
  if(self->super) self->super->destroy(s);
}

static void calculateDefaults(struct MP4TrackRunAtom *self, MP4TrackFragmentHeaderAtomPtr tfhd,
                              u32 flags_index)
{
  MP4TrackRunEntryPtr entry;

  if(self->samplecount == 0) return;

  entry = self->entries;

  if(tfhd->default_sample_duration == 0) tfhd->default_sample_duration = entry->sample_duration;
  if(tfhd->default_sample_size == 0) tfhd->default_sample_size = entry->sample_size;
  if((tfhd->default_sample_flags == 0) && (self->samplecount >= flags_index))
    tfhd->default_sample_flags = entry[flags_index - 1].sample_flags;
}

static void setFlags(struct MP4TrackRunAtom *self, MP4TrackFragmentHeaderAtomPtr tfhd)
{
  u32 flags;
  u32 i;
  MP4TrackRunEntryPtr entry;

  flags = 0;

  if(self->data_offset != 0) flags |= trun_data_offset_present;

  entry                    = self->entries;
  self->first_sample_flags = entry->sample_flags;

  if(entry->sample_flags != tfhd->default_sample_flags) flags |= trun_first_sample_flags_present;

  for(i = 0, entry = self->entries; i < self->samplecount; i++, entry++)
  {
    if(entry->sample_duration != tfhd->default_sample_duration)
      flags |= trun_sample_duration_present;
    if(entry->sample_size != tfhd->default_sample_size) flags |= trun_sample_size_present;
    if((i > 0) && (entry->sample_flags != tfhd->default_sample_flags))
      flags = (flags & ~trun_first_sample_flags_present) | trun_sample_flags_present;
    if(entry->sample_composition_time_offset != 0) flags |= trun_sample_composition_times_present;

    if((flags & trun_all_sample_flags) == trun_all_sample_flags) break;
  }
  self->flags = flags;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 flags;
  u32 i;
  MP4TrackRunEntryPtr entry;
  MP4TrackRunAtomPtr self = (MP4TrackRunAtomPtr)s;
  err                     = MP4NoErr;

  flags = self->flags;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUT32(samplecount);

  if(flags & trun_data_offset_present)
  {
    PUT32(data_offset);
  };
  if(flags & trun_first_sample_flags_present)
  {
    PUT32(first_sample_flags);
  };
  for(i = 0, entry = self->entries; i < self->samplecount; i++, entry++)
  {
    if(flags & trun_sample_duration_present)
    {
      PUT32_V(entry->sample_duration);
    };
    if(flags & trun_sample_size_present)
    {
      PUT32_V(entry->sample_size);
    };
    if(flags & trun_sample_flags_present)
    {
      PUT32_V(entry->sample_flags);
    };
    if(flags & trun_sample_composition_times_present)
    {
      PUT32_V(entry->sample_composition_time_offset);
    };
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  u32 flags;
  u32 entry_size;
  MP4TrackRunAtomPtr self = (MP4TrackRunAtomPtr)s;
  err                     = MP4NoErr;

  flags      = self->flags;
  entry_size = 0;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  self->size += 4;

  if(flags & trun_data_offset_present) self->size += 4;
  if(flags & trun_first_sample_flags_present) self->size += 4;
  if(flags & trun_sample_duration_present) entry_size += 4;
  if(flags & trun_sample_size_present) entry_size += 4;
  if(flags & trun_sample_flags_present) entry_size += 4;
  if(flags & trun_sample_composition_times_present) entry_size += 4;
  self->size += entry_size * self->samplecount;

bail:
  TEST_RETURN(err);

  return err;
}

static int bitcount(int v)
{
  unsigned int c; /* c accumulates the total bits set in v */
  for(c = 0; v; c++)
  {
    v &= v - 1; /* clear the least significant bit set */
  }
  return c;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 flags;
  u32 i;
  MP4TrackRunEntryPtr entry;
  MP4TrackRunAtomPtr self = (MP4TrackRunAtomPtr)s;
  u32 field_count, record_count, r, junk;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  flags = self->flags;
  GET32(samplecount);

  self->entries = (MP4TrackRunEntry *)calloc(self->samplecount, sizeof(MP4TrackRunEntry));

  field_count  = bitcount(flags & 0xFF);
  record_count = bitcount(flags & 0xFF00);

  if(flags & trun_data_offset_present)
  {
    GET32(data_offset);
    field_count--;
  };
  if(flags & trun_first_sample_flags_present)
  {
    GET32(first_sample_flags);
    field_count--;
  };
  while(field_count > 0)
  {
    GET32_V(junk);
    field_count--;
  }

  for(i = 0, entry = self->entries; i < self->samplecount; i++, entry++)
  {
    r = record_count;
    if(flags & trun_sample_duration_present)
    {
      GET32_V_MSG((entry->sample_duration), "duration");
      r--;
    };
    if(flags & trun_sample_size_present)
    {
      GET32_V_MSG((entry->sample_size), "size");
      r--;
    };
    if(flags & trun_sample_flags_present)
    {
      GET32_V_MSG((entry->sample_flags), "flags");
      r--;
    };
    if(flags & trun_sample_composition_times_present)
    {
      GET32_V_MSG((entry->sample_composition_time_offset), "comp offset");
      r--;
    };
    while(r > 0)
    {
      GET32_V(junk);
      r--;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateTrackRunAtom(MP4TrackRunAtomPtr *outAtom)
{
  MP4Err err;
  MP4TrackRunAtomPtr self;

  self = (MP4TrackRunAtomPtr)calloc(1, sizeof(MP4TrackRunAtom));
  TESTMALLOC(self)

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4TrackRunAtomType;
  self->name                  = "track fragment run";
  self->flags                 = 0;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  self->setFlags          = setFlags;
  self->calculateDefaults = calculateDefaults;
  *outAtom                = self;
bail:
  TEST_RETURN(err);

  return err;
}
