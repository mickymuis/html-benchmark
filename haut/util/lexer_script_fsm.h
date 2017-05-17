// This file is a finite state machine in the `fsm2array' format
// In order to build it, it has to be pre-processed using `gcc -E -P'
// The preprocessed file can then be fed to fsm2array
// The resulting transition table can be included inside a C-style array notation
#include "../state.h"
// Define the number of states and the number of inputs
L_SCRIPT_N_STATES 256

// This FSM defines the lexer rules inside a <script> or <style> element
// The input tuples consist of the current state and the new state of the lexer
// The output gives the new state of the script-lexer

// The fact that there can be different types of scripts in a HTML-page
// and the ambiguity of how the 'end' of a script is specified,
// makes this a hard problem to write an FSM for.
// The following rules are by no means comprehensive.

**, **          => { L_SCRIPT }

// Define single-quoted and double-quoted strings and the beginning of the closing tag
L_SCRIPT, '\''  => { L_SCRIPT_SINGLE_QUOTE_STRING }
L_SCRIPT, '"'   => { L_SCRIPT_DOUBLE_QUOTE_STRING }
L_SCRIPT, '<'   => { L_SCRIPT_LT }

// Single-quoted string with escaping by the \ character
L_SCRIPT_SINGLE_QUOTE_STRING, **        => { L_SCRIPT_SINGLE_QUOTE_STRING }
L_SCRIPT_SINGLE_QUOTE_STRING, '\\'      => { L_SCRIPT_SINGLE_QUOTE_STRING_ESCAPE }
L_SCRIPT_SINGLE_QUOTE_STRING, '\''      => { L_SCRIPT }

L_SCRIPT_SINGLE_QUOTE_STRING_ESCAPE, ** => { L_SCRIPT_SINGLE_QUOTE_STRING }

// Double-quoted string with escaping by the \ character
L_SCRIPT_DOUBLE_QUOTE_STRING, **        => { L_SCRIPT_DOUBLE_QUOTE_STRING }
L_SCRIPT_DOUBLE_QUOTE_STRING, '\\'      => { L_SCRIPT_DOUBLE_QUOTE_STRING_ESCAPE }
L_SCRIPT_DOUBLE_QUOTE_STRING, '\''      => { L_SCRIPT }

L_SCRIPT_DOUBLE_QUOTE_STRING_ESCAPE, ** => { L_SCRIPT_DOUBLE_QUOTE_STRING }

// Closing sequence for </script>

L_SCRIPT_LT, **         => { L_SCRIPT }
L_SCRIPT_LT, *s         => { L_SCRIPT_LT }
L_SCRIPT_LT, '/'        => { L_SCRIPT_SOLIDUS }

L_SCRIPT_SOLIDUS, **    => { L_SCRIPT }
L_SCRIPT_SOLIDUS, *s    => { L_SCRIPT_SOLIDUS }
L_SCRIPT_SOLIDUS, 's'   => { L_SCRIPT_S }
L_SCRIPT_SOLIDUS, 'S'   => { L_SCRIPT_S }

L_SCRIPT_S, **          => { L_SCRIPT }
L_SCRIPT_S, 'c'         => { L_SCRIPT_C }
L_SCRIPT_S, 'C'         => { L_SCRIPT_C }
L_SCRIPT_S, 't'         => { L_SCRIPT_STYLE_T }
L_SCRIPT_S, 'T'         => { L_SCRIPT_STYLE_T }

L_SCRIPT_C, **          => { L_SCRIPT }
L_SCRIPT_C, 'r'         => { L_SCRIPT_R }
L_SCRIPT_C, 'R'         => { L_SCRIPT_R }

L_SCRIPT_R, **          => { L_SCRIPT }
L_SCRIPT_R, 'i'         => { L_SCRIPT_I }
L_SCRIPT_R, 'I'         => { L_SCRIPT_I }

L_SCRIPT_I, **          => { L_SCRIPT }
L_SCRIPT_I, 'p'         => { L_SCRIPT_P }
L_SCRIPT_I, 'P'         => { L_SCRIPT_P }

L_SCRIPT_P, **          => { L_SCRIPT }
L_SCRIPT_P, 't'         => { L_SCRIPT_T }
L_SCRIPT_P, 'T'         => { L_SCRIPT_T }

L_SCRIPT_T, **          => { L_SCRIPT }
L_SCRIPT_T, *s          => { L_SCRIPT_T }
L_SCRIPT_T, '>'         => { L_SCRIPT_END }

// Closing sequence for </style>

L_SCRIPT_STYLE_T, **    => { L_SCRIPT }
L_SCRIPT_STYLE_T, 'y'   => { L_SCRIPT_STYLE_Y }
L_SCRIPT_STYLE_T, 'Y'   => { L_SCRIPT_STYLE_Y }

L_SCRIPT_STYLE_Y, **    => { L_SCRIPT }
L_SCRIPT_STYLE_Y, 'l'   => { L_SCRIPT_STYLE_L }
L_SCRIPT_STYLE_Y, 'L'   => { L_SCRIPT_STYLE_L }

L_SCRIPT_STYLE_L, **    => { L_SCRIPT }
L_SCRIPT_STYLE_L, 'e'   => { L_SCRIPT_STYLE_E }
L_SCRIPT_STYLE_L, 'E'   => { L_SCRIPT_STYLE_E }

L_SCRIPT_STYLE_E, **    => { L_SCRIPT }
L_SCRIPT_STYLE_E, *s    => { L_SCRIPT_STYLE_E }
L_SCRIPT_STYLE_E, '>'   => { L_SCRIPT_END }

