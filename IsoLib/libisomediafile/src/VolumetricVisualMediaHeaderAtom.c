/* This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4.
 * Those intending to use this software module in hardware or software products are advised that its
 * use may infringe existing patents. The original developer of this software module and his/her
 * company, the subsequent editors and their companies, and ISO/IEC have no liability for use of
 * this software module or modifications thereof in an implementation. Copyright is not released for
 * non MPEG-4 conforming products. Apple Computer, Inc. retains full right to use the code for its
 * own purpose, assign or donate the code to a third party and to inhibit third parties from using
 * the code for non MPEG-4 conforming products. This copyright notice must be included in all copies
 * or derivative works. Copyright (c) 1999.
 */

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err                                  = MP4NoErr;
  MP4VolumetricVisualMediaHeaderAtomPtr pSelf = (MP4VolumetricVisualMediaHeaderAtomPtr)s;

  if(pSelf == NULL)
  {
    err = MP4BadParamErr;
  }
  if(err) goto bail;

  if(pSelf->super) pSelf->super->destroy(s);

bail:
  TEST_RETURN(err);
  return;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err                                  = MP4NoErr;
  MP4VolumetricVisualMediaHeaderAtomPtr pSelf = (MP4VolumetricVisualMediaHeaderAtomPtr)s;

  if(pSelf == NULL) BAILWITHERROR(MP4BadParamErr);
  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err                                  = MP4NoErr;
  MP4VolumetricVisualMediaHeaderAtomPtr pSelf = (MP4VolumetricVisualMediaHeaderAtomPtr)s;
  if(pSelf == NULL || buffer == NULL) BAILWITHERROR(MP4BadParamErr);

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += pSelf->bytesWritten;

  /* Nothing to write: there are no members so far in this atom */

  assert(pSelf->bytesWritten == pSelf->size);
bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err                                  = MP4NoErr;
  MP4VolumetricVisualMediaHeaderAtomPtr pSelf = (MP4VolumetricVisualMediaHeaderAtomPtr)s;

  if(pSelf == NULL) BAILWITHERROR(MP4BadParamErr);

  err = pSelf->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  /* Nothing to read: there are no members so far in this atom */

  assert(pSelf->bytesRead == pSelf->size);
bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4CreateVisualMediaHeaderAtom(MP4VolumetricVisualMediaHeaderAtomPtr *pOut)
{
  MP4Err err;
  MP4VolumetricVisualMediaHeaderAtomPtr pSelf;

  pSelf =
    (MP4VolumetricVisualMediaHeaderAtomPtr)calloc(1, sizeof(MP4VolumetricVisualMediaHeaderAtom));
  TESTMALLOC(pSelf);

  err = MP4CreateFullAtom((MP4AtomPtr)pSelf);
  if(err) goto bail;

  /* members */
  pSelf->type = MP4VolumetricVisualMediaHeader;
  pSelf->name = "VolumetricVisualMediaHeaderBox";
  /* functions */
  pSelf->createFromInputStream = (cisfunc)createFromInputStream;
  pSelf->destroy               = destroy;
  pSelf->calculateSize         = calculateSize;
  pSelf->serialize             = serialize;

  *pOut = pSelf;
bail:
  TEST_RETURN(err);
  return err;
}
