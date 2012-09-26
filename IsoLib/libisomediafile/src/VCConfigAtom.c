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
	$Id: VCConfigAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include "MP4Descriptors.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	ISOVCConfigAtomPtr self;
	err = MP4NoErr;
	self = (ISOVCConfigAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	if ( self->spsList )
	{
		err = MP4DeleteLinkedList( self->spsList ); if (err) goto bail;
		self->spsList = NULL;
	}
	if ( self->ppsList )
	{
		err = MP4DeleteLinkedList( self->ppsList ); if (err) goto bail;
		self->ppsList = NULL;
	}
	if ( self->spsextList )
	{
		err = MP4DeleteLinkedList( self->spsextList ); if (err) goto bail;
		self->spsextList = NULL;
	}
	
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 x, i;
	ISOVCConfigAtomPtr self = (ISOVCConfigAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( (MP4AtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	x = 1;
	PUT8_V( x );			
		/* config_version */
	PUT8( profile );
	PUT8( profile_compatibility);
	PUT8( level );
	x = (self->complete_rep ? 0x80 : 0) | 0x7C | ((self->length_size - 1) & 3);
	PUT8_V( x );

	if (self->spsList) {
		u32 count;
		err = MP4GetListEntryCount( self->spsList, &count ); if (err) goto bail;
		if (count>>5) BAILWITHERROR( MP4BadParamErr );
		x = 0xE0 | (count & 0x1F);
		PUT8_V( x );

		for (i=0; i<count; i++) {
			MP4Handle b;
			u32 the_size;
			err = MP4GetListEntry( self->spsList, i, (char **) &b ); if (err) goto bail;
			err = MP4GetHandleSize( b, &the_size );  if (err) goto bail;
			PUT16_V( the_size );
			PUTBYTES( *b, the_size );
		}
	} else { x = 0xE0; PUT8_V( x ); }
		
	if (self->ppsList) {
		u32 count;
		err = MP4GetListEntryCount( self->ppsList, &count ); if (err) goto bail;
		PUT8_V( count );

		for (i=0; i<count; i++) {
			MP4Handle b;
			u32 the_size;
			err = MP4GetListEntry( self->ppsList, i, (char **) &b ); if (err) goto bail;
			err = MP4GetHandleSize( b, &the_size );  if (err) goto bail;
			PUT16_V( the_size );
			PUTBYTES( *b, the_size );
		}
	} else { x = 0; PUT8_V( x ); }
	
	if( self->profile  ==  100  ||  self->profile  ==  110  ||
	    self->profile  ==  122  ||  self->profile  ==  144 )
	{
		x = 0xFC | ((self->chroma_format)           & 3); PUT8_V( x );
		x = 0xF8 | ((self->bit_depth_luma_minus8)   & 7); PUT8_V( x );
		x = 0xF8 | ((self->bit_depth_chroma_minus8) & 7); PUT8_V( x );

		if (self->spsextList) {
			u32 count;
			err = MP4GetListEntryCount( self->spsextList, &count ); if (err) goto bail;
			PUT8_V( count );

			for (i=0; i<count; i++) {
				MP4Handle b;
				u32 the_size;
				err = MP4GetListEntry( self->spsextList, i, (char **) &b ); if (err) goto bail;
				err = MP4GetHandleSize( b, &the_size );  if (err) goto bail;
				PUT16_V( the_size );
				PUTBYTES( *b, the_size );
			}
		} else { x = 0; PUT8_V( x ); }
	}	

	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	ISOVCConfigAtomPtr self = (ISOVCConfigAtomPtr) s;
	u32 i;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( (MP4AtomPtr) s ); if (err) goto bail;
	self->size += 7;
	
	if (self->spsList) {
		u32 count;
		err = MP4GetListEntryCount( self->spsList, &count ); if (err) goto bail;
		if (count>>5) BAILWITHERROR( MP4BadParamErr );

		for (i=0; i<count; i++) {
			MP4Handle b;
			u32 the_size;
			err = MP4GetListEntry( self->spsList, i, (char **) &b ); if (err) goto bail;
			err = MP4GetHandleSize( b, &the_size );  if (err) goto bail;
			self->size += 2 + the_size;
		}
	}
	if (self->ppsList) {
		u32 count;
		err = MP4GetListEntryCount( self->ppsList, &count ); if (err) goto bail;
		if (count>>5) BAILWITHERROR( MP4BadParamErr );

		for (i=0; i<count; i++) {
			MP4Handle b;
			u32 the_size;
			err = MP4GetListEntry( self->ppsList, i, (char **) &b ); if (err) goto bail;
			err = MP4GetHandleSize( b, &the_size );  if (err) goto bail;
			self->size += 2 + the_size;
		}
	}

	if( self->profile  ==  100  ||  self->profile  ==  110  ||
	    self->profile  ==  122  ||  self->profile  ==  144 )
	{
		self->size += 4;

		if (self->spsextList) {
			u32 count;
			err = MP4GetListEntryCount( self->spsextList, &count ); if (err) goto bail;

			for (i=0; i<count; i++) {
				MP4Handle b;
				u32 the_size;
				err = MP4GetListEntry( self->spsextList, i, (char **) &b ); if (err) goto bail;
				err = MP4GetHandleSize( b, &the_size );  if (err) goto bail;
				self->size += 2 + the_size;
			}
		} 
	}	

bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	ISOVCConfigAtomPtr self = (ISOVCConfigAtomPtr) s;
	u32 x, count, i;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GET8_V( x );			/* config_version */
	if (x!=1) BAILWITHERROR( MP4BadDataErr );
	
	GET8( profile );
	GET8( profile_compatibility);
	GET8( level );
	GET8_V( x );
	self->complete_rep = (x & 0x80 ? 1 : 0);
	self->length_size = (x & 3) + 1;
	
	GET8_V( count ); count = count & 0x1F;	
	for (i=0; i<count; i++) {
		MP4Handle b;
		u32 the_size;
		GET16_V( the_size );
		err = MP4NewHandle( the_size, &b ); if (err) goto bail;
		
		GETBYTES_V_MSG( the_size, *b, "sps" );
		err = MP4AddListEntry( (void*) b, self->spsList ); if (err) goto bail;
	}
	GET8_V( count );	
	for (i=0; i<count; i++) {
		MP4Handle b;
		u32 the_size;
		GET16_V( the_size );
		err = MP4NewHandle( the_size, &b ); if (err) goto bail;
		
		GETBYTES_V_MSG( the_size, *b, "pps" );
		err = MP4AddListEntry( (void*) b, self->ppsList ); if (err) goto bail;
	}
	
	if( self->profile  ==  100  ||  self->profile  ==  110  ||
	    self->profile  ==  122  ||  self->profile  ==  144 )
	{
		GET8_V( x ); self->chroma_format           = x & 3;
		GET8_V( x ); self->bit_depth_luma_minus8   = x & 7;
		GET8_V( x ); self->bit_depth_chroma_minus8 = x & 7;

		GET8_V( count );	
		for (i=0; i<count; i++) {
			MP4Handle b;
			u32 the_size;
			GET16_V( the_size );
			err = MP4NewHandle( the_size, &b ); if (err) goto bail;
			
			GETBYTES_V_MSG( the_size, *b, "spsext" );
			err = MP4AddListEntry( (void*) b, self->spsextList ); if (err) goto bail;
		}
 
	}	
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addParameterSet( struct ISOVCConfigAtom *self, MP4Handle ps, u32 where )
{
	MP4Err err;
	MP4Handle b;
	u32 the_size;
	
	err = MP4NoErr;
	err = MP4GetHandleSize( ps, &the_size );  if (err) goto bail;
	err = MP4NewHandle( the_size, &b ); if (err) goto bail;
	memcpy( *b, *ps, the_size );
	
	switch (where & 3) {
		case AVCsps:
			err = MP4AddListEntry( (void*) b, self->spsList ); if (err) goto bail;
			break;
		case AVCpps:
			err = MP4AddListEntry( (void*) b, self->ppsList ); if (err) goto bail;
			break;
		case AVCspsext:
			err = MP4AddListEntry( (void*) b, self->spsextList ); if (err) goto bail;
			break;
		default:
			BAILWITHERROR( MP4BadParamErr );
	}
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getParameterSet( struct ISOVCConfigAtom *self, MP4Handle ps, u32 where, u32 index )
{
	MP4Err err;
	MP4Handle b;
	u32 the_size;
	
	err = MP4NoErr;
	
	switch (where & 3) {
		case AVCsps:
			err = MP4GetListEntry( self->spsList, index-1, (char**) &b  ); if (err) goto bail;
			break;
		case AVCpps:
			err = MP4GetListEntry( self->ppsList, index-1, (char**) &b  ); if (err) goto bail;
			break;
		case AVCspsext:
			err = MP4GetListEntry( self->spsextList, index-1, (char**) &b  ); if (err) goto bail;
			break;
		default:
			BAILWITHERROR( MP4BadParamErr );
	}
	err = MP4GetHandleSize( b, &the_size ); if (err) goto bail;
	err = MP4SetHandleSize( ps, the_size ); if (err) goto bail;
	memcpy( *ps, *b, the_size );
	
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateVCConfigAtom( ISOVCConfigAtomPtr *outAtom )
{
	MP4Err err;
	ISOVCConfigAtomPtr self;
	
	self = (ISOVCConfigAtomPtr) calloc( 1, sizeof(ISOVCConfigAtom) );
	TESTMALLOC( self );

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = ISOVCConfigAtomType;
	self->name                = "VCConfig";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->complete_rep			= 1;
	self->addParameterSet		= addParameterSet;
	self->getParameterSet		= getParameterSet;
	err = MP4MakeLinkedList( &self->spsList ); if (err) goto bail;
	err = MP4MakeLinkedList( &self->ppsList ); if (err) goto bail;
	err = MP4MakeLinkedList( &self->spsextList ); if (err) goto bail;
	
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
