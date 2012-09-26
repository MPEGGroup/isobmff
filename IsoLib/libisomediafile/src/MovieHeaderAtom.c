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
	$Id: MovieHeaderAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
    err = MP4NoErr;
	if ( s == NULL )
       BAILWITHERROR( MP4BadParamErr )
	if ( s->super )
		s->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;

	MP4MovieHeaderAtomPtr self = (MP4MovieHeaderAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
	buffer += self->bytesWritten;

	if ( self->version == 1 )
	{
		PUT64( creationTime );
		PUT64( modificationTime );
		PUT32( timeScale );
		PUT64( duration );
	}
	else
	{
		PUT32( creationTime );
		PUT32( modificationTime );
		PUT32( timeScale );
		PUT32( duration );
	}
	PUT32( qt_preferredRate );
	PUT16( qt_preferredVolume );
	PUTBYTES( self->qt_reserved, 10 );	
	PUT32( qt_matrixA );
	PUT32( qt_matrixB );
	PUT32( qt_matrixU );
	PUT32( qt_matrixC );
	PUT32( qt_matrixD );
	PUT32( qt_matrixV );
	PUT32( qt_matrixX );
	PUT32( qt_matrixY );
	PUT32( qt_matrixW );
	PUT32( qt_previewTime );
	PUT32( qt_previewDuration );
	PUT32( qt_posterTime );
	PUT32( qt_selectionTime );
	PUT32( qt_selectionDuration );
	PUT32( qt_currentTime );
	PUT32( nextTrackID );


	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4MovieHeaderAtomPtr self = (MP4MovieHeaderAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += (17*4)+(1*2)+10;
	if ( self->version == 1 )
		self->size += (3*8)+(1*4);
	else
		self->size += (4*4);
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4MovieHeaderAtomPtr self = (MP4MovieHeaderAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	if ( self->version == 1 )
	{
		GET64( creationTime );
		GET64( modificationTime );
		GET32( timeScale );
		GET64( duration );
	}
	else
	{
		u32 val;
		GET32_V_MSG( val, "creationTime" );
		self->creationTime = val;
		GET32_V_MSG( val, "modificationTime" );
		self->modificationTime = val;
		
		GET32( timeScale );
		GET32_V_MSG( val, "duration" );
		self->duration = val;
	}
	GET32( qt_preferredRate );
	GET16( qt_preferredVolume );
	GETBYTES( 10, qt_reserved );	
	GET32( qt_matrixA );
	GET32( qt_matrixB );
	GET32( qt_matrixU );
	GET32( qt_matrixC );
	GET32( qt_matrixD );
	GET32( qt_matrixV );
	GET32( qt_matrixX );
	GET32( qt_matrixY );
	GET32( qt_matrixW );
	GET32( qt_previewTime );
	GET32( qt_previewDuration );
	GET32( qt_posterTime );
	GET32( qt_selectionTime );
	GET32( qt_selectionDuration );
	GET32( qt_currentTime );
	GET32( nextTrackID );
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateMovieHeaderAtom( MP4MovieHeaderAtomPtr *outAtom )
{
	MP4Err err;
	MP4MovieHeaderAtomPtr self;
	
	self = (MP4MovieHeaderAtomPtr) calloc( 1, sizeof(MP4MovieHeaderAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4MovieHeaderAtomType;
	self->name                = "movie header";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;	
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->qt_preferredRate = (1<<16);
	self->qt_preferredVolume = (1<<8);
	self->qt_matrixA = (1<<16);
	self->qt_matrixD = (1<<16);
	self->qt_matrixW = (1<<30);
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
