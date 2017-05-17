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

# for Haut parser
HAUT_DIR = haut
.PHONY: haut

# for libxml2
LIBXML2_DIR = libxml2
.PHONY: libxml2

# General flags
CC = gcc
CFLAGS = -Wall -std=c99 -O2 -g -I$(LIBXML2_DIR)/include -I$(MYHTML_DIR)/include
LDFLAGS = -lpthread -lxml2 -lgumbo -lmyhtml_static -lhaut -lm -L$(LIBXML2_DIR) -L$(GUMBO_DIR) -L$(MYHTML_DIR)/lib -L$(HAUT_DIR)

OBJS = main.o benchmark.o util.o test_dummy.o test_hsp.o test_libxml2.o test_gumbo.o test_myhtml.o test_haut.o htmlstreamparser/htmlstreamparser.o
HEADERS = benchmark.h util.h test_dummy.h test_hsp.h test_libxml2.h test_gumbo.h test_myhtml.h test_haut.h htmlstreamparser/htmlstreamparser.h


all:		libxml2 gumbo myhtml haut benchmark

.PHONY: benchmark
benchmark:	$(OBJS)
		$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o:		%.c $(HEADERS)
		$(CC) $(CFLAGS) -c $< -o $@

libxml2:	
		(cd $(LIBXML2_DIR) && if [ ! -f Makefile ]; then ./configure --enable-static --without-zlib --without-lzma --without-python; fi)
		$(MAKE) -C $(LIBXML2_DIR)
		ar rcs $(LIBXML2_DIR)/libxml2.a $(LIBXML2_DIR)/*.o

gumbo:
	$(MAKE) -C $(GUMBO_DIR)

myhtml:
	$(MAKE) -C $(MYHTML_DIR) static

haut:
	$(MAKE) -C $(HAUT_DIR)

clean:
		rm -f benchmark
		rm -f *.o htmlstreamparser/*.o
		$(MAKE) -i -C $(LIBXML2_DIR) distclean
		rm -f $(LIBXML2_DIR)/libxml2.a
		$(MAKE) -i -C $(GUMBO_DIR) clean
		$(MAKE) -i -C $(MYHTML_DIR) clean
		$(MAKE) -i -C $(HAUT_DIR) clean

