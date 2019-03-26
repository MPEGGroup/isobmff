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
derivative works. Copyright (c) 2019.
*/

/*!
 @header MP4ToWAVOptions
 Provides commandline parsing for MP4_to_WAV.
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef MP4ToWAVOptions_h
#define MP4ToWAVOptions_h

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "Logger.h"
#include "StringUtils.h"

/*!
 * @typedef MP4ToWAVOptions
 * @brief Options, which can be parsed from the commandline.
 * @field inputFile File, from which PCM input data is read
 * @field outputFile File, to which the output data is written
 * @field isJustAskingForHelp Flag, for indication that the -h for help has been requested.
 */
typedef struct WAVToMP4OptionsStruct
{
    char    *inputFile;
    char    *outputFile;
    int     debugLevel;
    bool    isJustAskingForHelp;
} WAVToMP4Options;

/*!
 * @discussion Sets the fields of an option structure to default values
 * @param options Pointer to an option structure
 */
static inline void setDefaultValues (WAVToMP4Options *options)
{
    options->inputFile                          = NULL;
    options->outputFile                         = NULL;
    options->isJustAskingForHelp                = false;
    options->debugLevel                         = logLevel;
}

/*!
 * @discussion Prints the usage of the fields in options
 */
static inline void printUsage       ()
{
    printf("\n=============================================\n");
    printf("WAV to MPEG-4 File Format Converter\n");
    printf("based on ISO/IEC 14496-12 and ISO/IEC 23003-5\n");
    printf("Version 0.1 (2019)\n");
    printf("Purpose: Convert a mono or stereo WAV file into an MP4 file.");
    printf("\nUsage:\n");
    printf("\t-h\t\tPrints this usage help.\n");
    printf("\t-i\t\tWAV input file (mandatory)\n");
    printf("\t-o\t\tMPEG-4 output file (mandatory)\n");
    printf("\t-d\t\tDebuglevel. 0 = No Logging, 1 = Error, 2 = Warning, 3 = Info (Default), 4 = Debug, 5 = Trace\n");
    printf("\nExample:\n");
    printf("\tWAV_to_MP4 -i output.mp4 -o input.wav -d 3\n");
}

/*!
 * @discussion Prints the contents of the fields in options
 * @param options Pointer to an option structure
 */
static inline void printOptions     (WAVToMP4Options *options)
{
    logMsg(LOGLEVEL_INFO, "WAV audio inputfile: %s", options->inputFile);
    logMsg(LOGLEVEL_INFO, "MPEG-4 output file: %s", options->outputFile);
}

/*!
 * @discussion Deallocates the contents of option
 * @param options Pointer to an option structure
 */
static inline void freeOptions     (WAVToMP4Options *options)
{
    free(options->inputFile);
    free(options->outputFile);
}

/*!
 * @discussion Parses the arguments given with the command line and sets the fields of options
 * @param argc argc from main (Number of arguments)
 * @param argv argv from main (Actual argument strings)
 * @param options Pointer to an option structure
 * @return false, if there are errors. ture, if everything was ok
 */
static inline bool parseArguments   (int argc, char **argv, WAVToMP4Options *options)
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
        if ( !strcmp ( argv[i], "-o" ) )
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
    
    if (options->inputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -i (input file) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    if (options->outputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -o (output file) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    return true;
}

#endif
