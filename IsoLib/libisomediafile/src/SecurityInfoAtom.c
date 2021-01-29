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
  $Id: SecurityInfoAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

#ifdef ISMACrypt

static void destroy(MP4AtomPtr s)
{
  MP4SecurityInfoAtomPtr self = (MP4SecurityInfoAtomPtr)s;
  if(self == NULL) return;
  if(self->MP4OriginalFormat)
  {
    self->MP4OriginalFormat->destroy(self->MP4OriginalFormat);
    self->MP4OriginalFormat = NULL;
  }
  if(self->MP4SchemeType)
  {
    self->MP4SchemeType->destroy(self->MP4SchemeType);
    self->MP4SchemeType = NULL;
  }
  if(self->MP4SchemeInfo)
  {
    self->MP4SchemeInfo->destroy(self->MP4SchemeInfo);
    self->MP4SchemeInfo = NULL;
  }
  if(self->super) self->super->destroy(s);
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  MP4SecurityInfoAtomPtr self = (MP4SecurityInfoAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4SerializeCommonBaseAtomFields(s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  /* PUT32_V( 0 );	*/ /* version/flags */

  SERIALIZE_ATOM(MP4OriginalFormat);
  SERIALIZE_ATOM(MP4SchemeType);
  SERIALIZE_ATOM(MP4SchemeInfo);

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4SecurityInfoAtomPtr self = (MP4SecurityInfoAtomPtr)s;
  err                         = MP4NoErr;

  err = MP4CalculateBaseAtomFieldSize(s);
  if(err) goto bail;
  self->size += 0; /* version/flags */
  ADD_ATOM_SIZE(MP4OriginalFormat);
  ADD_ATOM_SIZE(MP4SchemeType);
  ADD_ATOM_SIZE(MP4SchemeInfo);
bail:
  TEST_RETURN(err);

  return err;
}

#define ADDCASE(atomName)                           \
  case atomName##AtomType:                          \
    if(self->atomName) BAILWITHERROR(MP4BadDataErr) \
    self->atomName = atom;                          \
    break

static MP4Err addAtom(MP4SecurityInfoAtomPtr self, MP4AtomPtr atom)
{
  MP4Err err;
  err = MP4NoErr;
  switch(atom->type)
  {
    ADDCASE(MP4OriginalFormat);
    ADDCASE(MP4SchemeType);
    ADDCASE(MP4SchemeInfo);
  /* todo: this default is wrong; we should be tolerant and accept unknown atoms */
  default:
    err = MP4InvalidMediaErr;
    goto bail;
    break;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{

  PARSE_ATOM_LIST(MP4SecurityInfoAtom)

#if 0
	MP4Err err; \
	
	MP4SecurityInfoAtomPtr self = (MP4SecurityInfoAtomPtr) s; \
	
	err = MP4NoErr; \
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr ) \
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail; \
	GET32_V( junk );			/* version/flags */
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

#endif

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSecurityInfoAtom(MP4SecurityInfoAtomPtr *outAtom)
{
  MP4Err err;
  MP4SecurityInfoAtomPtr self;

  self = (MP4SecurityInfoAtomPtr)calloc(1, sizeof(MP4SecurityInfoAtom));
  TESTMALLOC(self);

  err = MP4CreateBaseAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type = MP4SecurityInfoAtomType;
  self->name = "SecurityInfo";

  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}

#endif
