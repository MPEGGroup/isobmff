
#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>



static void destroy(MP4AtomPtr s)
{
	MP4Err err;

	MP4RestrictedSchemeInfoAtomPtr self = (MP4RestrictedSchemeInfoAtomPtr)s;
	err = MP4NoErr;

	if (self == NULL)
		BAILWITHERROR(MP4BadParamErr);
	
		
	DESTROY_ATOM_LIST_F(atomList);
	
	/*
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

	/*
	SERIALIZE_ATOM(MP4OriginalFormat);
	SERIALIZE_ATOM(MP4SchemeType);
	SERIALIZE_ATOM(MP4SchemeInfo);
	*/

	SERIALIZE_ATOM_LIST(atomList);

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
	
	ADD_ATOM_LIST_SIZE(atomList);
	
bail:
	TEST_RETURN(err);

	return err;
}

#define ADDCASE( atomName ) \
		case atomName ## AtomType: \
			if (self->atomName) \
				BAILWITHERROR(MP4BadDataErr); \
			self->atomName = atom; \
			break



static MP4Err addAtom(MP4RestrictedSchemeInfoAtomPtr self, MP4AtomPtr atom)
{
	MP4Err err;
	err = MP4NoErr;

	err = MP4AddListEntry(atom, self->atomList); if (err) goto bail;

	switch (atom->type)
	{
		ADDCASE(MP4OriginalFormat);
		ADDCASE(MP4SchemeType);
		ADDCASE(MP4SchemeInfo);
	}
bail:
	TEST_RETURN(err);

	return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{

	PARSE_ATOM_LIST(MP4RestrictedSchemeInfoAtom);





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

	err = MP4CreateBaseAtom((MP4AtomPtr)self); if (err) goto bail;

	self->type = MP4RestrictedSchemeInfoAtomType;
	self->name = "RestrictedSchemeInfoBox";
	self->createFromInputStream = (cisfunc)createFromInputStream;
	self->destroy = destroy;
	self->calculateSize = calculateSize;
	self->serialize = serialize;
	self->addAtom = addAtom;

	err = MP4MakeLinkedList(&self->atomList); if (err) goto bail;

	*outAtom = self;
bail:
	TEST_RETURN(err);

	return err;
}