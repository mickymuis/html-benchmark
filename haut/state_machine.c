/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017: Leiden University
 */

#include "state_machine.h"
#include "state.h"

#define INPUT_BITS 8 // for now

static const char* _lexer_transition[L_N_STATES][1<<INPUT_BITS] = {
    /* Here the output of the FSM-generator is inserted
     * Given a combination of current state and input character, 
     * this array gives the next state of the tokenizer */
#include "lexer_transitions.h"
};

#if 0
static const char* _lexer_script_transition[L_SCRIPT_N_STATES][1<<INPUT_BITS] = {
    /* Here the output of the FSM-generator is inserted
     * Given a combination of current state and input character, 
     * this array gives the next state of the tokenizer, in script-mode */
#include "lexer_script.h"
};

    /* Index of both lexer FSM's */
static const char*(*_lexer[])[1<<INPUT_BITS] = {
    _lexer_transition,
    _lexer_script_transition
};

#endif

static const char* _parser_transition[L_N_STATES][L_N_STATES] = {
    /* Here the output of the FSM-generator is inserted
     * Given a current and a new state,
     * this array gives the number of the event that must be emitted by the parser
     * as defined in events.h */
#include "parser_transitions.h"
};

inline int 
lexer_next_state( int lexer_state, char c ) {
    return _lexer_transition[lexer_state][(unsigned char)c][0];
  //  return (_lexer[mode])[lexer_state][(unsigned char)c][0];
}

inline const char* 
parser_next_state( int lexer_state, int next_lexer_state ) {
    return _parser_transition[lexer_state][next_lexer_state];
}

