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
        $Id: MP4TrackReader.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4TrackReader.h"
#include "MP4Impl.h"
#include <stdlib.h>

MP4Err MP4CreateOrdinaryTrackReader(MP4Movie theMovie, MP4Track theTrack,
                                    MP4TrackReaderPtr *outReader);
MP4Err MP4CreateMebxTrackReader(MP4Movie theMovie, MP4Track theTrack, MP4TrackReaderPtr *outReader);
MP4Err MP4CreateODTrackReader(MP4Movie theMovie, MP4Track theTrack, MP4TrackReaderPtr *outReader);
/* Guido : inserted to clean-up resources */
MP4Err MP4DisposeOrdinaryTrackReader(MP4TrackReaderPtr theReader);

MP4_EXTERN(MP4Err)
MP4CreateTrackReader(MP4Track theTrack, MP4TrackReader *outReader)
{
  MP4Err err;
  MP4Movie theMovie;
  MP4Media theMedia;
  MP4MediaInformationAtomPtr minf;
  MP4SampleTableAtomPtr stbl;
  MP4SampleDescriptionAtomPtr stsd;
  GenericSampleEntryAtomPtr entry;

  u32 handlerType;
  MP4TrackReaderPtr reader;
  err = MP4NoErr;
  if((theTrack == 0) || (outReader == 0)) BAILWITHERROR(MP4BadParamErr)
  err = MP4GetTrackMovie(theTrack, &theMovie);
  if(err) goto bail;
  err = MP4GetTrackMedia(theTrack, &theMedia);
  if(err) goto bail;
  err = MP4GetMediaHandlerDescription(theMedia, &handlerType, NULL);
  if(err) goto bail;
  switch(handlerType)
  {
  case MP4ObjectDescriptorHandlerType:
    err = MP4CreateODTrackReader(theMovie, theTrack, &reader);
    if(err) goto bail;
    break;

  case MP4MetaHandlerType:
    minf = (MP4MediaInformationAtomPtr)((MP4MediaAtomPtr)theMedia)->information;
    if(minf == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
    stbl = (MP4SampleTableAtomPtr)minf->sampleTable;
    if(stbl == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
    stsd = (MP4SampleDescriptionAtomPtr)stbl->SampleDescription;
    if(stsd == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
    if(stsd->getEntryCount(stsd) == 0)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
    err = stsd->getEntry(stsd, 1, &entry);
    if(err) goto bail;
    if(entry == NULL)
    {
      BAILWITHERROR(MP4InvalidMediaErr);
    }
    if(entry->type == MP4BoxedMetadataSampleEntryType)
    {
      err = MP4CreateMebxTrackReader(theMovie, theTrack, &reader);
      if(err) goto bail;
    }
    else
    {
      err = MP4CreateOrdinaryTrackReader(theMovie, theTrack, &reader);
      if(err) goto bail;
    }
    break;

  default:
    err = MP4CreateOrdinaryTrackReader(theMovie, theTrack, &reader);
    if(err) goto bail;
    break;
  }
  *outReader = (MP4TrackReader)reader;
bail:
  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetCurrentDecoderConfig(MP4TrackReader theReader, MP4Handle decoderConfigH)
{
  MP4Err err = MP4NoErr;
  MP4TrackReaderPtr reader;

  if((theReader == 0) || (decoderConfigH == 0)) BAILWITHERROR(MP4BadParamErr)
  reader = (MP4TrackReaderPtr)theReader;
  err    = MP4GetMediaDecoderConfig(reader->media, reader->sampleDescIndex, decoderConfigH);
  if(err) goto bail;
bail:
  assert((err == 0) || (err == MP4EOF));
  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetCurrentSampleDescription(MP4TrackReader theReader, MP4Handle sampleEntryH)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if((theReader == 0) || (sampleEntryH == 0)) BAILWITHERROR(MP4BadParamErr)
  reader = (MP4TrackReaderPtr)theReader;
  err    = MP4GetMediaSampleDescription(reader->media, reader->sampleDescIndex, sampleEntryH, NULL);
  if(err) goto bail;
bail:
  assert((err == 0) || (err == MP4EOF));
  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetCurrentSampleDescriptionIndex(MP4TrackReader theReader, u32 *index)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if(theReader == 0) BAILWITHERROR(MP4BadParamErr)
  reader = (MP4TrackReaderPtr)theReader;
  *index = reader->sampleDescIndex;
bail:
  assert((err == 0) || (err == MP4EOF));
  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetNextAccessUnit(MP4TrackReader theReader, MP4Handle outAccessUnit, u32 *outSize,
                                u32 *outSampleFlags, s32 *outCTS, s32 *outDTS)
{
  return MP4TrackReaderGetNextAccessUnitWithDuration(theReader, outAccessUnit, outSize,
                                                     outSampleFlags, outCTS, outDTS, 0);
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetNextAccessUnitWithDuration(MP4TrackReader theReader, MP4Handle outAccessUnit,
                                            u32 *outSize, u32 *outSampleFlags, s32 *outCTS,
                                            s32 *outDTS, u32 *outDuration)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if((theReader == 0) || (outAccessUnit == 0)) BAILWITHERROR(MP4BadParamErr)
  reader   = (MP4TrackReaderPtr)theReader;
  *outSize = 0;
  err = reader->getNextAccessUnit(reader, outAccessUnit, outSize, outSampleFlags, outCTS, outDTS,
                                  outDuration, 0);
  if(err) goto bail;
bail:
  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetNextAccessUnitWithPad(MP4TrackReader theReader, MP4Handle outAccessUnit,
                                       u32 *outSize, u32 *outSampleFlags, s32 *outCTS, s32 *outDTS,
                                       u8 *outPad)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if((theReader == 0) || (outAccessUnit == 0)) BAILWITHERROR(MP4BadParamErr)
  reader   = (MP4TrackReaderPtr)theReader;
  *outSize = 0;
  err = reader->getNextAccessUnit(reader, outAccessUnit, outSize, outSampleFlags, outCTS, outDTS, 0,
                                  outPad);
  if(err) goto bail;
bail:
  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderSetSLConfig(MP4TrackReader theReader, MP4SLConfig slConfig)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if((theReader == 0) || (slConfig == 0)) BAILWITHERROR(MP4BadParamErr)
  reader = (MP4TrackReaderPtr)theReader;
  err    = reader->setSLConfig(reader, slConfig);
  if(err) goto bail;
bail:
  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetNextPacket(MP4TrackReader theReader, MP4Handle outSample, u32 *outSize)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if((theReader == 0) || (outSample == 0)) BAILWITHERROR(MP4BadParamErr)
  reader   = (MP4TrackReaderPtr)theReader;
  *outSize = 0;
  err      = reader->getNextPacket(reader, outSample, outSize);
  if(err) goto bail;
bail:
  return err;
}

MP4_EXTERN(MP4Err)
MP4TrackReaderGetCurrentSampleNumber(MP4TrackReader theReader, u32 *sampleNumber)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if((theReader == 0) || (sampleNumber == 0)) BAILWITHERROR(MP4BadParamErr)
  reader        = (MP4TrackReaderPtr)theReader;
  *sampleNumber = reader->currentSampleNumber;

bail:
  return err;
}

MP4_EXTERN(MP4Err)
MP4SetMebxTrackReaderLocalKeyId(MP4TrackReader theReader, u32 local_key_id)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if(theReader == 0) BAILWITHERROR(MP4BadParamErr)
  reader = (MP4TrackReaderPtr)theReader;

  reader->mebx_local_key_id = local_key_id;

bail:
  TEST_RETURN(err);
  return err;
}

MP4_EXTERN(MP4Err)
MP4SelectFirstMebxTrackReaderKey(MP4TrackReader theReader, u32 key_namespace, MP4Handle key_value,
                                 u32 *outLocalKeyId)
{
  MP4TrackReaderPtr reader;
  MP4Handle sampleEntryH = NULL;
  MP4Err err             = MP4NoErr;
  u32 key_cnt            = 0;
  u32 found_local_id     = 0;
  int found              = 0;

  if((theReader == 0) || (key_value == 0)) BAILWITHERROR(MP4BadParamErr);
  reader = (MP4TrackReaderPtr)theReader;

  err = MP4NewHandle(0, &sampleEntryH);
  if(err) goto bail;
  err = MP4TrackReaderGetCurrentSampleDescription(theReader, sampleEntryH);
  if(err) goto bail;

  err = ISOGetMebxMetadataCount(sampleEntryH, &key_cnt);
  if(err) goto bail;

  for(u32 i = 0; i < key_cnt; i++)
  {
    u32 local_id   = 0;
    u32 ns         = 0;
    MP4Handle valH = NULL;

    err = MP4NewHandle(0, &valH);
    if(err) goto bail;

    err = ISOGetMebxMetadataConfig(sampleEntryH, i, &local_id, &ns, valH, NULL, NULL);
    if(err)
    {
      MP4DisposeHandle(valH);
      goto bail;
    }

    /* Check if this key and value matches */
    if(ns == key_namespace)
    {
      u32 inSize = 0, valSize = 0;
      MP4GetHandleSize(key_value, &inSize);
      MP4GetHandleSize(valH, &valSize);

      if(inSize == valSize && memcmp(*key_value, *valH, inSize) == 0)
      {
        /* Found first match - save it and stop searching */
        found_local_id = local_id;
        found          = 1;
        MP4DisposeHandle(valH);
        break;
      }
    }
    MP4DisposeHandle(valH);
  }

  if(!found)
  {
    err = MP4NotFoundErr;
    goto bail;
  }

  /* Set internal local_key_id and return if the user wanted */
  reader->mebx_local_key_id = found_local_id;
  if(outLocalKeyId) *outLocalKeyId = found_local_id;

  /* Note: This function selects the first match only.
   * Use MP4FindMebxKeyMatchByIndex to iterate through all matches. */

bail:
  if(sampleEntryH) MP4DisposeHandle(sampleEntryH);
  TEST_RETURN(err);
  return err;
}

MP4_EXTERN(MP4Err)
MP4FindMebxKeyMatchByIndex(MP4Handle sampleEntryH, u32 key_namespace, MP4Handle key_value,
                           u32 matchIndex, u32 *outAbsoluteIndex, u32 *outLocalKeyId)
{
  MP4Err err      = MP4NoErr;
  u32 key_cnt     = 0;
  u32 found_count = 0;

  if((sampleEntryH == NULL) || (key_value == NULL)) BAILWITHERROR(MP4BadParamErr);

  err = ISOGetMebxMetadataCount(sampleEntryH, &key_cnt);
  if(err) goto bail;

  for(u32 i = 0; i < key_cnt; i++)
  {
    u32 local_id   = 0;
    u32 ns         = 0;
    MP4Handle valH = NULL;

    err = MP4NewHandle(0, &valH);
    if(err) goto bail;

    err = ISOGetMebxMetadataConfig(sampleEntryH, i, &local_id, &ns, valH, NULL, NULL);
    if(err)
    {
      MP4DisposeHandle(valH);
      goto bail;
    }

    /* Check if this key and value matches */
    if(ns == key_namespace)
    {
      u32 inSize = 0, valSize = 0;
      MP4GetHandleSize(key_value, &inSize);
      MP4GetHandleSize(valH, &valSize);

      if(inSize == valSize && memcmp(*key_value, *valH, inSize) == 0)
      {
        /* This is a match - check if it's the one we're looking for */
        if(found_count == matchIndex)
        {
          /* Found the requested match */
          if(outAbsoluteIndex) *outAbsoluteIndex = i;
          if(outLocalKeyId) *outLocalKeyId = local_id;
          MP4DisposeHandle(valH);
          goto bail;
        }
        found_count++;
      }
    }
    MP4DisposeHandle(valH);
  }

  /* If we get here, matchIndex was beyond available matches */
  err = MP4NotFoundErr;

bail:
  TEST_RETURN(err);
  return err;
}

MP4_EXTERN(MP4Err)
MP4DisposeTrackReader(MP4TrackReader theReader)
{
  MP4Err err;
  MP4TrackReaderPtr reader;

  err = MP4NoErr;
  if(theReader == 0) BAILWITHERROR(MP4BadParamErr)
  reader = (MP4TrackReaderPtr)theReader;
  /* Guido : inserted to clean-up resources */
  err = MP4DisposeOrdinaryTrackReader(reader);
  if(err) goto bail;
  err = reader->destroy(reader);
  if(err) goto bail;
bail:
  TEST_RETURN(err);

  return err;
}
