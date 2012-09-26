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
	$Id: UserDataAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>


typedef struct UserDataMapRecord
{
	u32 atomType;
	MP4LinkedList atomList;
} UserDataMapRecord, *UserDataMapRecordPtr;

static MP4Err getEntryCount( struct MP4UserDataAtom *self, u32 userDataType, u32 *outCount );
static MP4Err getIndType( struct MP4UserDataAtom *self, u32 typeIndex, u32 *outType );
static MP4Err getItem( struct MP4UserDataAtom *self, MP4Handle userDataH, u32 userDataType, u32 itemIndex );
static MP4Err getTypeCount( struct MP4UserDataAtom *self, u32 *outCount );

static MP4Err findEntryForAtomType( MP4UserDataAtomPtr self, u32 atomType, UserDataMapRecordPtr *outRecord );

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4UserDataAtomPtr self;
	u32 i;
	u32 recordCount;
	u32 recordIndex;
	err = MP4NoErr;
	self = (MP4UserDataAtomPtr) s;
	if ( self == NULL ) BAILWITHERROR( MP4BadParamErr )
	err = MP4GetListEntryCount( self->recordList, &recordCount ); if (err) goto bail;
	for ( recordIndex = 0; recordIndex < recordCount; recordIndex++ )
	{
		UserDataMapRecordPtr p;
		err = MP4GetListEntry( self->recordList, recordIndex, (char **) &p ); if (err) goto bail;
		DESTROY_ATOM_LIST_V( p->atomList );
		free( p );
	}
	err = MP4DeleteLinkedList( self->recordList ); if (err) goto bail;
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err addUserData(struct MP4UserDataAtom *self, MP4Handle userDataH, u32 userDataType, u32 *outIndex )
{
   MP4Err MP4CreateUnknownAtom( MP4UnknownAtomPtr *outAtom );
   
    MP4Err err;
	UserDataMapRecordPtr rec;
	MP4UnknownAtomPtr atom;
	
	err = MP4CreateUnknownAtom( &atom ); if (err) goto bail;
	atom->type = userDataType;
	err = MP4GetHandleSize( userDataH, &atom->dataSize ); if (err) goto bail;
	if ( atom->dataSize > 0 )
	{
		atom->data = (char*) malloc( atom->dataSize );
		TESTMALLOC( atom->data )
		memcpy( atom->data, *userDataH,  atom->dataSize );
	}
	err = findEntryForAtomType( self, userDataType, &rec ); if (err) goto bail;
	if ( rec == NULL )
	{
		rec = (UserDataMapRecordPtr) calloc( 1, sizeof(struct UserDataMapRecord) );
		TESTMALLOC( rec )
		rec->atomType = atom->type;
		err = MP4MakeLinkedList( &rec->atomList ); if (err) goto bail;
		err = MP4AddListEntry( rec, self->recordList ); if (err) goto bail;
	}
	err = MP4AddListEntry( atom, rec->atomList ); if (err) goto bail;	
	if ( outIndex != NULL )
	{
		err = MP4GetListEntryCount( rec->atomList, outIndex ); if (err) goto bail;		
	}
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err findEntryForAtomType( MP4UserDataAtomPtr self, u32 atomType, UserDataMapRecordPtr *outRecord )
{
	MP4Err err;
	u32 i;
	u32 count;
	*outRecord = 0;
	err = MP4GetListEntryCount( self->recordList, &count ); if (err) goto bail;
	for ( i = 0; i < count; i++ )
	{
		UserDataMapRecordPtr p;
		err = MP4GetListEntry( self->recordList, i, (char **) &p ); if (err) goto bail;
		if ( (p != 0) && (p->atomType == atomType) )
		{
			*outRecord = p;
			break;
		}
	}
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err getTypeCount( struct MP4UserDataAtom *self, u32 *outCount )
{
	MP4Err err;

	err = MP4GetListEntryCount( self->recordList, outCount ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err getEntryCount( struct MP4UserDataAtom *self, u32 userDataType, u32 *outCount )
{
	MP4Err err;
	u32 i;
	u32 count;
	*outCount = 0;
	err = MP4GetListEntryCount( self->recordList, &count ); if (err) goto bail;
	for ( i = 0; i < count; i++ )
	{
		UserDataMapRecordPtr p;
		err = MP4GetListEntry( self->recordList, i, (char **) &p ); if (err) goto bail;
		if ( (p != 0) && (p->atomType == userDataType) && (p->atomList != 0) )
		{
			err = MP4GetListEntryCount( p->atomList, outCount ); if (err) goto bail;
			break;
		}
	}
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err getIndType( struct MP4UserDataAtom *self, u32 typeIndex, u32 *outType )
{
	MP4Err err;
	u32 count;
	UserDataMapRecordPtr p;
	*outType = 0;
	err = MP4GetListEntryCount( self->recordList, &count ); if (err) goto bail;
	if ( typeIndex > count )
	{
		BAILWITHERROR( MP4BadParamErr );
	}		
	err = MP4GetListEntry( self->recordList, typeIndex - 1, (char **) &p ); if (err) goto bail;
	*outType = p->atomType;
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err getItem( struct MP4UserDataAtom *self, MP4Handle userDataH, u32 userDataType, u32 itemIndex )
{
	MP4Err err;
	UserDataMapRecordPtr rec;
	MP4UnknownAtomPtr atom;

	err = findEntryForAtomType( self, userDataType, &rec ); if (err) goto bail;
	if ( rec == 0 )
	{
		BAILWITHERROR( MP4BadParamErr );
	}
	err = MP4GetListEntry( rec->atomList, itemIndex - 1, (char **) &atom ); if (err) goto bail;
	err = atom->calculateSize( (MP4AtomPtr) atom ); if (err) goto bail;
	err = MP4SetHandleSize( userDataH, atom->dataSize ); if (err) goto bail;
	memcpy( *userDataH, atom->data, atom->dataSize );
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err deleteItem( struct MP4UserDataAtom *self, u32 userDataType, u32 itemIndex )
{
	MP4Err err;
	UserDataMapRecordPtr rec;

	err = findEntryForAtomType( self, userDataType, &rec ); if (err) goto bail;
	if ( rec == 0 )
	{
		BAILWITHERROR( MP4BadParamErr );
	}
	err = MP4DeleteListEntry( rec->atomList, itemIndex - 1); if (err) goto bail;
bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err addAtom( MP4UserDataAtomPtr self, MP4AtomPtr atom )
{
	MP4Err err;
	UserDataMapRecordPtr rec;
	err = findEntryForAtomType( self, atom->type, &rec ); if (err) goto bail;
	if ( rec == 0 )
	{
		rec = (UserDataMapRecordPtr) calloc( 1, sizeof(struct UserDataMapRecord) );
		TESTMALLOC( rec )
		rec->atomType = atom->type;
		err = MP4MakeLinkedList( &rec->atomList ); if (err) goto bail;
		err = MP4AddListEntry( rec, self->recordList ); if (err) goto bail;
	}
	err = MP4AddListEntry( atom, rec->atomList );
bail:
	TEST_RETURN( err );
	return err;
}


static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 recordCount;
	u32 recordIndex;
	MP4UserDataAtomPtr self = (MP4UserDataAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    err = MP4GetListEntryCount( self->recordList, &recordCount ); if (err) goto bail;
	for ( recordIndex = 0; recordIndex < recordCount; recordIndex++ )
	{
		UserDataMapRecordPtr p;
		err = MP4GetListEntry( self->recordList, recordIndex, (char **) &p ); if (err) goto bail;
    	SERIALIZE_ATOM_LIST_V( p->atomList );
    }
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	u32 recordCount;
	u32 recordIndex;
	MP4UserDataAtomPtr self = (MP4UserDataAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	err = MP4GetListEntryCount( self->recordList, &recordCount ); if (err) goto bail;
	for ( recordIndex = 0; recordIndex < recordCount; recordIndex++ )
	{
		UserDataMapRecordPtr p;
		err = MP4GetListEntry( self->recordList, recordIndex, (char **) &p ); if (err) goto bail;
		ADD_ATOM_LIST_SIZE_V( p->atomList );
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	/* PARSE_ATOM_LIST(MP4UserDataAtom) */

	MP4Err err; 
	u32 junk;
	
	MP4UserDataAtomPtr self = (MP4UserDataAtomPtr) s;
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	while ( self->bytesRead < (self->size-4) )
	{ 
		MP4AtomPtr atom; 
		err = MP4ParseAtom( (MP4InputStreamPtr) inputStream, &atom ); 
			if (err) goto bail; 
		self->bytesRead += atom->size; 
		if ( ((atom->type)== MP4FreeSpaceAtomType) || ((atom->type)== MP4SkipAtomType)) 
			atom->destroy( atom ); 
		else { 
			err = addAtom( self, atom ); 
				if (err) goto bail; 
		} 
	} 
	if ((self->size - self->bytesRead) == 4) { GET32_V( junk ); }
	
	if ( self->bytesRead != self->size ) 
		BAILWITHERROR( MP4BadDataErr ) 

bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateUserDataAtom( MP4UserDataAtomPtr *outAtom )
{
	MP4Err err;
	MP4UserDataAtomPtr self;
	
	self = (MP4UserDataAtomPtr) calloc( 1, sizeof(MP4UserDataAtom) );
	TESTMALLOC( self )

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4UserDataAtomType;
	self->name                = "user data";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	err = MP4MakeLinkedList( &self->recordList ); if (err) goto bail;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->addUserData = addUserData;
	self->getEntryCount = getEntryCount;
	self->getIndType = getIndType;
	self->getItem = getItem;
	self->deleteItem = deleteItem;
	self->getTypeCount = getTypeCount;

	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
