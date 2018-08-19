BINDIR = ./tmpbin
all: build

shelfio.o : shelfio.c shelfio.h
	gcc -c -o $(BINDIR)/shelfio.o shelfio.c

shelf.o : shelf.c
	gcc -c -o $(BINDIR)/shelf.o shelf.c

es : shelf.o shelfio.o
	gcc -o es $(BINDIR)/shelf.o $(BINDIR)/shelfio.o

build: shelf.o shelfio.o es
