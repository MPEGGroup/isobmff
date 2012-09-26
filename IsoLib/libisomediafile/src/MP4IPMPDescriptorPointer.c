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
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include <stdlib.h>

static MP4Err calculateSize( struct MP4DescriptorRecord* s )
{
    MP4Err err;
    MP4IPMPDescriptorPointerPtr self = (MP4IPMPDescriptorPointerPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    self->size = DESCRIPTOR_TAG_LEN_SIZE;
    self->size += 1; /* IPMPDescriptorId */
    if ( self->ipmpDescriptorId == 0xFF )
    {
        self->size += 4; /* IPMPToolDescriptorId + IPMPEsId */
    }
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
    MP4Err err;
    MP4IPMPDescriptorPointerPtr self = (MP4IPMPDescriptorPointerPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
    buffer += DESCRIPTOR_TAG_LEN_SIZE;
    PUT8(ipmpDescriptorId);
    if ( self->ipmpDescriptorId == 0xFF )
    {
        PUT16(ipmpToolDescriptorId);
        PUT16(ipmpEsId);
    }
    if ( self->bytesWritten != self->size )
        BAILWITHERROR( MP4BadDataErr );
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err createFromInputStream( struct MP4DescriptorRecord* s, MP4InputStreamPtr inputStream )
{
    MP4Err err;
    MP4IPMPDescriptorPointerPtr self = (MP4IPMPDescriptorPointerPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    GET8(ipmpDescriptorId);
    if ( self->ipmpDescriptorId == 0xFF )
    {
        GET16(ipmpToolDescriptorId);
        GET16(ipmpEsId);
    }
    if ( self->bytesRead != self->size )
        BAILWITHERROR( MP4BadDataErr );
bail:
    TEST_RETURN( err );

    return err;
}

static void destroy( struct MP4DescriptorRecord* s )
{
    MP4Err err;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    free( s );
bail:
    return;
}

MP4Err MP4CreateIPMPDescriptorPointer( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
    SETUP_BASE_DESCRIPTOR( MP4IPMPDescriptorPointer );
    *outDesc = (MP4DescriptorPtr) self;

bail:
    TEST_RETURN( err );

    return err;
}

