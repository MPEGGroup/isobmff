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
derivative works. Copyright (c) 2014.
*/

/*!
 @header StringUtils
 StringUtils provides functions, which involve strings and are commonly used by the software
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef StringUtils_h
#define StringUtils_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "Logger.h"
#include "../w32/win32.h"

/*!
 * @discussion Appends the extention string to the base string
 * @warning Additional memory will be allocated
 * @param base The base string, to which the extention string will be appended
 * @param extention The extention string, which will be attached to the base strings
 * @return Pointer to the base string
 */
static inline char*  stringAppend         (char *base, const char *extention)
{
    size_t baseLen = 0;
    if (extention == NULL)
    {
        logMsg(LOGLEVEL_WARNING, "Called stringAppend with extention = NULL");
        return base;
    }
    
    if (base != NULL)
        baseLen = strlen(base);
    else
        logMsg(LOGLEVEL_TRACE, "Called stringAppend with base = NULL");
    
    logMsg(LOGLEVEL_TRACE, "Appending \"%s\" to \"%s\"", extention, base);
    base = realloc(base, (1 + baseLen + strlen(extention) * sizeof(char)));
    base[baseLen] = '\0';
    strcat(base, extention);
    
    logMsg(LOGLEVEL_TRACE, "Result of stringAppend: \"%s\"", base);
    return base;
}

/*!
 * @discussion Checks if a string represents a series of digits or not
 * @param s String, which will be checked
 * @return Pointer to the a new string
 */
static inline bool isNumeric (const char *s)
{
    char *p;
    
    logMsg(LOGLEVEL_TRACE, "Checking if \"%s\" is numeric", s);

    if (s == NULL || *s == '\0' || isspace(*s))
    {
        logMsg(LOGLEVEL_WARNING, "\"%s\" is NOT numeric!", s);
        return false;
    }
    strtod (s, &p);
    if (*p == '\0')
    {
        logMsg(LOGLEVEL_TRACE, "\"%s\" is numeric", s);
        return true;
    }
    
    logMsg(LOGLEVEL_WARNING, "\"%s\" is NOT numeric!", s);
    return false;
}

#endif
