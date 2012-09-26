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
	$Id: TrackReferenceAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	u32 i;
	MP4TrackReferenceAtomPtr self;
	err = MP4NoErr;
	self = (MP4TrackReferenceAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	DESTROY_ATOM_LIST
	
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	MP4TrackReferenceAtomPtr self = (MP4TrackReferenceAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    SERIALIZE_ATOM_LIST( atomList );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4TrackReferenceAtomPtr self = (MP4TrackReferenceAtomPtr ) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	ADD_ATOM_LIST_SIZE( atomList );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err findAtomOfType( struct MP4TrackReferenceAtom *self, u32 atomType, MP4AtomPtr *outAtom )
{
	MP4Err err;
	u32 entryCount;
	u32 i;
	MP4AtomPtr foundAtom;
	
	err = MP4NoErr;
	foundAtom = NULL;
	err = MP4GetListEntryCount( self->atomList, &entryCount ); if (err) goto bail;
	for ( i = 0; i < entryCount; i++ )
	{
		MP4AtomPtr anAtom;
		err = MP4GetListEntry( self->atomList, i, (char**) &anAtom ); if (err) goto bail;
		if ( anAtom->type == atomType )
		{
			foundAtom = anAtom;
			break;
		}
	}
	*outAtom = foundAtom;
bail:
	TEST_RETURN( err );
	return err;
}


static MP4Err addAtom( MP4TrackReferenceAtomPtr self, MP4AtomPtr atom )
{
	MP4Err err;
	if ( self == 0 )
		BAILWITHERROR( MP4BadParamErr );
	err = MP4AddListEntry( atom, self->atomList );
bail:
	TEST_RETURN( err );

	return err;
}



static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	/* PARSE_ATOM_LIST(MP4TrackReferenceAtom) */
	/* we need to have an empty proto list and a default type here, alas */
	u32 NoProtos[] = { 0 };
	MP4Err err;
	MP4TrackReferenceAtomPtr self = (MP4TrackReferenceAtomPtr) s;
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	while ( self->bytesRead < self->size )
	{
		MP4AtomPtr atom;
		err = MP4ParseAtomUsingProtoList( (MP4InputStreamPtr) inputStream, 
						NoProtos, MP4StreamDependenceAtomType, &atom );
			if (err) goto bail;
		self->bytesRead += atom->size;
		if ( ((atom->type)== MP4FreeSpaceAtomType) || ((atom->type)== MP4SkipAtomType))
			atom->destroy( atom );
		else {
			err = addAtom( self, atom );
				if (err) goto bail;
		}
	}
	if ( self->bytesRead != self->size )
		BAILWITHERROR( MP4BadDataErr )

bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateTrackReferenceAtom( MP4TrackReferenceAtomPtr *outAtom )
{
	MP4Err err;
	MP4TrackReferenceAtomPtr self;	
	self = (MP4TrackReferenceAtomPtr) calloc( 1, sizeof(MP4TrackReferenceAtom) );
	TESTMALLOC( self )

	err = MP4CreateBaseAtom( (MP4AtomPtr) self ); if ( err ) goto bail;
	self->type = MP4TrackReferenceAtomType;
	self->name                  = "track reference";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy               = destroy;
	err = MP4MakeLinkedList( &self->atomList ); if (err) goto bail;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->findAtomOfType = findAtomOfType;
	self->addAtom = addAtom;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
