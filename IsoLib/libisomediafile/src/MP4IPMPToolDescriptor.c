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
#include <string.h>

static MP4Err addIPMPXData( struct MP4DescriptorRecord* s, MP4IPMPXDataPtr data )
{
    MP4Err err;
    MP4IPMPToolDescriptorPtr self = (MP4IPMPToolDescriptorPtr) s;
    err = MP4NoErr;
    err = MP4AddListEntry( data, self->ipmpxData ); if (err) goto bail;
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err calculateSize( struct MP4DescriptorRecord* s )
{
    MP4Err err;
    u32 i;
    u32 ipmpxDataCount;
    MP4IPMPToolDescriptorPtr self = (MP4IPMPToolDescriptorPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    self->size = DESCRIPTOR_TAG_LEN_SIZE; /* tag + length word */ 
    self->size += 18; /* ipmpToolDescId + ipmpToolId */
    if ( self->ipmpToolId->upperPart == 0 
         && self->ipmpToolId->lowerPart == 0 )
    {
        self->size += 4; /* size of ByteArray */
        self->size += self->url->size;
    }
    else 
    {
        self->size += 1; /* initialize tag */
        if ( self->initData != NULL )
        {
            self->initData->calculateSize( self->initData );
            self->size += self->initData->size;
        }
        self->size += 1; /* ipmpxDataCount */
        err = MP4GetListEntryCount( self->ipmpxData, &ipmpxDataCount ); if (err) goto bail;
        for ( i = 0; i < ipmpxDataCount; i++ )
        {
            MP4IPMPXDataPtr entry;
            err = MP4GetListEntry( self->ipmpxData,
                                   i,
                                   (char **) &entry ); if (err) goto bail;
            entry->calculateSize( entry );
            self->size += entry->size;
        }
    }

bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
    MP4Err err;
    u32 ipmpxDataCount;
    u32 i;
    MP4IPMPToolDescriptorPtr self = (MP4IPMPToolDescriptorPtr) s;

    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
	err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
	buffer += DESCRIPTOR_TAG_LEN_SIZE;
    PUT16(ipmpToolDescriptorId);
    PUT64(ipmpToolId->upperPart);
    PUT64(ipmpToolId->lowerPart);
    if ( self->ipmpToolId->upperPart == 0 
         && self->ipmpToolId->lowerPart == 0 )
    {
        if ( self->url == NULL )
            BAILWITHERROR( MP4BadDataErr );
        PUT32_V(self->url->size);
        PUTBYTES(self->url->data, self->url->size);
    }
    else
    {
        PUT8_V((self->initData != NULL)?0x80:0X00); /* isInitialize flag */
        if ( self->initData != NULL )
        {
            err = self->initData->serialize( self->initData, buffer ); if (err) goto bail;
            buffer += self->initData->bytesWritten;
            self->bytesWritten += self->initData->bytesWritten;
        }
        err = MP4GetListEntryCount( self->ipmpxData, &ipmpxDataCount ); if (err) goto bail;
        PUT8_V(ipmpxDataCount);
        for ( i = 0; i < ipmpxDataCount; i++ )
        {
            MP4IPMPXDataPtr entry;
            err = MP4GetListEntry( self->ipmpxData,
                                   i,
                                   (char **) &entry ); if (err) goto bail;
            entry->serialize( entry, buffer );
            self->bytesWritten += entry->bytesWritten;
            buffer += entry->bytesWritten;
        }
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
    u32 i;
    u32 ipmpxDataCount;
    u8 initFlag;
    MP4IPMPToolDescriptorPtr self = (MP4IPMPToolDescriptorPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    GET16(ipmpToolDescriptorId);
    GET64(ipmpToolId->upperPart);
    GET64(ipmpToolId->lowerPart);
    if ( self->ipmpToolId->upperPart == 0 
         && self->ipmpToolId->lowerPart == 0 )
    {
        self->url = (MP4IPMPByteArrayPtr) calloc( 1, sizeof(MP4IPMPByteArray) );
        TESTMALLOC( self->url );
        GET32(url->size);
        self->url->data = (char *) calloc ( 1, self->url->size );
        TESTMALLOC( self->url->data );
        GETBYTES(self->url->size, url->data);
    }
    else
    {
        GET8_V(initFlag);
        if ( initFlag & 0x80 )
        {
            err = MP4CreateIPMPInitialize( &self->initData ); if (err) goto bail;
            err = self->initData->createFromInputStream( self->initData,
                                                         inputStream ); if (err) goto bail;
            self->bytesRead += self->initData->bytesRead;
        }
        GET8_V(ipmpxDataCount);
        for ( i = 0; i < ipmpxDataCount; i++ )
        {
            MP4IPMPXDataPtr data;
            err = MP4ParseIPMPXData( inputStream, &data ); if (err) goto bail;
            self->bytesRead += data->size;
            err = addIPMPXData( s, data ); if (err) goto bail;
        }
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
    u32 i;
    u32 ipmpxDataCount;
    MP4IPMPToolDescriptorPtr self = (MP4IPMPToolDescriptorPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    if ( self->ipmpToolId )
        free( self->ipmpToolId );
    if ( self->url )
    {
        if ( self->url->data )
            free( self->url->data );
        free( self->url );
    }
    if ( self->initData )
    {
        self->initData->destroy( self->initData );
        free( self->initData );
    }
    err = MP4GetListEntryCount( self->ipmpxData, &ipmpxDataCount ); if (err) goto bail;
    for ( i = 0; i < ipmpxDataCount; i++ )
    {
        MP4IPMPXDataPtr entry;
        err = MP4GetListEntry( self->ipmpxData,
                               i,
                               (char **) &entry ); if (err) goto bail;
        if ( entry )
            entry->destroy( entry );
    }
    err = MP4DeleteLinkedList( self->ipmpxData );
    free( s );

bail:
    return;
}

MP4Err MP4CreateIPMPToolDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
    SETUP_BASE_DESCRIPTOR( MP4IPMPToolDescriptor );
    err = MP4MakeLinkedList( &self->ipmpxData ); if (err) goto bail;
    self->ipmpToolId = (MP4IPMPToolIdPtr) calloc( 1, sizeof(MP4IPMPToolId) );
    TESTMALLOC(self->ipmpToolId);
    self->addIPMPXData = addIPMPXData;
    *outDesc = (MP4DescriptorPtr) self;

bail:
    TEST_RETURN( err );

    return err;
}
