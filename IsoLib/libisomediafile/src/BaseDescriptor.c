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
	$Id: BaseDescriptor.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include "MP4Impl.h"


MP4Err MP4EncodeBaseDescriptor( struct MP4DescriptorRecord* self, char* buffer )
{
	MP4Err err;
	u32 length;
	u8 vals[ 4 ];
	err = MP4NoErr;
	
	self->bytesWritten = 0;
	
	length = self->size - DESCRIPTOR_TAG_LEN_SIZE;
	vals[ 3 ] = (u8) (length & 0x7f);          length >>= 7;
	vals[ 2 ] = (u8) ((length & 0x7f) | 0x80); length >>= 7;
	vals[ 1 ] = (u8) ((length & 0x7f) | 0x80); length >>= 7;
	vals[ 0 ] = (u8) ((length & 0x7f) | 0x80);
	PUT8( tag );
	PUT8_V( vals[0] );
	PUT8_V( vals[1] );
	PUT8_V( vals[2] );
	PUT8_V( vals[3] );
bail:
	TEST_RETURN( err );

	return err;
}
