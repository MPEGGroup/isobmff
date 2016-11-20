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
 @header Options
 Provides commandline parsing
 @copyright Apple
 @updated 2014-12-05
 @author Armin Trattnig
 @version 1.0
 */

#ifndef Options_h
#define Options_h

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include "StringUtils.h"

extern "C"
{
    #include "Logger.h"
}

/*!
 @enum DISOIFF_ToolMode
 @abstract Mode for choosing what feature of the tool to use
 @constant ISOIFF_ToolMode_Write Reading HEVC Bitstream and creating ISO Media Based Image File Format
 @constant ISOIFF_ToolMode_Read Reading ISO Media Based Image File Format and creating HEVC Bitstream
 @constant ISOIFF_ToolMode_NotSet Indicates that a mode has not been set
 */
typedef enum ISOIFF_ToolMode
{
    ISOIFF_ToolMode_Write,
    ISOIFF_ToolMode_Read,
    ISOIFF_ToolMode_NotSet
} ISOIFF_ToolMode;

/*!
 * @typedef Options
 * @brief Options, which can be parsed from the commandline.
 * @field mode Mode for choosing what feature of the tool to use
 * @field inputFile File, from which input data is read
 * @field outputFile File, to which the output data is written
 * @field debugLevel Debug level, controls the detail of logging output
 * @field isJustAskingForHelp Flag, for indication that the -h for help has been requested.
 */
typedef struct OptionsStruct
{
    ISOIFF_ToolMode     mode;
    char                *inputFile;
    char                *outputFile;
    int                 debugLevel;
    bool                isJustAskingForHelp;
	int					width;
	int					height;
} Options;

/*!
 * @discussion Sets the fields of an option structure to default values
 * @param options Pointer to an option structure
 */
static inline void setDefaultValues (Options *options)
{
    options->mode                               = ISOIFF_ToolMode_NotSet;
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
    printf("\nUsage:\n");
    printf("\t-h\t\tPrints this usage help.\n");
    printf("\t-m\t\tMode (mandatory) (0 = read bitstream, write file format; 1 = read file format, write bitstream)\n");
    printf("\t-i\t\tInputfile (mandatory)\n");
    printf("\t-o\t\tOutputfile (mandatory)\n");
	printf("\t-s\t\tWidth Height (mandatory if m = 0)\n");
	printf("\t-o\t\tOutputfile (mandatory)\n");
    printf("\t-d\t\tDebuglevel. 0 = No Logging, 1 = Error, 2 = Warning, 3 = Info (Default), 4 = Debug, 5 = Trace\n");
    printf("\nExample:\n");
    printf("\tReading HEVC Bitstream and creating ISO Media Based Image File Format:\n");
    printf("\tisoiff_tool -m 0 -i inputFile.bit -o outputFile.mp4 -d 3 -s 1920 1080\n");
    printf("\tReading ISO Media Based Image File Format and creating HEVC Bitstream:\n");
    printf("\tisoiff_tool -m 1 -i inputFile.mp4 -o outputFile.bit -d 3\n");
}

/*!
 * @discussion Prints the contents of the fields in options
 * @param options Pointer to an option structure
 */
static inline void printOptions     (Options *options)
{
	if (options->mode == 0)
	{
		logMsg(LOGLEVEL_INFO, "Mode:(Write) Reading HEVC Bitstream and creating ISO Media Based Image File Format");
		logMsg(LOGLEVEL_INFO, "Mode:(Write) Image dimension: %dx%d", options->width, options->height);
	}
	else if (options->mode == 1)
	{
		logMsg(LOGLEVEL_INFO, "Mode: (Read) Reading ISO Media Based Image File Format and creating HEVC Bitstream");
	}
        
    
    logMsg(LOGLEVEL_INFO, "Inputfile: %s", options->inputFile);
    logMsg(LOGLEVEL_INFO, "Outputfile: %s", options->outputFile);
}

/*!
 * @discussion Deallocates the contents of option
 * @param options Pointer to an option structure
 */
static inline void freeOptions     (Options *options)
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
static inline bool parseArguments   (int argc, char **argv, Options *options)
{
	bool imageSizeSet = false;
    for (int i = 1; i < argc; ++i )
    {
        if ( !strcmp ( argv[i], "-h" ) )
        {
            printUsage();
            options->isJustAskingForHelp = true;
            return true;
        }
        else if ( !strcmp ( argv[i], "-i" ) )
        {
            options->inputFile = stringAppend(options->inputFile, argv[i + 1]);
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
        else if ( !strcmp ( argv[i], "-m" ) )
        {
            if (!isNumeric(argv[i + 1]))
            {
                logMsg(LOGLEVEL_ERROR, "Option -m (Mode): %s is not a number!", argv[i + 1]);
                printUsage();
                return false;
            }
            options->mode = (ISOIFF_ToolMode) atoi(argv[i + 1]);
            if ((options->debugLevel < 0) && (options->debugLevel > 1))
            {
                logMsg(LOGLEVEL_ERROR, "Option -m (Mode): %s is not a valid mode (must be [0, 1])", argv[i + 1]);
                printUsage();
                return false;
            }
            i++;
            continue;
        }
		else if (!strcmp(argv[i], "-s"))
		{
			imageSizeSet = true;
			options->width = atoi(argv[i + 1]);
			i++;
			options->height = atoi(argv[i + 1]);
			i++;
			continue;
		}
        else
        {
            printUsage();
            return false;
        }
    }
    
    if (options->mode == ISOIFF_ToolMode_NotSet)
    {
        logMsg(LOGLEVEL_ERROR, "Option -m (mode) is mandatory for program execution.");
        printUsage();
        return false;
    }

	if ((options->mode == ISOIFF_ToolMode_Write) && !imageSizeSet)
	{
		logMsg(LOGLEVEL_ERROR, "Option '-s width height' is mandatory for write mode (m = 0) execution.");
		printUsage();
		return false;
	}


    
    if (options->inputFile == NULL)
    {
        logMsg(LOGLEVEL_ERROR, "Option -i (inputFile) is mandatory for program execution.");
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