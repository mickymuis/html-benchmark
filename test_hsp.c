/*
 * Benchmark for HTML parsers, testing different HTML parsers on a large volume of HTML files
 *
 * Micky Faas <micky@edukitty.org>
 * (C) 2017, Leiden University
 */

#include "test_hsp.h"
#include "util.h"
#include "htmlstreamparser/htmlstreamparser.h"

#define TITLE_MAXLEN 70

#define TAG_IMG "img"
#define TAG_IMG_LEN 3
#define TAG_A "a"
#define TAG_A_LEN 1
#define TAG_ENDP "/p"
#define TAG_ENDP_LEN 2
#define TAG_ENDTITLE "/title"
#define TAG_ENDTITLE_LEN 6
#define ATTR_HREF "href"
#define ATTR_HREF_LEN 4
#define ATTR_SRC "src"
#define ATTR_SRC_LEN 3
#define ATTR_ALT "alt"
#define ATTR_ALT_LEN 3

static struct {
    HTMLSTREAMPARSER* hsp;
    char tag_buf[9];
    char attr_buf[9];
    char val_buf[128]; 	
    char inner_buf[8192];
} _test_hsp;

void
test_hsp_init() {
    /*a pointer to the HTMLSTREAMPARSER structure and initialization*/
    _test_hsp.hsp =html_parser_init();
}

void 
test_hsp_cleanup() {
    // release the hsp
    html_parser_cleanup(_test_hsp.hsp);
}

int
test_hsp( testresult_t* result, const char* htmlpage, size_t length ) {
    
    HTMLSTREAMPARSER *hsp =_test_hsp.hsp;
    html_parser_reset( _test_hsp.hsp );
    html_parser_set_tag_to_lower(_test_hsp.hsp, 1);   
    html_parser_set_attr_to_lower(_test_hsp.hsp, 1); 
    html_parser_set_tag_buffer(_test_hsp.hsp, _test_hsp.tag_buf, sizeof(_test_hsp.tag_buf));  
    html_parser_set_attr_buffer(_test_hsp.hsp, _test_hsp.attr_buf, sizeof(_test_hsp.attr_buf));    
    html_parser_set_val_buffer(_test_hsp.hsp, _test_hsp.val_buf, sizeof(_test_hsp.val_buf)-1); 
    html_parser_set_inner_text_buffer(_test_hsp.hsp, _test_hsp.inner_buf, sizeof(_test_hsp.inner_buf)-1);

    // HSP only uses a static amount of memory regardless of the number of processed pages
    result->total_bytes =sizeof(HTMLSTREAMPARSER) + 18 + 128 + 8192;


    // We collect all inner-text as a big string
    string_t innertext;
    string_init( &innertext );

    for ( int i = 0; i < length; i++) 
    {             
            html_parser_char_parse(hsp, ((char *)htmlpage)[i]);

            /* Case 1: found `<a href=', add link to queue and link-index */
            if (html_parser_cmp_tag( hsp, TAG_A, TAG_A_LEN )) {
                    if (html_parser_cmp_attr( hsp, ATTR_HREF, ATTR_HREF_LEN ))  
                            if (html_parser_is_in(hsp, HTML_VALUE_ENDED))
                            { 
                                result->total_href++;
                            }
            }
            /* Case 2: found `<img ...' */
            if (html_parser_cmp_tag( hsp, TAG_IMG, TAG_IMG_LEN )) {
                int img_alt =0, img_src =0;

                while( ++i < length-1 && !html_parser_is_in(hsp, HTML_TAG_END ) ) {
                    html_parser_char_parse(hsp, ((char *)htmlpage)[i]);
                    
                    if (html_parser_cmp_attr( hsp, ATTR_SRC, ATTR_SRC_LEN ))  {
                            if (html_parser_is_in(hsp, HTML_VALUE_ENDED) && !img_src )
                            {
                                result->total_src++;
                                img_src =1;
                            }
                    }
                    else if (html_parser_cmp_attr( hsp, ATTR_ALT, ATTR_ALT_LEN ))  {
                            if (html_parser_is_in(hsp, HTML_VALUE_ENDED) && !img_alt )
                            { 
                                size_t len = html_parser_val_length(hsp);
                                char *trim =html_parser_replace_spaces(html_parser_trim(html_parser_val(hsp), &len), &len);
                                *trim =0;

                                img_alt =1;
                                result->total_alt++;
                            }
                    }
                }
            }
            // Case 3: (end of) the <title> tag
            else if( html_parser_cmp_tag( hsp, TAG_ENDTITLE, TAG_ENDTITLE_LEN ) ) {
                if (html_parser_is_in(hsp, HTML_TAG_END)){

                    size_t len = html_parser_inner_text_length(hsp);
                    char *title_trim =html_parser_replace_spaces(html_parser_trim(html_parser_inner_text(hsp), &len), &len);
                    *title_trim=0;
                    
                    if( !len ) continue;
                }
            }
            // Case 4: inner text any other element
            else if (html_parser_is_in(hsp, HTML_TAG_END) && html_parser_is_in(hsp, HTML_CLOSING_TAG) ) {
                    size_t text_len = html_parser_inner_text_length(hsp);
                    char* text = html_parser_replace_spaces(html_parser_trim(html_parser_inner_text(hsp), &text_len), &text_len);
//                    text[text_len]=0;

//                    printf( "\n#%s#\n", text );
//                    *text=0;
                    
                    if( text_len ) {
                        result->total_innertext_bytes +=text_len;
                        string_append( &innertext, text, text_len );
                    }


            }
                  
    }		

//    printf( "INNER TEXT: #%s#\n", innertext.data );
    string_free( &innertext );

    return 0;
}
