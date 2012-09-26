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
	$Id: MediaDataAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define allocation_size 0x600000

/* define FORCE_OFFSET 0x100000000ll */

static MP4Err calculateSize( struct MP4Atom* s );

static void destroy( MP4AtomPtr s )
{
	MP4MediaDataAtomPtr self;
	self = (MP4MediaDataAtomPtr) s;
	if ( self->data )
	{
		free( self->data );
		self->data = NULL;
	}
	if ( self->super )
		self->super->destroy( s );
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	MP4MediaDataAtomPtr self = (MP4MediaDataAtomPtr) s;
	err = MP4NoErr;
	
	if (self->size == 1) { BAILWITHERROR( MP4NotImplementedErr ); }
	
	if (self->size > 0) {
		err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    	buffer += self->bytesWritten;
    	PUTBYTES( self->data, (u32) self->dataSize );
		assert( self->bytesWritten == self->size );
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err writeToFile( struct MP4MediaDataAtom *self, FILE *fd )
{
	MP4Err err;
	size_t written;
	char buffer[ 16 ];
#ifdef FORCE_OFFSET
	s32 sought;
#endif

	err = MP4NoErr;
	if ( fd == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = calculateSize( (MP4AtomPtr) self ); if (err) goto bail;
	if (self->size > 0) {
		err = MP4SerializeCommonBaseAtomFields( (MP4AtomPtr) self, buffer ); if (err) goto bail;
		written = fwrite( buffer, 1, self->bytesWritten, fd );
		if ( written != self->bytesWritten )
			BAILWITHERROR( MP4IOErr )
#ifdef FORCE_OFFSET
		sought = fseeko( fd, FORCE_OFFSET, SEEK_CUR );
		if ( sought != 0 )
				BAILWITHERROR( MP4IOErr );

		written = fwrite( self->data, 1, self->dataSize - FORCE_OFFSET, fd );
			if ( written != (self->dataSize - FORCE_OFFSET) )
				BAILWITHERROR( MP4IOErr )
#else
		written = fwrite( self->data, 1, (size_t) self->dataSize, fd );
			if ( written != self->dataSize )
				BAILWITHERROR( MP4IOErr )
#endif
		self->bytesWritten += written;
		/* this is not reliable because bytesWritten is only a 32 and we may have written more */
	}
bail:
	TEST_RETURN( err );

	return err;
}


static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4MediaDataAtomPtr self = (MP4MediaDataAtomPtr) s;
	err = MP4NoErr;
	
	if (self->dataSize == 0) self->size = 0;
	else {
		if ((self->dataSize) >> 32) {
			err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
			self->size64 = self->dataSize + self->size + 8;
			self->size = 1;
		}
		else {
			err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
		self->size += (u32) self->dataSize;
		}
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addData( struct MP4MediaDataAtom *self, MP4Handle dataH )
{
	MP4Err err;
	u32 size;
	size_t newSize;
	
	err = MP4NoErr;
	err = MP4GetHandleSize( dataH, &size ); if (err) goto bail;
	if ( size > 0 )
	{
#ifdef FORCE_OFFSET
    	newSize = self->dataSize + size - FORCE_OFFSET;
#else
    	newSize = (size_t) (self->dataSize + size);
#endif
    	if (newSize > self->allocatedSize) {
			self->allocatedSize += allocation_size;
			if (newSize > self->allocatedSize)
				 self->allocatedSize = newSize;
			
			if (self->data != NULL)
				self->data = (char*) realloc( self->data, self->allocatedSize );
				else
				self->data = (char*) calloc( self->allocatedSize, 1 );
			TESTMALLOC( self->data );
		}
#ifdef FORCE_OFFSET
		memcpy( self->data + self->dataSize - FORCE_OFFSET, *dataH, size );
#else
		memcpy( self->data + self->dataSize, *dataH, size );
#endif
		self->dataSize += size;
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addMdat( struct MP4MediaDataAtom *self, struct MP4MediaDataAtom *other_mdat )
{
	MP4Err err;
	u32 size;
	size_t newSize;
	
	err = MP4NoErr;
	if ((other_mdat->dataSize + self->dataSize) >> 32) { BAILWITHERROR( MP4NotImplementedErr ); }
	
	size = (u32) (other_mdat->dataSize);
	
	if ( size > 0 )
	{
    	newSize = (size_t) (self->dataSize + size);
		if (newSize > self->allocatedSize) {
			self->allocatedSize += allocation_size;
			if (newSize > self->allocatedSize)
				 self->allocatedSize = newSize;
			
			if (self->data != NULL)
				self->data = (char*) realloc( self->data, self->allocatedSize );
				else
				self->data = (char*) calloc( self->allocatedSize, 1 );
			TESTMALLOC( self->data );
		}

    	memcpy( self->data + self->dataSize, other_mdat->data, size );
    	self->dataSize += size;
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u64 bytesToRead;
	MP4MediaDataAtomPtr self = (MP4MediaDataAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream );

	if (s->size ==1)
		 bytesToRead = s->size64 - s->bytesRead;
	else bytesToRead = s->size - s->bytesRead;
	
	self->dataSize = 0;
	/* it's possible to cause read files to use the filemappingdatahandler by
		discarding the data here, and setting dataSize to 0;  note that files
		read this way cannot be re-written to another file, only read.  see also 
		the flag MP4OpenMovieInPlace, which needs to be propagated here, and then
		the file mapping stream should also have a 'skip data' entry.
		
		We currently do this for large files (> 2GB) as the entire input stream 
		machinery assumes 32-bit sizes, and so on */
	
	if ((bytesToRead >> 31) > 0) {
		self->data = NULL;
		self->dataSize = 0;
		self->dataOffset = self->streamOffset + self->bytesRead;
		err = inputStream->skipData( inputStream, bytesToRead, "data" );
	}
	else if (bytesToRead > 0) {
		self->data = (char*) calloc( 1, (size_t) bytesToRead );
		TESTMALLOC( self->data )
	
		self->dataOffset = self->streamOffset + self->bytesRead;
	
		err = inputStream->readData( inputStream, bytesToRead, self->data, "data" );
		if ( err ) goto bail;

		s->bytesRead += (u32) bytesToRead;
		self->dataSize = bytesToRead;
		self->allocatedSize = (u32) bytesToRead;
	}
	
bail:
	TEST_RETURN( err );

	if ( err && self->data )
	{
		free( self->data );
	}
	return err;
}

MP4Err MP4CreateMediaDataAtom( MP4MediaDataAtomPtr *outAtom )
{
	MP4Err err;
	MP4MediaDataAtomPtr self;
	
	self = (MP4MediaDataAtomPtr) calloc( 1, sizeof(MP4MediaDataAtom) );
	TESTMALLOC( self )

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4MediaDataAtomType;
	self->name                = "media data";
	self->destroy             = destroy;
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->data                = NULL;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->addData			    = addData;
	self->writeToFile			= writeToFile;
	self->addMdat				= addMdat;
	
#ifdef FORCE_OFFSET
	self->dataSize = FORCE_OFFSET;
#endif
	
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
