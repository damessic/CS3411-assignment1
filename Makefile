CC=gcc
CFLAGS=-g -ansi -pedantic-errors -Wall

default: all

all: ctar utar

ctar: ctar.o
	$(CC) -o ctar ctar.o

ctar.o: ctar.c
	$(CC) $(CFLAGS) -c ctar.c

utar: utar.o
	$(CC) -o utar utar.o

utar.o: utar.c
	$(CC) $(CFLAGS) -c utar.c

.PHONY: clean

clean:
	rm -rf ctar ctar.o utar utar.o
