CC = gcc
SDIR = ./src

all: not-grep

not-grep: $(SDIR)/not-grep.c $(SDIR)/regex_to_nfa.c
	$(CC) $(CFLAGS) -o not-grep $^