/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "test_myhtml.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <myhtml/myhtml.h>
#include <myhtml/serialization.h>
#include <myencoding/encoding.h>

// We overide MyHTML's malloc function in order to keep track of allocated memory
testresult_t* k_result;

void*
mycore_malloc( size_t size ) {
    k_result->total_bytes +=size;
    return malloc( size );
}

static string_t
innertext( myhtml_tree_node_t* node ) {
    if( !node ) goto empty;

    string_t contents;
    string_init( &contents );
//    size_t len;

/*    const char* text =myhtml_node_text(node, &len);
    if( text ) {
        string_append( &contents, text, len );
        return contents;
    }*/

    myhtml_tree_node_t* n = node;
    while( n ) {
        size_t len;
        const char* text =myhtml_node_text( n, &len );
        if( text ) {
            string_append( &contents, text, len );
        } else {
            myhtml_tree_node_t* child =n->child;
            string_t tmp =innertext( child );
            if( tmp.data ) {
                string_append( &contents, tmp.data, tmp.size );
                string_free( &tmp );
            }
        }
        n =n->next;
    }
    return contents;

empty:;
    string_t nil = {0,0};
    return nil;
}

int
test_myhtml( testresult_t* result, const char* htmlpage, size_t length ){
    k_result =result;

    // basic init
    myhtml_t* myhtml = myhtml_create();
    myhtml_init(myhtml, MyHTML_OPTIONS_DEFAULT, 1, 0);
    
    // first tree init 
    myhtml_tree_t* tree = myhtml_tree_create();
    myhtml_tree_init(tree, myhtml);
   
    // set parse flags
    myhtml_tree_parse_flags_set(tree,
                MyHTML_TREE_PARSE_FLAGS_SKIP_WHITESPACE_TOKEN|
                MyHTML_TREE_PARSE_FLAGS_WITHOUT_DOCTYPE_IN_TREE);
    // parse html
    myhtml_parse(tree, MyENCODING_UTF_8, htmlpage, length);
    
    // find title
    myhtml_collection_t *collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_TITLE, NULL);
    
    if(collection && collection->list && collection->length) {
        myhtml_tree_node_t *text_node = myhtml_node_child(collection->list[0]);
        
        if(text_node) {
            const char* text = myhtml_node_text(text_node, NULL);
            
          //  if(text)
          //      printf("Title: %s\n", text);
        }
    } 

    myhtml_collection_destroy( collection );

    // find links
    
    collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_A, NULL);
    
    for(size_t i = 0; i < collection->length; i++) {
        myhtml_tree_attr_t* a = myhtml_attribute_by_key( collection->list[i], "href", 4 );
        if( !a )
            continue;
        size_t len;
        const char *href =myhtml_attribute_value( a, &len ); 
        //printf( "A HREF=%s\n", href );
        result->total_href++;
        myhtml_attribute_free( tree, a );
    }
    
    myhtml_collection_destroy( collection );
    
    // find images
    
    collection = myhtml_get_nodes_by_tag_id(tree, NULL, MyHTML_TAG_IMG, NULL);
    int images =0;
    
    for(size_t i = 0; i < collection->length; i++) {
        myhtml_tree_attr_t* src = myhtml_attribute_by_key( collection->list[i], "src", 3 );
        myhtml_tree_attr_t* alt = myhtml_attribute_by_key( collection->list[i], "alt", 3 );
        if( src ) {
            images++;
            size_t len;
            const char *value =myhtml_attribute_value( src, &len ); 
            //printf( "IMG SRC=%s\n", value );
            result->total_src++;
            myhtml_attribute_free( tree, src );
        }
        if( alt ) {
            size_t len;
            const char *value =myhtml_attribute_value( alt, &len ); 
            //printf( "IMG ALT=%s\n", value );
            result->total_alt++;
            myhtml_attribute_free( tree, alt );
        }
    }
    
//    printf( "Total images: %d\n", images );
    
    myhtml_collection_destroy( collection );

    // collect innertext
    myhtml_tree_node_t* body =myhtml_tree_get_node_body( tree );
    if( body ) {
        string_t text =innertext( body );
        if( text.data ) {
            //printf( "\nINNER TEXT: #%s#\n", text.data );
            result->total_innertext_bytes +=text.size;
            string_free( &text );
        }
    }

    // release resources
    myhtml_tree_destroy(tree);
    myhtml_destroy(myhtml);
    
    return 0;
}
