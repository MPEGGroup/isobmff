
#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

// OMAF: TODO review code

static void destroy(MP4AtomPtr s)
{
	MP4Err err;
	MP4TrackTypeAtomPtr self;
	u32 i;
	err = MP4NoErr;
	self = (MP4TrackTypeAtomPtr)s;
	if (self == NULL)
		BAILWITHERROR(MP4BadParamErr)

	if (self->super)
		self->super->destroy(s);
bail:
	TEST_RETURN(err);

	return;
}



static ISOErr serialize(struct MP4Atom* s, char* buffer)
{
	ISOErr err;
	u32 i;
	MP4TrackTypeAtomPtr self = (MP4TrackTypeAtomPtr)s;

	err = ISONoErr;

	err = MP4SerializeCommonBaseAtomFields(s, buffer); if (err) goto bail;
	buffer += self->bytesWritten;

	PUT32(majorBrand);
	PUT32(minorVersion);

	for (i = 0; i < self->itemCount; i++)
	{
		PUT32_V((self->compatibilityList[i]));
	}

	assert(self->bytesWritten == self->size);
bail:
	TEST_RETURN(err);

	return err;
}

static ISOErr calculateSize(struct MP4Atom* s)
{
	ISOErr err;
	MP4TrackTypeAtomPtr self = (MP4TrackTypeAtomPtr)s;
	err = ISONoErr;

	err = MP4CalculateBaseAtomFieldSize(s); if (err) goto bail;
	self->size += 2 * sizeof(u32);								/* brand and minorVersion */
	self->size += self->itemCount * sizeof(u32);				/* compatibilityList */
bail:
	TEST_RETURN(err);

	return err;
}

static ISOErr getBrand(struct MP4TrackTypeAtom *self, u32* standard, u32* minorversion)
{
	*standard = self->majorBrand;
	*minorversion = self->minorVersion;

	return MP4NoErr;
}

static u32 getStandard(struct MP4TrackTypeAtom *self, u32 standard)
{
	u32 i;
	u32 outval;

	outval = 0;

	for (i = 0; i<self->itemCount; i++) {
		if (self->compatibilityList[i] == standard) {
			outval = standard;
			break;
		}
	}
	return outval;
}

/* add a track type to the compatibility list */
static ISOErr addStandard(struct MP4TrackTypeAtom *self, u32 standard)
{
	ISOErr err;
	err = ISONoErr;

	if (!getStandard(self, standard)) {
		self->itemCount++;
		self->compatibilityList = (u32*)realloc(self->compatibilityList, self->itemCount * sizeof(u32));
		TESTMALLOC(self->compatibilityList);
		self->compatibilityList[self->itemCount - 1] = (u32)standard;
	}
bail:
	TEST_RETURN(err);

	return err;
}

static ISOErr setBrand(struct MP4TrackTypeAtom *self, u32 standard, u32 minorversion)
{
	u32 oldstandard;
	MP4Err err;
	oldstandard = self->majorBrand;

	self->majorBrand = standard;
	self->minorVersion = minorversion;

	/* in the compatibility list are also the new major brand, and the old one, if any */
	if (oldstandard) { err = addStandard(self, oldstandard); if (err) return err; }
	err = addStandard(self, standard);    if (err) return err;

	return err;
}

static ISOErr createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
	ISOErr err;
	u32 items = 0;
	long bytesToRead;
	char typeString[8];
	char msgString[80];
	MP4TrackTypeAtomPtr self = (MP4TrackTypeAtomPtr)s;

	err = ISONoErr;
	if (self == NULL)
		BAILWITHERROR(ISOBadParamErr)
		err = self->super->createFromInputStream(s, proto, (char*)inputStream); if (err) goto bail;

	GET32(majorBrand);
	MP4TypeToString(self->majorBrand, typeString);
	sprintf(msgString, " major brand is '%s'", typeString);
	inputStream->msg(inputStream, msgString);

	GET32(minorVersion);

	bytesToRead = self->size - self->bytesRead;
	if (bytesToRead < ((long) sizeof(u32)))							/* there must be at least one item in the compatibility list */
		BAILWITHERROR(ISOBadDataErr)

	if (self->compatibilityList)
		free(self->compatibilityList);

	self->compatibilityList = (u32 *)calloc(1, bytesToRead);
	TESTMALLOC(self->compatibilityList);

	while (bytesToRead > 0)
	{
		if (bytesToRead < ((long) sizeof(u32)))						/* we need to read a full u32 */
			BAILWITHERROR(ISOBadDataErr)

			GET32(compatibilityList[items]);
		MP4TypeToString(self->compatibilityList[items], typeString);
		sprintf(msgString, " minor brand is '%s'", typeString);
		inputStream->msg(inputStream, msgString);
		items++;
		bytesToRead = self->size - self->bytesRead;
	}

	self->itemCount = items;
bail:
	TEST_RETURN(err);

	return err;
}

ISOErr MP4CreateTrackTypeAtom(MP4TrackTypeAtomPtr *outAtom)
{
	ISOErr err;
	MP4TrackTypeAtomPtr self;

	self = (MP4TrackTypeAtomPtr)calloc(1, sizeof(MP4TrackTypeAtom));
	TESTMALLOC(self);

	err = MP4CreateBaseAtom((MP4AtomPtr)self);
	if (err) goto bail;

	self->type = MP4TrackTypeAtomType;
	self->name = "track type atom";
	self->destroy = destroy;
	self->createFromInputStream = (cisfunc)createFromInputStream;
	self->calculateSize = calculateSize;
	self->serialize = serialize;
	self->addStandard = addStandard;
	self->setBrand = setBrand;
	self->getBrand = getBrand;
	self->getStandard = getStandard;

	self->majorBrand = 0; /* was ISOISOBrand */
	self->minorVersion = (u32)0;
	self->compatibilityList = (u32 *)calloc(1, sizeof(u32));
	TESTMALLOC(self->compatibilityList);

	/* self->compatibilityList[0]	= ISOISOBrand; */
	/* self->compatibilityList[1]	= ISOISOBrand; */
	/* No, MPEG-21 and meta movies are not ISOM branded */
	self->itemCount = (u32)0;

	*outAtom = self;
bail:
	TEST_RETURN(err);

	return err;
}
