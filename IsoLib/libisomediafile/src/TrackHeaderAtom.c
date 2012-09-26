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
	$Id: TrackHeaderAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
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
	MP4TrackHeaderAtomPtr self = (MP4TrackHeaderAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	if ( self->version == 1 )
	{
		PUT64( creationTime );
		PUT64( modificationTime );
		PUT32( trackID );
		PUT32( qt_reserved1 );
		PUT64( duration );
	}
	else
	{
		PUT32( creationTime );
		PUT32( modificationTime );
		PUT32( trackID );
		PUT32( qt_reserved1 );
		PUT32( duration );
	}
	PUTBYTES(self->qt_reserved2, 8 );
	PUT16( qt_layer );
	PUT16( qt_alternateGroup );
	PUT16( qt_volume );
	PUT16( qt_reserved3 );
	PUT32( qt_matrixA );
	PUT32( qt_matrixB );
	PUT32( qt_matrixU );
	PUT32( qt_matrixC );
	PUT32( qt_matrixD );
	PUT32( qt_matrixV );
	PUT32( qt_matrixX );
	PUT32( qt_matrixY );
	PUT32( qt_matrixW );
	PUT32( qt_trackWidth );
	PUT32( qt_trackHeight );

	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4TrackHeaderAtomPtr self = (MP4TrackHeaderAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += (8 + (4*2)+(11*4));
	self->size += self->version == 1 ? (3*8)+(2*4) : (5*4);
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4TrackHeaderAtomPtr self = (MP4TrackHeaderAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	if ( self->version == 1 )
	{
		GET64_MSG( creationTime, NULL );
		/* DEBUG_SPRINTF( "creationTime = %llu", self->creationTime ); */
		/* commented out to avoid compiler warnings on the %ll (not an ansi modifier) dws */
		
		GET64_MSG( modificationTime, NULL );
		/* DEBUG_SPRINTF( "modificationTime = %llu", self->modificationTime ); */
		
		GET32( trackID );
		GET32( qt_reserved1 );
		GET64_MSG( duration, NULL );
		/* DEBUG_SPRINTF( "duration = %llu", self->duration ); */
	}
	else
	{
		u32 val;
		GET32_V_MSG( val, "creationTime" );
		self->creationTime = val;
		GET32_V_MSG( val, "modificationTime" );
		self->modificationTime = val;
		
		GET32( trackID );
		GET32( qt_reserved1 );
		GET32_V_MSG( val, "duration" );
		self->duration = val;
	}
	GETBYTES( 8, qt_reserved2 );
	GET16( qt_layer );
	GET16( qt_alternateGroup );
	GET16( qt_volume );
	GET16( qt_reserved3 );
	GET32( qt_matrixA );
	GET32( qt_matrixB );
	GET32( qt_matrixU );
	GET32( qt_matrixC );
	GET32( qt_matrixD );
	GET32( qt_matrixV );
	GET32( qt_matrixX );
	GET32( qt_matrixY );
	GET32( qt_matrixW );
	GET32( qt_trackWidth );
	GET32( qt_trackHeight );
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateTrackHeaderAtom( MP4TrackHeaderAtomPtr *outAtom )
{
	MP4Err err;
	MP4TrackHeaderAtomPtr self;
	
	self = (MP4TrackHeaderAtomPtr) calloc( 1, sizeof(MP4TrackHeaderAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4TrackHeaderAtomType;
	self->name					= "track header";
	self->flags					= 7;				/* track is enabled,in movie, in preview, by default */
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy				= destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->qt_matrixA = (1<<16);
	self->qt_matrixD = (1<<16);
	self->qt_matrixW = (1<<30);
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
