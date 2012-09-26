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
	$Id: MP4FileMappingInputStream.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4InputStream.h"
#include "MP4Impl.h"
#include <stdlib.h>
#include <string.h>

#define CHECK_AVAIL( bytes ) \
	if ( self->available < bytes ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	}
static MP4Err readData( struct MP4InputStreamRecord *s, u64 bytes, char *outData, char *msg );

static void doIndent( struct MP4InputStreamRecord *self )
{
	u32 i;
	for ( i = 0; i < self->indent; i++ )
		fprintf( stdout, "    " );
}

static void msg( struct MP4InputStreamRecord *self, char *msg )
{
	if ( self->debugging )
	{
		doIndent( self );
		fprintf( stdout, "%s\n", msg );
	}
}

static void destroy( struct MP4InputStreamRecord *s )
{
	free(s);
}

static FileMappingObject getFileMappingObject( struct MP4InputStreamRecord *s )
{
	MP4FileMappingInputStreamPtr self = (MP4FileMappingInputStreamPtr) s;
	return self->mapping;
}

static MP4Err read8( struct MP4InputStreamRecord *s,  u32 *outVal, char *msg )
{
	MP4Err err;
	u8 val;
	MP4FileMappingInputStreamPtr self = (MP4FileMappingInputStreamPtr) s;
	err = MP4NoErr;

	err = readData( s, 1, (char*) &val, NULL ); if (err) goto bail;
	*outVal = val;
	if ( msg && self->debugging )
	{
		doIndent( s );
		fprintf( stdout, "%s = %d\n", msg, val );
	}

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err read16( struct MP4InputStreamRecord *s, u32 *outVal, char *msg )
{
	u8 a;
	int val;
	MP4Err err;
	MP4FileMappingInputStreamPtr self = (MP4FileMappingInputStreamPtr) s;
	err = MP4NoErr;

	CHECK_AVAIL( 2 )
	err = readData( s, 1, (char *) &a, NULL ); if (err) goto bail;
	val = a << 8;
	err = readData( s, 1, (char *) &a, NULL ); if (err) goto bail;
	val |= a;
	*outVal = val;
	if ( msg && self->debugging )
	{
		doIndent( s );
		fprintf( stdout, "%s = %d\n", msg, val );
	}

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err read32( struct MP4InputStreamRecord *s, u32 *outVal, char *msg )
{
	unsigned int hw;
	unsigned int lw;
	MP4Err err;
	MP4FileMappingInputStreamPtr self = (MP4FileMappingInputStreamPtr) s;
	err = MP4NoErr;

	CHECK_AVAIL( 4 )
	err = read16( s, (u32 *) &hw, NULL ); if (err) goto bail;
	err = read16( s, (u32 *) &lw, NULL ); if (err) goto bail;
	*outVal = (hw << 16) | lw;
	if ( msg && self->debugging )
	{
		doIndent( s );
		fprintf( stdout, "%s = %d\n", msg, *outVal );
	}
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err skipData( struct MP4InputStreamRecord *s, u64 bytes, char *msg )
{
	MP4Err err;
	MP4FileMappingInputStreamPtr self = (MP4FileMappingInputStreamPtr) s;
	
	err = MP4NoErr;
	
	CHECK_AVAIL( bytes )
	self->available      -= bytes;
	self->current_offset += bytes;

	if ( msg && self->debugging )
	{
		doIndent( s );
		fprintf( stdout, "%s = [%lld bytes of data skipped]\n", msg, bytes );
	}

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err readData( struct MP4InputStreamRecord *s, u64 bytes, char *outData, char *msg )
{
	MP4Err err;
	MP4FileMappingInputStreamPtr self = (MP4FileMappingInputStreamPtr) s;
	u32 to_read;
	FileMappingObject themap;
	
	err = MP4NoErr;
	themap = self->mapping;

	if (bytes >> 32) { BAILWITHERROR( MP4NotImplementedErr ); }

	to_read = (u32) bytes;
	CHECK_AVAIL( to_read );
	
	if (( themap->copyData )) {
		err = themap->copyData( themap, self->current_offset, outData, to_read );
	}
	else
	{
		memcpy( outData, (self->base) + (self->current_offset), to_read );
	}
	
	self->available      -= to_read;
	self->current_offset += to_read;
	if ( msg && self->debugging )
	{
		doIndent( s );
		fprintf( stdout, "%s = [%d bytes of data]\n", msg, to_read );
	}

bail:
	TEST_RETURN( err );

	return err;
}

static u64 getStreamOffset( struct MP4InputStreamRecord *s )
{
	MP4FileMappingInputStreamPtr self = (MP4FileMappingInputStreamPtr) s;
	return (self->current_offset);
}

MP4Err MP4CreateFileMappingInputStream( struct FileMappingObjectRecord* mapping, MP4InputStreamPtr *outStream )
{
	MP4Err err;
	MP4FileMappingInputStreamPtr is;
	
	err = MP4NoErr;
	is = (MP4FileMappingInputStreamPtr) calloc( 1, sizeof(MP4FileMappingInputStream) );
	TESTMALLOC( is )
	is->available = mapping->size64;
	is->base      = mapping->data;
	is->current_offset = 0;
	is->destroy   = destroy;
	is->read8     = read8;
	is->read16    = read16;
	is->read32    = read32;
	is->readData  = readData;
	is->skipData  = skipData;
	is->getStreamOffset = getStreamOffset;
	is->getFileMappingObject = getFileMappingObject;
	is->mapping   = mapping;
	is->msg = msg;
	*outStream = (MP4InputStreamPtr) is;
bail:
	TEST_RETURN( err );

	return err;
}
