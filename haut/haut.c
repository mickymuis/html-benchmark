/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "haut.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "state_machine.h"
#include "state.h"

//#define DEBUG_PRINT

static string_t
default_entity_event( haut_t* p, strfragment_t* html_entity ) {
    string_t placeholder;
    string_init( &placeholder );
    string_append( &placeholder, "?", 1 );
    return placeholder;
};
void    
default_document_begin_event     ( struct haut* p ){
#ifdef DEBUG_PRINT
    fprintf( stderr, "Debug: document begin\n" );
#endif
}
void            
default_document_end_event       ( struct haut* p ){
#ifdef DEBUG_PRINT
    fprintf( stderr, "Debug: document end\n" );
#endif
}

void            
default_element_open_event    ( struct haut* p, strfragment_t* name ){
#ifdef DEBUG_PRINT
    printf( "Debug: element open: `%.*s'\n", name->size, name->data );
#endif
}
void            
default_element_close_event   ( struct haut* p, strfragment_t* name ){
#ifdef DEBUG_PRINT
    printf( "Debug: element close: `%.*s'\n", name->size, name->data );
#endif
}

void            
default_attribute_event       ( struct haut* p, strfragment_t* key, strfragment_t* value ){
#ifdef DEBUG_PRINT
    printf( "Debug: attribute: %.*s=\"%.*s\"\n", key->size, key->data, 
            (value != NULL ? value->size : 0 ),
            (value != NULL ? value->data : "" ) );
#endif
}

void            
default_comment_event         ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: comment: `%.*s'\n", text->size, text->data );
#endif
}
void            
default_innertext_event       ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: innertext: `%.*s'\n", text->size, text->data );
#endif
}
void            
default_cdata_event           ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: CDATA: `%.*s'\n", text->size, text->data );
#endif
}
void            
default_doctype_event         ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: DOCTYPE: `%.*s'\n", text->size, text->data );
#endif
}
void            
default_script_event          ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: script: `%.*s'\n", text->size, text->data );
#endif
}

void            
default_error_event           ( struct haut* p, error_t err ){
#ifdef DEBUG_PRINT
    printf( "Debug: Syntax error on line %d, column %d:\n", 
            p->position.row, p->position.col );

    printf( "\t`%.*s%.*s'\n", 40, (p->buffer + p->position.offset) - 20, 20, (p->buffer + p->position.offset));
    printf( "\t                                          ^\n" );
#endif
}

static void*
default_allocator( void* userdata, size_t size ) {
    return malloc( size );
}

static void
default_deallocator( void* userdata, void* ptr ) {
    free( ptr );
}

const haut_event_handler_t DEFAULT_EVENT_HANDLER = {
    .document_begin=default_document_begin_event,
    .document_end  =default_document_end_event,
    .element_open  =default_element_open_event,
    .element_close =default_element_close_event,
    .attribute     =default_attribute_event,
    .comment       =default_comment_event,
    .innertext     =default_innertext_event,
    .doctype       =default_doctype_event,
    .script        =default_script_event,
    .cdata         =default_cdata_event,
    .error         =default_error_event,
    .entity        =default_entity_event
};

const haut_opts_t DEFAULT_PARSER_OPTS = {
    .allocator      =default_allocator,
    .deallocator    =default_deallocator,
    .flags   =FLAG_NONE
};

const haut_position_t POSITION_BEGIN ={
    .row           =1,
    .col           =1,
    .offset        =0
};

static inline int
has_mutable_buffer( haut_t* p ) {
    return ( p->opts.flags & FLAG_MUTABLE_BUFFER );
}

static inline char
current_char( haut_t* p ) {
    return *( p->buffer + p->position.offset );
}

static inline int
at_end( haut_t* p ) {
    return (p->length == p->position.offset + 1);
}

/*static inline int
consume_char( haut_t* p ) {
    char c =current_char( p );
    token_t t =consume_token_char( p->state.token, c );

    switch( t ) {

    };
}*/

static inline void
emit_error( haut_t* p, int error ) {
    p->state.last_error =error;
    if( p->events.error != NULL )
        p->events.error( p, error );
}

static inline void
set_token_begin( haut_t* p, int offs ) {
            p->state.current_token.data = p->buffer + p->position.offset + offs;
            p->state.current_token.size =0;
}

static inline void
set_token_end( haut_t* p, int offs ) {
            p->state.current_token.size = ((p->buffer + p->position.offset) - p->state.current_token.data) + offs;
}

static inline void
dispatch_parser_action( haut_t* p, int state, int* lexer_state ) {
    switch( state ) {
        /* Public events */
        default:
        case P_NONE:
            break;
        case P_DOCUMENT_BEGIN:
            p->events.document_begin( p );
            break;
        case P_DOCUMENT_END:
            p->events.document_end( p );
            break;

        case P_ELEMENT_OPEN:
            set_token_end( p, 0 );
            p->state.last_elem =p->state.current_token;
            p->events.element_open( p, &p->state.last_elem );
            break;

        case P_ELEMENT_CLOSE:
            set_token_end( p, 0 );
            p->state.last_elem =p->state.current_token;
            p->events.element_close( p, &p->state.last_elem );
            break;

        case P_ATTRIBUTE:
            set_token_end( p, 0 );
            p->events.attribute( p, &p->state.attr_key, &p->state.current_token );
            break;
        
        case P_ATTRIBUTE_VOID:
            set_token_end( p, 0 );
            p->state.attr_key = p->state.current_token;
            p->events.attribute( p, &p->state.attr_key, NULL );
            break;

        case P_INNERTEXT:
            set_token_end( p, 0 );
            p->events.innertext( p, &p->state.current_token );
            break;

        case P_COMMENT:
            p->events.comment( p, &p->state.current_token );
            break;
        
        case P_CDATA:
            p->events.cdata( p, &p->state.current_token );
            break;

        case P_DOCTYPE:
            set_token_end( p, 0 );
            p->events.doctype( p, &p->state.current_token );
            break;

        case P_ENTITY:
            break;

        case P_ERROR:
            p->events.error( p, ERROR_NONE );
            break;

        case P_TOKEN_BEGIN:
            set_token_begin( p, 0 );
            break;
        case P_TOKEN_END:
            set_token_end( p, 0 );
            break;
        case P_ATTRIBUTE_KEY:
            set_token_end( p, 0 );
            p->state.attr_key = p->state.current_token;
            break;
        case P_ELEMENT_END:
            if( strncasecmp( p->state.last_elem.data, "script", 5 ) == 0 ) {
                set_token_begin( p, 1 );
                *lexer_state =L_SCRIPT;
            }
            break;
        case P_VOID_ELEMENT_END:
            break;
        case P_SCRIPT_END:
            p->events.script( p, &p->state.current_token );
            break;
        case P_ENTITY_BEGIN:
            break;
    }
}

/* */

void
haut_init(  haut_t* p ) {
    memset( p, 0, sizeof( haut_t ) );
    p->opts =DEFAULT_PARSER_OPTS;
    p->events =DEFAULT_EVENT_HANDLER;
    p->position =POSITION_BEGIN;
}

void
haut_destroy( haut_t* p ) {

}

void
haut_set_input( haut_t* p, const char* buffer, size_t len ) {
    p->buffer =(char*)buffer;
    p->length =len;
    haut_disable( p, FLAG_MUTABLE_BUFFER );
}

void
haut_set_input_mutable( haut_t* p, char* buffer, size_t len ) {
    p->buffer =(char*)buffer;
    p->length =len;
    haut_enable( p, FLAG_MUTABLE_BUFFER );
}

void
haut_parse( haut_t* p ) {
    static int error=0;
    char c;
    int lexer_state =L_BEGIN, next_lexer_state;
    char *parser_state;
    
    while( !at_end( p ) ) {
        c =current_char( p );

        next_lexer_state =lexer_next_state( lexer_state, c );

        parser_state =parser_next_state( lexer_state, next_lexer_state );

        for( int k =0; k < 2; k++ )
            dispatch_parser_action( p, parser_state[k], &next_lexer_state );
        
      /*  if( parser_state[0] == P_ERROR ) {
            printf( "Syntax error on line %d, column %d (lexer transition %d -> %d) #%d:\n", 
                    p->position.row, p->position.col,
                    lexer_state, next_lexer_state, ++error );

            printf( "\t`%.*s%.*s'\n", 80, (p->buffer + p->position.offset) - 20, 20, (p->buffer + p->position.offset));
            printf( "\t                                                                                    ^\n" );

           //printf( "\t`%.*s☃%.*s'\n\n", p->position.offset, p->buffer, 20, (p->buffer + p->position.offset));
        }*/

        lexer_state =next_lexer_state;

        p->position.offset++;
        if( c == '\n' ) {
            p->position.row++;
            p->position.col = 1;
        } else if( c != '\r' )
            p->position.col++;
    }
}

void
haut_set_opts( haut_t* p, haut_opts_t opts ) {
    p->opts =opts;
}

void
haut_set_eventhandler( haut_t* p, haut_event_handler_t e ) {
    p->events =e;
}

void
haut_enable( haut_t* p, haut_flag_t flag ) {
    p->opts.flags |= flag;
}

void
haut_disable( haut_t* p, haut_flag_t flag ) {
    p->opts.flags &= ~flag;
}

