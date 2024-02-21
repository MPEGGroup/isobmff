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
 * or derivative works. Copyright (c) 1999.
 */

/* Created for Nokia FAVS project by Tampere University of Technology */

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy(MP4AtomPtr s)
{
  u32 i;
  MP4SubSampleInformationAtomPtr self;
  self = (MP4SubSampleInformationAtomPtr)s;
  if(self == NULL) return;
  if(self->entry_count)
  {
    for(i = 0; i < self->entry_count; i++)
    {
      if(self->subsample_count[i])
      {
        free(self->subsample_size[i]);
        self->subsample_size[i] = NULL;
        free(self->subsample_priority[i]);
        self->subsample_priority[i] = NULL;
        free(self->discardable[i]);
        self->discardable[i] = NULL;
        free(self->codec_specific_parameters[i]);
        self->codec_specific_parameters[i] = NULL;
      }
    }
    free(self->subsample_size);
    self->subsample_size = NULL;
    free(self->subsample_priority);
    self->subsample_priority = NULL;
    free(self->discardable);
    self->discardable = NULL;
    free(self->codec_specific_parameters);
    self->codec_specific_parameters = NULL;

    free(self->subsample_count);
    self->subsample_count = NULL;
    free(self->sample_delta);
    self->sample_delta = NULL;
  }
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i, j;
  MP4SubSampleInformationAtomPtr self = (MP4SubSampleInformationAtomPtr)s;
  err                                 = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUT32(entry_count);
  for(i = 0; i < self->entry_count; i++)
  {
    PUT32(sample_delta[i]);
    PUT16(subsample_count[i]);
    for(j = 0; j < self->subsample_count[i]; j++)
    {
      if(self->version == 1)
      {
        PUT32(subsample_size[i][j]);
      }
      else
      {
        PUT16(subsample_size[i][j]);
      }
      PUT8(subsample_priority[i][j]);
      PUT8(discardable[i][j]);
      PUT32(codec_specific_parameters[i][j]);
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
  u32 i;
  MP4SubSampleInformationAtomPtr self = (MP4SubSampleInformationAtomPtr)s;
  err                                 = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  self->size += 4 + (self->entry_count * 6);
  for(i = 0; i < self->entry_count; i++)
  {
    self->size += self->subsample_count[i] * (self->version == 1 ? 10 : 8);
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addEntry(MP4SubSampleInformationAtom *s, u32 sample_delta, u32 subsample_count,
                       MP4Handle subsample_size_array, MP4Handle subsample_priority_array,
                       MP4Handle subsample_discardable_array)
{
  MP4Err err;
  MP4SubSampleInformationAtomPtr self;
  u32 i;
  u32 current_entry;

  err = MP4NoErr;
  if(s == NULL) BAILWITHERROR(MP4BadParamErr);
  self          = (MP4SubSampleInformationAtomPtr)s;
  current_entry = self->entry_count;

  /* re-allocate basic structures */
  self->sample_delta = (u32 *)realloc(self->sample_delta, (self->entry_count + 1) * sizeof(u32));
  TESTMALLOC(self->sample_delta);
  self->subsample_count =
    (u32 *)realloc(self->subsample_count, (self->entry_count + 1) * sizeof(u32));
  TESTMALLOC(self->subsample_count);

  self->subsample_size =
    (u32 **)realloc(self->subsample_size, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->subsample_size);
  self->subsample_size[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->subsample_size[current_entry]);

  self->subsample_priority =
    (u32 **)realloc(self->subsample_priority, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->subsample_priority);
  self->subsample_priority[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->subsample_priority[current_entry]);

  self->discardable = (u32 **)realloc(self->discardable, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->discardable);
  self->discardable[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->discardable[current_entry]);

  self->codec_specific_parameters =
    (u32 **)realloc(self->codec_specific_parameters, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->codec_specific_parameters);
  self->codec_specific_parameters[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->codec_specific_parameters[current_entry]);

  self->sample_delta[current_entry]    = sample_delta;
  self->subsample_count[current_entry] = subsample_count;
  for(i = 0; i < subsample_count; i++)
  {
    if(((u32 *)*subsample_size_array)[i] > 0xFFFF)
    {
      self->version = 1;
    }
    self->subsample_size[current_entry][i]            = ((u32 *)*subsample_size_array)[i];
    self->subsample_priority[current_entry][i]        = ((u32 *)*subsample_priority_array)[i];
    self->discardable[current_entry][i]               = ((u32 *)*subsample_discardable_array)[i];
    self->codec_specific_parameters[current_entry][i] = 0;
  }

  self->entry_count++;

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err addEntry2(MP4SubSampleInformationAtom *s, u32 sample_delta, u32 subsample_count,
                        MP4Handle subsample_size_array, MP4Handle subsample_priority_array,
                        MP4Handle subsample_discardable_array,
                        MP4Handle codec_specific_parameters_array)
{
  MP4Err err;
  MP4SubSampleInformationAtomPtr self;
  u32 i;
  u32 current_entry;

  err = MP4NoErr;
  if(s == NULL) BAILWITHERROR(MP4BadParamErr);
  self          = (MP4SubSampleInformationAtomPtr)s;
  current_entry = self->entry_count;

  /* re-allocate basic structures */
  self->sample_delta = (u32 *)realloc(self->sample_delta, (self->entry_count + 1) * sizeof(u32));
  TESTMALLOC(self->sample_delta);
  self->subsample_count =
    (u32 *)realloc(self->subsample_count, (self->entry_count + 1) * sizeof(u32));
  TESTMALLOC(self->subsample_count);

  self->subsample_size =
    (u32 **)realloc(self->subsample_size, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->subsample_size);
  self->subsample_size[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->subsample_size[current_entry]);

  self->subsample_priority =
    (u32 **)realloc(self->subsample_priority, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->subsample_priority);
  self->subsample_priority[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->subsample_priority[current_entry]);

  self->discardable = (u32 **)realloc(self->discardable, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->discardable);
  self->discardable[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->discardable[current_entry]);

  self->codec_specific_parameters =
    (u32 **)realloc(self->codec_specific_parameters, (self->entry_count + 1) * sizeof(u32 *));
  TESTMALLOC(self->codec_specific_parameters);
  self->codec_specific_parameters[current_entry] = (u32 *)calloc(subsample_count, sizeof(u32));
  TESTMALLOC(self->codec_specific_parameters[current_entry]);

  self->sample_delta[current_entry]    = sample_delta;
  self->subsample_count[current_entry] = subsample_count;
  for(i = 0; i < subsample_count; i++)
  {
    if(((u32 *)*subsample_size_array)[i] > 0xFFFF)
    {
      self->version = 1;
    }
    self->subsample_size[current_entry][i]     = ((u32 *)*subsample_size_array)[i];
    self->subsample_priority[current_entry][i] = ((u32 *)*subsample_priority_array)[i];
    self->discardable[current_entry][i]        = ((u32 *)*subsample_discardable_array)[i];
    self->codec_specific_parameters[current_entry][i] =
      ((u32 *)*codec_specific_parameters_array)[i];
  }

  self->entry_count++;

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i, j;
  MP4SubSampleInformationAtomPtr self = (MP4SubSampleInformationAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET32(entry_count);

  self->sample_delta = (u32 *)calloc(self->entry_count, sizeof(u32));
  TESTMALLOC(self->sample_delta);
  self->subsample_count = (u32 *)calloc(self->entry_count, sizeof(u32));
  TESTMALLOC(self->subsample_count);

  self->subsample_size = (u32 **)calloc(self->entry_count, sizeof(u32 *));
  TESTMALLOC(self->subsample_size);
  self->subsample_priority = (u32 **)calloc(self->entry_count, sizeof(u32 *));
  TESTMALLOC(self->subsample_priority);
  self->discardable = (u32 **)calloc(self->entry_count, sizeof(u32 *));
  TESTMALLOC(self->discardable);
  self->codec_specific_parameters = (u32 **)calloc(self->entry_count, sizeof(u32 *));
  TESTMALLOC(self->codec_specific_parameters);

  for(i = 0; i < self->entry_count; i++)
  {
    GET32(sample_delta[i]);
    GET16(subsample_count[i]);
    self->subsample_size[i] = (u32 *)calloc(self->subsample_count[i], sizeof(u32));
    TESTMALLOC(self->subsample_count[i]);
    self->subsample_priority[i] = (u32 *)calloc(self->subsample_count[i], sizeof(u32));
    TESTMALLOC(self->subsample_count[i]);
    self->discardable[i] = (u32 *)calloc(self->subsample_count[i], sizeof(u32));
    TESTMALLOC(self->subsample_count[i]);
    self->codec_specific_parameters[i] = (u32 *)calloc(self->subsample_count[i], sizeof(u32));
    TESTMALLOC(self->subsample_count[i]);
    for(j = 0; j < self->subsample_count[i]; j++)
    {
      if(self->version == 1)
      {
        GET32(subsample_size[i][j]);
      }
      else
      {
        GET16(subsample_size[i][j]);
      }
      GET8(subsample_priority[i][j]);
      GET8(discardable[i][j]);
      GET32(codec_specific_parameters[i][j]);
    }
  }
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSubSampleInformationAtom(MP4SubSampleInformationAtomPtr *outAtom)
{
  MP4Err err;
  MP4SubSampleInformationAtomPtr self;

  self = (MP4SubSampleInformationAtomPtr)calloc(1, sizeof(MP4SubSampleInformationAtom));
  TESTMALLOC(self)

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = MP4SubSampleInformationAtomType;
  self->name                  = "sub-sample information";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addEntry              = addEntry;
  self->addEntry2             = addEntry2;
  *outAtom                    = self;
  self->entry_count           = 0;

bail:
  TEST_RETURN(err);
  return err;
}
