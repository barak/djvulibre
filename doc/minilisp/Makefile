SHELL=/bin/sh

MINIEXPDIR=../../libdjvu

VPATH=${MINIEXPDIR}

OPTS=-g -O2

CXXFLAGS=-Wall ${OPTS} -I${MINIEXPDIR}

all: minilisp

minilisp: minilisp.o miniexp.o
	${CXX} ${CXXFLAGS} -o $@ minilisp.o miniexp.o

miniexp.o: miniexp.cpp

minilisp.o: minilisp.cpp miniexp.h

clean: FORCE
	-rm minilisp minilisp.o miniexp.o

FORCE:

