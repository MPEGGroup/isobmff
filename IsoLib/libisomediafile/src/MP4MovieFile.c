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
  $Id: MP4MovieFile.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "ISOMovies.h"
#include "MP4Atoms.h"
#include "MJ2Atoms.h"
#include <string.h>

MP4Err MP4MovieCreateDescriptors(MP4Movie theMovie);

static MP4Err LocalPutMovieIntoHandle(MP4Movie theMovie, MP4Handle movieH, u32 include_mdat)
{
  MP4Err err;
  MP4PrivateMovieRecordPtr movie;
  MP4MovieAtomPtr moov;
  MP4MediaDataAtomPtr mdat;
  MP4AtomPtr meta;
  MP4AtomPtr meco;
  u32 prepend_size;
  u32 pre_data_size;
  u32 move_up;
  u32 mdat_size;
  u32 moov_size;

  char *write_location;
  MP4AtomPtr headatom;

  err = MP4NoErr;
  if(theMovie == NULL) BAILWITHERROR(MP4BadParamErr);
  /* if movieH is NULL, then we append the movie into the prepend_handle */

  movie = (MP4PrivateMovieRecordPtr)theMovie;
  moov  = (MP4MovieAtomPtr)movie->moovAtomPtr;
  meta  = movie->meta;
  meco  = movie->meco;

  if(movie->fileType == ISOMPEG4FileType) err = MP4MovieCreateDescriptors(theMovie);
  else if(moov)
  {
    err = moov->calculateDuration(moov);
    if(err) goto bail;
  }

  if(movie->prepend_handle) MP4GetHandleSize(movie->prepend_handle, &prepend_size);
  else
    prepend_size = 0;
  pre_data_size = prepend_size;

  if(prepend_size == 0)
  {
    /* add the signature atom to the front of the file */
    headatom = movie->sgnt;
    if(headatom)
    {
      err = headatom->calculateSize(headatom);
      if(err) goto bail;
      pre_data_size += headatom->size;
    }

    /* add the file type atom after the signature atom */
    headatom = movie->ftyp;
    if(headatom)
    {
      err = headatom->calculateSize(headatom);
      if(err) goto bail;
      pre_data_size += headatom->size;
    }

    /* add the header atom after the file type atom */
    headatom = movie->jp2h;
    if(headatom)
    {
      err = headatom->calculateSize(headatom);
      if(err) goto bail;
      pre_data_size += headatom->size;
    }
  }

  if(meta)
  {
    err = meta->calculateSize(meta);
    if(err) goto bail;
    pre_data_size += meta->size;
  }

  if(meco)
  {
    err = meco->calculateSize(meco);
    if(err) goto bail;
    pre_data_size += meco->size;
  }

  if(moov)
  {
    err = moov->calculateSize((MP4AtomPtr)moov);
    if(err) goto bail;
    moov_size = moov->size;
  }
  else
    moov_size = 0;

  move_up = pre_data_size + moov_size + 8;

  mdat = (MP4MediaDataAtomPtr)movie->mdat;
  if(mdat)
  {
    err = mdat->calculateSize((MP4AtomPtr)mdat);
    if(err) goto bail;
    if((include_mdat) && (mdat->size == 1))
    {
      BAILWITHERROR(MP4NotImplementedErr);
    }

    if(mdat->size == 1) move_up += 8;

    mdat_size = (include_mdat ? mdat->size : 0);

    /* adjust mdat-related chunk offsets so they reflect the offset of the mdat within the movie
     * handle */
    if(moov)
    {
      err = moov->mdatMoved(moov, 0, mdat->dataSize, move_up);
      if(err) goto bail;
    }
    if(meta)
    {
      ISOMetaAtomPtr myMeta;
      myMeta = (ISOMetaAtomPtr)meta;
      err    = myMeta->mdatMoved(myMeta, 0, mdat->dataSize, move_up);
      if(err) goto bail;
    }

    if(meco)
    {
      ISOAdditionalMetaDataContainerAtomPtr myMeco;
      myMeco = (ISOAdditionalMetaDataContainerAtomPtr)meco;
      err    = myMeco->mdatMoved(myMeco, 0, mdat->dataSize, move_up);
      if(err) goto bail;
    }
  }
  else
    mdat_size = 0;

  if(movieH == NULL)
  {
    if(movie->prepend_handle == NULL)
      err = MP4NewHandle(pre_data_size + moov_size + mdat_size, &(movie->prepend_handle));
    else
      err = MP4SetHandleSize(movie->prepend_handle, pre_data_size + moov_size + mdat_size);
    if(err) goto bail;
    write_location = (*(movie->prepend_handle)) + prepend_size;
  }
  else
  {
    err = MP4SetHandleSize(movieH, pre_data_size + moov_size + mdat_size);
    if(err) goto bail;

    if(movie->prepend_handle != NULL)
      memcpy((char *)*movieH, (char *)*(movie->prepend_handle), prepend_size);
    write_location = (*movieH) + prepend_size;
  }

  if(prepend_size == 0)
  {
    /* add the signature atom to the front of the file */
    headatom = movie->sgnt;
    if(headatom)
    {
      err = headatom->serialize(headatom, write_location);
      if(err) goto bail;
      write_location += headatom->size;
    }

    /* add the file type atom after the signature atom */
    headatom = movie->ftyp;
    if(headatom)
    {
      err = headatom->serialize(headatom, write_location);
      if(err) goto bail;
      write_location += headatom->size;
    }

    /* add the header atom after the file type atom */
    headatom = movie->jp2h;
    if(headatom)
    {
      err = headatom->serialize(headatom, write_location);
      if(err) goto bail;
      write_location += headatom->size;
    }
  }

  if(meta)
  {
    err = meta->serialize(meta, write_location);
    if(err) goto bail;
    write_location += meta->size;
  }

  if(meco)
  {
    err = meco->serialize(meco, write_location);
    if(err) goto bail;
    write_location += meco->size;
  }

  if(moov)
  {
    err = moov->serialize((MP4AtomPtr)moov, write_location);
    if(err) goto bail;
    write_location += moov->size;
  }

  if(mdat)
  {
    /* back out the adjust mdat-related chunk offsets  */
    if(moov)
    {
      err = moov->mdatMoved(moov, move_up, mdat->dataSize + move_up, -((s32)move_up));
      if(err) goto bail;
    }
    if(meta)
    {
      ISOMetaAtomPtr myMeta;
      myMeta = (ISOMetaAtomPtr)meta;
      err    = myMeta->mdatMoved(myMeta, move_up, mdat->dataSize + move_up, -((s32)move_up));
      if(err) goto bail;
    }

    if(meco)
    {
      ISOAdditionalMetaDataContainerAtomPtr myMeco;
      myMeco = (ISOAdditionalMetaDataContainerAtomPtr)meco;
      err    = myMeco->mdatMoved(myMeco, move_up, mdat->dataSize + move_up, -((s32)move_up));
      if(err) goto bail;
    }

    if(include_mdat)
    {
      err = mdat->serialize((MP4AtomPtr)mdat, write_location);
      if(err) goto bail;
      write_location += mdat->size;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

MP4_EXTERN(MP4Err)
MP4PutMovieIntoHandle(MP4Movie theMovie, MP4Handle movieH)
{
  return LocalPutMovieIntoHandle(theMovie, movieH, 1);
}

MP4_EXTERN(MP4Err)
MP4WriteMovieToFile(MP4Movie theMovie, const char *filename)
{
  MP4Err err;
  size_t written;
  MP4Handle movieH;
  FILE *fd;
  u32 handleSize;
  MP4PrivateMovieRecordPtr movie;
  MP4MediaDataAtomPtr mdat;

  movie = (MP4PrivateMovieRecordPtr)theMovie;
  mdat  = (MP4MediaDataAtomPtr)movie->mdat;

  err    = MP4NoErr;
  movieH = NULL;
  fd     = NULL;
  if((theMovie == NULL) || (filename == NULL)) BAILWITHERROR(MP4BadParamErr);
#ifdef macintosh
  {
    int len;
    int i;
    char *s;
    if(*filename == '/') filename++;
    s   = (char *)filename;
    len = strlen(s);
    for(i = 0; i < len; i++)
    {
      if(s[i] == '/') s[i] = ':';
    }
  }
#endif
  fd = fopen(filename, "wb");
  if(fd == NULL) BAILWITHERROR(MP4IOErr);
  err = MP4NewHandle(0, &movieH);
  if(err) goto bail;
  err = LocalPutMovieIntoHandle(theMovie, movieH, 0);
  if(err) goto bail;
  err = MP4GetHandleSize(movieH, &handleSize);
  if(err) goto bail;
  written = fwrite(*movieH, 1, handleSize, fd);
  if(written != handleSize) BAILWITHERROR(MP4IOErr);
  err = mdat->writeToFile(mdat, fd);

bail:
  if(movieH)
  {
    MP4DisposeHandle(movieH);
    movieH = NULL;
  }
  if(fd)
  {
    fclose(fd);
    fd = NULL;
  }

  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOStartMovieFragment(MP4Movie theMovie)
{
  MP4Err MP4CreateMovieFragmentAtom(MP4MovieFragmentAtomPtr * outAtom);
  MP4Err MP4CreateMovieFragmentHeaderAtom(MP4MovieFragmentHeaderAtomPtr * outAtom);
  MP4Err MP4CreateMediaDataAtom(MP4MediaDataAtomPtr * outAtom);

  MP4Err err;
  MP4PrivateMovieRecordPtr movie;
  MP4MovieFragmentAtomPtr moof;
  MP4MovieFragmentHeaderAtomPtr mfhd;
  MP4MovieExtendsAtomPtr mvex;
  MP4MediaDataAtomPtr mdat;
  MP4MovieAtomPtr moov;
  u32 fragment_sequence;

  err   = MP4NoErr;
  movie = (MP4PrivateMovieRecordPtr)theMovie;

  if((movie->fileType == ISO3GPPFileType) || (movie->fileType == ISOQuickTimeFileType))
    BAILWITHERROR(ISONotImplementedErr);

  if((movie->moovAtomPtr)->type == MP4MovieAtomType)
  {
    moov = (MP4MovieAtomPtr)(movie->moovAtomPtr);

    mvex = (MP4MovieExtendsAtomPtr)moov->mvex;
    if(mvex == NULL) BAILWITHERROR(MP4BadParamErr)
    err = mvex->setSampleDescriptionIndexes(mvex, (MP4AtomPtr)moov);
    if(err) goto bail;
  }

  err = LocalPutMovieIntoHandle(theMovie, NULL, 1);
  if(err) goto bail;

  if((movie->moovAtomPtr)->type == MP4MovieFragmentAtomType)
  {
    moof              = (MP4MovieFragmentAtomPtr)movie->moovAtomPtr;
    mfhd              = (MP4MovieFragmentHeaderAtomPtr)moof->mfhd;
    fragment_sequence = mfhd->sequence_number + 1;
    moof->destroy(movie->moovAtomPtr); /* the old moof */
  }
  else
  {
    movie->true_moov  = movie->moovAtomPtr;
    fragment_sequence = 1;
  }

  moov = (MP4MovieAtomPtr)(movie->true_moov);

  mvex = (MP4MovieExtendsAtomPtr)moov->mvex;

  if(mvex == NULL) BAILWITHERROR(MP4BadParamErr)

  err = MP4CreateMovieFragmentAtom((MP4MovieFragmentAtomPtr *)&moof);
  if(err) goto bail;
  movie->moovAtomPtr = (MP4AtomPtr)moof;

  /*	if (movie->movieFragments == NULL) {
                  err = MP4MakeLinkedList( &movie->movieFragments ); if (err) goto bail;
          }
          err = MP4AddListEntry( moof, movie->movieFragments ); if (err) goto bail;
  */

  err = MP4CreateMovieFragmentHeaderAtom(&mfhd);
  if(err) goto bail;
  mfhd->sequence_number = fragment_sequence;
  moof->mfhd            = (MP4AtomPtr)mfhd;

  mdat = (MP4MediaDataAtomPtr)movie->mdat;
  if(mdat) mdat->destroy((MP4AtomPtr)mdat);
  err = MP4CreateMediaDataAtom(&mdat);
  if(err) goto bail;
  movie->mdat = (MP4AtomPtr)mdat;

  mvex->maketrackfragments(mvex, moof, moov, mdat);

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOAddDelayToTrackFragmentDecodeTime(MP4Movie theMovie, u32 delay)
{
  u32 i;
  MP4Err err;
  MP4PrivateMovieRecordPtr movie;
  MP4MovieFragmentAtomPtr moof;
  MP4TrackFragmentAtomPtr traf;
  MP4TrackExtendsAtomPtr trex;
  MP4TrackFragmentDecodeTimeAtomPtr tfdt;

  err   = MP4NoErr;
  movie = (MP4PrivateMovieRecordPtr)theMovie;

  if(movie->moovAtomPtr->type != MP4MovieFragmentAtomType) BAILWITHERROR(MP4InvalidMediaErr);

  moof = (MP4MovieFragmentAtomPtr)movie->moovAtomPtr;

  for(i = 0; i < moof->atomList->entryCount; i++)
  {
    MP4AtomPtr moofEntry;
    MP4GetListEntry(moof->atomList, i, (char **)&moofEntry);
    if(moofEntry->type == MP4TrackFragmentAtomType)
    {
      traf = (MP4TrackFragmentAtomPtr)moofEntry;
      trex = (MP4TrackExtendsAtomPtr)traf->trex;
      tfdt = (MP4TrackFragmentDecodeTimeAtomPtr)traf->tfdt;
      trex->baseMediaDecodeTime += delay;
      tfdt->baseMediaDecodeTime += delay;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

ISO_EXTERN(MP4Err)
ISOSetCompositonToDecodePropertiesForFragments(MP4Movie theMovie, u32 trackID,
                                               s32 compositionToDTSShift,
                                               s32 leastDecodeToDisplayDelta,
                                               s32 greatestDecodeToDisplayDelta,
                                               s32 compositionStartTime, s32 compositionEndTime)
{
  MP4Err err;
  MP4PrivateMovieRecordPtr movie;
  MP4MovieAtomPtr moov;
  MP4MovieExtendsAtomPtr mvex;

  err   = MP4NoErr;
  movie = (MP4PrivateMovieRecordPtr)theMovie;

  moov = (MP4MovieAtomPtr)(movie->moovAtomPtr);

  mvex = (MP4MovieExtendsAtomPtr)moov->mvex;
  if(mvex == NULL) BAILWITHERROR(MP4BadParamErr)

  err = mvex->setCompositionToDecodeProperties(
    mvex, trackID, compositionToDTSShift, leastDecodeToDisplayDelta, greatestDecodeToDisplayDelta,
    compositionStartTime, compositionEndTime);
bail:
  TEST_RETURN(err);

  return err;
}
