#
# benchmark,  benchmark different HTML parsers on a large volume of HTML files
#
# Micky Faas <micky@edukitty.org>
# Copyright (C) 2017  Leiden University, The Netherlands.
#

CC = gcc
CFLAGS = -Wall -std=c99 -O2
LDFLAGS =
FSM2ARRAY = ./fsm2array
GENTAGS = ./gen_tags

OBJS = entities.o tags.o main.o 
HEADERS = entities.h tags.h

all:		gen_tags entities

entities:	$(OBJS)
		$(CC) $(CFLAGS) $^ -o $@

gen_tags:	gen_tags.c
		$(CC) gen_tags.c -o gen_tags

%.o:		%.c $(HEADERS)
		$(CC) $(CFLAGS) -c $< -o $@

.PHONY: entities.c
entities.c:	entities_fsm.h
		$(CC) -E -P entities_fsm.h -o entities_fsm.in
		cat entities_fsm.in | $(FSM2ARRAY) -b 33 > entities_transitions.h
.PHONY: tags.c
tags.c:		tags_fsm.h
		$(CC) -E -P tags_fsm.h -o tags_fsm.in
		cat tags_fsm.in | $(FSM2ARRAY) -i -b 47 > tags_transitions.h

.PHONY: tags_fsm.h
tags_fsm.h:	gen_tags
		$(GENTAGS) html5_tags.in tags.h tags_fsm.h

clean:
		rm -f entities
		rm -f gen_tags
		rm -f *.o
		rm -f tags_fsm.in
		rm -f entities_fsm.in



