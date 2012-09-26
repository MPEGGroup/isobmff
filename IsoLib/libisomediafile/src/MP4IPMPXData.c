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
derivative works. Copyright (c) 1999, 2000.
*/

#include "MP4IPMPXData.h"
#include "MP4Movies.h"
#include "MP4Impl.h"

#define GET_IPMPXDATA_BYTE( varName, what ) \
	err = inputStream->read8( inputStream, &varName, what ); if (err) goto bail; \
	bytesRead ++

MP4Err MP4ParseIPMPXData( MP4InputStreamPtr inputStream, MP4IPMPXDataPtr *outData )
{
    MP4Err err;
    u32 tag;
    u32 size;
    u32 val;
    MP4IPMPXDataPtr data;
    u32 bytesRead;
    u32 sizeBytes;
    u32 version;
  
    err = MP4NoErr;
    if ( (inputStream == NULL) || (outData == NULL) )
		BAILWITHERROR( MP4BadParamErr )
	bytesRead = 0;
	GET_IPMPXDATA_BYTE( tag, "class tag" );

    /* get size */
    size = 0;
    sizeBytes = 1;
    do
	{
		GET_IPMPXDATA_BYTE( val, "size byte" );
		sizeBytes++;
		size <<= 7;
		size |= val & 0x7F;
	} while ( val & 0x80 );
	size += sizeBytes;

    switch ( tag )
    {
        /* for now, just the default IPMPXData is implemented */
        default:
            err = MP4CreateIPMPXDefaultData( tag, size, bytesRead, &data ); if (err) goto bail;
            break;
    }

    /* read the version */
    err = inputStream->read8( inputStream, &version, NULL ); if (err) goto bail;
    data->version = version;
    data->bytesRead++;
    if ( data->version != 0x01 ) /* only version 1 is currently supported */
        BAILWITHERROR( MP4BadDataErr )

    /* continue to read the stream */
    err = data->createFromInputStream( data, inputStream ); if (err) goto bail;
    *outData = data;
bail:
    TEST_RETURN( err );

    return err;
}

MP4Err MP4EncodeBaseIPMPXData( struct MP4IPMPXDataRecord* self, char* buffer )
{
	MP4Err err;
	u32 length;
	u8 vals[ 4 ];
	err = MP4NoErr;
	
	self->bytesWritten = 0;
	
	length = self->size - IPMPXDATA_TAG_LEN_SIZE;
	vals[ 3 ] = (u8) (length & 0x7f);          length >>= 7;
	vals[ 2 ] = (u8) ((length & 0x7f) | 0x80); length >>= 7;
	vals[ 1 ] = (u8) ((length & 0x7f) | 0x80); length >>= 7;
	vals[ 0 ] = (u8) ((length & 0x7f) | 0x80);
	PUT8( tag );
	PUT8_V( vals[0] );
	PUT8_V( vals[1] );
	PUT8_V( vals[2] );
	PUT8_V( vals[3] );
    PUT8( version );
bail:
	TEST_RETURN( err );

	return err;
}
