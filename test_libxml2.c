/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "test_libxml2.h"
#include "util.h"
#include <libxml/HTMLparser.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    testresult_t *result;
    int inside_body;
    int inside_title;
    int inside_script;
    int inside_text;
    string_t innertext;

} parser_state;

static void 
_startElement (void * context,  const xmlChar * name,  const xmlChar ** atts) {
    parser_state* state =(parser_state*)context;
    testresult_t *result =state->result;

//    printf( "starts %s\n", (const char*)name );
    
    if( strcmp( (const char*)name, "title" ) ==0 )
        state->inside_title =1;
    else if( strcmp( (const char*)name, "body" ) == 0 )
        state->inside_body =1;
    else if( strcmp( (const char*)name, "script" ) == 0)
        state->inside_script =1;
    else {
        int i =0;
        while( atts ) {
            const char *att =(const char*)atts[i];
            if( !att ) break;
            const char *val =(const char*)atts[i+1];
            if( !val ) break;
            
            if( strcmp( (const char*)name, "img" ) == 0 ) {
                if( strcmp( att, "src" ) == 0 ) {
                    //printf( "have img src: %s\n", val );
                    result->total_src++;
                } else if( strcmp( att, "alt" ) == 0 ) {
                    //printf( "have img alt: %s\n", val );
                    result->total_alt++;

                }
            }
            
            if( strcmp( (const char*)name, "a" ) == 0 ) {
                if( strcmp( att, "href" ) == 0 ) {
                    //printf( "have a href: %s\n", val );
                    result->total_href++;
                } 
            }

            i+=2;


        }

    }
    state->inside_text =0;
}

static void 
_endElement(void *context, const xmlChar *name) {
    parser_state* state =(parser_state*)context;

    if( strcmp( (const char*)name, "title" ) == 0 )
        state->inside_title =0;
    else if( strcmp( (const char*)name, "body" ) == 0 )
        state->inside_body =0;
    else if( strcmp( (const char*)name, "script" ) == 0 )
        state->inside_script =0;

    state->inside_text =0;

}

static int
isAllWhitespace( const char* str, int len ) {
    for(int i =0; i < len; i++ )
        if( !isspace( str[i]  ))
                return 0;
    return 1;
}

static void 
_characters(void *context, const xmlChar *ch, int len) {
    parser_state* state =(parser_state*)context;
    testresult_t *result =state->result;
    if( state->inside_body && !state->inside_script ) {
        if( isAllWhitespace( (const char*)ch, len ) ){ // Compact large chuncks of whitespace
            if( state->inside_text ) {
             //   result->total_innertext_bytes ++;
                string_append( &state->innertext, " ", 1 );
            }
        }
        else {
//            result->total_innertext_bytes += len;
            state->inside_text =1;
//            fprintf( stdout, "\n#%s#\n", (const char*)ch );
            string_append( &state->innertext, (const char*)ch, len );
        }
    }
}

int
test_libxml2( testresult_t* result, const char* htmlpage, size_t length ) {

    htmlSAXHandler handler;
    memset( &handler, 0, sizeof( htmlSAXHandler ) );

    handler.startElement =_startElement;
    handler.endElement =_endElement;
    handler.characters =_characters;

    parser_state state;
    memset( &state, 0, sizeof( parser_state ) );
    state.result =result;
    string_init( &state.innertext );

    xmlCharEncoding charEnc = xmlDetectCharEncoding( (unsigned char*)htmlpage, length);

    htmlParserCtxtPtr ctxt =htmlCreatePushParserCtxt( &handler, (void*)&state, htmlpage, length, NULL, charEnc );

    htmlCtxtUseOptions( ctxt, HTML_PARSE_RECOVER | HTML_PARSE_NONET |
                    HTML_PARSE_COMPACT | HTML_PARSE_NOERROR | HTML_PARSE_NOWARNING | HTML_PARSE_NOBLANKS );

    htmlParseDocument( ctxt );

    htmlFreeParserCtxt( ctxt );

    result->total_innertext_bytes +=state.innertext.size;
//    printf( "INNER TEXT: #\%s#\n", state.innertext.data );

    string_free( &state.innertext );

    result->total_bytes += sizeof( htmlSAXHandler ) + sizeof( htmlParserCtxt ) + sizeof( parser_state );

    return 0;
}
