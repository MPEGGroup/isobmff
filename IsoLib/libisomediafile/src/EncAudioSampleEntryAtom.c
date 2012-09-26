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
	$Id: EncAudioSampleEntryAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4EncAudioSampleEntryAtomPtr self;
	err = MP4NoErr;
	self = (MP4EncAudioSampleEntryAtomPtr) s;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
		
	if ( self->SecurityInfo )
	{
		self->SecurityInfo->destroy( self->SecurityInfo );
		self->SecurityInfo = NULL;
	}
	DESTROY_ATOM_LIST_F( ExtensionAtomList )
	
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	MP4EncAudioSampleEntryAtomPtr self = (MP4EncAudioSampleEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	PUTBYTES( self->reserved1, 6 );
	PUT16( dataReferenceIndex );
	PUTBYTES( self->reserved2, 8 );
	PUT16( reserved3 );
	PUT16( reserved4 );
	PUT32( reserved5 );
	PUT16( timeScale );
	PUT16( reserved6 );
	SERIALIZE_ATOM_LIST( ExtensionAtomList );
	SERIALIZE_ATOM( SecurityInfo );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4EncAudioSampleEntryAtomPtr self = (MP4EncAudioSampleEntryAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += 14 + (1*4)+(5*2);
	ADD_ATOM_SIZE( SecurityInfo );
	ADD_ATOM_LIST_SIZE( ExtensionAtomList );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addAtom( MP4EncAudioSampleEntryAtomPtr self, MP4AtomPtr atom )
{
   MP4Err err;
   err = MP4NoErr;
   if ( atom == NULL )
      BAILWITHERROR( MP4BadParamErr );
   if (atom->type == MP4SecurityInfoAtomType)
   		self->SecurityInfo = atom;
   else { err = MP4AddListEntry( atom, self->ExtensionAtomList ); if (err) goto bail; }
  bail:
   TEST_RETURN( err );

   return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4EncAudioSampleEntryAtomPtr self = (MP4EncAudioSampleEntryAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GETBYTES( 6, reserved1 );
	GET16( dataReferenceIndex );
	GETBYTES( 8, reserved2 );
	GET16( reserved3 );
	GET16( reserved4 );
	GET32( reserved5 );
	GET16( timeScale );
	GET16( reserved6 );
	while ( self->bytesRead < self->size )
	{ 
		MP4AtomPtr atom; 
		err = MP4ParseAtom( (MP4InputStreamPtr) inputStream, &atom ); 
			if (err) goto bail; 
		self->bytesRead += atom->size; 
		if ( ((atom->type)== MP4FreeSpaceAtomType) || ((atom->type)== MP4SkipAtomType)) 
			atom->destroy( atom );
		else {
			err = addAtom( self, atom );
				if (err) goto bail;
		}
	}
	if ( self->bytesRead != self->size ) 
		BAILWITHERROR( MP4BadDataErr )
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err transform(struct MP4Atom *s, u32 sch_type, u32 sch_version, char* sch_url )
{
	MP4Err err;
	
	MP4EncBaseSampleEntryAtomPtr self = (MP4EncBaseSampleEntryAtomPtr) s;
	MP4OriginalFormatAtomPtr fmt;
	MP4SecurityInfoAtomPtr sinf;
	MP4SecuritySchemeAtomPtr schm;
	MP4SchemeInfoAtomPtr schi;
	char* sch_url_copy = NULL;
	
	err = MP4CreateOriginalFormatAtom( &fmt ); if (err) goto bail;
	fmt->original_format = self->type;

	err = MP4CreateSchemeInfoAtom( &schi ); if (err) goto bail;
	
	err = MP4CreateSecuritySchemeAtom( &schm ); if (err) goto bail;
	schm->scheme_type    = sch_type;
	schm->scheme_version = sch_version;
	
	if (sch_url) {
		sch_url_copy = (char*) calloc( 1, strlen(sch_url)+1 );
	    TESTMALLOC( sch_url_copy );
	    memcpy( sch_url_copy, sch_url, strlen(sch_url)+1 );
		schm->scheme_url = sch_url_copy; sch_url_copy = NULL;
	}
	else schm->scheme_url = NULL;

	err = MP4CreateSecurityInfoAtom( &sinf ); if (err) goto bail;
	sinf->MP4OriginalFormat = (MP4AtomPtr) fmt; fmt = NULL;
	sinf->MP4SecurityScheme = (MP4AtomPtr) schm; schm = NULL;
	sinf->MP4SchemeInfo     = (MP4AtomPtr) schi; schi = NULL;
	
	self->type = self->enc_type;
	self->SecurityInfo = (MP4AtomPtr) sinf;
	
bail:
	if (fmt)  fmt->destroy( (MP4AtomPtr) fmt );
	if (schm) schm->destroy( (MP4AtomPtr) schm );
	if (schi) schi->destroy( (MP4AtomPtr) schi );
	if (sch_url_copy) free( sch_url_copy );
	
	TEST_RETURN( err );

	return err;
}

static MP4Err untransform(struct MP4Atom *s )
{
	MP4Err err;
	
	MP4EncBaseSampleEntryAtomPtr self = (MP4EncBaseSampleEntryAtomPtr) s;
	MP4OriginalFormatAtomPtr fmt;
	MP4SecurityInfoAtomPtr sinf;

	err = MP4NoErr;
	
	sinf = (MP4SecurityInfoAtomPtr) self->SecurityInfo;
	if (!sinf) { err = MP4BadParamErr; goto bail; }
	
	fmt  = (MP4OriginalFormatAtomPtr) sinf->MP4OriginalFormat;
	if (!fmt)  { err = MP4BadDataErr; goto bail; }
	
	self->type = fmt->original_format;
	self->SecurityInfo = NULL;
	
	sinf->destroy( (MP4AtomPtr) sinf );
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err addSchemeInfoAtom(struct MP4Atom *s, struct MP4Atom *theAtom )
{
	MP4Err err;
	
	MP4EncBaseSampleEntryAtomPtr self = (MP4EncBaseSampleEntryAtomPtr) s;
	MP4SchemeInfoAtomPtr  schi;
	MP4SecurityInfoAtomPtr sinf;

	sinf = (MP4SecurityInfoAtomPtr) self->SecurityInfo;
	if (!sinf) { err = MP4BadParamErr; goto bail; }
	
	schi = (MP4SchemeInfoAtomPtr) sinf->MP4SchemeInfo;
	if (!schi)  { err = MP4BadDataErr; goto bail; }
	
	err = schi->addAtom( schi, theAtom ); if (err) goto bail;
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getSchemeInfoAtom(struct MP4Atom *s, u32 theType, struct MP4Atom **theAtom )
{
	MP4Err err;
	
	MP4EncBaseSampleEntryAtomPtr self = (MP4EncBaseSampleEntryAtomPtr) s;
	MP4SchemeInfoAtomPtr  schi;
	MP4SecurityInfoAtomPtr sinf;

	err = MP4NoErr;
	
	sinf = (MP4SecurityInfoAtomPtr) self->SecurityInfo;
	if (!sinf) { err = MP4BadParamErr; goto bail; }
	
	schi = (MP4SchemeInfoAtomPtr) sinf->MP4SchemeInfo;
	if (!schi)  { err = MP4BadDataErr; goto bail; }
	
	err = MP4BadParamErr;
	*theAtom = NULL;
	
	if ( schi->atomList )
	{
		u32 count;
		u32 i;
		struct MP4Atom* desc;
		err = MP4GetListEntryCount( schi->atomList, &count ); if (err) goto bail;
		for ( i = 0; i < count; i++ )
		{
			err = MP4GetListEntry( schi->atomList, i, (char **) &desc ); if (err) goto bail;
			if ( desc && (desc->type == theType))
			{
				*theAtom = desc;
				break;
				err = MP4NoErr;
			}
		}
	}
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getScheme(struct MP4Atom *s, u32* sch_type, u32* sch_version, char** sch_url  )
{
	MP4Err err;
	
	MP4EncBaseSampleEntryAtomPtr self = (MP4EncBaseSampleEntryAtomPtr) s;
	MP4SecuritySchemeAtomPtr  schm;
	MP4SecurityInfoAtomPtr sinf;
	char* sch_url_copy;

	err = MP4NoErr;
	
	sinf = (MP4SecurityInfoAtomPtr) self->SecurityInfo;
	if (!sinf) { err = MP4BadParamErr; goto bail; }
	
	schm = (MP4SecuritySchemeAtomPtr) sinf->MP4SecurityScheme;
	if (!schm)  { err = MP4BadDataErr; goto bail; }
	
	*sch_type    = schm->scheme_type;
	*sch_version = schm->scheme_version;
	
	if (sch_url) {
		sch_url_copy = (char*) calloc( 1, strlen(schm->scheme_url)+1 );
	    TESTMALLOC( sch_url_copy );
	    memcpy( sch_url_copy, schm->scheme_url, strlen(schm->scheme_url)+1 );
		*sch_url = sch_url_copy;
	}
	
bail:
	TEST_RETURN( err );

	return err;
}


MP4Err MP4CreateEncBaseAtom( MP4EncBaseSampleEntryAtomPtr self )
{
	MP4Err err;
	err = MP4NoErr;
	
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = MP4CreateBaseAtom( (MP4AtomPtr) self );

	self->untransform 		= untransform;
	self->addSchemeInfoAtom = addSchemeInfoAtom;
	self->getSchemeInfoAtom = getSchemeInfoAtom;
	self->getScheme			= getScheme;
	self->transform 		= transform;
	err = MP4MakeLinkedList( &self->ExtensionAtomList ); if (err) goto bail;
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateEncAudioSampleEntryAtom( MP4EncAudioSampleEntryAtomPtr *outAtom )
{
	MP4Err err;
	MP4EncAudioSampleEntryAtomPtr self;
	
	self = (MP4EncAudioSampleEntryAtomPtr) calloc( 1, sizeof(MP4EncAudioSampleEntryAtom) );
	TESTMALLOC( self );

	err = MP4CreateEncBaseAtom( (MP4EncBaseSampleEntryAtomPtr) self );
	if ( err ) goto bail;
	self->type                  = MP4EncAudioSampleEntryAtomType;
	self->name                  = "protected audio sample entry";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy               = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	self->enc_type				= MP4EncAudioSampleEntryAtomType;
	self->reserved3 = 2;
	self->reserved4 = 16;
	self->timeScale = 44100;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
