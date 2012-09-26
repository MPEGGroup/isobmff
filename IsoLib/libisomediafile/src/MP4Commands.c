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
	$Id: MP4Commands.c,v 1.2 2002/09/25 18:31:08 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include "MP4Impl.h"

#define GET_DESCRIPTOR_BYTE( varName, what ) \
	err = inputStream->read8( inputStream, &varName, what ); if (err) goto bail; \
	bytesRead ++

MP4Err MP4ParseCommand( MP4InputStreamPtr inputStream, MP4DescriptorPtr *outDesc )
{
	MP4Err err;
	u32	tag;
	u32 size;
	u32 val;
	u32 sizeBytes;
	MP4DescriptorPtr desc;
	u32 bytesRead;
	char msgbuf[ 80 ];
	
	if ( (inputStream == NULL) || (outDesc == NULL) )
		BAILWITHERROR( MP4BadParamErr )
	bytesRead = 0;
	GET_DESCRIPTOR_BYTE( tag, "class tag" );
	
	/* get size */
	size = 0;
	sizeBytes = 1;
	do
	{
		GET_DESCRIPTOR_BYTE( val, "size byte" );
		sizeBytes++;
		size <<= 7;
		size |= val & 0x7F;
	} while ( val & 0x80 );
	size += sizeBytes;

	switch ( tag )
	{

		case MP4ObjectDescriptorUpdateTag:
			err = MP4CreateObjectDescriptorUpdate( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
			
		case MP4ESDescriptorUpdateTag:
			err = MP4CreateESDescriptorUpdate( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
			
        case MP4IPMP_ToolDescriptorUpdateTag:
            err = MP4CreateIPMPToolDescriptorUpdate( tag, size, bytesRead, &desc ); if (err) goto bail;
            break;

		default:
			err = MP4CreateDefaultCommand( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
	}
	sprintf( msgbuf, "command is %s", desc->name );
	err = desc->createFromInputStream( desc, inputStream ); if (err) goto bail;
	*outDesc = desc;
bail:
	TEST_RETURN( err );

	return err;
}
