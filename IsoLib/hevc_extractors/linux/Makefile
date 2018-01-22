# $Id: Makefile,v 1.2 1999/11/20 08:19:15 mc Exp $

VPATH = . ../src

CC = gcc
CXX = g++

CPPFLAGS = -I. -I../src -I../../libisomediafile/linux -I../../libisomediafile/src -O -g -Wall

SOURCES = hevc_extractors.cpp HEVCExtractorReader.cpp

OBJECTS = $(SOURCES:.cpp=.o)

LIBPATHS = -L../../libisomediafile/linux/libisomediafile

all:	hevc_extractors

hevc_extractors:	$(OBJECTS)
	$(CXX) $(CPPFLAGS) $(LIBPATHS) -o extractor_player $(OBJECTS) -lisomediafile
	-rm $(OBJECTS)
