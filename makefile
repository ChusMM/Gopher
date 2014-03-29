#fichero makefile para compilar y enlazar
CFLAGS= -c -Wall
LNKFLAGS = -Wall
CC = gcc

DIREXEC = ./
DIRBIN = bin/
DIRLIB = ./
DIRSRC = ./

EXEC=gophermxd

all: $(EXEC)

$(EXEC) : gophermxd.o SocketPasivoTCP.o notiferror.o
	$(CC) $(LNKFLAGS) -o $(DIREXEC)$(EXEC) $(DIRBIN)gophermxd.o $(DIRBIN)SocketPasivoTCP.o $(DIRBIN)notiferror.o

gophermxd.o : $(DIRSRC)gophermxd.c $(DIRLIB)notiferror.h $(DIRLIB)SocketPasivoTCP.h
	$(CC) $(CFLAGS) -I $(DIRLIB) -o $(DIRBIN)gophermxd.o $(DIRSRC)gophermxd.c

SocketPasivoTCP.o : $(DIRSRC)SocketPasivoTCP.c $(DIRLIB)SocketPasivoTCP.h
	$(CC) $(CFLAGS) -I $(DIRLIB) -o $(DIRBIN)SocketPasivoTCP.o $(DIRSRC)SocketPasivoTCP.c

notiferror.o : $(DIRSRC)notiferror.c $(DIRLIB)notiferror.h
	$(CC) $(CFLAGS) -I $(DIRLIB) -o $(DIRBIN)notiferror.o $(DIRSRC)notiferror.c
