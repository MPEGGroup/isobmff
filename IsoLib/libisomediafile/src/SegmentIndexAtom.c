/*
*This software module was originally developed by InterDigital, Inc.
* in the course of development of MPEG-4.
* This software module is an implementation of a part of one or
* more MPEG-4 tools as specified by MPEG-4.
* ISO/IEC gives users of MPEG-4 free license to this
* software module or modifications thereof for use in hardware
* or software products claiming conformance to MPEG-4 only for evaluation and testing purposes.
* Those intending to use this software module in hardware or software
* products are advised that its use may infringe existing patents.
* The original developer of this software module and his/her company,
* the subsequent editors and their companies, and ISO/IEC have no
* liability for use of this software module or modifications thereof
* in an implementation.
*
* Copyright is not released for non MPEG-4 conforming
* products. InterDigital, Inc. retains full right to use the code for its own
* purpose, assign or donate the code to a third party and to
* inhibit third parties from using the code for non
* MPEG-4 conforming products.
*
* This copyright notice must be included in all copies or
* derivative works.
*/

/**
* @file SegmentIndexAtom.c
* @author Ahmed Hamza <Ahmed.Hamza@InterDigital.com>
* @date April 19, 2018
* @brief Implements functions for reading and writing SegmentIndexAtom instances.
*/


#include "MP4Atoms.h"
#include <stdlib.h>


static u32 getReferenceCount(struct MP4SegmentIndexAtom *self)
{
    u32 referenceCount = 0;
    MP4GetListEntryCount(self->referencesList, &referenceCount);
    return referenceCount;
}

static MP4Err addReference(struct MP4SegmentIndexAtom *self)
{
    MP4Err err;


    err = MP4NoErr;

bail:
    TEST_RETURN(err);

    return err;
}

static void destroy(MP4AtomPtr s)
{
    MP4Err err;
    MP4SegmentIndexAtomPtr self;

    self = (MP4SegmentIndexAtomPtr)s;
    if (self == NULL)
        BAILWITHERROR(MP4BadParamErr);


    if (self->super)
        self->super->destroy(s);

bail:
    TEST_RETURN(err);

    return;
}


static MP4Err serialize(struct MP4Atom* s, char* buffer)
{
    MP4Err err;

    MP4SegmentIndexAtomPtr self = (MP4SegmentIndexAtomPtr)s;
    err = MP4NoErr;

bail:
    TEST_RETURN(err);

    return err;
}


static MP4Err calculateSize(struct MP4Atom* s)
{
    MP4Err err;
    MP4SegmentIndexAtomPtr self = (MP4SegmentIndexAtomPtr)s;
    err = MP4NoErr;

    err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s); if (err) goto bail;




bail:
    TEST_RETURN(err);

    return err;
}



static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
    MP4Err err;
    u32 i;
    u32 tmp32;

    MP4SegmentIndexAtomPtr self = (MP4SegmentIndexAtomPtr)s;

    err = MP4NoErr;
    if (self == NULL)
        BAILWITHERROR(MP4BadParamErr);

    err = self->super->createFromInputStream(s, proto, (char*)inputStream);



    assert(self->bytesRead == self->size);
bail:
    TEST_RETURN(err);

    return err;
}





MP4Err MP4CreateSegmentIndexAtom(MP4SegmentIndexAtomPtr *outAtom)
{
    MP4Err err;
    MP4SegmentIndexAtomPtr self;

    self = (MP4SegmentIndexAtomPtr)calloc(1, sizeof(MP4SegmentIndexAtom));
    TESTMALLOC(self);

    err = MP4CreateFullAtom((MP4AtomPtr)self); if (err) goto bail;

    self->type = MP4SegmentIndexAtomType;
    self->name = "SegmentIndexBox";
    self->destroy = destroy;
    self->createFromInputStream = (cisfunc)createFromInputStream;
    self->calculateSize = calculateSize;
    self->serialize = serialize;
    self->addReference = addReference;

    *outAtom = self;
bail:
    TEST_RETURN(err);

    return err;
}