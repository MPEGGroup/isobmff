# $Id: Makefile,v 1.2 2014/10/14 at Exp $

VPATH = . ../../src

CFLAGS = -I. -I../../src -I../../../libisomediafile/linux -I../../../libisomediafile/src -O -g -Wall -w -std=c99
CC = gcc

ISOIFF_SOURCES = \
	Logger.c \
	isoiff.c \
	HEVCDecoderConfigRecord.c \
	HEVCItemData.c \
	HEVCConfigurationAtom.c \
	ImageSpatialExtentsPropertyAtom.c
ISOIFF_OBJECTS = $(ISOIFF_SOURCES:.c=.o)

libisomediafile = ../../../libisomediafile/linux/libisomediafile/libisomediafile.a


all:	ISOIFF

ISOIFF:	$(ISOIFF_OBJECTS)
	ar r libisoiff.a  $(ISOIFF_OBJECTS)
	-ranlib libisoiff.a
	-rm $(ISOIFF_OBJECTS)
%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

