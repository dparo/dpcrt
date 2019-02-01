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
