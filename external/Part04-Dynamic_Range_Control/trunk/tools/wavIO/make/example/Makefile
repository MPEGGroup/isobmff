

GENERALROOT=../../../../
MAKEROOT=../

-include $(GENERALROOT)/general/makefile.all

ifeq "$(MAKELEVEL)" "0"
SRCS = wavIO_example.c

SRCDIRS = ./
INCL    = -I../../include


# COMLIBS ##############################################################

COMLIBS += -lwavIO


# SUMMARIZE VARIABLES ##################################################

OPTIONS = SRCDIRS="$(SRCDIRS)" INCL="$(INCL)" INCLDEP=1 DEFFLAGS="$(DEFFLAGS)"

all: $(GENERALROOT)/general/makefile.all wavIO_example

wavIO_example:
	@$(MAKE) TARGET=$@ $(OPTIONS) SRCS="$(SRCS)" COMLIBS="$(COMLIBS)" binary

clean:
	@$(MAKE) TARGET=wavIO_example SRCS="$(SRCS)" clean


endif

