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
	$Id: MemoryFileMappingObject.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/
#include "FileMappingObject.h"
#include <stdlib.h>

static MP4Err doClose( struct FileMappingObjectRecord *s );
static MP4Err doOpen( struct FileMappingObjectRecord *s, const char *filename );
static MP4Err isYourFile( struct FileMappingObjectRecord *s, const char *filename, u32 *outSameFile );

static MP4Err  destroy( struct FileMappingObjectRecord* s )
{
	MP4Err err;

	free( s );
	
	err = MP4NoErr;
	return err;
}

static MP4Err doOpen( struct FileMappingObjectRecord* s, const char *filename )
{
	MP4Err err;
	err = MP4NoErr;
	if ((s==NULL) || ( filename != NULL ))
		BAILWITHERROR( MP4BadParamErr );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err doClose( struct FileMappingObjectRecord* s)
{
	MP4Err err;
	
	if (s==NULL)
		err = MP4BadParamErr;
	err = MP4NoErr;
	return err;
}

static MP4Err isYourFile( struct FileMappingObjectRecord* s, const char *filename, u32 *outSameFile )
{
	MP4Err err;
	err = MP4NoErr;
	if (s==NULL)
		err = MP4BadParamErr;
	*outSameFile = (filename == NULL);
	return err;
}


MP4Err MP4CreateMemoryFileMappingObject( char *src, u32 size, FileMappingObject *outObject )
{
	MP4Err err;
	FileMappingObject self;
	
	err = MP4NoErr;
	self = (FileMappingObject) calloc( 1, sizeof(FileMappingObjectRecord) );
	TESTMALLOC( self );
	self->destroy    = destroy;
	self->open       = doOpen;
	self->close      = doClose;
	self->isYourFile = isYourFile;
	self->copyData   = 0;
	
	self->data = src;
	self->size64 = size;
	*outObject = (FileMappingObject) self;
bail:
	TEST_RETURN( err );

	return err;
}
