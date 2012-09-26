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
	$Id: MP4Descriptors.c,v 1.3 2002/09/25 18:31:08 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include "MP4Impl.h"

#define GET_DESCRIPTOR_BYTE( varName, what ) \
	err = inputStream->read8( inputStream, &varName, what ); if (err) goto bail; \
	bytesRead ++

MP4Err MP4ParseDescriptor( MP4InputStreamPtr inputStream, MP4DescriptorPtr *outDesc )
{
	MP4Err err;
	u32	tag;
	u32 size;
	u32 val;
	MP4DescriptorPtr desc;
	u32 bytesRead;
	u32 sizeBytes;
	char msgbuf[ 80 ];
	
	if ( (inputStream == NULL) || (outDesc == NULL) )
		BAILWITHERROR( MP4BadParamErr )
	inputStream->msg( inputStream, "{" );
	inputStream->indent++;
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
		case MP4InitialObjectDescriptorTag:
		case MP4_IOD_Tag:
			err = MP4CreateInitialObjectDescriptor( MP4_IOD_Tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
			
		case MP4ES_DescriptorTag:
			err = MP4CreateES_Descriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
			
		case MP4DecoderConfigDescriptorTag:
			err = MP4CreateDecoderConfigDescriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
			
		case MP4ES_ID_IncDescriptorTag:
			err = MP4CreateES_ID_IncDescriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
			
		case MP4ES_ID_RefDescriptorTag:
			err = MP4CreateES_ID_RefDescriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
			
		case MP4SLConfigDescriptorTag:
			err = MP4CreateSLConfigDescriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
		
		case MP4ObjectDescriptorTag:
		case MP4_OD_Tag:
			err = MP4CreateObjectDescriptor( MP4ObjectDescriptorTag, size, bytesRead, &desc ); if (err) goto bail;
			break;

        case MP4IPMP_ToolTag:
            err = MP4CreateIPMPTool( tag, size, bytesRead, &desc ); if (err) goto bail;
            break;

        case MP4IPMP_ToolListDescriptorTag:
            err = MP4CreateIPMPToolListDescriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
            break;

        case MP4IPMP_ToolDescriptorTag:
            err = MP4CreateIPMPToolDescriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
            break;

        case MP4IPMP_DescriptorPointerTag:
        case MP4_IPMP_ToolDescriptorPointerTag:
            err = MP4CreateIPMPDescriptorPointer( tag, size, bytesRead, &desc ); if (err) goto bail;
            break;

		default:
			err = MP4CreateDefaultDescriptor( tag, size, bytesRead, &desc ); if (err) goto bail;
			break;
	}
	sprintf( msgbuf, "descriptor is %s", desc->name );
	inputStream->msg( inputStream, msgbuf );
	err = desc->createFromInputStream( desc, inputStream ); if (err) goto bail;
	*outDesc = desc;
	inputStream->indent--;
	inputStream->msg( inputStream, "}" );
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CopyDescriptor( MP4DescriptorPtr inDesc, MP4DescriptorPtr *outDesc )
{
	MP4Err err;
	MP4Handle myHandle;
	MP4InputStreamPtr is;
	
	is = NULL; myHandle = NULL;
	
	err = inDesc->calculateSize( inDesc ); if (err) goto bail;
	err = MP4NewHandle(inDesc->size, &myHandle); if (err) goto bail;
	err = inDesc->serialize( inDesc, *myHandle ); if (err) goto bail;
	
	err = MP4CreateMemoryInputStream(*myHandle, inDesc->size, &is ); if (err) goto bail;
	is->debugging = 0;
		

	err = MP4ParseDescriptor( is, outDesc ); if (err) goto bail;

bail:
	if (is) {
		is->destroy( is );
		is = NULL;
	}
	
	if (myHandle)
	{
		err = MP4DisposeHandle( myHandle );
		myHandle = NULL;
	}
 

	TEST_RETURN( err );

	return err;
}

