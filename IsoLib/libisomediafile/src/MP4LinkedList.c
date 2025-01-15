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
        $Id:
*/
#include "MP4LinkedList.h"
#include "MP4Impl.h"
#include <stdlib.h>
#include <string.h>

MP4Err MP4MakeLinkedList(MP4LinkedList *outList)
{
  MP4Err err;
  MP4LinkedList newList;
  err = MP4NoErr;

  newList = (MP4LinkedList)calloc(1, sizeof(MP4List));
  TESTMALLOC(newList)
  newList->foundEntryNumber = -1;
  newList->entryCount       = 0;
  *outList                  = newList;
bail:
  return err;
}

MP4Err MP4PrependListEntry(MP4LinkedList list, void *item)
{
  MP4Err err;
  MP4ListItemPtr entry;

  err   = MP4NoErr;
  entry = (MP4ListItemPtr)calloc(1, sizeof(MP4ListItem));
  TESTMALLOC(entry);
  entry->data = item;
  if(list->head == NULL)
  {
    list->entryCount = 1;
    list->tail       = entry;
  }
  else
  {
    list->entryCount += 1;
    entry->link = list->head;
  }
  list->head             = entry;
  list->foundEntryNumber = 0;
  list->foundEntry       = entry;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4AddListEntry(void *item, MP4LinkedList list)
{
  MP4Err err;
  MP4ListItemPtr entry;

  err = MP4NoErr;
  if(list == NULL)
  {
    BAILWITHERROR(MP4BadParamErr);
  }
  entry = (MP4ListItemPtr)calloc(1, sizeof(MP4ListItem));
  TESTMALLOC(entry);
  entry->data = item;
  if(list->head == NULL)
  {
    list->head       = entry;
    list->entryCount = 1;
  }
  else
  {
    list->entryCount += 1;
    list->tail->link = entry;
  }
  list->tail             = entry;
  list->foundEntryNumber = list->entryCount - 1;
  list->foundEntry       = entry;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4DeleteLinkedList(MP4LinkedList list)
{
  MP4Err err;
  MP4ListItemPtr entry;
  err = MP4NoErr;
  if(list == NULL) BAILWITHERROR(MP4BadParamErr);
  if(list->head != NULL)
  {
    entry = list->head;
    do
    {
      MP4ListItemPtr lnk = entry->link;
      free(entry);
      entry = lnk;
    } while(entry);
  }
  /*
          else
                  free( list );
  */
  free(list);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4GetListEntryCount(MP4LinkedList list, u32 *outCount)
{
  MP4Err err;
  err = MP4NoErr;

  if((list == 0) || (outCount == 0))
  {
    BAILWITHERROR(MP4BadParamErr)
  }
  *outCount = list->entryCount;
bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4GetListEntry(MP4LinkedList list, u32 itemNumber, char **outItem)
{
  MP4Err err;
  MP4ListItemPtr entry;
  u32 i;
  err = MP4NoErr;
  if((list == NULL) || (outItem == NULL) || (itemNumber >= list->entryCount))
    BAILWITHERROR(MP4BadParamErr)

  if(itemNumber < (u32)list->foundEntryNumber)
  {
    /* if cached entry is too far, find the head */
    list->foundEntryNumber = 0;
    list->foundEntry       = list->head;
  }
  entry = list->foundEntry;
  for(i = list->foundEntryNumber; i < itemNumber; i++)
  {
    assert(entry->link != NULL);
    entry = entry->link;
  }
  list->foundEntryNumber = itemNumber;
  list->foundEntry       = entry;
  *outItem               = (char *)entry->data;
  if(list->foundEntryNumber == 0)
  {
    assert(list->foundEntry == list->head);
  }
bail:
  TEST_RETURN(err);
  return err;
}

MP4Err MP4DeleteListEntry(MP4LinkedList list, u32 itemNumber)
{
  MP4Err err;
  MP4ListItemPtr entry, lnk, previous;
  u32 i;
  err = MP4NoErr;
  if((list == NULL) || (itemNumber >= list->entryCount)) BAILWITHERROR(MP4BadParamErr);

  entry    = list->head;
  previous = list->head;

  for(i = 0; i < itemNumber; i++)
  {
    assert(entry->link != NULL);
    previous = entry;
    entry    = entry->link;
  }

  lnk = entry->link;
  free(entry);

  if(itemNumber == 0) list->head = lnk;
  else
    previous->link = lnk;

  if(itemNumber == list->entryCount - 1) list->tail = previous;

  list->foundEntryNumber = 0;
  list->foundEntry       = list->head;
  list->entryCount -= 1;

bail:
  TEST_RETURN(err);

  return err;
}
