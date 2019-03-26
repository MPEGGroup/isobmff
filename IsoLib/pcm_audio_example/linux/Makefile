# $Id: Makefile,v 1.2 2014/10/14 at Exp $

VPATH = . ../src
ISOLIBPATH = ../../libisomediafile
DRCREFSWPATH = ../../Part04-Dynamic_Range_Control/trunk

CFLAGS = -I. -I../src -I$(ISOLIBPATH)/linux -I$(ISOLIBPATH)/src -I$(DRCREFSWPATH)/tools/wavIO/include -O -g -Wall -w -std=c99

CC = gcc

SOURCES = $(wildcard ../src/*.c)
OBJECTS = $(SOURCES:.c=.o)
WAV_to_MP4_OBJECTS = $(filter-out ../src/MP4_to_WAV.o,$(OBJECTS))
MP4_to_WAV_OBJECTS = $(filter-out ../src/WAV_to_MP4.o,$(OBJECTS))


libisomediafile = $(ISOLIBPATH)/linux/libisomediafile/libisomediafile.a
libwavIO = $(DRCREFSWPATH)/tools/wavIO/lib/Linux_x86_64_D_O0/libwavIO.a

all:	MP4_to_WAV WAV_to_MP4

WAV_to_MP4:	$(WAV_to_MP4_OBJECTS) $(libisomediafile) $(libwavIO)
	$(CC) $(CFLAGS) -o WAV_to_MP4 $(WAV_to_MP4_OBJECTS) $(libisomediafile) $(libwavIO) -lm

MP4_to_WAV:	$(MP4_to_WAV_OBJECTS) $(libisomediafile) $(libwavIO)
	$(CC) $(CFLAGS) -o MP4_to_WAV $(MP4_to_WAV_OBJECTS) $(libisomediafile) $(libwavIO) -lm

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

