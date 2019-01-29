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
 @header CompareWAVOptions
 Provides commandline parsing for CompareWAV.
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef CompareWAVOptions_h
#define CompareWAVOptions_h

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "Logger.h"
#include "StringUtils.h"

enum {
    WAV_A,
    WAV_B,
    N_WAV
};

/*!
 * @typedef CompareWAVOptions
 * @brief Options, which can be parsed from the commandline.
 * @field inputFileA File, from which PCM input data is read
 * @field inputFileB File, to which the output data is written
 * @field isJustAskingForHelp Flag, for indication that the -h for help has been requested.
 */
typedef struct CompareWAVOptionsStruct
{
    char    *inputFile[N_WAV];
    int     debugLevel;
    bool    isJustAskingForHelp;
} CompareWAVOptions;

/*!
 * @discussion Sets the fields of an option structure to default values
 * @param options Pointer to an option structure
 */
static inline void setDefaultValues (CompareWAVOptions *options)
{
    for (int i=0; i<N_WAV; i++) {
        options->inputFile[i]                   = NULL;
    }
    options->isJustAskingForHelp                = false;
    options->debugLevel                         = logLevel;
}

/*!
 * @discussion Prints the usage of the fields in options
 */
static inline void printUsage       ()
{
    printf("\n=============================================\n");
    printf("WAV file comparsion tool for ISO/IEC 23003-5\n");
    printf("Version 0.1 (2019)\n");
    printf("Purpose: verification of reference code output file.");
    printf("\nUsage:\n");
    printf("\t-h\t\tPrints this usage help.\n");
    printf("\t-a\t\tWAV input file A (mandatory)\n");
    printf("\t-b\t\tWAV intput file B (mandatory)\n");
    printf("\t-d\t\tDebuglevel. 0 = No Logging, 1 = Error, 2 = Warning, 3 = Info (Default), 4 = Debug, 5 = Trace\n");
    printf("\nExample:\n");
    printf("\tCompareWAV -a ref.wav -b test.wav -d 3\n");
    printf("\n=============================================\n");
}

/*!
 * @discussion Prints the contents of the fields in options
 * @param options Pointer to an option structure
 */
static inline void printOptions     (CompareWAVOptions *options)
{
    logMsg(LOGLEVEL_INFO, "WAV input file A: %s", options->inputFile[0]);
    logMsg(LOGLEVEL_INFO, "WAV input file B: %s", options->inputFile[1]);
}

/*!
 * @discussion Deallocates the contents of option
 * @param options Pointer to an option structure
 */
static inline void freeOptions     (CompareWAVOptions *options)
{
    for (int i=0; i<N_WAV; i++) {
        free(options->inputFile[i]);
    }
}

/*!
 * @discussion Parses the arguments given with the command line and sets the fields of options
 * @param argc argc from main (Number of arguments)
 * @param argv argv from main (Actual argument strings)
 * @param options Pointer to an option structure
 * @return false, if there are errors. ture, if everything was ok
 */
static inline bool parseArguments   (int argc, const char **argv, CompareWAVOptions *options)
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
            options->inputFile[WAV_A] = stringAppend(options->inputFile[WAV_A], argv[i + 1]);
            i++;
            continue;
        }
        if ( !strcmp ( argv[i], "-b" ) )
        {
            options->inputFile[WAV_B] = stringAppend(options->inputFile[WAV_B], argv[i + 1]);
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
    
    if (options->inputFile[WAV_A] == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -a (input file A) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    if (options->inputFile[WAV_B] == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -b (input file B) is mandatory for program execution.");
        printUsage();
        return false;
    }
    
    return true;
}

#endif
