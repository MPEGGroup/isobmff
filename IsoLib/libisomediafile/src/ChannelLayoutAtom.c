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
 derivative works. Copyright (c) 2014.
 */
/*
 $Id: ChannelLayoutAtom.c,v 1.1.1.1 2014/09/18 08:10:00 armin Exp $
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4ChannelLayoutAtomPtr self = (MP4ChannelLayoutAtomPtr) s;
    err = MP4NoErr;
    
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
        
        if ( self->super )
            self->super->destroy( s );
    
    while (self->definedLayouts->entryCount > 0)
    {
        MP4ChannelLayoutDefinedLayout *definedLayout;
        MP4GetListEntry(self->definedLayouts, 0, (char**) &definedLayout);
        free (definedLayout);
        MP4DeleteListEntry(self->definedLayouts, 0);
    }
    MP4DeleteLinkedList(self->definedLayouts);
    
bail:
	TEST_RETURN( err );
    
	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
    u16                             i;
	MP4Err                          err;
    u8                              tmp8;
    MP4ChannelLayoutDefinedLayout   *definedLayoutStruct;
	MP4ChannelLayoutAtomPtr         self = (MP4ChannelLayoutAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    
	PUT8(stream_structure);
    if (self->stream_structure & 1) /* channelStructured */
    {
        PUT8(definedLayout);
        
        if (self->definedLayout == 0)
        {
            for (i = 0; i < self->channelCount; i++)
            {
                MP4GetListEntry(self->definedLayouts, i, (char**) &definedLayoutStruct);
                if (definedLayoutStruct->explicit_position)
                {
                    tmp8 = (definedLayoutStruct->explicit_position << 7) + definedLayoutStruct->elevation;
                    PUT8_V(tmp8);
                    PUT8_V(definedLayoutStruct->azimuth);
                }
                else
                {
                    tmp8 = (definedLayoutStruct->explicit_position << 7) + definedLayoutStruct->speaker_position;
                    PUT8_V(tmp8);
                }
            }
        }
        else
        {
            PUT64(omittedChannelsMap);
        }
    }
    
    if (self->stream_structure & 2) /* objectStructured */
    {
        PUT8(object_count);
    }
    
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err  err;
    u16     i;
    MP4ChannelLayoutDefinedLayout   *definedLayoutStruct;
	MP4ChannelLayoutAtomPtr         self = (MP4ChannelLayoutAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	
    self->size += 1;
    if (self->stream_structure & 1) /* channelStructured */
    {
        self->size += 1;
        if (self->definedLayout == 0)
        {
            for (i = 0; i < self->channelCount; i++)
            {
                self->size += 1;
                MP4GetListEntry(self->definedLayouts, i, (char**) &definedLayoutStruct);
                if (definedLayoutStruct->explicit_position)
                {
                    self->size += 1;
                }
            }
        }
        else
        {
            self->size += 8;
        }
    }
    
    if (self->stream_structure & 2) /* objectStructured */
    {
        self->size += 1;
    }

    
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
    u32     tmp8;
	MP4Err  err;
    MP4ChannelLayoutDefinedLayout   *definedLayoutStruct;
	MP4ChannelLayoutAtomPtr         self = (MP4ChannelLayoutAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
        err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;


    GET8(stream_structure);
    if (self->stream_structure & 1) /* channelStructured */
    {
        GET8(definedLayout);
        
        if (self->definedLayout == 0)
        {
            while ( self->bytesRead < self->size)
            {
                MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
                definedLayoutStruct = calloc(1, sizeof(MP4ChannelLayoutDefinedLayout));

                GET8_V(tmp8);
                definedLayoutStruct->explicit_position = tmp8 >> 7;
                if (definedLayoutStruct->explicit_position)
                {
                    definedLayoutStruct->elevation = tmp8 & 0x7F;
                    GET8_V(definedLayoutStruct->azimuth);
                }
                else
                {
                    definedLayoutStruct->speaker_position = tmp8 & 0x7F;
                }
                MP4AddListEntry(definedLayoutStruct, self->definedLayouts);
            }
        }
        else
        {
            GET64(omittedChannelsMap);
        }
    }
    
    if (self->stream_structure & 2) /* objectStructured */
    {
        GET8(object_count);
    }

	assert( self->bytesRead == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateChannelLayoutAtom( MP4ChannelLayoutAtomPtr *outAtom )
{
	MP4Err err;
	MP4ChannelLayoutAtomPtr self;
	
	self = (MP4ChannelLayoutAtomPtr) calloc( 1, sizeof(MP4ChannelLayoutAtom) );
	TESTMALLOC( self );
    
	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type                  = MP4ChannelLayoutAtomType;
	self->name                  = "channel layout";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy               = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
    
    err = MP4MakeLinkedList(&self->definedLayouts); if ( err ) goto bail;
    
	*outAtom = self;
bail:
	TEST_RETURN( err );
    
	return err;
}
