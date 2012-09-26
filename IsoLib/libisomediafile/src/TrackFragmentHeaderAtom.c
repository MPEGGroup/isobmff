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
	$Id: TrackFragmentHeaderAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
    err = MP4NoErr;
	if ( s == NULL )
       BAILWITHERROR( MP4BadParamErr )
	if ( s->super )
		s->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 flags;
	MP4TrackFragmentHeaderAtomPtr self = (MP4TrackFragmentHeaderAtomPtr) s;
	err = MP4NoErr;
	
	flags = self->flags;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    PUT32( trackID );
    if (flags & tfhd_base_data_offset_present)			
    	{ PUT64( base_data_offset ); };
    if (flags & tfhd_sample_description_index_present) 	
    	{ PUT32( sample_description_index ); };
    if (flags & tfhd_default_sample_duration_present) 	
    	{ PUT32( default_sample_duration ); };
    if (flags & tfhd_default_sample_size_present) 		
    	{ PUT32( default_sample_size ); };
    if (flags & tfhd_default_sample_flags_present) 		
    	{ PUT32( default_sample_flags ); };
    	
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	u32 flags;
	MP4TrackFragmentHeaderAtomPtr self = (MP4TrackFragmentHeaderAtomPtr) s;
	err = MP4NoErr;
	
	flags = self->flags;

	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += 4;
    if (flags & tfhd_base_data_offset_present) 			self->size += 8;
    if (flags & tfhd_sample_description_index_present) 	self->size += 4;
    if (flags & tfhd_default_sample_duration_present) 	self->size += 4;
    if (flags & tfhd_default_sample_size_present) 		self->size += 4;
    if (flags & tfhd_default_sample_flags_present) 		self->size += 4;
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 flags;
	MP4TrackFragmentHeaderAtomPtr self = (MP4TrackFragmentHeaderAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	flags = self->flags;
	GET32( trackID );

    if (flags & tfhd_base_data_offset_present)			{ GET64( base_data_offset ); };
    if (flags & tfhd_sample_description_index_present) 	{ GET32( sample_description_index ); };
    if (flags & tfhd_default_sample_duration_present) 	{ GET32( default_sample_duration ); };
    if (flags & tfhd_default_sample_size_present) 		{ GET32( default_sample_size ); };
    if (flags & tfhd_default_sample_flags_present) 		{ GET32( default_sample_flags ); };
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateTrackFragmentHeaderAtom( MP4TrackFragmentHeaderAtomPtr *outAtom )
{
	MP4Err err;
	MP4TrackFragmentHeaderAtomPtr self;
	
	self = (MP4TrackFragmentHeaderAtomPtr) calloc( 1, sizeof(MP4TrackFragmentHeaderAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4TrackFragmentHeaderAtomType;
	self->name					= "track fragment header";
	self->flags					= 0;	
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy				= destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
