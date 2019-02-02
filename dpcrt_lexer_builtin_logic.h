/*
 * Copyright (C) 2019  Davide Paro
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef HGUARD_347dd5552b1b47e0bb8c91e25c672c29
#define HGUARD_347dd5552b1b47e0bb8c91e25c672c29

#include "dpcrt_utils.h"
#include "dpcrt_lexer.h"

__BEGIN_DECLS




/* Debug stuff */
void lex_debug_ensure_starts_with(struct lexer *lex, char *string);

/* Character based matching */
bool is_whitespace_char ( char c );
bool is_newline_char ( char c );
bool is_digit_char ( char c );
bool is_punctuator_char (char c);
bool is_alpha_char ( char c );
bool is_whitespace_char ( char c );



/* Generic Logic*/
bool is_whitespace ( struct lexer *lex );
void eat_whitespaces ( struct lexer *lex);
void lex_whitespaces ( struct lexer *lex, MArena *tokens_arena);
void lex_identifier_or_keyword( struct lexer *lex, MArena *tokens_arena );


/* C11 Specific Logic */
bool is_c11_numeric_constant(struct lexer *lex);
enum token_type lex_c11_numeric_constant( struct lexer *lex, MArena *tokens_arena);
bool is_c11_punctuator(struct lexer *lex);
enum token_type lex_c11_punctuator ( struct lexer *lex, MArena *tokens_arena );
bool is_c11_comment (struct lexer *lex);
bool is_c11_char_const_or_string_literal ( struct lexer *lex );
void lex_c11_char_const_or_string_literal ( struct lexer *lex, struct token *token );

/* XML Specific Logic */
bool is_xml_comment(struct lexer *lex);
void lex_xml_comment ( struct lexer *lex, MArena *tokens_arena);
bool is_xml_string_literal(struct lexer *lex);
void lex_xml_string_literal(struct lexer *lex, MArena *tokens_arena);
bool is_xml_escape_sequence(struct lexer *lex);
enum token_type lex_xml_escape_sequence(struct lexer *lex, MArena *tokens_arena);








__END_DECLS

#endif /* HGUARD_347dd5552b1b47e0bb8c91e25c672c29 */
