
#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>



static void destroy(MP4AtomPtr s)
{
	MP4Err err;

	MP4RestrictedSchemeInfoAtomPtr self = (MP4RestrictedSchemeInfoAtomPtr)s;
	err = MP4NoErr;

	if (self == NULL)
		BAILWITHERROR(MP4BadParamErr)
	if (self->MP4OriginalFormat)
	{
		self->MP4OriginalFormat->destroy(self->MP4OriginalFormat);
		self->MP4OriginalFormat = NULL;
	}
	if (self->MP4SchemeType)
	{
		self->MP4SchemeType->destroy(self->MP4SchemeType);
		self->MP4SchemeType = NULL;
	}
	if (self->MP4SchemeInfo)
	{
		self->MP4SchemeInfo->destroy(self->MP4SchemeInfo);
		self->MP4SchemeInfo = NULL;
	}
	// Ahmed: This should be a list (zero or more CompatibleSchemeTypeBox are possible)
	/*
	if (self->MP4CompatibleSchemeType)
	{
	// TODO
	}
	*/

	if (self->super)
		self->super->destroy(s);
bail:
	TEST_RETURN(err);

	return;
}

static MP4Err serialize(struct MP4Atom* s, char* buffer)
{
	MP4Err err;
	MP4RestrictedSchemeInfoAtomPtr self = (MP4RestrictedSchemeInfoAtomPtr)s;
	err = MP4NoErr;

	err = MP4SerializeCommonBaseAtomFields(s, buffer); if (err) goto bail;
	buffer += self->bytesWritten;

	/* PUT32_V( 0 );	*/	/* version/flags */

	SERIALIZE_ATOM(MP4OriginalFormat);
	SERIALIZE_ATOM(MP4SchemeType);
	SERIALIZE_ATOM(MP4SchemeInfo);
	//SERIALIZE_ATOM(MP4CompatibleSchemeType);

	assert(self->bytesWritten == self->size);
bail:
	TEST_RETURN(err);

	return err;
}

static MP4Err calculateSize(struct MP4Atom* s)
{
	MP4Err err;
	MP4RestrictedSchemeInfoAtomPtr self = (MP4RestrictedSchemeInfoAtomPtr)s;
	err = MP4NoErr;

	err = MP4CalculateBaseAtomFieldSize(s); if (err) goto bail;
	self->size += 0;		/* version/flags */
	ADD_ATOM_SIZE(MP4OriginalFormat);
	ADD_ATOM_SIZE(MP4SchemeType);
	ADD_ATOM_SIZE(MP4SchemeInfo);
	//ADD_ATOM_SIZE(MP4CompatibleSchemeType);
bail:
	TEST_RETURN(err);

	return err;
}

#define ADDCASE( atomName ) \
		case atomName ## AtomType: \
if (self->atomName) \
	BAILWITHERROR(MP4BadDataErr) \
	self->atomName = atom; \
	break



static MP4Err addAtom(MP4RestrictedSchemeInfoAtomPtr self, MP4AtomPtr atom)
{
	MP4Err err;
	err = MP4NoErr;
	switch (atom->type)
	{
		ADDCASE(MP4OriginalFormat);
		ADDCASE(MP4SchemeType);
		ADDCASE(MP4SchemeInfo);

	default:
		/* TODO: this default is wrong; we should be tolerant and accept unknown atoms */
		//err = MP4InvalidMediaErr;
		goto bail;
		break;
	}
bail:
	TEST_RETURN(err);

	return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{

	PARSE_ATOM_LIST(MP4RestrictedSchemeInfoAtom)

#if 0
		MP4Err err; \

		MP4RestrictedSchemeInfoAtomPtr self = (MP4RestrictedSchemeInfoAtomPtr)s; \

		err = MP4NoErr; \
	if (self == NULL)	BAILWITHERROR(MP4BadParamErr) \
		err = self->super->createFromInputStream(s, proto, (char*)inputStream); if (err) goto bail; \
		GET32_V(junk);			/* version/flags */
	while (self->bytesRead < self->size) \
	{ \
	MP4AtomPtr atom; \
	err = MP4ParseAtom((MP4InputStreamPtr)inputStream, &atom); \
	if (err) goto bail; \
		self->bytesRead += atom->size; \
	if (((atom->type) == MP4FreeSpaceAtomType) || ((atom->type) == MP4SkipAtomType)) \
		atom->destroy(atom); \
	else {
		\
			err = addAtom(self, atom); \
		if (err) goto bail; \
	} \
	} \
	if (self->bytesRead != self->size) \
		BAILWITHERROR(MP4BadDataErr) \

#endif


	bail:
	TEST_RETURN(err);

	return err;
}



MP4Err MP4CreateRestrictedSchemeInfoAtom(MP4RestrictedSchemeInfoAtomPtr *outAtom)
{
	MP4Err err;
	MP4RestrictedSchemeInfoAtomPtr self;

	self = (MP4RestrictedSchemeInfoAtomPtr)calloc(1, sizeof(MP4RestrictedSchemeInfoAtom));
	TESTMALLOC(self);

	err = MP4CreateBaseAtom((MP4AtomPtr)self);
	if (err) goto bail;

	self->type = MP4RestrictedSchemeInfoAtomType;
	self->name = "RestrictedSchemeInfo";
	self->createFromInputStream = (cisfunc)createFromInputStream;
	self->destroy = destroy;
	self->calculateSize = calculateSize;
	self->serialize = serialize;

	*outAtom = self;
bail:
	TEST_RETURN(err);

	return err;
}