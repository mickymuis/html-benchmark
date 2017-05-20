/*
 * gen_entities - Generate Finite State Machine for parsing of HTML5 entities
 *                given the original HTML5 specification on character references
 *                in JSON format
 *
 * Micky E. Faas, University of Leiden (C) 2017
 * https://github.com/mickymuis/haut-html
 *
 * This file is based on the example for the json-parser 
 * Copyright (C) 2015 Mirko Pasqualetti  All rights reserved.
 * https://github.com/udp/json-parser
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "json.h"

/*
 * Test for json.c
 *
 * Compile with
 *         gcc -o gen_entities gen_entities.c json.c -lm
 *
 * USAGE: ./gen_entities <json file>
 */

// The range of input ASCII characters, 
// minimal set for HTML character references
#define FIRST_CHAR 59 // ';'
#define LAST_CHAR 122 // 'z'
#define NUM_STATES 3 // Only `none', `success' and `error'

#define ONLY_ONE_CODEPOINT

static uint32_t global_mask =0;

static int
process_codepoints( json_value* value ) {
    if (value == NULL) {
        return 1;
    }
    if( value->type != json_array ) {
        fprintf( stderr, "ERROR: expected array `codepoints' " );
        return 1;
    }
    printf( "{ " );


    int length, x;
    length = value->u.object.length;

    for (x = 0; x < length; x++) {
        if( value->u.array.values[x]->type != json_integer ) {
            fprintf( stderr, "ERROR: expected integer in array `codepoints' " );
            return 1;
        }
        printf("%" PRId64 "", value->u.array.values[x]->u.integer);

        global_mask |=value->u.array.values[x]->u.integer;

#ifdef ONLY_ONE_CODEPOINT
        break;
#endif
        if( x != length-1 )
            printf( ", ");
    }
    printf( " }\n" );
    return 0;

}

static int
process_entity_object( json_value* value ) {
    if (value == NULL) {
        return 1;
    }
    if( value->type != json_object ) {
        fprintf( stderr, "ERROR: expected object " );
        return 1;
    }
    int length, x;
    length = value->u.object.length;
    for (x = 0; x < length; x++) {
        if( strcmp( value->u.object.values[x].name, "codepoints" ) == 0 ) {
            if( process_codepoints(value->u.object.values[x].value) != 0 )
                return 1;
            return 0;
        }
    }
    fprintf( stderr, "ERROR: no such field `codepoints' " );
    return 1;
}

static int
process_root( json_value* value ) {
    if (value == NULL) {
        return 1;
    }
    if( value->type != json_object ) {
        fprintf( stderr, "ERROR: expected root object\n" );
        return 1;
    }
    int length, x;
    length = value->u.object.length;

    printf( "%%! %d %d\n\n", NUM_STATES, LAST_CHAR - FIRST_CHAR ); 
    printf( "**, ** => { 2 }\n" );

    for (x = 0; x < length; x++) {
        const char *name =value->u.object.values[x].name;
        if( name[strlen(name)-1] != ';' ) {
            continue;
        }
        printf("0, \"%s\" => ", name+1);
        if( process_entity_object( value->u.object.values[x].value ) != 0 ) {
            fprintf( stderr, "in `%s'\n", name);
            return 1;
        }
    }
    return 0;
}

int 
main(int argc, char** argv)
{
    char* filename;
    FILE *fp;
    struct stat filestatus;
    int file_size;
    char* file_contents;
    json_char* json;
    json_value* value;

    if (argc != 2) {
        fprintf(stderr, "%s <json html entities file>\n", argv[0]);
        return 1;
    }
    filename = argv[1];

    if ( stat(filename, &filestatus) != 0) {
        fprintf(stderr, "File %s not found\n", filename);
        return 1;
    }
    file_size = filestatus.st_size;
    file_contents = (char*)malloc(filestatus.st_size);
    if ( file_contents == NULL) {
        fprintf(stderr, "ERROR: unable to allocate %d bytes\n", file_size);
        return 1;
    }

    fp = fopen(filename, "rt");
    if (fp == NULL) {
        fprintf(stderr, "ERROR: Unable to open %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    if ( fread(file_contents, file_size, 1, fp) != 1 ) {
        fprintf(stderr, "ERROR: Unable to read content of %s\n", filename);
        fclose(fp);
        free(file_contents);
        return 1;
    }
    fclose(fp);

    json = (json_char*)file_contents;

    value = json_parse(json,file_size);

    if (value == NULL) {
        fprintf(stderr, "ERROR: Unable to parse data\n");
        free(file_contents);
        exit(1);
    }

    process_root(value);

    json_value_free(value);
    free(file_contents);

    fprintf( stderr, "Used bits: 0x%x\n", global_mask );
    return 0;
}
