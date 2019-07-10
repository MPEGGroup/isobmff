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
	$Id: CompactTrackFragmentRunAtom.c,v 1.1.1.1 2019/07/10 07:26:14 tiledmedia $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

/*  Return the size, in byte, associate with the given index value */
static u8 f(u8 index)
{
	switch(index) {
		case 0:
			return 0;
		case 1:
			return 1;
		case 2:
			return 2;
		default:
			return 4;
	}
}

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4CompactTrackRunAtomPtr self;

	self = (MP4CompactTrackRunAtomPtr) s;
	if ( self == NULL ) BAILWITHERROR( MP4BadParamErr )
	if ( self->sample_duration )
	{
		free( self->sample_duration );
		self->sample_duration = NULL;
	}
	if ( self->sample_size )
	{
		free( self->sample_size );
		self->sample_size = NULL;
	}
	if ( self->sample_flags )
	{
		free( self->sample_flags );
		self->sample_flags = NULL;
	}
	if ( self->sample_composition_time_offset )
	{
		free( self->sample_composition_time_offset );
		self->sample_composition_time_offset = NULL;
	}
	if ( self->super )
	{
		self->super->destroy( s );
	}
bail:
	TEST_RETURN( err );

	return;
}


static void setFlags (struct MP4CompactTrackRunAtom* self)
{
	MP4Err err;
	u32 i;
	ctrn_flags_ptr parsedFlags;
	err = MP4NoErr;


	self->flags = 0;
	parsedFlags = (ctrn_flags_ptr) &self->flags;
	
	if (self->data_offset != 0) {
		parsedFlags->data_offset_present = 1;
		if (self->data_offset < (1 << 16) ) {
			parsedFlags->data_offset_16 = 1;
		}
	}

	if (self->composition_multiplier != 0) {
		parsedFlags->composition_multiplier_present = 1;
	}

	if (self->sample_count > 0) {
		/* Set first sample flags */
		if ((self->first_sample_duration != 0) || (self->first_sample_size != 0) ||
			(self->first_sample_flags != 0) || (self->first_sample_composition_time_offset != 0)) {
			parsedFlags->first_sample_info_present = 1;
			if (self->first_sample_duration != 0) {
				if (self->first_sample_duration < (1 << 8)) {
					parsedFlags->first_duration_size_index = 1;
				} else if ( self->first_sample_duration < (1 << 16) ) {
					parsedFlags->first_duration_size_index = 2;
				} else {
					parsedFlags->first_duration_size_index = 3;
				}
			}
			if (self->first_sample_size != 0) {
				if (self->first_sample_size < (1 << 8)) {
					parsedFlags->first_sample_size_index = 1;
				} else if ( self->first_sample_size < (1 << 16) ) {
					parsedFlags->first_sample_size_index = 2;
				} else {
					parsedFlags->first_sample_size_index = 3;
				}
			}
			if (self->first_sample_flags != 0) {
				if (self->first_sample_flags < (1 << 8)) {
					parsedFlags->first_flags_size_index = 1;
				} else if ( self->first_sample_flags < (1 << 16) ) {
					parsedFlags->first_flags_size_index = 2;
				} else {
					parsedFlags->first_flags_size_index = 3;
				}
			}
			if (self->first_sample_composition_time_offset != 0) {
				if (self->first_sample_composition_time_offset < (1 << 8)) {
					parsedFlags->first_composition_size_index = 1;
				} else if ( self->first_sample_composition_time_offset < (1 << 16) ) {
					parsedFlags->first_composition_size_index = 2;
				} else {
					parsedFlags->first_composition_size_index = 3;
				}
			}
		}
		/* Set other sample flags */
		int nb_samples = self->sample_count - (parsedFlags->first_sample_info_present ? 1 : 0);
		for (i = 0; i < nb_samples; ++i) {
			if ( self->sample_duration[i] != 0 && parsedFlags->duration_size_index < 3 ) {
				if (self->sample_duration[i] < (1 << 8) && parsedFlags->duration_size_index < 1) {
					parsedFlags->duration_size_index = 1;
				} else if ( self->sample_duration[i] < (1 << 16) && parsedFlags->duration_size_index < 2) {
					parsedFlags->duration_size_index = 2;
				} else {
					parsedFlags->duration_size_index = 3;
				}
			}

			if ( self->sample_size[i] != 0 && parsedFlags->sample_size_index < 3 ) {
				if (self->sample_size[i] < (1 << 8) && parsedFlags->sample_size_index < 1) {
					parsedFlags->sample_size_index = 1;
				} else if ( self->sample_size[i] < (1 << 16) && parsedFlags->sample_size_index < 2) {
					parsedFlags->sample_size_index = 2;
				} else {
					parsedFlags->sample_size_index = 3;
				}
			}

			if ( self->sample_flags[i] != 0 && parsedFlags->flags_size_index < 3 ) {
				if (self->sample_flags[i] < (1 << 8) && parsedFlags->flags_size_index < 1) {
					parsedFlags->flags_size_index = 1;
				} else if ( self->sample_flags[i] < (1 << 16) && parsedFlags->flags_size_index < 2) {
					parsedFlags->flags_size_index = 2;
				} else {
					parsedFlags->flags_size_index = 3;
				}
			}

			if ( self->sample_composition_time_offset[i] != 0 && parsedFlags->composition_size_index < 3 ) {
				if (self->sample_composition_time_offset[i] < (1 << 8) && parsedFlags->composition_size_index < 1) {
					parsedFlags->composition_size_index = 1;
				} else if ( self->sample_composition_time_offset[i] < (1 << 16) && parsedFlags->composition_size_index < 2) {
					parsedFlags->composition_size_index = 2;
				} else {
					parsedFlags->composition_size_index = 3;
				}
			}
		}

		

	}
		
	return;

}

#define PUT_DYNAMIC(member, index) \
	switch(f(index)) {\
	case 0:\
		break;\
	case 1:\
		PUT8(member);\
		break;\
	case 2:\
		PUT16(member);\
		break;\
	case 4:\
		PUT32(member);\
		break;\
	}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;
	MP4CompactTrackRunAtomPtr self = (MP4CompactTrackRunAtomPtr) s;
	ctrn_flags_ptr parsedFlags;
	

	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
	
	parsedFlags = (ctrn_flags_ptr) &self->flags;
	PUT16( sample_count );

	if (parsedFlags->data_offset_present) {
		if (parsedFlags->data_offset_16) {
			/* Read 16bits but store value in 32bits variable */
			u16 data_offset_tmp = self->data_offset;
			PUT16_V( data_offset_tmp );
		} else {
			PUT32( data_offset );
		}
	}

	if (parsedFlags->composition_multiplier_present) {
		PUT16( composition_multiplier );
	}

	if (parsedFlags->first_sample_info_present) {
		PUT_DYNAMIC(first_sample_duration, parsedFlags->first_duration_size_index);
		PUT_DYNAMIC(first_sample_size, parsedFlags->first_sample_size_index);
		PUT_DYNAMIC(first_sample_flags, parsedFlags->first_flags_size_index);
		PUT_DYNAMIC(first_sample_composition_time_offset, parsedFlags->composition_size_index);
	}

	int array_size = self->sample_count - (parsedFlags->first_sample_info_present ? 1 : 0);

	/* read the sample_duration */
	for (i = 0; i < array_size; ++i) {
		PUT_DYNAMIC(sample_duration[i], parsedFlags->duration_size_index);
	}
	for (i = 0; i < self->sample_count; ++i) {
		PUT_DYNAMIC(sample_size[i], parsedFlags->sample_size_index);
	}
	for (i = 0; i < self->sample_count; ++i) {
		PUT_DYNAMIC(sample_flags[i], parsedFlags->flags_size_index);
	}
	for (i = 0; i < self->sample_count; ++i) {
		PUT_DYNAMIC(sample_composition_time_offset[i], parsedFlags->composition_size_index);
	}

	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	u32 entry_size;
	MP4CompactTrackRunAtomPtr self = (MP4CompactTrackRunAtomPtr) s;
	err = MP4NoErr;
	ctrn_flags_ptr parsedFlags;
	
	parsedFlags = (ctrn_flags_ptr) &self->flags;

	/* Compute the size of the Box header */
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	/* sample_count */
	self->size += 2;

	if (parsedFlags->data_offset_present) {
		if (parsedFlags->data_offset_16) {
			self->size += 2;
		} else {
			self->size += 4;
		}
	}

	if (parsedFlags->composition_multiplier_present) {
		self->size += 2;
	}

	if (parsedFlags->first_sample_info_present) {
		self->size += f(parsedFlags->first_duration_size_index);
		self->size += f(parsedFlags->first_sample_size_index);
		self->size += f(parsedFlags->first_flags_size_index);
		self->size += f(parsedFlags->first_composition_size_index);
	}

	int array_size = self->sample_count - (parsedFlags->first_sample_info_present ? 1 : 0);

	self->size += array_size * f(parsedFlags->duration_size_index);
	self->size += array_size * f(parsedFlags->sample_size_index);
	self->size += array_size * f(parsedFlags->flags_size_index);
	self->size += array_size * f(parsedFlags->composition_size_index);
	
bail:
	TEST_RETURN( err );

	return err;
}

#define GET_DYNAMIC( member, index) \
	self->member = 0;\
	switch(f(index)) {\
	case 0:\
		break;\
	case 1:\
		GET8(member);\
		break;\
	case 2:\
		GET16(member);\
		break;\
	case 4:\
		GET32(member);\
		break;\
	}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 i;
	MP4CompactTrackRunAtomPtr self = (MP4CompactTrackRunAtomPtr) s;
	ctrn_flags_ptr parsedFlags;
	

	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	parsedFlags = (ctrn_flags_ptr) &self->flags;
	GET16( sample_count );

	if (parsedFlags->data_offset_present) {
		if (parsedFlags->data_offset_16) {
			/* Read 16bits but store value in 32bits variable */
			u16 data_offset_tmp = 0;
			GET16_V( data_offset_tmp );
			self->data_offset = data_offset_tmp;
		} else {
			GET32( data_offset );
		}
	}

	if (parsedFlags->composition_multiplier_present) {
		GET16( composition_multiplier );
	}

	if (parsedFlags->first_sample_info_present) {
		GET_DYNAMIC(first_sample_duration, parsedFlags->first_duration_size_index);
		GET_DYNAMIC(first_sample_size, parsedFlags->first_sample_size_index);
		GET_DYNAMIC(first_sample_flags, parsedFlags->first_flags_size_index);
		GET_DYNAMIC(first_sample_composition_time_offset, parsedFlags->composition_size_index);
	}

	int array_size = self->sample_count - (parsedFlags->first_sample_info_present ? 1 : 0);

	/* read the sample_duration */
	for (i = 0; i < array_size; ++i) {
		GET_DYNAMIC(sample_duration[i], parsedFlags->duration_size_index);
	}
	for (i = 0; i < self->sample_count; ++i) {
		GET_DYNAMIC(sample_size[i], parsedFlags->sample_size_index);
	}
	for (i = 0; i < self->sample_count; ++i) {
		GET_DYNAMIC(sample_flags[i], parsedFlags->flags_size_index);
	}
	for (i = 0; i < self->sample_count; ++i) {
		GET_DYNAMIC(sample_composition_time_offset[i], parsedFlags->composition_size_index);
	}

bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateCompactTrackRunAtom( MP4CompactTrackRunAtomPtr *outAtom )
{
	MP4Err err;
	MP4CompactTrackRunAtomPtr self;
	
	
	self = (MP4CompactTrackRunAtomPtr) calloc( 1, sizeof(MP4CompactTrackRunAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4CompactTrackRunAtomType;
	self->name					= "compact track fragment run";
	self->flags					= 0;	
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy				= destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	
	self->setFlags 			= setFlags;
	
	*outAtom = self; 
bail:
	TEST_RETURN( err );

	return err;
}
