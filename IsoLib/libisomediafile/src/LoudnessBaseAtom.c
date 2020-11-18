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
  $Id: LoudnessBaseAtom.c,v 1.1.1.1 2014/09/19 08:10:00 armin Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  u32 i;
  MP4LoudnessBaseAtomPtr self = (MP4LoudnessBaseAtomPtr)s;
  err                         = MP4NoErr;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr);

  for(i = 0; i < self->measurement_count; i++)
  {
    MP4LoudnessBaseMeasurement *measurement;
    MP4GetListEntry(self->measurements, i, (char **)&measurement);
    free(measurement);
  }

  err = MP4DeleteLinkedList(self->measurements);
  if(err) goto bail;

  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);

  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  u8 tmp8;
  MP4Err err;
  u8 i;
  MP4LoudnessBaseAtomPtr self = (MP4LoudnessBaseAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  tmp8 = (self->reserved << 5) + (self->downmix_ID >> 2);
  PUT8_V(tmp8);
  tmp8 = (self->downmix_ID << 6) + self->DRC_set_ID;
  PUT8_V(tmp8);
  tmp8 = (self->bs_sample_peak_level >> 4) & 0x00FF;
  PUT8_V(tmp8);
  tmp8 = (self->bs_sample_peak_level & 0x000F) << 4;
  tmp8 += (self->bs_true_peak_level >> 8) & 0x000F;
  PUT8_V(tmp8);
  tmp8 = self->bs_true_peak_level & 0x00FF;
  PUT8_V(tmp8);
  tmp8 = (self->measurement_system_for_TP << 4) + self->reliability_for_TP;
  PUT8_V(tmp8);
  PUT8(measurement_count);

  for(i = 0; i < self->measurement_count; i++)
  {
    MP4LoudnessBaseMeasurement *measurement;
    MP4GetListEntry(self->measurements, i, (char **)&measurement);
    PUT8_V(measurement->method_definition);
    PUT8_V(measurement->method_value);
    tmp8 = (measurement->measurement_system << 4) + measurement->reliability;
    PUT8_V(tmp8);
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4LoudnessBaseAtomPtr self = (MP4LoudnessBaseAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += 7;
  self->size += 3 * self->measurement_count;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 tmp8;
  u8 tmpVar;
  u8 i;
  MP4LoudnessBaseAtomPtr self = (MP4LoudnessBaseAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET8_V(tmp8);
  self->reserved = (u8)(tmp8 >> 5);
  tmpVar         = (tmp8 & 0x1F) << 2;

  GET8_V(tmp8);
  tmpVar += (u8)(tmp8 >> 6);
  self->downmix_ID = tmpVar;
  self->DRC_set_ID = tmp8 & 0x3F;

  GET8_V(tmp8);
  self->bs_sample_peak_level = (s16)tmp8;

  GET8_V(tmp8);
  self->bs_sample_peak_level = (s16)((self->bs_sample_peak_level << 4) + (tmp8 >> 4));
  self->bs_sample_peak_level = self->bs_sample_peak_level << 4;
  self->bs_sample_peak_level = self->bs_sample_peak_level >> 4; /* get sign right */
  self->bs_true_peak_level   = tmp8 & 0x0F;

  GET8_V(tmp8);
  self->bs_true_peak_level = (s16)((self->bs_true_peak_level << 8) + tmp8);
  self->bs_true_peak_level = self->bs_true_peak_level << 4;
  self->bs_true_peak_level = self->bs_true_peak_level >> 4; /* get sign right */

  GET8_V(tmp8);
  self->measurement_system_for_TP = (u8)(tmp8 >> 4);
  self->reliability_for_TP        = tmp8 & 0x0F;

  GET8(measurement_count);

  err = MP4MakeLinkedList(&self->measurements);
  if(err) goto bail;
  for(i = 0; i < self->measurement_count; i++)
  {
    MP4LoudnessBaseMeasurement *measurement;
    measurement = calloc(1, sizeof(MP4LoudnessBaseMeasurement));
    GET8_V(measurement->method_definition);
    GET8_V(measurement->method_value);
    GET8_V(tmp8);
    measurement->measurement_system = (u8)(tmp8 >> 4);
    measurement->reliability        = tmp8 & 0x0F;
    MP4AddListEntry(measurement, self->measurements);
  }

  assert(self->bytesRead == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateLoudnessBaseAtom(MP4LoudnessBaseAtomPtr *outAtom, u32 type)
{
  MP4Err err;
  MP4LoudnessBaseAtomPtr self;

  err  = MP4NoErr;
  self = (MP4LoudnessBaseAtomPtr)calloc(1, sizeof(MP4LoudnessBaseAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;

  if(type == MP4TrackLoudnessInfoAtomType)
  {
    self->name = "track loudness info";
  }
  else if(type == MP4AlbumLoudnessInfoAtomType)
  {
    self->name = "album loudness info";
  }
  else
  {
    err = MP4BadParamErr;
    goto bail;
  }

  self->type                  = type;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->reserved              = 0;

  err = MP4MakeLinkedList(&self->measurements);
  if(err) goto bail;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
