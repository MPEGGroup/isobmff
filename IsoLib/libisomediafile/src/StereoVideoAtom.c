#include "MP4Atoms.h"
#include <stdlib.h>


static void destroy(MP4AtomPtr s)
{
	MP4Err err;
	MP4StereoVideoAtomPtr self;

	self = (MP4StereoVideoAtomPtr)s;
	if (self == NULL) BAILWITHERROR(MP4BadParamErr)
		DESTROY_ATOM_LIST_F(atomList);

	if (self->super)
		self->super->destroy(s);

bail:
	TEST_RETURN(err);

	return;
}


static MP4Err addAtom(MP4StereoVideoAtomPtr self, MP4AtomPtr atom)
{
	MP4Err err;
	err = MP4NoErr;

	if (self == 0)
		BAILWITHERROR(MP4BadParamErr);

	err = MP4AddListEntry(atom, self->atomList); if (err) goto bail;

bail:
	TEST_RETURN(err);

	return err;
}

static MP4Err serialize(struct MP4Atom* s, char* buffer)
{
	MP4Err err;
	u32 tmp32;
	u8 tmp8;
	u32 i;
	MP4StereoVideoAtomPtr self = (MP4StereoVideoAtomPtr)s;
	err = MP4NoErr;

	if (self->size > 0) {

		err = MP4SerializeCommonBaseAtomFields(s, buffer); if (err) goto bail;
		buffer += self->bytesWritten;

		tmp32 = (self->reserved << 2) + self->single_view_allowed;
		PUT32_V(tmp32);
		PUT32(stereo_scheme);
		PUT32(length);
		
		for (i = 0; i < self->length; i++) {
			tmp8 = self->stereo_indication_type[i];
			PUT8_V(tmp8);
		}

		SERIALIZE_ATOM_LIST(atomList);

		assert(self->bytesWritten == self->size);

	}

bail:
	TEST_RETURN(err);

	return err;
}


static MP4Err calculateSize(struct MP4Atom* s)
{
	MP4Err err;
	MP4StereoVideoAtomPtr self = (MP4StereoVideoAtomPtr)s;
	err = MP4NoErr;

	err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s); if (err) goto bail;

	self->size += (3 * 4) + self->length;

bail:
	TEST_RETURN(err);

	return err;
}


static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
	MP4Err err;
	u32 i;
	u32 tmp32;

	MP4StereoVideoAtomPtr self = (MP4StereoVideoAtomPtr)s;

	err = MP4NoErr;
	if (self == NULL)	
		BAILWITHERROR(MP4BadParamErr);
		
	err = self->super->createFromInputStream(s, proto, (char*)inputStream);

	GET32_V(tmp32);
	self->reserved = (tmp32 >> 2) & 0x3FFF;
	self->stereo_indication_type = (u8)(tmp32 & 0x3);
	GET32(stereo_scheme);
	GET32(length);
	self->stereo_indication_type = calloc(self->length, sizeof(u8));
	for (i = 0; i < self->length; i++) {
		GET8_V(self->stereo_indication_type[i]);
	}

	assert(self->bytesRead == self->size);
bail:
	TEST_RETURN(err);

	return err;
}
