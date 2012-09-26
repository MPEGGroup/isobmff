# $Id: Makefile,v 1.1 2002/09/27 13:38:51 julien Exp $

VPATH = . ../src

CFLAGS = -I. -I../src -I../../libisomediafile/linux -I../../libisomediafile/src -O -g -Wall

CC = gcc

protectAudioMovie_src = protectAudioMovie.c

protectAudioMovie_obj := $(protectAudioMovie_src:.c=.o)

libisomediafile = ../../libisomediafile/linux/libisomediafile/libisomediafile.a

all:	protectAudioMovie

protectAudioMovie:	$(protectAudioMovie_obj) $(libisomediafile)
	$(CC) -o protectAudioMovie $(protectAudioMovie_obj) $(libisomediafile)

clean:
	rm -f protectAudioMovie*

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

#
# Include all dependency files
#
-include $(protectAudioMovie_src:.c=.d)
