/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */


#include "benchmark.h"
#include "util.h"
#include "gumbo-parser/gumbo.h"
#include <string.h>
#include <stdlib.h>

/* Some of this code was taken from the examples/ subdirectory in the Gumbo repository 
 * It is licensed under the Apache license 2.0 */

static const char* 
find_title(const GumboNode* root) {
    if( root->type != GUMBO_NODE_ELEMENT ) return "";

    const GumboVector* root_children = &root->v.element.children;
    GumboNode* head = NULL;
    for (int i = 0; i < root_children->length; ++i) {
        GumboNode* child = root_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT &&
                child->v.element.tag == GUMBO_TAG_HEAD) {
            head = child;
            break;
        }
    }
    if( !head )
        return "";

    GumboVector* head_children = &head->v.element.children;
    for (int i = 0; i < head_children->length; ++i) {
        GumboNode* child = head_children->data[i];
        if (child->type == GUMBO_NODE_ELEMENT &&
                child->v.element.tag == GUMBO_TAG_TITLE) {
            if (child->v.element.children.length != 1) {
                return "";
            }
            GumboNode* title_text = child->v.element.children.data[0];
            if( title_text->type != GUMBO_NODE_TEXT && title_text->type != GUMBO_NODE_WHITESPACE)
                return "";
            return title_text->v.text.text;
        }
    }
    return "";
}

static void 
search_for_links(GumboNode* node, testresult_t* result ) {
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }
    GumboAttribute* attr;
    if (node->v.element.tag == GUMBO_TAG_A &&
            (attr = gumbo_get_attribute(&node->v.element.attributes, "href"))) {
        //    printf( "A HREF = %s\n", attr->value );
        result->total_href++;
    }
    if (node->v.element.tag == GUMBO_TAG_IMG &&
            (attr = gumbo_get_attribute(&node->v.element.attributes, "src"))) {
        //    printf( "IMG SRC = %s\n", attr->value );
        result->total_src++;
    }
    if (node->v.element.tag == GUMBO_TAG_IMG &&
            (attr = gumbo_get_attribute(&node->v.element.attributes, "alt"))) {
        //    printf( "IMG ALT = %s\n", attr->value );
        result->total_alt++;
    }

    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        search_for_links( (GumboNode*) children->data[i], result );
    }
}

static string_t
innertext( GumboNode* node) {
    if (node->type == GUMBO_NODE_TEXT) {
        string_t text;
        string_init( &text );
        string_append( &text, node->v.text.text, strlen( node->v.text.text ) );
        return text;
    } else if (node->type == GUMBO_NODE_ELEMENT &&
            node->v.element.tag != GUMBO_TAG_SCRIPT &&
            node->v.element.tag != GUMBO_TAG_STYLE &&
            node->v.element.tag != GUMBO_TAG_TITLE ) {

        string_t contents;
        string_init( &contents );

        GumboVector* children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            string_t text = innertext( (GumboNode*) children->data[i] );
            if (i != 0 && text.size ) {
                string_append( &contents, " ", 1 );
            }
            if( text.size ) {
                string_append( &contents, text.data, text.size );
                string_free( &text );
            }
        }
        return contents;
    } else {
        string_t nil = {0,0};
        return nil;
    }
}


void*
custom_malloc( void* userdata, size_t s ) {
    ((testresult_t*)userdata)->total_bytes += s;
    return malloc( s );
}

int
test_gumbo( testresult_t* result, const char* htmlpage, size_t length ) {

    GumboOptions opts =kGumboDefaultOptions;
    opts.allocator =custom_malloc;
    opts.userdata =result;
    
    GumboOutput* output = gumbo_parse_with_options( &opts, htmlpage, length );

    /*const char* title = */find_title(output->root);
    search_for_links( output->root, result );
    //    printf("\n TITLE: %s\n", title);
    string_t text =innertext( output->root );
    //printf( "INNER TEXT: #%s#\n", text.data );
    result->total_innertext_bytes += text.size;
    string_free( &text );

    gumbo_destroy_output(&kGumboDefaultOptions, output);
    return 0;
}



