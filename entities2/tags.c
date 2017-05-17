#include "tags.h"
#include <sys/types.h>
#include <inttypes.h>

static uint16_t __tagname_transition[][TAG__N_INPUTS] = {
    #include "tags_transitions.h"
};

int decode_tag( const char* str, size_t len ) {

    int state =TAG_NONE;

    for( size_t i =0; i < len; i++ ) {
        state = __tagname_transition[state][str[i]-TAG__FIRST_CHAR];
    }
    
    state = __tagname_transition[state][TAG__EOF];
    if( state < TAG__N )
        return state; 

    return TAG_UNKNOWN;
}
