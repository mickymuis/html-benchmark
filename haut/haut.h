/*
 * Haut - a lightweight html tokenizer
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#ifndef LMTH_H
#define LMTH_H

#include "string_util.h"

enum error {
    ERROR_NONE          =0,
    ERROR_TRUNCATED,
    ERROR_SYNTAX_ERROR
};

typedef enum error error_t;
typedef int tag_t;

struct haut;

typedef void*           (*allocatorfunc)         ( void* userdata, size_t size );
typedef void            (*deallocatorfunc)       ( void* userdata, void* ptr );

typedef void            (*document_begin_event)  ( struct haut* );
typedef void            (*document_end_event)    ( struct haut* );

typedef void            (*element_open_event)    ( struct haut*, tag_t tag, strfragment_t* name );
typedef void            (*element_close_event)   ( struct haut*, tag_t tag, strfragment_t* name );

typedef void            (*attribute_event)       ( struct haut*, strfragment_t* key, strfragment_t* value );

typedef void            (*comment_event)         ( struct haut*, strfragment_t* text );
typedef void            (*innertext_event)       ( struct haut*, strfragment_t* text );
typedef void            (*cdata_event)           ( struct haut*, strfragment_t* text );
typedef void            (*doctype_event)         ( struct haut*, strfragment_t* text );
typedef void            (*script_event)          ( struct haut*, strfragment_t* text );

typedef void            (*error_event)           ( struct haut*, error_t err );

typedef string_t        (*entity_event)          ( struct haut*, strfragment_t* html_entity );


typedef struct {
    document_begin_event document_begin;
    document_end_event  document_end;
    element_open_event  element_open;
    element_close_event element_close;
    attribute_event     attribute;
    comment_event       comment;
    innertext_event     innertext;
    cdata_event         cdata;
    doctype_event       doctype;
    script_event        script;
    error_event         error;
    entity_event        entity;
} haut_event_handler_t;

extern const haut_event_handler_t DEFAULT_EVENT_HANDLER;

typedef struct {
    allocatorfunc       allocator;
    deallocatorfunc     deallocator;
    int                 flags;
} haut_opts_t;

extern const haut_opts_t DEFAULT_PARSER_OPTS;

typedef enum {
    FLAG_NONE                   = 0,
    FLAG_COMPACT_WHITESPACE     = 1,
    FLAG_STRICT                 = 2,
    FLAG_MUTABLE_BUFFER         = 3        
} haut_flag_t;

typedef struct {
    unsigned int row;
    unsigned int col;
    size_t offset;
} haut_position_t;

extern const haut_position_t POSITION_BEGIN;

typedef struct {
    int last_tag;
    strfragment_t last_elem;
    strfragment_t attr_key;
    strfragment_t current_token;
    int depth;
    int last_error;
    int last_warning;
} haut_state_t;

struct haut {
	haut_event_handler_t events;
	haut_opts_t opts;

        void* userdata;

        haut_state_t state;

        char* buffer;
        size_t length;

        haut_position_t position;
};
typedef struct haut haut_t;

void
haut_init(  haut_t* p );

void
haut_destroy( haut_t* p );

void
haut_set_input( haut_t* p, const char* buffer, size_t len );

void
haut_set_input_mutable( haut_t* p, char* buffer, size_t len );

void
haut_parse( haut_t* p );

void
haut_set_opts( haut_t* p, haut_opts_t opts );

void
haut_set_eventhandler( haut_t* p, haut_event_handler_t e );

void
haut_enable( haut_t* p, haut_flag_t flag );

void
haut_disable( haut_t* p, haut_flag_t flag );


#endif
