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

/*
  $Id: AudioSampleEntryAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  MP4AudioSampleEntryAtomPtr self;
  err  = MP4NoErr;
  self = (MP4AudioSampleEntryAtomPtr)s;
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
  MP4AudioSampleEntryAtomPtr self = (MP4AudioSampleEntryAtomPtr)s;
  err                             = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;
  PUTBYTES(self->reserved, 6);
  PUT16(dataReferenceIndex);
  PUTBYTES(self->reserved2, 8);
  PUT16(reserved3);
  PUT16(reserved4);
  PUT32(reserved5);
  PUT16(timeScale);
  PUT16(reserved6);
  SERIALIZE_ATOM_LIST(ExtensionAtomList);
  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4AudioSampleEntryAtomPtr self = (MP4AudioSampleEntryAtomPtr)s;
  err                             = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  self->size += 14 + (1 * 4) + (5 * 2);
  ADD_ATOM_LIST_SIZE(ExtensionAtomList);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  MP4AudioSampleEntryAtomPtr self = (MP4AudioSampleEntryAtomPtr)s;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GETBYTES(6, reserved);
  GET16(dataReferenceIndex);
  GETBYTES(8, reserved2);
  GET16(reserved3);
  GET16(reserved4);
  GET32(reserved5);
  GET16(timeScale);
  GET16(reserved6);

  /* parse boxes and check if we encounter QTFF sample description */
  while(self->bytesRead < self->size)
  {
    MP4AtomPtr atm;
    u64 currentOffset               = 0;
    u64 available                   = inputStream->available;
    u32 indent                      = inputStream->indent;
    MP4FileMappingInputStreamPtr fm = (MP4FileMappingInputStreamPtr)inputStream;
    currentOffset                   = fm->current_offset;
    err                             = MP4ParseAtom(inputStream, &atm);

    if(self->bytesRead + atm->size > self->size && self->reserved2[1] == 1)
    {
      /* most likely we are parsing QTFF SoundDescriptionV1, rewind and parse 16 more bytes */
      atm->destroy(atm);
      inputStream->available = available;
      inputStream->indent    = indent;
      fm->current_offset     = currentOffset;

      GET32(qtSamplesPerPacket);
      GET32(qtbytesPerPacket);
      GET32(qtbytesPerFrame);
      GET32(qtbytesPerSample);
    }
    else if(err == MP4NoErr)
    {
      self->bytesRead += atm->size;
      if(((atm->type) == MP4FreeSpaceAtomType) || ((atm->type) == MP4SkipAtomType))
        atm->destroy(atm);
      else
      {
        err = MP4AddListEntry((void *)atm, self->ExtensionAtomList);
        if(err) goto bail;
      }
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateAudioSampleEntryAtom(MP4AudioSampleEntryAtomPtr *outAtom)
{
  MP4Err err;
  MP4AudioSampleEntryAtomPtr self;

  self = (MP4AudioSampleEntryAtomPtr)calloc(1, sizeof(MP4AudioSampleEntryAtom));
  TESTMALLOC(self);

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type = MP4AudioSampleEntryAtomType;
  self->name = "audio sample entry";
  err        = MP4MakeLinkedList(&self->ExtensionAtomList);
  if(err) goto bail;
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->reserved3             = 2;
  self->reserved4             = 16;
  self->timeScale             = 44100;
  *outAtom                    = self;
bail:
  TEST_RETURN(err);

  return err;
}
