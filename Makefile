.KEEP_STATE:

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

all: ofiles neo

CC = g++
CCFLAGS = -w -O3
OFILES = Genome.o Creature.o Neoterics.o NeoWindow.o # NeoPortal.o
HFILES = Genome.H Creature.H Neoterics.H NeoWindow.H
FORMS = NeoForms.o
FORMSLIBS = -L/usr/X11R6/lib -L/usr/X11/lib -L/sw/lib -lX11 -lforms -lflimage
FORMSINCLUDES = -I/usr/X11R6/include -I/sw/include
INFOFILES = README COPYING Authors xneoterics.png
#TCPLIBS = -Lsockets -ltcp++-dave
#TCPINCLUDES = -Isockets

VERSION = 0.90

ofiles: ${OFILES}

neo: ${OFILES} ${HFILES} ${FORMS}
	${CC} -o $@ ${CCFLAGS} ${OFILES} ${FORMS} ${FORMSLIBS} ${TCPLIBS} -lm 
	strip $@

dist: neo ${INFOFILES}
	mkdir xNeoterics-${VERSION}-bin
	cp neo ${INFOFILES} xNeoterics-${VERSION}-bin
	tar cf - xNeoterics-${VERSION}-bin | gzip -c >xNeoterics-${VERSION}-bin.tar.gz
	rm -rf xNeoterics-${VERSION}-bin

srcdist: ${HFILES} ${INFOFILES}
	mkdir xNeoterics-${VERSION}-src
	cp ${INFOFILES} *.h *.C *.H *.fd Makefile xNeoterics-${VERSION}-src
	tar cf - xNeoterics-${VERSION}-src | gzip -c >xNeoterics-${VERSION}-src.tar.gz
	rm -rf xNeoterics-${VERSION}-src

clean: ${OFILES}
	rm *.o

.SUFFIXES: .C .o
.C.o:
	${CC} ${FORMSINCLUDES} ${TCPINCLUDES} -c ${CCFLAGS} $<
.c.o:
	${cc} ${FORMSINCLUDES} ${TCPINCLUDES} -c ${CFLAGS} $<
