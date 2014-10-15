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
 @header MP4toDRCOptions
 Provides commandline parsing for MP4_to_DRC.
 @copyright Apple
 @updated 2014-10-06
 @author Armin Trattnig
 @version 1.0
 */

#ifndef MP4toDRCOptions_h
#define MP4toDRCOptions_h

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "Logger.h"
#include "StringUtils.h"

/*!
 * @typedef MP4toDRCOptions
 * @brief Options, which can be parsed from the commandline.
 * @field inputFile MP4 input file
 * @field wavOutputFile File, to which the output wav data is written
 * @field drcOutputFile File, to which the output drc bitstream data is written
 * @field debugLevel Determines the level of detail of the logging output
 * @field isJustAskingForHelp Flag, for indication that the -h for help has been requested.
 */
typedef struct MP4toDRCOptionsStruct
{
    char    *inputFile;
    char    *wavOutputFile;
    char    *drcOutputFile;
    int     audioTrackNumber;
    int     drcTrackNumber;
    int     debugLevel;
    bool    isJustAskingForHelp;
} MP4toDRCOptions;

/*!
 * @discussion Sets the fields of an option structure to default values
 * @param options Pointer to an option structure
 */
static inline void setDefaultValues (MP4toDRCOptions *options)
{
    options->inputFile                          = NULL;
    options->wavOutputFile                      = NULL;
    options->drcOutputFile                      = NULL;
    options->audioTrackNumber                   = 1;
    options->drcTrackNumber                     = 2;
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
    printf("\t-i\t\tMP4 input file(mandatory)\n");
    printf("\t-b\t\tDRC bitstream output file (mandatory)\n");
    printf("\t-a\t\tWAV output file (mandatory)\n");
    printf("\t-t\t\tTrack number of the audio track. (default = 1)\n");
    printf("\t-m\t\tTrack number of the DRC metadata track. (default = 2)\n");
    printf("\t-d\t\tDebuglevel. 0 = No Logging, 1 = Error, 2 = Warning, 3 = Info (Default), 4 = Debug, 5 = Trace\n");
    printf("\nExample:\n");
    printf("\tMP4_to_DRC -i inputFile.mp4 -a wavOutputFile.wav -b drcOutputBitStream.bit -d 3\n");
}

/*!
 * @discussion Prints the contents of the fields in options
 * @param options Pointer to an option structure
 */
static inline void printOptions     (MP4toDRCOptions *options)
{
    logMsg(LOGLEVEL_INFO, "MP4 Inputfile: %s", options->inputFile);
    logMsg(LOGLEVEL_INFO, "Audiotrack Number: %d", options->audioTrackNumber);
    logMsg(LOGLEVEL_INFO, "DRC track Number: %d", options->drcTrackNumber);
    logMsg(LOGLEVEL_INFO, "WAV audio output file: %s", options->wavOutputFile);
    logMsg(LOGLEVEL_INFO, "DRC bitstream output file: %s", options->drcOutputFile);
}

/*!
 * @discussion Parses the arguments given with the command line and sets the fields of options
 * @param argc argc from main (Number of arguments)
 * @param argv argv from main (Actual argument strings)
 * @param options Pointer to an option structure
 * @return false, if there are errors. ture, if everything was ok
 */
static inline bool parseArguments   (int argc, char **argv, MP4toDRCOptions *options)
{
    for (int i = 1; i < argc; ++i )
    {
        if ( !strcmp ( argv[i], "-h" ) )
        {
            printUsage();
            options->isJustAskingForHelp = true;
            return true;
        }
        if ( !strcmp ( argv[i], "-i" ) )
        {
            options->inputFile = stringAppend(options->inputFile, argv[i + 1]);
            i++;
            continue;
        }
        if ( !strcmp ( argv[i], "-b" ) )
        {
            options->drcOutputFile = stringAppend(options->drcOutputFile, argv[i + 1]);
            i++;
            continue;
        }
        else if ( !strcmp ( argv[i], "-a" ) )
        {
            options->wavOutputFile = stringAppend(options->wavOutputFile, argv[i + 1]);
            i++;
            continue;
        }
        else if ( !strcmp ( argv[i], "-t" ) )
        {
            if (!isNumeric(argv[i + 1]))
            {
                logMsg(LOGLEVEL_ERROR, "Option -t (Audio Track Number): %s is not a number!", argv[i + 1]);
                printUsage();
                return false;
            }
            options->audioTrackNumber = atoi(argv[i + 1]);
            i++;
            continue;
        }
        else if ( !strcmp ( argv[i], "-m" ) )
        {
            if (!isNumeric(argv[i + 1]))
            {
                logMsg(LOGLEVEL_ERROR, "Option -m (DRC Track Number): %s is not a number!", argv[i + 1]);
                printUsage();
                return false;
            }
            options->drcTrackNumber = atoi(argv[i + 1]);
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
    
    if (options->inputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -a (WAV Audio Inputfile) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    if (options->wavOutputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -b (DRC Bitstream Inputfile) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    if (options->drcOutputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -o (outputfile) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    return true;
}

#endif