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
 @header Logger
Logger provides basic logging mechanisms. The desired loglevel and output can be 
 set.
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef Logger_h
#define Logger_h

/*!
 @definedblock Loglevels supported by Logger
 @abstract Loglevels determine the amount of log created by the software
 @discussion All messages will be logged with logLvl < logLevel
 @define LOGLEVEL_OFF Logging is completely disabled
 @define LOGLEVEL_ERROR Errors, which are critcal to program execution will be logged
 @define LOGLEVEL_WARNING Warning messages are logged, which inform the user of unexpected results
 @define LOGLEVEL_INFO Messages, which provide general information about what is going on
 @define LOGLEVEL_DEBUG More detailed description of what is happening
 @define LOGLEVEL_TRACE Extremely detailed description of what is happening
 */

#define LOGLEVEL_OFF        0
#define LOGLEVEL_ERROR      1
#define LOGLEVEL_WARNING    2
#define LOGLEVEL_INFO       3
#define LOGLEVEL_DEBUG      4
#define LOGLEVEL_TRACE      5

/*! @/definedblock */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

/*!
Global variable logLevel. Can be set anywhere in the program. Represents the current log level.
 */
extern int      logLevel;
/*!
Global variable logOutput. Can be set anywhere in the program. File pointer to the output of the log messages. 
 */
extern FILE     *logOutput;

/*!
 * @discussion Writes a log message to logOutput with the desired loglevel
 * @warning Global variables logLevel and logOutput must be initialized before calling logMsg
 * @param logLvl Loglevel for the message, use the defined Loglevels
 * @param format Message, working exactly like printf(format, ...)
 */
extern void     logMsg      (int logLvl, const char* format, ... );

#endif
