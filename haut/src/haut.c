/*
 * Haut - a lightweight html tokenizer
 *
 * https://github.com/mickymuis/haut-html
 *
 * Micky Faas <micky@edukitty.org>
 * Copyright 2017-2018
 * Leiden Institute of Advanced Computer Science, The Netherlands
 */

#include "../include/haut/haut.h"
#include "../include/haut/state_machine.h"
#include "../include/haut/tag.h"
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include "state.h"

/* This struct contains the internal state of the parser
 * and is opaque to the user of the API */
struct haut_state {
    haut_tag_t last_tag;
    haut_error_t last_error;
    
    strfragment_t attr_key_ptr;
    strbuffer_t attr_key_buffer;
    
    // Fragment that points to the current token, 
    // can point to either token_buffer or token_chunk_ptr
    strfragment_t token_ptr;
    // Local copy of the current token (if applicable)
    strbuffer_t token_buffer;
    // Fragment that points to the current token in the current chunk
    strfragment_t token_chunk_ptr;
    // Special token pointer when we are parsing an entity (character reference)
    strfragment_t entity_token_ptr;
    // Whether we are collecting a token at all (true)
    // false if token_ptr points to a meaningfull token
    bool in_token;
    
    // Current lexer state and its 'one-entry stack'
    int lexer_state;
    int lexer_saved_state;
};

//#define DEBUG_PRINT

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
default_element_open_event    ( struct haut* p, haut_tag_t tag, strfragment_t* name ){
#ifdef DEBUG_PRINT
    printf( "Debug: element open: `%.*s (%d)'\n", (int)name->size, name->data, tag );
#endif
}
void            
default_element_close_event   ( struct haut* p, haut_tag_t tag, strfragment_t* name ){
#ifdef DEBUG_PRINT
    printf( "Debug: element close: `%.*s (%d)'\n", (int)name->size, name->data, tag );
#endif
}

void            
default_attribute_event       ( struct haut* p, strfragment_t* key, strfragment_t* value ){
#ifdef DEBUG_PRINT
    printf( "Debug: attribute: %.*s=\"%.*s\"\n", (int)key->size, key->data, 
            (value != NULL ? (int)value->size : 0 ),
            (value != NULL ? value->data : "" ) );
#endif
}

void            
default_comment_event         ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: comment: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_innertext_event       ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: innertext: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_cdata_event           ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: CDATA: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_doctype_event         ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: DOCTYPE: `%.*s'\n", (int)text->size, text->data );
#endif
}
void            
default_script_event          ( struct haut* p, strfragment_t* text ){
#ifdef DEBUG_PRINT
    printf( "Debug: script: `%.*s'\n", (int)text->size, text->data );
#endif
}

void            
default_error_event           ( struct haut* p, haut_error_t err ){
#ifdef DEBUG_PRINT
    printf( "Debug: Syntax error on line %d, column %d:\n", 
            p->position.row, p->position.col );

    printf( "\t`%.*s%.*s'\n", 40, (p->input + p->position.offset) - 20, 20, (p->input + p->position.offset));
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
};

const haut_opts_t DEFAULT_PARSER_OPTS = {
    .allocator      =default_allocator,
    .deallocator    =default_deallocator,
    .flags          =FLAG_NONE
};

const haut_position_t POSITION_BEGIN ={
    .row           =1,
    .col           =1,
    .offset        =0
};

static inline char
current_char( haut_t* p ) {
    return *( p->input + p->position.offset );
}

static inline int
at_end( haut_t* p ) {
    return (p->length == p->position.offset + 1);
}

static inline void
emit_error( haut_t* p, int error ) {
    p->state->last_error =error;
    if( p->events.error != NULL )
        p->events.error( p, error );
}

static inline void
set_token_chunk_begin( haut_t* p, int offs ) {
    p->state->token_chunk_ptr.data = p->input + p->position.offset + offs;
    p->state->token_chunk_ptr.size =0;
    p->state->in_token =true;
}

static inline void
set_token_chunk_end( haut_t* p, int offs ) {
    p->state->token_chunk_ptr.size = ((p->input + p->position.offset) - p->state->token_chunk_ptr.data) + offs;
    p->state->in_token =false;
}

static inline void store_current_token( haut_t* p, int offs );
static inline void clear_current_token( haut_t* p );

static inline void
begin_token( haut_t* p, int offs ) {
    set_token_chunk_begin( p, offs );
}

static inline void
end_token( haut_t* p, int offs ) {
    if( p->state->token_buffer.size > 0 ) {
        // We have a (partial) token stored locally
        store_current_token( p, offs );
        p->state->token_ptr = strbuffer_to_fragment( p->state->token_buffer );
        p->state->in_token =false;
    } else {
        // The entire token is inside this chunk
        set_token_chunk_end( p, offs );
        p->state->token_ptr = p->state->token_chunk_ptr;
    }
}

static inline void
store_current_token( haut_t* p, int offs ) {
    set_token_chunk_end( p, offs );
    strbuffer_append( 
            &p->state->token_buffer,
            p->state->token_chunk_ptr.data,
            p->state->token_chunk_ptr.size );
    set_token_chunk_begin( p, offs );
}

static inline void
clear_current_token( haut_t* p ) {
    strbuffer_clear( &p->state->token_buffer );
}

static inline void
dispatch_parser_action( haut_t* p, int state, int* lexer_next_state ) {
    size_t offset;
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
            end_token( p, 0 );
            p->state->last_tag =decode_tag( p->state->token_ptr.data, p->state->token_ptr.size );

            p->events.element_open( p, p->state->last_tag, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_ELEMENT_CLOSE:
            end_token( p, 0 );
            p->state->last_tag =decode_tag( p->state->token_ptr.data, p->state->token_ptr.size );
            
            p->events.element_close( p, p->state->last_tag, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_ATTRIBUTE:
            end_token( p, 0 );
            p->events.attribute( p, &p->state->attr_key_ptr, &p->state->token_ptr );
            p->state->attr_key_ptr.data = NULL;
            strbuffer_clear( &p->state->attr_key_buffer );
            clear_current_token( p );
            break;
        
        case P_ATTRIBUTE_VOID:
            end_token( p, 0 );
            p->events.attribute( p, &p->state->token_ptr, NULL );
            clear_current_token( p );
            break;

        case P_INNERTEXT:
            end_token( p, 0 );
            p->events.innertext( p, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_TEXT:
            break;

        case P_COMMENT:
            end_token( p, 0 );
            p->events.comment( p, &p->state->token_ptr );
            clear_current_token( p );
            break;
        
        case P_CDATA:
            end_token( p, 0 );
            p->events.cdata( p, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_DOCTYPE:
            end_token( p, 0 );
            p->events.doctype( p, &p->state->token_ptr );
            clear_current_token( p );
            break;

        case P_ENTITY_BEGIN:
            if( !p->state->in_token ) {
                begin_token( p, 0 );
                p->state->lexer_saved_state = L_INNERTEXT;
            } else
                p->state->lexer_saved_state = p->state->lexer_state;
            store_current_token( p, 1 );
            //fprintf( stderr, "DEBUG: entity begin `%.*s'\n", p->state->token_buffer.size, p->state->token_buffer.data );
            break;

        case P_ENTITY:
            offset = p->state->token_buffer.size;
            end_token( p, 0 );
            p->state->entity_token_ptr.data = p->state->token_buffer.data + offset;
            p->state->entity_token_ptr.size = p->state->token_buffer.size - offset;
            //fprintf( stderr, "DEBUG: entity token `%.*s' (%d)\n", p->state->entity_token_ptr.size, p->state->entity_token_ptr.data, p->state->entity_token_ptr.size);
            char32_t entity = decode_entity( p->state->entity_token_ptr.data, p->state->entity_token_ptr.size );
            strbuffer_t tmp;
            u32toUTF8( &tmp, entity );
            //fprintf( stderr, "DEBUG: decoded HTML-entity with Unicode %d (%s)\n", entity, tmp.data );
            // Append the decoded entity to whatever token we were parsing
            p->state->token_buffer.size =offset -1;
            strbuffer_append( &p->state->token_buffer, tmp.data, tmp.size );
            strbuffer_free( &tmp );
            // Return the lexer to the token we were parsing before the entity was encountered
            *lexer_next_state = p->state->lexer_saved_state;
            set_token_chunk_begin( p, 1 );
            break;

        case P_ERROR:
            p->events.error( p, ERROR_SYNTAX_ERROR );
            break;

        case P_TOKEN_BEGIN:
            if( !p->state->in_token )
                begin_token( p, 0 );
            break;
        case P_TOKEN_END:
            end_token( p, 0 );
            break;
        case P_SAVE_TOKEN:
            store_current_token( p, -1 );
            break;
        case P_ATTRIBUTE_KEY:
            end_token( p, 0 );
            p->state->attr_key_ptr = p->state->token_ptr;
            break;
        case P_ELEMENT_END:
            if( p->state->last_tag == TAG_SCRIPT ) {
                begin_token( p, 1 );
                *lexer_next_state =L_SCRIPT;
            }
            break;
        case P_VOID_ELEMENT_END:
            break;
        case P_SCRIPT_END:
            p->events.script( p, &p->state->token_ptr );
            break;
        case P_SAVE_LEXER_STATE:
            p->state->lexer_saved_state = p->state->lexer_state;
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
    p->state = (struct haut_state*)malloc( sizeof( struct haut_state ) );
    memset( p->state, 0, sizeof( struct haut_state ) );
    strbuffer_init( &p->state->token_buffer );
    strbuffer_init( &p->state->attr_key_buffer );
    p->state->lexer_state = L_BEGIN;
}

void
haut_destroy( haut_t* p ) {
    strbuffer_free( &p->state->token_buffer );
    strbuffer_free( &p->state->attr_key_buffer );
    free( p->state );
}

void
haut_setInput( haut_t* p, const char* buffer, size_t len ) {
    p->input =(char*)buffer;
    p->length =len;
    p->position.offset =0;
}

void
haut_parse( haut_t* p ) {
    char c;
    int next_lexer_state;
    const char *parser_state;
    
    while( !at_end( p ) ) {
        c =current_char( p );

        next_lexer_state =lexer_next_state( p->state->lexer_state, c );

        parser_state =parser_next_state( p->state->lexer_state, next_lexer_state );

        for( int k =0; k < 2; k++ )
            dispatch_parser_action( p, parser_state[k], &next_lexer_state );
        
        p->state->lexer_state =next_lexer_state;

        p->position.offset++;
        if( c == '\n' ) {
            p->position.row++;
            p->position.col = 1;
        } else if( c != '\r' )
            p->position.col++;
    }
}

void
haut_parseChunk( haut_t* p, const char* buffer, size_t len ) {
    haut_setInput( p, buffer, len );
    if( p->state->in_token )
        set_token_chunk_begin( p, 0 );
    haut_parse( p );
    if( p->state->attr_key_ptr.data ) {
        strbuffer_copyFragment( &p->state->attr_key_buffer, 0, &p->state->attr_key_ptr );
        p->state->attr_key_ptr.data = p->state->attr_key_buffer.data;
        p->state->attr_key_ptr.size = p->state->attr_key_buffer.size;
    }
    store_current_token( p, 0 );
}

haut_tag_t
haut_currentElementTag( haut_t* p ) {
    return p->state->last_tag;
}

void
haut_setOpts( haut_t* p, haut_opts_t opts ) {
    p->opts =opts;
}

void
haut_setEventHandler( haut_t* p, haut_event_handler_t e ) {
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

