/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

int lexer_next_state( int lexer_state, char c );

const char* parser_next_state( int lexer_state, int next_lexer_state );

#endif
