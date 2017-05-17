#include "entities.h"
#include <sys/types.h>
#include <inttypes.h>

#pragma pack(1)

static const uint32_t __entity_transition[][63] = {
    #include "entities_transitions.h"
};

int decode_entity( const char* str, size_t len ) {

    int state =E__BEGIN;

    for( size_t i =0; i < len; i++ ) {
        state = __entity_transition[state][str[i]-59];
        if( state < E__N )
            return state;
    }

    return state;
}
