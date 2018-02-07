/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "test_haut.h"
#include <haut/haut.h>
#include <haut/tag.h>

#include <strings.h>

typedef struct {
    int in_body;
    testresult_t* result;
    strbuffer_t innertext;
    int images;
} haut_data;

void
element_open( haut_t* p, haut_tag_t tag, strfragment_t* name ) {
    
    if( haut_currentElementTag( p ) == TAG_BODY ) {
        ((haut_data*)p->userdata)->in_body =1;
    }
    
}

void
attribute( haut_t* p, strfragment_t* key, strfragment_t* value ) {
    if( ((haut_data*)p->userdata)->in_body != 1 )
        return;
    if( haut_currentElementTag( p ) == TAG_A ) {
        if( strncasecmp( key->data, "href", key->size ) == 0 )
            ((haut_data*)p->userdata)->result->total_href++;

    } else if( haut_currentElementTag( p ) == TAG_IMG ) {
        if( strncasecmp( key->data, "src", key->size ) == 0 ) {
            ((haut_data*)p->userdata)->images++;
            ((haut_data*)p->userdata)->result->total_src++;
        }
        else if( strncasecmp( key->data, "alt", key->size ) == 0 )
            ((haut_data*)p->userdata)->result->total_alt++;
    }

}

void 
innertext( haut_t* p, strfragment_t* text ) {
    if( ((haut_data*)p->userdata)->in_body == 1 && text) {
        strbuffer_t* buf = &((haut_data*)p->userdata)->innertext;
        strbuffer_append( buf, " ", 1 );
        strbuffer_append( buf, text->data, text->size );
        return;
    } 

    if( haut_currentElementTag( p ) == TAG_TITLE ) {
        // We have a title!
    }
}

int
test_haut( testresult_t* result, const char* htmlpage, size_t length ){
    
    // User data
    haut_data state ={ .in_body =0, .result =result, .images =0 };
    strbuffer_init( &state.innertext );

    // Construct parser object
    haut_t p;
    haut_init( &p );

    // Set the buffer
    haut_setInput( &p, (char*)htmlpage, length );

    // Setup event handlers
    p.events.element_open =element_open;
    p.events.attribute =attribute;
    p.events.innertext =innertext;
    p.userdata =(void*)&state;

    // Begin parsing, tokens will be sent to the given event handlers
    haut_parse( &p );

    haut_destroy( &p );

    // Cleanup
    result->total_innertext_bytes += state.innertext.size;
    //printf( "\n%.*s\n", state.innertext.size, state.innertext.data );
    strbuffer_free( &state.innertext );

//    printf( "Total images: %d\n", state.images );

    result->total_bytes +=sizeof( haut_t );
    return 0;
}
