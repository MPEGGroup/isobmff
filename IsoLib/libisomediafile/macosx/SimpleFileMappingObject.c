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
        $Id: SimpleFileMappingObject.c,v 1.1 2002/10/08 10:27:59 fred Exp $
*/
#include "SimpleFileMappingObject.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>

#define DO_MAP 1

#define WINDOW_RESIDUE 0x02000000
#define WINDOW_BITS 0xFFFFFFFFFF000000ll

static MP4Err doClose(struct FileMappingObjectRecord *s);
static MP4Err doOpen(struct FileMappingObjectRecord *s, const char *filename);
static MP4Err isYourFile(struct FileMappingObjectRecord *s, const char *filename, u32 *outSameFile);
static MP4Err copyData(struct FileMappingObjectRecord *s, u64 offset, char *dst, u32 bytes);

static MP4Err destroy(struct FileMappingObjectRecord *s)
{
  MP4Err err;
  SimpleFileMappingObject self = (SimpleFileMappingObject)s;

  err = MP4NoErr;
  if(self->data) err = doClose(s);
  if(err) goto bail;
  free(s);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err doOpen(struct FileMappingObjectRecord *s, const char *pathname)
{
  MP4Err err;
  struct stat info;
  char *separator;

  SimpleFileMappingObject self = (SimpleFileMappingObject)s;
  err                          = MP4NoErr;

  self->fd = open(pathname, O_RDONLY);
  if(self->fd < 0) BAILWITHERROR(MP4FileNotFoundErr);
  self->fileName = (char *)calloc(1, strlen(pathname) + 1);
  if(self->fileName == NULL) BAILWITHERROR(MP4NoMemoryErr)
  strcpy(self->fileName, pathname);

  if(fstat(self->fd, &info) < 0) BAILWITHERROR(MP4IOErr);
  self->size64 = info.st_size;
#ifdef DO_MAP
  self->window = 0;
  if(self->size64 > WINDOW_RESIDUE) self->window_size = WINDOW_RESIDUE;
  else
    self->window_size = self->size64;

  self->data =
    mmap(0, self->window_size, PROT_READ, MAP_FILE + MAP_PRIVATE, self->fd, self->window);

  if(!(self->data) || (self->data == MAP_FAILED))
  { /* printf("Error was %d %s\n",errno,strerror( errno )); */
    BAILWITHERROR(MP4IOErr);
  }
#else
  if(info.st_size >> 32)
  {
    BAILWITHERROR(MP4NotImplementedErr);
  }
  self->size64 = info.st_size;
  self->data   = (char *)malloc(self->size64);
  if(self->data == NULL) BAILWITHERROR(MP4IOErr);
  if(read(self->fd, self->data, self->size64) != ((s32)self->size64))
  {
    BAILWITHERROR(MP4IOErr);
  }
#endif
  /* make parent */
  if((separator = strrchr(pathname, '\\')) != 0 || (separator = strrchr(pathname, '/')) != 0)
  {
    int len      = separator - pathname;
    self->parent = (char *)calloc(1, len + 2);
    TESTMALLOC(self->parent);
    strncpy(self->parent, pathname, len);
    strcat(self->parent, "/");
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err doClose(struct FileMappingObjectRecord *s)
{
  MP4Err err;
  SimpleFileMappingObject self = (SimpleFileMappingObject)s;

  err = MP4NoErr;
  if(self->data)
  {
#ifdef DO_MAP
    err = munmap(self->data, self->window_size);
    if(err)
    {
      err = MP4IOErr;
    }
#else
    free(self->data);
#endif
  }
  self->data = NULL;
  if(self->fileName) free(self->fileName);
  self->fileName = NULL;
  if(self->parent) free(self->parent);
  self->parent = NULL;

  close(self->fd);

  return err;
}

static MP4Err isYourFile(struct FileMappingObjectRecord *s, const char *filename, u32 *outSameFile)
{
  MP4Err err;
  SimpleFileMappingObject self = (SimpleFileMappingObject)s;
  int result;
  err = MP4NoErr;
#if defined(_MSC_VER)
  result = _stricmp(self->fileName, filename);
#else
  result = strcmp(self->fileName, filename);
#endif
  *outSameFile = result ? 0 : 1;
  return err;
}

static MP4Err copyData(struct FileMappingObjectRecord *s, u64 offset, char *dst, u32 bytes)
{
  MP4Err err;
  SimpleFileMappingObject self = (SimpleFileMappingObject)s;

  err = MP4NoErr;

  if((offset + bytes) > self->size64)
  {
    BAILWITHERROR(MP4IOErr);
  }

#ifdef DO_MAP
  if(self->window != (u64)(offset & WINDOW_BITS))
  {
    err = munmap(self->data, self->window_size);
    if(err)
    {
      BAILWITHERROR(MP4IOErr);
    }
    self->window = (offset & WINDOW_BITS);

    if((self->size64 - self->window) > WINDOW_RESIDUE) self->window_size = WINDOW_RESIDUE;
    else
      self->window_size = (self->size64 - self->window);

    self->data =
      mmap(0, self->window_size, PROT_READ, MAP_FILE + MAP_PRIVATE, self->fd, self->window);
    if(!(self->data) || (self->data == MAP_FAILED))
    {
      BAILWITHERROR(MP4IOErr);
    }
  }

  offset -= self->window;
  if((offset + bytes) > self->window_size)
  {
    u32 len;
    len = self->window_size - offset;

    memcpy(dst, self->data + offset, len);
    err = copyData(s, self->window + self->window_size, dst + len, bytes - len);
  }
  else
  {
    memcpy(dst, self->data + offset, bytes);
  }
#else
  memcpy(dst, self->data + offset, bytes);
#endif

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSimpleFileMappingObject(char *filename, FileMappingObject *outObject)
{
  MP4Err err;
  SimpleFileMappingObject self;

  err  = MP4NoErr;
  self = (SimpleFileMappingObject)calloc(1, sizeof(SimpleFileMappingObjectRecord));
  TESTMALLOC(self);
  self->destroy    = destroy;
  self->open       = doOpen;
  self->close      = doClose;
  self->isYourFile = isYourFile;
  self->copyData   = copyData;
  self->window     = 0;

  err = doOpen((FileMappingObject)self, filename);
  if(err) goto bail;
  *outObject = (FileMappingObject)self;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateFileMappingObject(char *urlString, FileMappingObject *outObject)
{
  char *pathname = urlString;
  if((strncmp(urlString, "file://", 7) == 0) || (strncmp(urlString, "file|//", 7) == 0))
  {
    pathname += 7;
  }
  return MP4CreateSimpleFileMappingObject(pathname, outObject);
}

MP4Err MP4AssertFileExists(char *pathName)
{
  MP4Err err;
  struct stat buf;

  err = stat(pathName, &buf);
  if(err)
  {
    BAILWITHERROR(MP4FileNotFoundErr);
  }
bail:
  return err;
}
