#pragma once

#include "stz2/types.h"

#include <md4c.h>

typedef struct
{
    i32s order;
    i32s levels;
    i32s parents;

} TreeDF;

typedef struct
{
    Strs   headers;
    TreeDF tree;

} MdTOC;

MdTOC md_get_toc(Buf* b, Str text);

// ### function `md_parse`
// provided by `"md4c.h"`
//
// → `int`
// Parameters:
// - `const MD_CHAR * text (aka const char *)`
// - `MD_SIZE size (aka unsigned int)`
// - `const MD_PARSER * parser (aka const struct MD_PARSER *)`
// - `void * userdata`
//
// Parse the Markdown document stored in the string 'text' of size 'size'.
// The parser provides callbacks to be called during the parsing so the caller can render the document on the screen or convert the Markdown to another format.
// Zero is returned on success. If a runtime error occurs (e.g. a memory fails), -1 is returned. If the processing is aborted due any callback returning non-zero, the return value of the callback is returned.
//
// int md_parse(const MD_CHAR *text, MD_SIZE size, const MD_PARSER *parser,
//              void *userdata)
//
// MD_HTML render = { process_output, userdata, renderer_flags, 0, { 0 } };
//
// typedef struct MD_HTML_tag MD_HTML;
// struct MD_HTML_tag {
//     void (*process_output)(const MD_CHAR*, MD_SIZE, void*);
//     void* userdata;
//     unsigned flags;
//     int image_nesting_level;
//     char escape_map[256];
// };
//
// MD_PARSER parser = {
//     0,
//     parser_flags,
//     enter_block_callback,
//     leave_block_callback,
//     enter_span_callback,
//     leave_span_callback,
//     text_callback,
//     debug_log_callback,
//     NULL
// };
// /* Parser structure.
//  */
// typedef struct MD_PARSER {
//     /* Reserved. Set to zero.
//      */
//     unsigned abi_version;
//
//     /* Dialect options. Bitmask of MD_FLAG_xxxx values.
//      */
//     unsigned flags;
//
//     /* Caller-provided rendering callbacks.
//      *
//      * For some block/span types, more detailed information is provided in a
//      * type-specific structure pointed by the argument 'detail'.
//      *
//      * The last argument of all callbacks, 'userdata', is just propagated from
//      * md_parse() and is available for any use by the application.
//      *
//      * Note any strings provided to the callbacks as their arguments or as
//      * members of any detail structure are generally not zero-terminated.
//      * Application has to take the respective size information into account.
//      *
//      * Any rendering callback may abort further parsing of the document by
//      * returning non-zero.
//      */
//     int (*enter_block)(MD_BLOCKTYPE /*type*/, void* /*detail*/, void* /*userdata*/);
//     int (*leave_block)(MD_BLOCKTYPE /*type*/, void* /*detail*/, void* /*userdata*/);
//
//     int (*enter_span)(MD_SPANTYPE /*type*/, void* /*detail*/, void* /*userdata*/);
//     int (*leave_span)(MD_SPANTYPE /*type*/, void* /*detail*/, void* /*userdata*/);
//
//     int (*text)(MD_TEXTTYPE /*type*/, const MD_CHAR* /*text*/, MD_SIZE /*size*/, void* /*userdata*/);
//
//     /* Debug callback. Optional (may be NULL).
//      *
//      * If provided and something goes wrong, this function gets called.
//      * This is intended for debugging and problem diagnosis for developers;
//      * it is not intended to provide any errors suitable for displaying to an
//      * end user.
//      */
//     void (*debug_log)(const char* /*msg*/, void* /*userdata*/);
//
//     /* Reserved. Set to NULL.
//      */
//     void (*syntax)(void);
// } MD_PARSER;
//
