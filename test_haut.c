/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "test_haut.h"
#include "haut/haut.h"

#include <strings.h>

typedef struct {
    int in_body;
    testresult_t* result;
    string_t innertext;
    int images;
} haut_data;

void
element_open( haut_t* p, strfragment_t* name ) {
    
    if( strncasecmp( name->data, "body", name->size ) == 0 ) {
        ((haut_data*)p->userdata)->in_body =1;
    }
    
}

void
attribute( haut_t* p, strfragment_t* key, strfragment_t* value ) {
    if( ((haut_data*)p->userdata)->in_body != 1 )
        return;
    strfragment_t elem =p->state.last_elem;
    if( strncasecmp( elem.data, "a", elem.size ) == 0 ) {
        if( strncasecmp( key->data, "href", key->size ) == 0 )
            ((haut_data*)p->userdata)->result->total_href++;

    } else if( strncasecmp( elem.data, "img", elem.size ) == 0 ) {
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
        string_t* buf = &((haut_data*)p->userdata)->innertext;
        string_append( buf, " ", 1 );
        string_append( buf, text->data, text->size );
        return;
    } 

    strfragment_t elem =p->state.last_elem;
    if( strncasecmp( elem.data, "title", elem.size ) == 0 ) {
        // We have a title!
    }
}

int
test_haut( testresult_t* result, const char* htmlpage, size_t length ){
    
    // User data
    haut_data state ={ .in_body =0, .result =result, .images =0 };
    string_init( &state.innertext );

    // Construct parser object
    haut_t p;
    haut_init( &p );

    // Set the buffer and allow it to be modified (speedup)
    haut_set_input_mutable( &p, (char*)htmlpage, length );

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
    string_free( &state.innertext );

//    printf( "Total images: %d\n", state.images );

    result->total_bytes +=sizeof( haut_t );
    return 0;
}
