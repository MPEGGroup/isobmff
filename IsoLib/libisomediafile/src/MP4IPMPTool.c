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

static MP4Err calculateSize( struct MP4DescriptorRecord* s )
{
	MP4Err err;
    u32 numAlternates;
    u32 numUrls;
    u32 i;
	MP4IPMPToolPtr self = (MP4IPMPToolPtr) s;
	
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
	self->size = DESCRIPTOR_TAG_LEN_SIZE; /* tag + length word */ 
	self->size += 17; /* IPMPToolId + Flags */
    err = MP4GetListEntryCount( self->alternateToolIds, 
                                &numAlternates ); if (err) goto bail;
    if ( numAlternates != 0 ) 
    {
        self->size += 1; /* numAlternates */
        self->size += 16 * numAlternates; /* alternateToolIds */
    }

    if ( self->toolParamDesc !=  NULL)
    {
        self->size += 4; /* size of the size */
        self->size += self->toolParamDesc->size; /* size of the data */
    }

    err = MP4GetListEntryCount( self->toolUrls, 
                                &numUrls ); if (err) goto bail;
    self->size += 1; /* numUrls */
    for ( i = 0; i < numUrls; i++ )
    {
        MP4IPMPByteArrayPtr entry;
        
        err = MP4GetListEntry( self->toolUrls, 
                               i, 
                               (char **) &entry ); if (err) goto bail;
        self->size += entry->size + 4; /* data + dataSize */
    }
    
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
	MP4Err err;
    u32 numUrls;
    u32 numAlternates;
    u8 flags;
    u32 i;

    MP4IPMPToolPtr self = (MP4IPMPToolPtr) s;

    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
    buffer += DESCRIPTOR_TAG_LEN_SIZE;

    PUT64_V(self->toolId->upperPart);
    PUT64_V(self->toolId->lowerPart);

    err = MP4GetListEntryCount( self->alternateToolIds, 
                                &numAlternates ); if (err) goto bail;
    flags = ((numAlternates > 0)?1:0) * 0x80
            + ((self->toolParamDesc != NULL)?1:0) * 0x40;
    PUT8_V(flags);

    if ( numAlternates > 0 ) 
    {
        PUT8_V(numAlternates);
        for ( i = 0; i < numAlternates; i++ ) 
        {
            MP4IPMPToolIdPtr entry;
            err = MP4GetListEntry( self->alternateToolIds, 
                                   i, 
                                   (char **) &entry ); if (err) goto bail;
            PUT64_V(entry->upperPart);
            PUT64_V(entry->lowerPart);
        }
    }

    if ( self->toolParamDesc != NULL )
    {
        PUT32_V(self->toolParamDesc->size);
        PUTBYTES(self->toolParamDesc->data, self->toolParamDesc->size);
    }

    err = MP4GetListEntryCount( self->toolUrls,
                                &numUrls ); if (err) goto bail;
    
    PUT8_V(numUrls);
    for ( i = 0; i < numUrls; i++ )
    {
        MP4IPMPByteArrayPtr entry;
        err = MP4GetListEntry( self->toolUrls,
                               i,
                               (char **) &entry ); if (err) goto bail;
        PUT32_V(entry->size);
        PUTBYTES(entry->data, entry->size);
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
    u8 flags;
    u32 i;
    u32 numUrls;

	MP4IPMPToolPtr self = (MP4IPMPToolPtr) s;
	err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    GET64_V(self->toolId->upperPart);
    GET64_V(self->toolId->lowerPart);
    GET8_V(flags);

    if (flags & 0x80) /* isAltGroup */
    {
        u32 numAlternates;
        GET8_V(numAlternates);
        
        for ( i = 0; i < numAlternates; i++ )
        {
            MP4IPMPToolIdPtr id;

            id = (MP4IPMPToolIdPtr) calloc( 1, sizeof(MP4IPMPToolId) );
            TESTMALLOC(id);
            GET64_V(id->upperPart);
            GET64_V(id->lowerPart);
            err = MP4AddListEntry( id, 
                                   self->alternateToolIds ); if (err) goto bail;
        }
    }

    if ( flags & 0x40 ) /* isParametric */
    {
        self->toolParamDesc = (MP4IPMPByteArrayPtr) calloc( 1, sizeof(MP4IPMPByteArray));
        TESTMALLOC( self->toolParamDesc );

        GET32(toolParamDesc->size);
        self->toolParamDesc->data = (char *)calloc( 1, self->toolParamDesc->size );
        TESTMALLOC(self->toolParamDesc->data);
        GETBYTES(self->toolParamDesc->size, toolParamDesc->data);
    }

    GET8_V(numUrls);
    for ( i = 0; i < numUrls; i++ ) 
    {
        MP4IPMPByteArrayPtr entry;
        entry = (MP4IPMPByteArrayPtr) calloc( 1, sizeof(MP4IPMPByteArray));
        TESTMALLOC(entry);
        GET32_V(entry->size);
        entry->data = (char *) calloc( 1, entry->size );
        TESTMALLOC(entry->data);
        GETBYTES_V(entry->size, entry->data);
        err = MP4AddListEntry( entry, self->toolUrls ); if (err) goto bail;
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
    u32 numUrls;
    u32 numAlternates;
    u32 i;

	MP4IPMPToolPtr self = (MP4IPMPToolPtr) s;
    err = MP4NoErr;
    if ( s == NULL )
        BAILWITHERROR( MP4BadParamErr );
    if ( self->toolId )
        free( self->toolId );

    err = MP4GetListEntryCount( self->alternateToolIds, 
                                &numAlternates ); if (err) goto bail;
    for ( i = 0; i < numAlternates; i++ )
    {
        MP4IPMPToolIdPtr entry;
        err = MP4GetListEntry( self->alternateToolIds,
                               i,
                               (char **) &entry ); if (err) goto bail;
        if ( entry ) 
            free( entry );
    }
    err = MP4DeleteLinkedList( self->alternateToolIds ); if (err) goto bail;

    if ( self->toolParamDesc )
    {
        if ( self->toolParamDesc->data )
            free( self->toolParamDesc->data );
        free ( self->toolParamDesc );
    }

    err = MP4GetListEntryCount( self->toolUrls, &numUrls ); if (err) goto bail;
    for ( i = 0; i < numUrls; i++ )
    {
        MP4IPMPByteArrayPtr entry;
        err = MP4GetListEntry( self->toolUrls,
                               i,
                               (char **) &entry ); if (err) goto bail;
        if ( entry ) 
        {
            if ( entry->data )
                free( entry->data );
            free( entry );
        }
    }
    err = MP4DeleteLinkedList( self->toolUrls ); if (err) goto bail;
 
	free( s );

bail:
    return;
}

MP4Err MP4CreateIPMPTool( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
	SETUP_BASE_DESCRIPTOR( MP4IPMPTool )
    
    self->toolId = (MP4IPMPToolIdPtr) calloc( 1, sizeof(MP4IPMPToolId) );
    TESTMALLOC(self->toolId);
    
    err = MP4MakeLinkedList( &self->alternateToolIds ); if (err) goto bail;    
    err = MP4MakeLinkedList( &self->toolUrls ); if (err) goto bail;
	
    *outDesc = (MP4DescriptorPtr) self;
bail:
	TEST_RETURN( err );

	return err;
}
