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
	$Id: MP4Impl.h,v 1.3 2002/10/01 12:49:19 julien Exp $
*/
#ifndef INCLUDED_MP4IMPL_H
#define INCLUDED_MP4IMPL_H
#include <assert.h>
#include <stdio.h>
#ifdef __linux
#include <string.h>
#endif

#ifndef NULL
	#define NULL 0
#endif

#define TESTMALLOC(ptr) \
	if ( (ptr) == 0 ) \
	{ \
		err = MP4NoMemoryErr; \
		goto bail; \
	}

#define BAILWITHERROR(v) \
	{ \
		err = (v); \
		goto bail; \
	}

#define TEST_ATOM_TYPE( atomName ) \
	if ( self->type != atomName ## Type ) \
	{ \
		err = MP4BadParamErr; \
		goto bail; \
	} \

#define DEBUG_MSG( str ) \
	inputStream->msg( inputStream,  str );

#define DEBUG_SPRINTF( fmt, var ) \
	{ \
		char buf[ 80 ]; \
		sprintf( buf, fmt, (var) ); \
		inputStream->msg( inputStream,  buf ); \
	}

#define GETBYTES( length, membername ) \
	err = inputStream->readData( inputStream, length, self->membername, #membername ); if (err) goto bail; \
	self->bytesRead += length

#define GETBYTES_V( length, var ) \
	err = inputStream->readData( inputStream, length, var, NULL ); if (err) goto bail; \
	self->bytesRead += length

#define GETBYTES_MSG( length, membername, msg ) \
	err = inputStream->readData( inputStream, length, self->membername, msg ); if (err) goto bail; \
	self->bytesRead += length

#define GETBYTES_V_MSG( length, var, msg ) \
	err = inputStream->readData( inputStream, length, var, msg ); if (err) goto bail; \
	self->bytesRead += length

#define GET64( membername ) \
	{ \
	u64 val64; \
		u32 val32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, #membername ); if (err) goto bail; \
		val64 = val32; \
		val64 <<= 32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, #membername ); if (err) goto bail; \
		val64 += val32; \
		self->membername = val64; \
	} \
	self->bytesRead += 8
	
#define GET64_MSG( membername, msg ) \
	{ \
	u64 val64; \
		u32 val32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, msg ); if (err) goto bail; \
		val64 = val32; \
		val64 <<= 32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, msg ); if (err) goto bail; \
		val64 += val32; \
		self->membername = val64; \
	} \
	self->bytesRead += 8
	
#define GET64_V( varname ) \
	{ \
	u64 val64; \
		u32 val32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, #varname ); if (err) goto bail; \
		val64 = val32; \
		val64 <<= 32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, #varname ); if (err) goto bail; \
		val64 += val32; \
		varname = val64; \
	} \
	self->bytesRead += 8

#define GET64_V_MSG( varname, msg ) \
	{ \
	u64 val64; \
		u32 val32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, msg ); if (err) goto bail; \
		val64 = val32; \
		val64 <<= 32; \
		err = inputStream->read32( inputStream, (u32 *) &val32, msg ); if (err) goto bail; \
		val64 += val32; \
		varname = val64; \
	} \
	self->bytesRead += 8

#define GET32( membername ) \
	err = inputStream->read32( inputStream, (u32 *) &self->membername, #membername ); if (err) goto bail; \
	self->bytesRead += 4

#define GET32_MSG( membername, msg ) \
	err = inputStream->read32( inputStream, (u32 *) &self->membername, msg ); if (err) goto bail; \
	self->bytesRead += 4

#define GET32_V( varname ) \
	err = inputStream->read32( inputStream, (u32 *) &varname, #varname ); if (err) goto bail; \
	self->bytesRead += 4

#define GET32_V_MSG( varname, msg ) \
	err = inputStream->read32( inputStream, (u32 *) &varname, msg ); if (err) goto bail; \
	self->bytesRead += 4

#define GET16( membername ) \
	err = inputStream->read16( inputStream, (u32 *) &self->membername, #membername ); if (err) goto bail; \
	self->bytesRead += 2

#define GET16_MSG( membername, msg ) \
	err = inputStream->read16( inputStream, (u32 *) &self->membername, msg ); if (err) goto bail; \
	self->bytesRead += 2

#define GET16_V( varname ) \
	err = inputStream->read16( inputStream, (u32 *) &varname , #varname); if (err) goto bail; \
	self->bytesRead += 2

#define GET16_V_MSG( varname, msg ) \
	err = inputStream->read16( inputStream, (u32 *) &varname , msg ); if (err) goto bail; \
	self->bytesRead += 2

#define GET8( membername ) \
	err = inputStream->read8( inputStream, (u32 *) &self->membername, #membername ); if (err) goto bail; \
	self->bytesRead += 1

#define GET8_MSG( membername, msg ) \
	err = inputStream->read8( inputStream, (u32 *) &self->membername, msg ); if (err) goto bail; \
	self->bytesRead += 1

#define GET8_V( varname ) \
	err = inputStream->read8( inputStream, (u32 *) &varname, #varname ); if (err) goto bail; \
	self->bytesRead += 1

#define GET8_V_MSG( varname, msg ) \
	err = inputStream->read8( inputStream, (u32 *) &varname, msg ); if (err) goto bail; \
	self->bytesRead += 1

#define GETATOM(membername) \
	err = MP4ParseAtom( inputStream, &self->membername ); if (err) goto bail; \
	self->bytesRead += self->membername->size;

#define GETATOM_V(varname) \
	err = MP4ParseAtom( inputStream, &varname ); if (err) goto bail; \
	self->bytesRead += varname->size;

#define GETATOM_LIST(listname) \
	{ while (self->bytesRead < self->size) { \
		MP4AtomPtr atm; \
		err = MP4ParseAtom( inputStream, &atm ); if (err) goto bail; \
		self->bytesRead += atm->size; \
		if ( ((atm->type)== MP4FreeSpaceAtomType) || ((atm->type)== MP4SkipAtomType)) \
			atm->destroy( atm ); \
			else { err = MP4AddListEntry( (void*) atm, self->listname ); if (err) goto bail; } \
	} }

#define PUTBYTES( src, len ) \
	if ( (self->bytesWritten + len) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	memcpy( buffer, src, len ); \
	buffer += len; \
	self->bytesWritten += len
	
#define PUT8( member ) \
	if ( (self->bytesWritten + 1) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) self->member; \
	buffer += 1; \
	self->bytesWritten += 1
	
#define PUT8_V( var ) \
	if ( (self->bytesWritten + 1) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) (var); \
	buffer += 1; \
	self->bytesWritten += 1

#define PUT16( member ) \
	if ( (self->bytesWritten + 2) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((self->member >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (self->member & 0xff); \
	buffer += 1; \
	self->bytesWritten += 2

#define PUT16_V( var ) \
	if ( (self->bytesWritten + 2) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((var >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (var & 0xff); \
	buffer += 1; \
	self->bytesWritten += 2

#define PUT24( member ) \
	if ( (self->bytesWritten + 3) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((self->member >> 16) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (self->member & 0xff); \
	buffer += 1; \
	self->bytesWritten += 3

#define PUT24_V( var ) \
	if ( (self->bytesWritten + 3) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((var >> 16) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (var & 0xff); \
	buffer += 1; \
	self->bytesWritten += 3

#define PUT32( member ) \
	if ( (self->bytesWritten + 4) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((self->member >> 24) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 16) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (self->member & 0xff); \
	buffer += 1; \
	self->bytesWritten += 4

#define PUT32_V( var ) \
	if ( (self->bytesWritten + 4) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((var >> 24) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 16) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (var & 0xff); \
	buffer += 1; \
	self->bytesWritten += 4

#define PUT64( member ) \
	if ( (self->bytesWritten + 8) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((self->member >> 56) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 48) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 40) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 32) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 24) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 16) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((self->member >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (self->member & 0xff); \
	buffer += 1; \
	self->bytesWritten += 8

#define PUT64_V( var ) \
	if ( (self->bytesWritten + 8) > self->size ) \
	{ \
		err = MP4IOErr; \
		goto bail; \
	} \
	*(u8*) buffer = (u8) ((var >> 56) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 48) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 40) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 32) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 24) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 16) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) ((var >> 8) & 0xff); \
	buffer += 1; \
	*(u8*) buffer = (u8) (var & 0xff); \
	buffer += 1; \
	self->bytesWritten += 8

#define ADD_DESCRIPTOR_SIZE( descName ) \
	if ( self->descName ) \
	{ \
		err = self->descName->calculateSize( self->descName ); if (err) goto bail; \
		self->size += self->descName->size; \
	} \

#define ADD_ATOM_SIZE( descName ) \
	if ( self->descName ) \
	{ \
		err = self->descName->calculateSize( self->descName ); if (err) goto bail; \
		self->size += self->descName->size; \
	} \


#define ADD_DESCRIPTOR_LIST_SIZE( listName ) \
	if ( self->listName ) \
	{ \
		err = MP4GetListEntryCount( self->listName, &count ); if (err) goto bail; \
		for ( i = 0; i < count; i++ ) \
		{ \
			err = MP4GetListEntry( self->listName, i, (char **) &desc ); if (err) goto bail; \
			if ( desc ) \
			{ \
				err = desc->calculateSize( desc ); if (err) goto bail; \
				self->size += desc->size; \
			} \
		} \
	} \

#define ADD_ATOM_LIST_SIZE( listName ) \
	if ( self->listName ) \
	{ \
		u32 count; \
		u32 i; \
		err = MP4GetListEntryCount( self->listName, &count ); if (err) goto bail; \
		for ( i = 0; i < count; i++ ) \
		{ \
			MP4AtomPtr desc; \
			err = MP4GetListEntry( self->listName, i, (char **) &desc ); if (err) goto bail; \
			if ( desc ) \
			{ \
				err = desc->calculateSize( desc ); if (err) goto bail; \
				self->size += desc->size; \
			} \
		} \
	} \
	
	
#define ADD_ATOM_LIST_SIZE_V( listName ) \
	if ( listName ) \
	{ \
		u32 count; \
		u32 i; \
		err = MP4GetListEntryCount( listName, &count ); if (err) goto bail; \
		for ( i = 0; i < count; i++ ) \
		{ \
			MP4AtomPtr desc; \
			err = MP4GetListEntry( listName, i, (char **) &desc ); if (err) goto bail; \
			if ( desc ) \
			{ \
				err = desc->calculateSize( desc ); if (err) goto bail; \
				self->size += desc->size; \
			} \
		} \
	} \

#define SERIALIZE_DESCRIPTOR( desc ) \
	if ( self->desc ) \
	{ \
		if ( self->bytesWritten + self->desc->size > self->size ) \
		{ \
			err = MP4IOErr; \
			goto bail; \
		} \
		err = self->desc->serialize( self->desc, buffer ); if (err) goto bail; \
		buffer += self->desc->bytesWritten; \
		self->bytesWritten += self->desc->bytesWritten; \
	}

#define SERIALIZE_ATOM( desc ) \
	if ( self->desc ) \
	{ \
		if ( self->bytesWritten + self->desc->size > self->size ) \
		{ \
			err = MP4IOErr; \
			goto bail; \
		} \
		err = self->desc->serialize( self->desc, buffer ); if (err) goto bail; \
		buffer += self->desc->bytesWritten; \
		self->bytesWritten += self->desc->bytesWritten; \
	}

#define SERIALIZE_DESCRIPTOR_LIST( listName ) \
	if ( self->listName ) \
	{ \
		u32 count; \
		u32 i; \
		struct MP4DescriptorRecord* desc; \
		err = MP4GetListEntryCount( self->listName, &count ); if (err) goto bail; \
		for ( i = 0; i < count; i++ ) \
		{ \
			err = MP4GetListEntry( self->listName, i, (char **) &desc ); if (err) goto bail; \
			if ( desc ) \
			{ \
				if ( self->bytesWritten + desc->size > self->size ) \
				{ \
					err = MP4IOErr; \
					goto bail; \
				} \
				err = desc->serialize( desc, buffer ); if (err) goto bail; \
				self->bytesWritten += desc->bytesWritten; \
				buffer += desc->bytesWritten; \
			} \
		} \
	}

#define SERIALIZE_ATOM_LIST( listName ) \
	if ( self->listName ) \
	{ \
		u32 count; \
		u32 i; \
		struct MP4Atom* desc; \
		err = MP4GetListEntryCount( self->listName, &count ); if (err) goto bail; \
		for ( i = 0; i < count; i++ ) \
		{ \
			err = MP4GetListEntry( self->listName, i, (char **) &desc ); if (err) goto bail; \
			if ( desc ) \
			{ \
				if ( self->bytesWritten + desc->size > self->size ) \
				{ \
					err = MP4IOErr; \
					goto bail; \
				} \
				err = desc->serialize( desc, buffer ); if (err) goto bail; \
				self->bytesWritten += desc->bytesWritten; \
				buffer += desc->bytesWritten; \
			} \
		} \
	}
	
#define SERIALIZE_ATOM_LIST_V( listName ) \
	if ( listName ) \
	{ \
		u32 count; \
		u32 i; \
		struct MP4Atom* desc; \
		err = MP4GetListEntryCount( listName, &count ); if (err) goto bail; \
		for ( i = 0; i < count; i++ ) \
		{ \
			err = MP4GetListEntry( listName, i, (char **) &desc ); if (err) goto bail; \
			if ( desc ) \
			{ \
				if ( self->bytesWritten + desc->size > self->size ) \
				{ \
					err = MP4IOErr; \
					goto bail; \
				} \
				err = desc->serialize( desc, buffer ); if (err) goto bail; \
				self->bytesWritten += desc->bytesWritten; \
				buffer += desc->bytesWritten; \
			} \
		} \
	}


#define IMPLEMENT_NEW_ADDATOM(atomname) \
static MP4Err addAtom( atomname ## Ptr self, MP4AtomPtr atom ) \
{\
	MP4Err err;\
	err = MP4AddListEntry( atom, self->atomList); \
	return err; \
}

#define PARSE_ATOM_INCLUDES(atomname) \
	while ( self->bytesRead < self->size ) \
	{ \
		MP4AtomPtr atom; \
		err = MP4ParseAtom( (MP4InputStreamPtr) inputStream, &atom ); \
			if (err) goto bail; \
		self->bytesRead += atom->size; \
		if ( ((atom->type)== MP4FreeSpaceAtomType) || ((atom->type)== MP4SkipAtomType)) \
			atom->destroy( atom ); \
		else { \
			err = addAtom( self, atom ); \
				if (err) goto bail; \
		} \
	} \
	if ( self->bytesRead != self->size ) \
		BAILWITHERROR( MP4BadDataErr )

#define PARSE_ATOM_LIST(atomname) \
	MP4Err err; \
	atomname ## Ptr self = (atomname ## Ptr) s; \
	err = MP4NoErr; \
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr ) \
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail; \
	PARSE_ATOM_INCLUDES(atomname)


#define DESTROY_ATOM_LIST \
	if ( self->atomList ) \
	{ \
		u32 atomListSize; \
		err = MP4GetListEntryCount( self->atomList, &atomListSize ); if (err) goto bail; \
		for ( i = 0; i < atomListSize; i++ ) \
		{ \
			MP4AtomPtr a; \
			err = MP4GetListEntry( self->atomList, i, (char **) &a ); if (err) goto bail; \
			if ( a ) \
				a->destroy( a ); \
		} \
		err = MP4DeleteLinkedList( self->atomList ); if (err) goto bail; \
	}

#define DESTROY_ATOM_LIST_V( atomList ) \
	if ( atomList ) \
	{ \
		u32 atomListSize; \
		err = MP4GetListEntryCount( atomList, &atomListSize ); if (err) goto bail; \
		for ( i = 0; i < atomListSize; i++ ) \
		{ \
			MP4AtomPtr a; \
			err = MP4GetListEntry( atomList, i, (char **) &a ); if (err) goto bail; \
			if ( a ) \
				a->destroy( a ); \
		} \
		err = MP4DeleteLinkedList( atomList ); if (err) goto bail; \
	}

#define DESTROY_ATOM_LIST_F( atomList ) \
	if ( self->atomList ) \
	{ \
		u32 atomListSize, i; \
		err = MP4GetListEntryCount( self->atomList, &atomListSize ); if (err) goto bail; \
		for ( i = 0; i < atomListSize; i++ ) \
		{ \
			MP4AtomPtr a; \
			err = MP4GetListEntry( self->atomList, i, (char **) &a ); if (err) goto bail; \
			if ( a ) \
				a->destroy( a ); \
		} \
		err = MP4DeleteLinkedList( self->atomList ); if (err) goto bail; \
	}
	
#define SETUP_BASE_DESCRIPTOR( classname ) \
	MP4Err err; \
	classname ## Ptr self; \
	err = MP4NoErr; \
	self = (classname ## Ptr) calloc( 1, sizeof(classname) ); \
	TESTMALLOC( self ) \
	self->tag  = tag; \
	self->size = size; \
	self->name = #classname; \
	self->bytesRead = bytesRead; \
	self->createFromInputStream = createFromInputStream; \
	self->calculateSize = calculateSize; \
	self->serialize = serialize; \
	self->destroy = destroy;

#define DESTROY_DESCRIPTOR( member ) \
	if ( self->member ) \
		self->member->destroy( self->member )

#define DESTROY_DESCRIPTOR_LIST( list ) \
	if ( self->list ) \
	{ \
		MP4Err err; \
		u32 listSize; \
		u32 i; \
		err = MP4GetListEntryCount( self->list, &listSize ); if (err) goto bail; \
		for ( i = 0; i < listSize; i++ ) \
		{ \
			MP4DescriptorPtr a; \
			err = MP4GetListEntry( self->list, i, (char **) &a ); if (err) goto bail; \
			if ( a ) \
				a->destroy( a ); \
		} \
		err = MP4DeleteLinkedList( self->list ); if (err) goto bail; \
	}

#define DESTROY_DESCRIPTOR_LIST_V( list ) \
	if ( list ) \
	{ \
		MP4Err err; \
		u32 listSize; \
		u32 i; \
		err = MP4GetListEntryCount( list, &listSize ); if (err) goto bail; \
		for ( i = 0; i < listSize; i++ ) \
		{ \
			MP4DescriptorPtr a; \
			err = MP4GetListEntry( list, i, (char **) &a ); if (err) goto bail; \
			if ( a ) \
				a->destroy( a ); \
		} \
		err = MP4DeleteLinkedList( list ); if (err) goto bail; \
	}	


#endif
