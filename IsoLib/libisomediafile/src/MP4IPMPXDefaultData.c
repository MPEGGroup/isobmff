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

#include "MP4IPMPXData.h"
#include <string.h>

static MP4Err calculateSize( struct MP4IPMPXDataRecord* s )
{
    MP4Err err;

    MP4IPMPXDefaultDataPtr self = (MP4IPMPXDefaultDataPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    self->size = 6; /* tag + length word + version */
    self->size += self->dataLength;
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err serialize( struct MP4IPMPXDataRecord* s, char* buffer )
{
    MP4Err err;

    MP4IPMPXDefaultDataPtr self = (MP4IPMPXDefaultDataPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    err = MP4EncodeBaseIPMPXData( s, buffer ); if (err) goto bail;
    buffer += IPMPXDATA_TAG_LEN_SIZE + 1; /* +1 for self->version */
    PUTBYTES( self->data, self->dataLength );
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err createFromInputStream( struct MP4IPMPXDataRecord* s, MP4InputStreamPtr inputStream )
{
    MP4Err err;

    MP4IPMPXDefaultDataPtr self = (MP4IPMPXDefaultDataPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    self->dataLength = self->size - self->bytesRead;
    self->data = (char*) calloc( 1, self->dataLength );
    TESTMALLOC( self->data );
    GETBYTES( self->dataLength, data );
bail:
    TEST_RETURN( err );

    return err;
}

static void destroy( struct MP4IPMPXDataRecord* s )
{
    MP4Err err;

    MP4IPMPXDefaultDataPtr self = (MP4IPMPXDefaultDataPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    if ( self->data )
	{
		free( self->data );
		self->data = NULL;
	}
	free( s );

bail:
    return;
}

MP4Err MP4CreateIPMPXDefaultData( u32 tag, u32 size, u32 bytesRead, MP4IPMPXDataPtr *outData )
{
    MP4Err err;
	MP4IPMPXDefaultDataPtr self;
    err = MP4NoErr;
	self = (MP4IPMPXDefaultDataPtr) calloc( 1, sizeof(MP4IPMPXDefaultData) );
	TESTMALLOC( self );
	self->tag  = tag;
	self->size = size;
	self->name = "MP4IPMPXDefaultData";
	self->bytesRead = bytesRead;
	self->createFromInputStream = createFromInputStream;
	self->calculateSize = calculateSize;
	self->serialize = serialize;
	self->destroy = destroy;
    *outData = (MP4IPMPXDataPtr) self;
bail:
    TEST_RETURN( err );
    
    return err;
}
