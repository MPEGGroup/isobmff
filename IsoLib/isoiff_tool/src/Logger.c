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
 * or derivative works. Copyright (c) 2014.
 */

#include "Logger.h"

int logLevel    = LOGLEVEL_INFO;
FILE *logOutput = NULL;

void logMsg(int logLvl, const char *format, ...)
{
  if(logLvl > logLevel) return;

  switch(logLvl)
  {
  case LOGLEVEL_ERROR:
    fprintf(logOutput, "\nError: ");
    break;
  case LOGLEVEL_WARNING:
    fprintf(logOutput, "Warning: ");
    break;
  case LOGLEVEL_INFO:
    fprintf(logOutput, "Info: ");
    break;
  case LOGLEVEL_DEBUG:
    fprintf(logOutput, "Debug: ");
    break;
  case LOGLEVEL_TRACE:
    fprintf(logOutput, "Trace: ");
    break;
  }

  va_list args;
  va_start(args, format);
  vfprintf(logOutput, format, args);
  va_end(args);

  fprintf(logOutput, "\n");
}
