

GENERALROOT=../../../
MAKEROOT=../

-include $(GENERALROOT)/general/makefile.all

ifeq "$(MAKELEVEL)" "0"
SRCS = readonlybitbuf.c

SRCDIRS = ../src
INCL    = -I../include

OPTIONS = SRCDIRS="$(SRCDIRS)" INCL="$(INCL)" INCLDEP=1 DEFFLAGS="$(DEFFLAGS)"

all: $(GENERALROOT)/general/makefile.all libreadonlybitbuf$(LIB_SUFFIX)

libreadonlybitbuf$(LIB_SUFFIX):
	@$(MAKE) TARGET=$@ $(OPTIONS) SRCS="$(SRCS)" COMLIBS="$(LIB_COMLIBS)" library

clean:
	@$(MAKE) TARGET=libreadonlybitbuf$(LIB_SUFFIX) SRCS="$(SRCS)" clean


endif

