all: srv cli

CC = gcc

srv: srv.c
	$(CC) -o srv srv.c -lcrypto

cli: cli.c
	$(CC) -o cli cli.c
