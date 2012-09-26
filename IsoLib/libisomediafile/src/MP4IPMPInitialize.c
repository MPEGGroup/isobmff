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

static MP4Err addIPMPXData( MP4IPMPInitializePtr self, MP4IPMPXDataPtr data )
{
    MP4Err err;

    err = MP4NoErr;
    err = MP4AddListEntry( data, self->ipmpxData ); if (err) goto bail;
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err calculateSize( MP4IPMPInitializePtr self )
{
    MP4Err err;
    u32 i;
    u32 ipmpxDataCount;
    err = MP4NoErr;

    self->size = 3; /* controlPointCode + sequence code + numData */
    
    /* calculate the size of the IPMPXData */
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
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err serialize( MP4IPMPInitializePtr self, char* buffer )
{
    MP4Err err;
    u32 ipmpxDataCount;
    u32 i;

    err = MP4NoErr;
    PUT8(controlPointCode);
    PUT8(sequenceCode);
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
bail:
    TEST_RETURN( err );

    return err;
}

static MP4Err createFromInputStream( MP4IPMPInitializePtr self, MP4InputStreamPtr inputStream )
{
    MP4Err err;
    u32 ipmpxDataCount;
    u32 i;
    
    err = MP4NoErr;
    GET8(controlPointCode);
    GET8(sequenceCode);
    GET8_V(ipmpxDataCount);
    for ( i = 0; i < ipmpxDataCount; i++ )
    {
        MP4IPMPXDataPtr data;
        err = MP4ParseIPMPXData( inputStream, &data ); if (err) goto bail;
        self->bytesRead += data->size;
        err = addIPMPXData( self, data ); if (err) goto bail;
    }


bail:
    TEST_RETURN( err );

    return err;
}

static void destroy( MP4IPMPInitializePtr self )
{
    MP4Err err;
    u32 i;
    u32 ipmpxDataCount;
    
    err = MP4NoErr;
    err = MP4GetListEntryCount( self->ipmpxData, &ipmpxDataCount ); if (err) goto bail;
    for ( i = 0; i < ipmpxDataCount; i++ )
    {
        MP4IPMPXDataPtr data;
        err = MP4GetListEntry( self->ipmpxData,
                               i,
                               (char **) &data ); if (err) goto bail;
        if ( data )
            data->destroy( data );
    }
    free( self );

bail:
    return;
}

MP4Err MP4CreateIPMPInitialize( MP4IPMPInitializePtr *outIpmpInit )
{
    MP4Err err;
    MP4IPMPInitializePtr self;
    err = MP4NoErr;
    self = (MP4IPMPInitializePtr) calloc( 1, sizeof(MP4IPMPInitialize) );
    TESTMALLOC(self);
    self->name = "MP4IPMPInitialize";
    err = MP4MakeLinkedList( &self->ipmpxData ); if (err) goto bail;
    self->calculateSize = calculateSize;
    self->serialize = serialize;
    self->createFromInputStream = createFromInputStream;
    self->destroy = destroy;
    self->addIPMPXData = addIPMPXData;
    *outIpmpInit = self;
bail:
    TEST_RETURN( err );

    return err;
}




