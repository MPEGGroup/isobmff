# $Id: Makefile,v 1.2 2014/10/14 at Exp $

VPATH = . ../../src

CC = gcc
CXX = g++

CPPFLAGS = -I. -I../../src -I../../../libisomediafile/linux -I../../../libisomediafile/src -I../../../HM/source/Lib -I../../../HM/source/Lib/TLibCommon -I../../../HM/source/Lib/TLibDecoder -O -g -Wall -w

SOURCES = \
	StringUtils.cpp \
	Options.cpp \
	hevc_tool.cpp \
	isoiff_tool_main.cpp
OBJECTS = $(SOURCES:.cpp=.o)

LIBPATHS = -L../isoiff -L../../../libisomediafile/linux/libisomediafile -L../../../HM/lib

all:	isoiff_tool

isoiff_tool:	$(OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBPATHS) -o isoiff_tool $(OBJECTS) -lisoiff -lisomediafile  -lTLibDecoderStatic -lTLibCommonStatic 
	-rm $(OBJECTS)

