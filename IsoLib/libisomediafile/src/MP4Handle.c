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
	$Id: MP4Handle.c,v 1.4 2002/10/08 10:11:54 fred Exp $
*/
#include "MP4Movies.h"
#include <stdlib.h>
#include <string.h>

typedef struct
{
	char *data;
	char *truedata;
	u32  signature;
	u32  size;
	u32  allocatedSize;
	u32  offset;
} handleStruct, *actualHandle;

/* The handle offset makes the handle point to an offset into the datablock.  
   Setting the offset thus 'reserves space' at the front of the block, and
   reduces its effective size.  The handle size is thus set and reported without
   the offset.
 */

#define HANDLE_SIGNATURE 0x1234

MP4_EXTERN ( MP4Err )
MP4NewHandle( u32 handleSize, MP4Handle *outHandle )
{
	MP4Err err;
	MP4Handle ret;
	actualHandle h;
	ret = NULL;
	err = MP4NoErr;
	if ( outHandle == NULL )
	{
		err = MP4BadParamErr;
		goto bail;
	}
	h = (actualHandle) calloc( 1, sizeof(handleStruct) );
	if ( h != NULL )
	{
		if ( handleSize )
			h->data = (char*) calloc( 1, handleSize );
		if ( handleSize && h->data == NULL )
		{
			free( h );
			err = MP4NoMemoryErr;
			goto bail;
		}
		else
		{
			h->signature     = HANDLE_SIGNATURE;
			h->size          = handleSize;
			h->allocatedSize = handleSize;
			h->offset		 = 0;
			h->truedata		 = h->data;
			ret = (MP4Handle) h;
		}
	}
	*outHandle =  ret;
bail:
	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetHandleSize( MP4Handle theHandle, u32 *outSize )
{
	MP4Err err;
	actualHandle h = (actualHandle) theHandle;
	
	err = MP4NoErr;

	if ( (h == NULL) || (h->signature != HANDLE_SIGNATURE) || (outSize == NULL) )
	{
		err = MP4BadParamErr;
		goto bail;
	}
	*outSize = h->size;
bail:
	return err;
}

MP4_EXTERN ( MP4Err )
MP4SetHandleSize( MP4Handle theHandle, u32 requestedSize )
{
	MP4Err err;
	actualHandle h = (actualHandle) theHandle;
	err = MP4NoErr;
	if ( h == NULL || (h->signature != HANDLE_SIGNATURE) )
		err = MP4BadParamErr;
	else
	{
		if ( h->allocatedSize >= requestedSize )
		{
			h->size = requestedSize;
		}
		else
		{
			char *p = NULL;
			
			if ( h->data == NULL)
				p = (char*) malloc( requestedSize + h->offset );
			else
				p = (char*) realloc( h->truedata, requestedSize + h->offset );
			if ( p )
			{
				h->truedata = p;
				h->data = &( p[ h->offset ]);
				h->size = requestedSize;
				h->allocatedSize = requestedSize;
			}
			else
				err = MP4NoMemoryErr;
		}
	}
	return err;
}

MP4_EXTERN ( MP4Err )
MP4SetHandleOffset( MP4Handle theHandle, u32 offset )
{
	MP4Err err;
	actualHandle h = (actualHandle) theHandle;
	
	
	err = MP4NoErr;
	if ( h == NULL || (h->signature != HANDLE_SIGNATURE) )
		err = MP4BadParamErr;
	else
	{
		s32 adjust;
		char *p;
		
		adjust = offset - h->offset;
		
		if (adjust == 0)
			return MP4NoErr;
		else if ((offset > (h->size + h->offset)) && (h->size != 0))
			return MP4BadParamErr;
		
		if ((h->size == 0) && (adjust > 0)) {
			err = MP4SetHandleSize( theHandle, adjust );
			if (err) return err;
		}
		
		h->offset = offset;
		h->size -= adjust;
		h->allocatedSize -= adjust;
		p = h->truedata;
		h->data = &( p[ offset ] );
	}
	return err;
}

MP4_EXTERN ( MP4Err )
MP4DisposeHandle( MP4Handle theHandle )
{
	MP4Err err;
	actualHandle h = (actualHandle) theHandle;
	err = MP4NoErr;
	if ( h == NULL || (h->signature != HANDLE_SIGNATURE) )
		err = MP4BadParamErr;
	else
	{
		if ( h->data )
			free( h->truedata );
		free( h );
	}
	return err;
}

MP4_EXTERN ( MP4Err )
MP4HandleCat(MP4Handle theDstHandle, MP4Handle theSrcHandle)  
{
	MP4Err err;
	u32 requestedSize, offset;
	actualHandle dsth = (actualHandle) theDstHandle;
	actualHandle srch = (actualHandle) theSrcHandle;
	err = MP4NoErr;

	if ( ( dsth == NULL || (dsth->signature != HANDLE_SIGNATURE) ) || 
		 ( srch == NULL || (srch->signature != HANDLE_SIGNATURE) )   )
	{
		err = MP4BadParamErr;
		goto bail;
	}

	requestedSize = dsth->size + srch->size;
	offset = dsth->size;
	
	if ( dsth->allocatedSize >= requestedSize )
	{
		dsth->size = requestedSize;
	}
	else
	{
		char *p = NULL;

		if ( dsth->data == NULL)
			p = (char*) malloc( srch->size + dsth->offset );
		else
			p = (char*) realloc( dsth->truedata, requestedSize + dsth->offset );
			/* that used to be srch->size, not requestedsize;  surely wrong? dws */
			
		if ( p )
		{
			dsth->data = &( p[ dsth->offset ] );
			dsth->truedata = p;
			dsth->size = requestedSize;
			dsth->allocatedSize = requestedSize;
		}
		else
			err = MP4NoMemoryErr;
	}
	memcpy(dsth->data + offset, srch->data, srch->size);  		

bail :
	return err;
}


