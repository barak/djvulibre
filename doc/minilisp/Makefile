SHELL=/bin/sh

MINIEXPDIR=../../libdjvu

VPATH=${MINIEXPDIR}

CXXFLAGS=-Wall -g -O2 -I${MINIEXPDIR}

all: minilisp

minilisp: minilisp.o miniexp.o
	${CXX} -o $@ minilisp.o miniexp.o

miniexp.o: miniexp.cpp

minilisp.o: minilisp.cpp miniexp.h

clean: FORCE
	-rm minilisp minilisp.o miniexp.o

FORCE:

