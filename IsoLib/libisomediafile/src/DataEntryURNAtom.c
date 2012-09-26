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
	$Id: DataEntryURNAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4DataEntryURNAtomPtr self = (MP4DataEntryURNAtomPtr) s;
    err = MP4NoErr;
	if ( s == NULL )
		BAILWITHERROR( MP4BadParamErr )
		
	if ( self->nameURN ) {
		free( self->nameURN );
		self->nameURN = NULL;
	}
	
	if ( s->super )
		s->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err getOffset( struct MP4DataEntryAtom *self, u64 *outOffset )
{
	MP4Err err;
	
	err = MP4NoErr;
	if ( outOffset == NULL )
		BAILWITHERROR( MP4BadParamErr );
	if ( self->mdat )
	{
		MP4MediaDataAtomPtr mdat;
		mdat = (MP4MediaDataAtomPtr) self->mdat;
		self->offset = mdat->dataSize;
	}
	*outOffset = self->offset;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSampleReference( struct MP4DataEntryAtom *self, u64 dataOffset, MP4Handle sizesH )
{
	MP4Err err;
	
	err = MP4NoErr;
	dataOffset = dataOffset;
	sizesH = sizesH;
	if ( self->mdat )
		BAILWITHERROR( MP4BadParamErr );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSamples( struct MP4DataEntryAtom *self, MP4Handle sampleH )
{
	u32 size;
	MP4Err err;
	
	err = MP4NoErr;
	err = MP4GetHandleSize( sampleH, &size ); if (err) goto bail;
	if ( self->mdat )
	{
		MP4MediaDataAtomPtr mdat;
		mdat = (MP4MediaDataAtomPtr) self->mdat;
		err = mdat->addData( mdat, sampleH ); if (err) goto bail;	
		self->offset = mdat->dataSize;
	}
	else
		self->offset += size;
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	MP4DataEntryURNAtomPtr self = (MP4DataEntryURNAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;	
    if ( (self->flags & 1) == 0 )
	{
		if ( self->location )
		{
			u32 len = strlen( self->location ) + 1;
			PUTBYTES( self->location, len );
			if ( self->name )
			{
				len = strlen( self->name ) + 1;
				PUTBYTES( self->name, len );
			}
		}
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4DataEntryURNAtomPtr self = (MP4DataEntryURNAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	if ( (self->flags & 1) == 0 )
	{
		if ( self->location )
			self->size += 1 + strlen( self->location );
		if ( self->name )
			self->size += 1 + strlen( self->name );
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	long bytesToRead;
	MP4DataEntryURNAtomPtr self = (MP4DataEntryURNAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	bytesToRead = self->size - self->bytesRead;
	if ( bytesToRead < 0 )
		BAILWITHERROR( MP4BadDataErr )
	
	self->nameURN = (char*) calloc( 1, bytesToRead );
	if ( self->nameURN == NULL )
		BAILWITHERROR( MP4NoMemoryErr )
	if ( bytesToRead )
	{
		char *cp = self->nameURN;
		self->location = NULL;
		GETBYTES( bytesToRead, nameURN );
		self->locationLength = bytesToRead;
		while ( bytesToRead-- > 0 )
		{
			self->nameLength++;
			if ( *cp++ == 0 )
			{
				self->location = cp;
				break;
			}
		}
		/* !!! HERE !!! */
		self->locationLength -= self->nameLength;
		if ( self->location == NULL )
			BAILWITHERROR( MP4BadDataErr )
	}
	else
		self->location = self->nameURN;
bail:
	TEST_RETURN( err );

	if ( err )
	{
		if ( self->nameURN )
			free( self->nameURN );
	}
	return err;
}

MP4Err MP4CreateDataEntryURNAtom( MP4DataEntryURNAtomPtr *outAtom )
{
	MP4Err err;
	MP4DataEntryURNAtomPtr self;
	
	self = (MP4DataEntryURNAtomPtr) calloc( 1, sizeof(MP4DataEntryURNAtom) );
	if ( self == NULL )
		BAILWITHERROR( MP4NoMemoryErr )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4DataEntryURNAtomType;
	self->name                = "data entry URN";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->getOffset = getOffset;
	self->addSamples = addSamples;
	self->addSampleReference = addSampleReference;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
