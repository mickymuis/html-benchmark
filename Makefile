#
# benchmark,  benchmark different HTML parsers on a large volume of HTML files
#
# Micky Faas <micky@edukitty.org>
# Copyright (C) 2017  Leiden University, The Netherlands.
#

# for gumbo-parser
GUMBO_DIR = gumbo-parser
.PHONY: gumbo

# for MyHTML parser
MYHTML_DIR = myhtml
.PHONY: myhtml

# for Lmth parser
HAUT_DIR = haut
.PHONY: haut

# General flags
CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g -I/usr/include/libxml2 -I$(MYHTML_DIR)/include
LDFLAGS = -lpthread -lxml2 -lgumbo -lmyhtml_static -lhaut -L$(GUMBO_DIR) -L$(MYHTML_DIR)/lib -L$(HAUT_DIR)

OBJS = main.o benchmark.o util.o test_dummy.o test_hsp.o test_libxml2.o test_gumbo.o test_myhtml.o test_haut.o htmlstreamparser/htmlstreamparser.o
HEADERS = benchmark.h util.h test_dummy.h test_hsp.h test_libxml2.h test_gumbo.h test_myhtml.h test_haut.h htmlstreamparser/htmlstreamparser.h


all:		gumbo myhtml haut benchmark

.PHONY: benchmark
benchmark:	$(OBJS)
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o:		%.c $(HEADERS)
		$(CC) $(CFLAGS) -c $< -o $@

gumbo:
	$(MAKE) -C $(GUMBO_DIR)

myhtml:
	$(MAKE) -C $(MYHTML_DIR) static

haut:
	$(MAKE) -C $(HAUT_DIR)

clean:
		rm -f benchmark
		rm -f *.o htmlstreamparser/*.o
		$(MAKE) -C $(GUMBO_DIR) clean
		$(MAKE) -i -C $(MYHTML_DIR) clean
		$(MAKE) -C $(HAUT_DIR) clean

