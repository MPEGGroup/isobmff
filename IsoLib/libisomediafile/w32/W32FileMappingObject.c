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
	$Id: W32FileMappingObject.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "W32FileMappingObject.h"
#include <string.h>
#include <sys/stat.h>

static MP4Err doClose( struct FileMappingObjectRecord *s );
static MP4Err doOpen( struct FileMappingObjectRecord *s, const char *filename );
static MP4Err isYourFile( struct FileMappingObjectRecord *s, const char *filename, u32 *outSameFile );

static MP4Err  destroy( struct FileMappingObjectRecord *s )
{
	MP4Err err;
	Win32FileMappingObject self = (Win32FileMappingObject) s;
	
	err = MP4NoErr;
	if ( self->data )
		err = doClose( s ); if (err) goto bail;
	if ( self->parent )
	{
	    free( self->parent );
	}

	free( s );
bail:
	return err;
}

static MP4Err doOpen( struct FileMappingObjectRecord *s, const char *filename )
{
	MP4Err err;
	Win32FileMappingObject self = (Win32FileMappingObject) s;
	HANDLE fileHandle;
	HANDLE fileMappingHandle;	
	/*DWORD  fileSizeHigh; */
	DWORD  fileSizeLow;
	char *separator;
	
	err = MP4NoErr;
	if ( self->data )
		doClose( s );
	self->fileName = _strdup( filename );
	TESTMALLOC( self->fileName );
	
	fileHandle = CreateFile( filename, GENERIC_READ, FILE_SHARE_READ, NULL,
									OPEN_EXISTING,
									(FILE_ATTRIBUTE_READONLY | FILE_FLAG_SEQUENTIAL_SCAN),
									NULL );
	if ( fileHandle == INVALID_HANDLE_VALUE )
		BAILWITHERROR( MP4FileNotFoundErr )
	fileSizeLow = GetFileSize( fileHandle, NULL );
	if ( fileSizeLow == 0xFFFFFFFF )
		BAILWITHERROR( MP4IOErr )
	self->size = fileSizeLow;
	self->size64 = fileSizeLow;
	fileMappingHandle = CreateFileMapping( fileHandle, NULL, PAGE_READONLY,	0, 0, NULL );
	if ( fileMappingHandle == NULL )
		BAILWITHERROR( MP4IOErr )
		
	self->data = MapViewOfFile( fileMappingHandle, FILE_MAP_READ, 0, 0, 0 );
	if ( self->data == NULL )
		BAILWITHERROR( MP4IOErr )

	CloseHandle( fileHandle );
	CloseHandle( fileMappingHandle );		
	/* make parent */
	if ( (separator = strrchr(filename, '\\')) || (separator = strrchr(filename, '/')) )
	{
	    int len = separator - filename;
	    if ( self->parent )
		{
	    	free( self->parent );
		}

	    self->parent = (char *) calloc( 1, len + 2 );
	    TESTMALLOC( self->parent );
	    strncpy( self->parent, filename, len + 1 );
	    /* strcat( self->parent, "/" ); */
	}
bail:
	return err;
}

static MP4Err doClose( struct FileMappingObjectRecord *s )
{
	BOOL val;
	MP4Err err;
	Win32FileMappingObject self = (Win32FileMappingObject) s;
	
	err = MP4NoErr;
	val = UnmapViewOfFile( self->data );
	if ( val==0 )
		err = MP4IOErr;
	self->data = NULL;
	if ( self->fileName )
		free( self->fileName );
	self->fileName = NULL;
	if ( self->parent )
	{
		free( self->parent );
	}
	self->parent = NULL;
	
	return err;
}

static MP4Err isYourFile( struct FileMappingObjectRecord *s, const char *filename, u32 *outSameFile )
{
	MP4Err err;
	Win32FileMappingObject self = (Win32FileMappingObject) s;
	int result;
	err = MP4NoErr;
	
	result = _stricmp( self->fileName, filename );
	*outSameFile = result ? 0 : 1;
	return err;
}


MP4Err MP4CreateWin32FileMappingObject( char *filename, FileMappingObject *outObject )
{
	MP4Err err;
	Win32FileMappingObject self;
	
	err = MP4NoErr;
	self = (Win32FileMappingObject) calloc( 1, sizeof(Win32FileMappingObjectRecord) );
	TESTMALLOC( self );
	self->destroy    = destroy;
	self->open       = doOpen;
	self->close      = doClose;
	self->isYourFile = isYourFile;
	err = doOpen( (FileMappingObject) self, filename ); if (err) goto bail;
	*outObject = (FileMappingObject) self;
bail:
	return err;
}

MP4Err MP4CreateFileMappingObject( char *urlString, FileMappingObject *outObject )
{
    char *pathname = urlString;
    if ( (strncmp(urlString, "file://", 7) == 0) || (strncmp(urlString, "file|//", 7) == 0) )
    {
        pathname += 7;
    }
	return MP4CreateWin32FileMappingObject( pathname, outObject );
}

MP4Err MP4AssertFileExists( char *pathName )
{
	MP4Err err;
    struct stat buf;

	err = stat( pathName, &buf );
	if ( err )
    {
       BAILWITHERROR( MP4FileNotFoundErr );
    }
bail:
    return err;
}
