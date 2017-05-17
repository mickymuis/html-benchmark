#
# liblmth, Lightweight HTML lexer
#
# Micky Faas <micky@edukitty.org>
# Copyright (C) 2017  Leiden University, The Netherlands.
#

CC = gcc
CFLAGS = -Wall -std=c99 -O2

all:		fsm2array example lexer parser

fsm2array:	fsm2array.c
		$(CC) $(CFLAGS) fsm2array.c -o fsm2array

example:	fsm2array example_fsm.h
		$(CC) -E -P example_fsm.h -o example_fsm.in
		cat example_fsm.in | ./fsm2array > example.h

lexer:		fsm2array lexer_transitions_fsm.h
		$(CC) -E -P lexer_transitions_fsm.h -o lexer_transitions_fsm.in
		cat lexer_transitions_fsm.in | ./fsm2array > ../lexer_transitions.h

lexer_script:	fsm2array lexer_script_fsm.h
		$(CC) -E -P lexer_script_fsm.h -o lexer_script_fsm.in
		cat lexer_script_fsm.in | ./fsm2array > ../lexer_script.h

parser:		fsm2array parser_transitions_fsm.h
		$(CC) -E -P parser_transitions_fsm.h -o parser_transitions_fsm.in
		cat parser_transitions_fsm.in | ./fsm2array > ../parser_transitions.h
clean:
		rm -f fsm2array
		rm -f example_fsm.in
		rm -f lexer_transitions_fsm.in
		rm -f parser_transitions_fsm.in
		rm -f example.h



