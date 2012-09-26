# $Id: Makefile,v 1.2 1999/11/20 22:59:39 mc Exp $

VPATH = . ../src

CFLAGS = -I. -I../src -I../../libisomediafile/osxs -I../../libisomediafile/src -O -g -Wall -ansi -pedantic -Wno-long-long

CC = cc

playAudioMovieSample_src = playAudioMovieSample.c

playAudioMovieSample_obj := $(playAudioMovieSample_src:.c=.o)

makeAudioMovieSample_src = makeAudioMovieSample.c

makeAudioMovieSample_obj := $(makeAudioMovieSample_src:.c=.o)

libisomediafile = ../../libisomediafile/osxs/libisomediafile/libisomediafile.a

all:	makeAudioMovieSample playAudioMovieSample

makeAudioMovieSample:	$(makeAudioMovieSample_obj) $(libisomediafile)
	$(CC) -o makeAudioMovieSample $(makeAudioMovieSample_obj) $(libisomediafile)

playAudioMovieSample:	$(playAudioMovieSample_obj) $(libisomediafile)
	$(CC) -o playAudioMovieSample $(playAudioMovieSample_obj) $(libisomediafile)

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

clean:
	-rm $(makeAudioMovieSample_obj) $(makeAudioMovieSample_src:.c=.d) $(playAudioMovieSample_obj) $(playAudioMovieSample_src:.c=.d) makeAudioMovieSample playAudioMovieSample

#
# Include all dependency files
#
-include $(playAudioMovieSample_src:.c=.d)
-include $(makeAudioMovieSample_src:.c=.d)
