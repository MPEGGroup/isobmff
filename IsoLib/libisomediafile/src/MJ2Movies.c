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
  $Id: MJ2Movies.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "ISOMovies.h"
#include "MP4Atoms.h"
#include "MJ2Atoms.h"
#include "MP4InputStream.h"
#include "MP4Impl.h"
#include "MovieTracks.h"
#include "FileMappingObject.h"
#include "MP4Descriptors.h"
#include "MdatDataHandler.h"

#ifdef macintosh
#include <OSUtils.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

ISO_EXTERN ( ISOErr )
MJ2NewMovie( ISOMovie *outMovie )
{
	ISOErr MP4CreateMediaDataAtom( MP4MediaDataAtomPtr *outAtom );
	ISOErr MP4CreateMovieAtom( MP4MovieAtomPtr *outAtom );
	ISOErr MP4CreateMovieHeaderAtom( MP4MovieHeaderAtomPtr *outAtom );
	
	   
	u64 now;
	ISOErr err;
	MP4PrivateMovieRecordPtr	movie;
	MP4MovieAtomPtr				moov;
	MP4MovieHeaderAtomPtr		mvhd;
	MP4MediaDataAtomPtr			mdat;
	ISOFileTypeAtomPtr			ftyp;
#ifdef JP2KSTILLIMAGE
	MJ2HeaderAtomPtr			jp2h;
#endif
	MJ2JPEG2000SignatureAtomPtr	sgnt;

	movie = (MP4PrivateMovieRecordPtr) calloc( 1, sizeof(MP4PrivateMovieRecord) );
	if ( movie == NULL )
	{
		err = MP4NoMemoryErr;
		goto bail;
	}
	movie->referenceCount     = 1;
	movie->prepend_handle = NULL;
	err = MP4GetCurrentTime( &now ); if (err) goto bail;
	err = MP4CreateMovieAtom( (MP4MovieAtomPtr *) &movie->moovAtomPtr ); if (err) goto bail;
	moov = (MP4MovieAtomPtr) movie->moovAtomPtr;
	err = MP4CreateMovieHeaderAtom( &mvhd ); if (err) goto bail;
	mvhd->nextTrackID = 1;
	mvhd->creationTime = now;
	mvhd->modificationTime = now;
	err = moov->addAtom( moov, (MP4AtomPtr) mvhd ); if (err) goto bail;
	moov->setTimeScale( moov, 600 );
	err = MP4CreateMediaDataAtom( &mdat );
	movie->mdat = (MP4AtomPtr) mdat;
	{
	   MP4DataHandlerPtr dh;
	   
	   err = MP4CreateMdatDataHandler( mdat, &dh ); if (err) goto bail;
	   movie->inMemoryDataHandler = dh;
	}
	movie->fileType = ISOMotionJPEG2000FileType;
	
	err = MJ2CreateFileTypeAtom( &ftyp ); if (err) goto bail;
	err = ftyp->setBrand( ftyp, MJ2JPEG2000Brand, 0 );  if (err) goto bail;
	err = ftyp->addStandard( ftyp, ISOISOBrand );  if (err) goto bail;
	err = ftyp->calculateSize( (MP4AtomPtr) ftyp ); if (err) goto bail;
	movie->ftyp = (MP4AtomPtr) ftyp;
	
	err = MJ2CreateSignatureAtom( &sgnt ); if (err) goto bail;
	err = sgnt->calculateSize( (MP4AtomPtr) sgnt ); if (err) goto bail;
	movie->sgnt = (MP4AtomPtr) sgnt;
	
	/* create JPEG-2000 specific atoms */
	/* a header atom only occurs if a still image occurs */
#ifdef JP2KSTILLIMAGE
	err = MJ2CreateHeaderAtom( &jp2h ); if (err) goto bail;
	err = jp2h->calculateSize( (MP4AtomPtr) jp2h ); if (err) goto bail;
	movie->jp2h = (MP4AtomPtr) jp2h;
	
	{
		/* add in the required subatoms */
		MJ2ImageHeaderAtomPtr	imag;
		MJ2BitsPerComponentAtomPtr	bpco;
		MJ2ColorSpecificationAtomPtr	colr;
	
		err = MJ2CreateImageHeaderAtom( &imag ); if (err) goto bail;
		err = MJ2CreateBitsPerComponentAtom( &bpco ); if (err) goto bail;
		err = MJ2CreateColorSpecificationAtom( &colr ); if (err) goto bail;
	
		err = imag->calculateSize( (MP4AtomPtr) imag ); if (err) goto bail;
		err = bpco->calculateSize( (MP4AtomPtr) bpco ); if (err) goto bail;
		err = colr->calculateSize( (MP4AtomPtr) colr ); if (err) goto bail;
		
		jp2h->addAtom(jp2h, (ISOAtomPtr) imag);
		jp2h->addAtom(jp2h, (ISOAtomPtr) bpco);
		jp2h->addAtom(jp2h, (ISOAtomPtr) colr);
	}
#endif


	*outMovie = (ISOMovie) movie;
bail:
	TEST_RETURN( err );

	return err;	
}
