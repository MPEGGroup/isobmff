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
 @header DRCtoMP4Options
 Provides commandline parsing for DRC_to_MP4.
 @copyright Apple
 @updated 2014-09-18
 @author Armin Trattnig
 @version 1.0
 */

#ifndef DRCtoMP4Options_h
#define DRCtoMP4Options_h

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "Logger.h"
#include "StringUtils.h"

/*!
 * @typedef DRCtoMP4Options
 * @brief Options, which can be parsed from the commandline.
 * @field drcBitstreamInputFile File, from which drc bitstream input data is read
 * @field outputFile File, to which the output data is written
 * @field isJustAskingForHelp Flag, for indication that the -h for help has been requested.
 */
typedef struct DRCtoMP4OptionsStruct
{
    char    *drcBitstreamInputFile;
    char    *wavInputFile;
    char    *outputFile;
    int     debugLevel;
    bool    isJustAskingForHelp;
} DRCtoMP4Options;

/*!
 * @discussion Sets the fields of an option structure to default values
 * @param options Pointer to an option structure
 */
static inline void setDefaultValues (DRCtoMP4Options *options)
{
    options->drcBitstreamInputFile              = NULL;
    options->wavInputFile                       = NULL;
    options->outputFile                         = NULL;
    options->isJustAskingForHelp                = false;
    options->debugLevel                         = logLevel;
}

/*!
 * @discussion Prints the usage of the fields in options
 */
static inline void printUsage       ()
{
    printf("\nUsage:\n");
    printf("\t-h\t\tPrints this usage help.\n");
    printf("\t-a\t\tWAV Audio Inputfile (mandatory)\n");
    printf("\t-b\t\tDRC Bitstream Inputfile (mandatory)\n");
    printf("\t-o\t\tOutputfile (mandatory)\n");
    printf("\t-d\t\tDebuglevel. 0 = No Logging, 1 = Error, 2 = Warning, 3 = Info (Default), 4 = Debug, 5 = Trace\n");
    printf("\nExample:\n");
    printf("\tDRC_to_MP4 -a audioInput.wav -b drcInputBitStream.bit -o output.mp4 -d 3\n");
}

/*!
 * @discussion Prints the contents of the fields in options
 * @param options Pointer to an option structure
 */
static inline void printOptions     (DRCtoMP4Options *options)
{
    logMsg(LOGLEVEL_INFO, "WAV Audio Inputfile: %s", options->wavInputFile);
    logMsg(LOGLEVEL_INFO, "DRC Bitstream Inputfile: %s", options->drcBitstreamInputFile);
    logMsg(LOGLEVEL_INFO, "Outputfile: %s", options->outputFile);
}

/*!
 * @discussion Parses the arguments given with the command line and sets the fields of options
 * @param argc argc from main (Number of arguments)
 * @param argv argv from main (Actual argument strings)
 * @param options Pointer to an option structure
 * @return false, if there are errors. ture, if everything was ok
 */
static inline bool parseArguments   (int argc, char **argv, DRCtoMP4Options *options)
{
    for (int i = 1; i < argc; ++i )
    {
        if ( !strcmp ( argv[i], "-h" ) )
        {
            printUsage();
            options->isJustAskingForHelp = true;
            return true;
        }
        if ( !strcmp ( argv[i], "-a" ) )
        {
            options->wavInputFile = stringAppend(options->wavInputFile, argv[i + 1]);
            i++;
            continue;
        }
        if ( !strcmp ( argv[i], "-b" ) )
        {
            options->drcBitstreamInputFile = stringAppend(options->drcBitstreamInputFile, argv[i + 1]);
            i++;
            continue;
        }
        else if ( !strcmp ( argv[i], "-o" ) )
        {
            options->outputFile = stringAppend(options->outputFile, argv[i + 1]);
            i++;
            continue;
        }
        else if ( !strcmp ( argv[i], "-d" ) )
        {
            if (!isNumeric(argv[i + 1]))
            {
                logMsg(LOGLEVEL_ERROR, "Option -d (Debuglevel): %s is not a number!", argv[i + 1]);
                printUsage();
                return false;
            }
            options->debugLevel = atoi(argv[i + 1]);
            if ((options->debugLevel < LOGLEVEL_OFF) && (options->debugLevel > LOGLEVEL_TRACE))
            {
                logMsg(LOGLEVEL_ERROR, "Option -d (Debuglevel): %s is not a valid debug level (must be [0 - 5])", argv[i + 1]);
                printUsage();
                return false;
            }
            i++;
            continue;
        }
        else
        {
            printUsage();
            return false;
        }
    }
    
    if (options->wavInputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -a (WAV Audio Inputfile) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    if (options->drcBitstreamInputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -b (DRC Bitstream Inputfile) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    if (options->outputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -o (outputfile) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    return true;
}

#endif