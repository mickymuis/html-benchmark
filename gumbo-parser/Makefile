#
# benchmark,  benchmark different HTML parsers on a large volume of HTML files
#
# This is a placeholder for the orignal build scripts from gumbo-parser
# Please see:
# https://github.com/google/gumbo-parser
#
# Micky Faas <micky@edukitty.org>
# Copyright (C) 2017  Leiden University, The Netherlands.
#

CC = gcc
CFLAGS = -Wall -std=c99 -O2
AR = ar rcs
LDFLAGS =

OBJS = attribute.o char_ref.o error.o parser.o string_buffer.o string_piece.o tag.o tokenizer.o utf8.o util.o vector.o
HEADERS = attribute.h char_ref.h error.h parser.h string_buffer.h string_piece.h tag.h tokenizer.h utf8.h util.h vector.h

all:		libgumbo.a

libgumbo.a:	$(OBJS)
		$(AR) $@ $^

%.o:		%.c $(HEADERS)
		$(CC) $(CFLAGS) -c $< -o $@

clean:
		rm -f libgumbo.a
		rm -f *.o


