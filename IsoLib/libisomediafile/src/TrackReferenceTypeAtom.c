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
	$Id: TrackReferenceTypeAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy( MP4AtomPtr s )
{
	MP4TrackReferenceTypeAtomPtr self;
	self = (MP4TrackReferenceTypeAtomPtr) s;
	if ( self->trackIDs )
	{
		free( self->trackIDs );
		self->trackIDs = NULL;
	}
	if ( self->super )
		self->super->destroy( s );
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;
	MP4TrackReferenceTypeAtomPtr self = (MP4TrackReferenceTypeAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
	buffer += self->bytesWritten;
	for ( i = 0; i < self->trackIDCount; i++ )
	{
		PUT32( trackIDs[i] );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4TrackReferenceTypeAtomPtr self = (MP4TrackReferenceTypeAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += (self->trackIDCount * sizeof(u32));
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	long bytesToRead;
	u32 i;
	MP4TrackReferenceTypeAtomPtr self = (MP4TrackReferenceTypeAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream );

	bytesToRead = s->size - s->bytesRead;
	self->trackIDCount = bytesToRead / sizeof(u32);
	self->trackIDs = (u32*) calloc( 1, bytesToRead );
	TESTMALLOC( self->trackIDs )
	
	for ( i = 0; i < self->trackIDCount; i++ )
	{
		GET32( trackIDs[i] );
	}

bail:
	TEST_RETURN( err );

	if ( err && self->trackIDs )
	{
		free( self->trackIDs );
	}
	return err;
}

static MP4Err addTrackID( struct MP4TrackReferenceTypeAtom *self, u32 trackID )
{
	MP4Err err;
	u32 i;
	err = MP4NoErr;
	if ( trackID == 0 )
		BAILWITHERROR( MP4BadParamErr );

	/* JLF 12 / 00: don't add a dep if already here !! */
	for (i = 0; i < self->trackIDCount; i++) {
		if (self->trackIDs[i] == trackID) return err;
	}

	if (self->trackIDs != NULL) 
		self->trackIDs = (u32*) realloc( self->trackIDs, (self->trackIDCount+1)*sizeof(u32) );
		else
		self->trackIDs = (u32*) calloc( self->trackIDCount+1, sizeof(u32) );
	TESTMALLOC( self->trackIDs );
	self->trackIDs[ self->trackIDCount++ ] = trackID;
bail:
	TEST_RETURN( err );
	return err;
}

MP4Err MP4CreateTrackReferenceTypeAtom( u32 atomType, MP4TrackReferenceTypeAtomPtr *outAtom )
{
	MP4Err err;
	MP4TrackReferenceTypeAtomPtr self;
	
	self = (MP4TrackReferenceTypeAtomPtr) calloc( 1, sizeof(MP4TrackReferenceTypeAtom) );
	TESTMALLOC( self )

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = atomType;
	self->name                = "track reference type";
	self->destroy             = destroy;
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->addTrackID = addTrackID;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
