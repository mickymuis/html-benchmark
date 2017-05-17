#
# libhaut, Lightweight HTML tokenizer
#
# Micky Faas <micky@edukitty.org>
# Copyright (C) 2017  Leiden University, The Netherlands.
#

CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g
AR = ar rcs
LDFLAGS =

OBJS = haut.o string_util.o state_machine.o
HEADERS = haut.h string_util.h state_machine.h parser_transitions.h lexer_transitions.h state.h

all:		libhaut.a

libhaut.a:	$(OBJS)
		$(AR) $@ $^

%.o:		%.c $(HEADERS)
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -f libhaut.a
		rm -f *.o


