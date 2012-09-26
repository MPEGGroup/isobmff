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
/*
	$Id: SLConfigDescriptor.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Descriptors.h"
#include "MP4Movies.h"
#include <stdlib.h>

static MP4Err calculateSize( struct MP4DescriptorRecord* s )
{
	MP4Err err;
	MP4SLConfigDescriptorPtr self = (MP4SLConfigDescriptorPtr) s;
	err = MP4NoErr;
	if ( s == 0 )
			BAILWITHERROR( MP4BadParamErr );
	self->size = DESCRIPTOR_TAG_LEN_SIZE; /* tag + length word */
	self->size += 1; /* predefined */
	if ( self->predefined == 0 )
	{
		self->size += 15;
	}
	if ( self->durationFlag )
		self->size += 8;
	if ( self->useTimestampsFlag == 0 )
	{
		u32 tslen = self->timestampLength * 2;
		u32 tsbytes = tslen / 8;
		if ( tslen & 0x7F )
			tsbytes += 1;
		self->size += tsbytes;
	}	

bail:
	TEST_RETURN( err );

	return err;
}

#define MAKEFLAG( member, bit ) \
		if ( self->member ) \
			flags |= (1 << (bit))

static MP4Err serialize( struct MP4DescriptorRecord* s, char* buffer )
{
	u32 flags;
	MP4Err err;

	MP4SLConfigDescriptorPtr self = (MP4SLConfigDescriptorPtr) s;
	err = MP4NoErr;
	err = MP4EncodeBaseDescriptor( s, buffer ); if (err) goto bail;
	buffer += DESCRIPTOR_TAG_LEN_SIZE;

	PUT8( predefined );
	if ( self->predefined == 0 )
	{
		u32 lengths;
		flags = 0;
		MAKEFLAG( useAccessUnitStartFlag,       7 );
		MAKEFLAG( useAccessUnitEndFlag,         6 );
		MAKEFLAG( useRandomAccessPointFlag,     5 );
		MAKEFLAG( useRandomAccessUnitsOnlyFlag, 4 );
		MAKEFLAG( usePaddingFlag,               3 );
		MAKEFLAG( useTimestampsFlag,            2 );
		MAKEFLAG( useIdleFlag,                  1 );
		MAKEFLAG( durationFlag,                 0 );
		PUT8_V( flags );
		PUT32( timestampResolution );
		PUT32( OCRResolution );
		PUT8( timestampLength );
		PUT8( OCRLength );
		PUT8( AULength );
		PUT8( instantBitrateLength );
		lengths = self->degradationPriorityLength << 12;
		lengths |= self->AUSeqNumLength << 7;
		lengths |= self->packetSeqNumLength << 2;
		lengths |= 3;
		PUT16_V( lengths );
	}
	if ( self->durationFlag )
	{
		PUT32( timeScale );
		PUT16( AUDuration );
		PUT16( CUDuration );
	}
	if ( self->useTimestampsFlag == 0 )
	{
		BAILWITHERROR( MP4NotImplementedErr );
	}

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err handlePredefined( MP4SLConfigDescriptorPtr self )
{
	MP4Err err;
	
	err = MP4NoErr;
	
	switch ( self->predefined )
	{
		case SLConfigPredefinedMP4:
			self->useTimestampsFlag = 1;
			break;
		
		default:
			BAILWITHERROR( MP4InvalidMediaErr )
	}

bail:
	TEST_RETURN( err );

	return err;
}

#define SETFLAG( member, bit ) \
		self->member = flags & (1 << (bit)) ? 1 : 0

static MP4Err createFromInputStream( struct MP4DescriptorRecord* s, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32  flags;
	MP4SLConfigDescriptorPtr self = (MP4SLConfigDescriptorPtr) s;
	err = MP4NoErr;

	GET8( predefined );
	if ( self->predefined )
	{
		err = handlePredefined( self ); if (err) goto bail;
	}
	else
	{
		
		u32 lengths;
		
		GET8_V_MSG( flags, NULL );
		SETFLAG( useAccessUnitStartFlag,       7 );
		SETFLAG( useAccessUnitEndFlag,         6 );
		SETFLAG( useRandomAccessPointFlag,     5 );
		SETFLAG( useRandomAccessUnitsOnlyFlag, 4 );
		SETFLAG( usePaddingFlag,               3 );
		SETFLAG( useTimestampsFlag,            2 );
		SETFLAG( useIdleFlag,                  1 );
		SETFLAG( durationFlag,                 0 );
		
		DEBUG_SPRINTF( "useAccessUnitStartFlag = %d",       self->useAccessUnitStartFlag );
		DEBUG_SPRINTF( "useAccessUnitEndFlag = %d",         self->useAccessUnitEndFlag );
		DEBUG_SPRINTF( "useRandomAccessPointFlag = %d",     self->useRandomAccessPointFlag );
		DEBUG_SPRINTF( "useRandomAccessUnitsOnlyFlag = %d", self->useRandomAccessUnitsOnlyFlag );
		DEBUG_SPRINTF( "usePaddingFlag = %d",               self->usePaddingFlag );
		DEBUG_SPRINTF( "useTimestampsFlag = %d",            self->useTimestampsFlag );
		DEBUG_SPRINTF( "useIdleFlag = %d",                  self->useIdleFlag );
		DEBUG_SPRINTF( "durationFlag = %d",                 self->durationFlag );
		
		GET32( timestampResolution );
		GET32( OCRResolution );
		GET8( timestampLength );
		GET8( OCRLength );
		GET8( AULength );
		GET8( instantBitrateLength );
		GET16_V_MSG( lengths, NULL );
		self->degradationPriorityLength = lengths >> 12;
		self->AUSeqNumLength = (lengths >> 7) & 0x1F;
		self->packetSeqNumLength = (lengths >> 2) & 0x1F;
		
		DEBUG_SPRINTF( "degradationPriorityLength = %d", self->degradationPriorityLength );
		DEBUG_SPRINTF( "AUSeqNumLength = %d",            self->AUSeqNumLength );
		DEBUG_SPRINTF( "packetSeqNumLength = %d",        self->packetSeqNumLength );
	}
		
	if ( self->durationFlag )
	{
		GET32( timeScale );
		GET16( AUDuration );
		GET16( CUDuration );
	}
	if ( self->useTimestampsFlag == 0 )
	{
		u32 timestampLength;
		char timestamps[ 1024 ];
		timestampLength = 2 * self->timestampLength;
		if ( timestampLength & 0x7 )
			timestampLength = 1 + timestampLength / 8;
		else
			timestampLength /= 8;
		if ( timestampLength > 1024 )
		{
			BAILWITHERROR( MP4NotImplementedErr );
		}
		else
		{
			GETBYTES_V_MSG( timestampLength, timestamps, "timestamps+pad" );
		}
	}
	/* for compatibility with systems V1, where there is an extra 0x7F  */
	if (self->bytesRead==(self->size - 1)) { u32 junk; GET8_V_MSG( junk, NULL ); }

bail:
	TEST_RETURN( err );

	return err;
}
	
static void destroy( struct MP4DescriptorRecord* s )
{
	MP4SLConfigDescriptorPtr self = (MP4SLConfigDescriptorPtr) s;
	DESTROY_DESCRIPTOR_LIST( extensionDescriptors )

	free( s );	
bail:
	return;
}

MP4Err MP4CreateSLConfigDescriptor( u32 tag, u32 size, u32 bytesRead, MP4DescriptorPtr *outDesc )
{
	SETUP_BASE_DESCRIPTOR( MP4SLConfigDescriptor )
	*outDesc = (MP4DescriptorPtr) self;
bail:
	TEST_RETURN( err );

	return err;
}

MP4_EXTERN ( MP4Err )
MP4NewSLConfig( MP4SLConfigSettingsPtr settings, MP4SLConfig *outSLConfig )
{
    MP4Err err;
	MP4DescriptorPtr desc;
    if ( (settings == 0) || (outSLConfig == 0) )
    {
        BAILWITHERROR( MP4BadParamErr );
    }
	err = MP4CreateSLConfigDescriptor( MP4SLConfigDescriptorTag, 0, 0, &desc ); if (err) goto bail;
	err = MP4SetSLConfigSettings( (MP4SLConfig) desc, settings ); if (err) goto bail;
	*outSLConfig = (MP4SLConfig) desc;
bail:
	TEST_RETURN( err );
	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetSLConfigSettings( MP4SLConfig config, MP4SLConfigSettingsPtr outSettings  )
{
    MP4Err err = MP4NoErr;
	MP4SLConfigDescriptorPtr slconfig = (MP4SLConfigDescriptorPtr) config;    
    if ( (outSettings == 0) || (config == 0) )
    {
        BAILWITHERROR( MP4BadParamErr );
    }
    outSettings->predefined                   = slconfig->predefined;
    outSettings->useAccessUnitStartFlag       = slconfig->useAccessUnitStartFlag;
    outSettings->useAccessUnitEndFlag         = slconfig->useAccessUnitEndFlag;
    outSettings->useRandomAccessPointFlag     = slconfig->useRandomAccessPointFlag;
    outSettings->useRandomAccessUnitsOnlyFlag = slconfig->useRandomAccessUnitsOnlyFlag;
    outSettings->usePaddingFlag               = slconfig->usePaddingFlag;
    outSettings->useTimestampsFlag            = slconfig->useTimestampsFlag;
    outSettings->useIdleFlag                  = slconfig->useIdleFlag;
    outSettings->durationFlag                 = slconfig->durationFlag;
    outSettings->timestampResolution          = slconfig->timestampResolution;
    outSettings->OCRResolution                = slconfig->OCRResolution;
    outSettings->timestampLength              = slconfig->timestampLength;
    outSettings->OCRLength                    = slconfig->OCRLength;
    outSettings->AULength                     = slconfig->AULength;
    outSettings->instantBitrateLength         = slconfig->instantBitrateLength;
    outSettings->degradationPriorityLength    = slconfig->degradationPriorityLength;
    outSettings->AUSeqNumLength               = slconfig->AUSeqNumLength;
    outSettings->packetSeqNumLength           = slconfig->packetSeqNumLength;
    outSettings->timeScale                    = slconfig->timeScale;
    outSettings->AUDuration                   = slconfig->AUDuration;
    outSettings->CUDuration                   = slconfig->CUDuration;
    outSettings->startDTS                     = slconfig->startDTS;
    outSettings->startCTS                     = slconfig->startCTS;
bail:
	TEST_RETURN( err );
	return err;
}

MP4_EXTERN ( MP4Err )
MP4SetSLConfigSettings( MP4SLConfig config, MP4SLConfigSettingsPtr settings  )
{
    MP4Err err = MP4NoErr;
	MP4SLConfigDescriptorPtr slconfig = (MP4SLConfigDescriptorPtr) config;    
    if ( (settings == 0) || (config == 0) )
    {
        BAILWITHERROR( MP4BadParamErr );
    }
    slconfig->predefined                   = settings->predefined;
    slconfig->useAccessUnitStartFlag       = settings->useAccessUnitStartFlag;
    slconfig->useAccessUnitEndFlag         = settings->useAccessUnitEndFlag;
    slconfig->useRandomAccessPointFlag     = settings->useRandomAccessPointFlag;
    slconfig->useRandomAccessUnitsOnlyFlag = settings->useRandomAccessUnitsOnlyFlag;
    slconfig->usePaddingFlag               = settings->usePaddingFlag;
    slconfig->useTimestampsFlag            = settings->useTimestampsFlag;
    slconfig->useIdleFlag                  = settings->useIdleFlag;
    slconfig->durationFlag                 = settings->durationFlag;
    slconfig->timestampResolution          = settings->timestampResolution;
    slconfig->OCRResolution                = settings->OCRResolution;
    slconfig->timestampLength              = settings->timestampLength;
    slconfig->OCRLength                    = settings->OCRLength;
    slconfig->AULength                     = settings->AULength;
    slconfig->instantBitrateLength         = settings->instantBitrateLength;
    slconfig->degradationPriorityLength    = settings->degradationPriorityLength;
    slconfig->AUSeqNumLength               = settings->AUSeqNumLength;
    slconfig->packetSeqNumLength           = settings->packetSeqNumLength;
    slconfig->timeScale                    = settings->timeScale;
    slconfig->AUDuration                   = settings->AUDuration;
    slconfig->CUDuration                   = settings->CUDuration;
    slconfig->startDTS                     = settings->startDTS;
    slconfig->startCTS                     = settings->startCTS;
bail:
	TEST_RETURN( err );
	return err;
}

