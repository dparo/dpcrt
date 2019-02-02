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
#include "dpcrt_lexer_builtin_logic.h"
#include "dpcrt_lexer_core.h"



bool
is_newline_char ( char c )
{
    bool result = false;
    if ( c == '\n')
    {
        result = true;
    }
    else
    {
        result = false;
    }
    return result;
}




bool
is_digit_char ( char c )
{
    bool result = false;
    if ( (c >= '0' && c <= '9'))
    {
        result = true;
    }
    return result;
}

bool
is_alpha_char ( char c )
{
    return ( (c >= 'A' && c <= 'Z')
             || (c >= 'a' && c <= 'z')
             || (c == '_')
         # if LEXER_DOLLAR_SIGN_VALID_IDENTIFIER
             || (c == '$')
         # endif
        );
}


bool
is_whitespace_char ( char c )
{
    bool result = ((c == ' ')
                   || (c == '\t')
                   || (c == '\n')
                   || (c == '\r')
                   || (c == '\v')
                   || (c == '\f'));
    return (result);
}

bool
is_whitespace ( struct lexer *lex )
{
    bool result = false;
    char c = next(lex);
    if (is_whitespace_char(c))
    {
        result = true;
    }
    undo_staging_area(lex);
    return result;
}





void
eat_whitespaces ( struct lexer *lex)
{
    while( !is_end(lex) )
    {
        char c = next(lex);
        if ( is_whitespace_char(c))
        {
            lex_reject(lex);
        }
        else
        {
            break;
        }
    }
    undo_staging_area(lex);
}



void
lex_debug_ensure_starts_with(struct lexer *lex,
                             char *string)
{
    (void) lex, (void) string;
#if __DEBUG
    undo_staging_area(lex);
    for (char *c = string; *c; c++)
    {
        char temp = next(lex);
        assert(temp == *c);
    }
    undo_staging_area(lex);
#endif
}


void
lex_xml_comment ( struct lexer *lex,
                  MArena *tokens_arena)
{
    lex_debug_ensure_starts_with(lex, "<!--");

    // Do not store on the output the start of the comment
    lex_reject_cnt(lex, 4);

    char c;

    while ( !is_end(lex) )
    {
        c = next(lex);
        if ( c == '-')
        {
            c = next(lex);
            if ( c == '-')
            {
                c = next(lex);
                if  ( c == '>' )
                {
                    lex_reject_cnt(lex, 3);
                    break;
                }
                else
                {
                    lex_accept_cnt(lex, tokens_arena, 3);
                }
            }
            else
            {
                lex_accept_cnt(lex, tokens_arena, 2);
            }
        }
        else
        {
            lex_accept(lex, tokens_arena);
        }
    }
}


bool
is_punctuator_char (char c)
{
    // NOTE: See ascii table
#if LEXER_DOLLAR_SIGN_VALID_Identifier
    if ( c == '$' )
    {
        return false;
    }
#endif
    return ( (c >= 0x21 && c <= 0x2F)
             || (c >= 0x3A && c <= 0x40)
             || (c >= 0x5B && c <= 0x5E)
             || (c >= 0x7B) );
}

bool
is_c11_comment (struct lexer *lex)
{
    (void) lex;
    invalid_code_path("needs implementation");
    return false;
}




void
lex_whitespaces ( struct lexer *lex, MArena *tokens_arena)
{
    while( !is_end(lex) )
    {
        char c = next(lex);
        if ( is_whitespace_char(c))
        {
            lex_accept(lex, tokens_arena);
        }
        else
        {
            break;
        }
    }
}


bool
is_xml_comment(struct lexer *lex)
{
    bool result = false;
    // Xml Comment starting chars    <!--
    char c0 = next(lex);
    char c1 = next(lex);
    char c2 = next(lex);
    char c3 = next(lex);

    if ( c0 == '<' && c1 == '!' && c2 == '-' && c3 == '-' )
    {
        result = true;
    }
    else
    {
        result = false;
    }

    undo_staging_area(lex);
    return result;
}

bool
is_xml_string_literal(struct lexer *lex)
{
    char c0 = next(lex);

    bool result = ( (c0 == '\'') || (c0 == '"'));

    undo_staging_area(lex);
    return result;
}

void
lex_xml_string_literal(struct lexer *lex, MArena *tokens_arena)
{
    char delimiter = next(lex);
    assert_msg( delimiter == '\'' || delimiter == '"', "call `is_start_of_xml_string_literal()` before this function");
    lex_reject(lex);

    bool found_ending_delimiter = false;

    while ( !is_end(lex) )
    {
        char c0 = next(lex);
        if ( c0 == '\\')
        {
            char c1 = next(lex);
            if (c1 == delimiter) {
                lex_reject_cnt(lex, 2);
                lex_emit(lex, tokens_arena, c1);
            } else if (c1 == '\\') {
                lex_reject_cnt(lex, 2);
                lex_emit(lex, tokens_arena, '\\');
            } else if (c1 == '\n') {
                lex_reject_cnt(lex, 2);
                lex_emit(lex, tokens_arena, '\n');
            } else if (c1 == '\f') {
                lex_reject_cnt(lex, 2);
                lex_emit(lex, tokens_arena, '\f');
            } else if (c1 == '\t') {
                lex_reject_cnt(lex, 2);
                lex_emit(lex, tokens_arena, '\t');
            } else {
                lex_reject(lex);
            }
        }
        else if (c0 == delimiter)
        {
            found_ending_delimiter = true;
            lex_reject(lex);
            break;
        }
        else if (c0 == '\n')
        {
            errfmt (lex, LexerErr_InvalidString, "Invalid use of new line while lexing string at line: %d, column: %d\n", lex->line_num, lex->column);
            lex_reject(lex);
            break;
        }
        else
        {
            lex_accept(lex, tokens_arena);
        }
    }

    if (!found_ending_delimiter)
    {
        errfmt(lex, LexerErr_PrematureEndOfString, "Hitted END OF FILE prematurely while lexing inside a string. String was not closed before end of file");
    }
}

bool
is_xml_escape_sequence(struct lexer *lex)
{

    bool result = false;

    /* &lt; &gt; &amp; &apos; &quot; */
    char c0 = next(lex);
    char c1 = next(lex);
    char c2 = next(lex);
    char c3 = next(lex);
    char c4 = next(lex);
    char c5 = next(lex);

    result = ( c0 == '&' && c1 == 'l' && c2 == 't' && c3 == ';')
        || ( c0 == '&' && c1 == 'g' && c2 == 't' && c3 == ';')
        || ( c0 == '&' && c1 == 'a' && c2 == 'm' && c3 == 'p' && c4 == ';')
        || ( c0 == '&' && c1 == 'a' && c2 == 'p' && c3 == 'o' && c4 == 's' && c5 == ';')
        || ( c0 == '&' && c1 == 'q' && c2 == 'u' && c3 == 'o' && c4 == 't' && c5 == ';');

    undo_staging_area(lex);
    return result;
}

enum token_type
lex_xml_escape_sequence(struct lexer *lex, MArena *tokens_arena)
{
    enum token_type result = TokenType_Null;
    /* &lt; &gt; &amp; &apos; &quot; */
    char c0 = next(lex);
    char c1 = next(lex);
    char c2 = next(lex);
    char c3 = next(lex);
    char c4 = next(lex);
    char c5 = next(lex);

    if ( c0 == '&' && c1 == 'l' && c2 == 't' && c3 == ';')
    {
        lex_reject_cnt(lex, 4);
        lex_emit(lex, tokens_arena, '<');
        result = TokenType_Less;
    }
    else if ( c0 == '&' && c1 == 'g' && c2 == 't' && c3 == ';')
    {
        lex_reject_cnt(lex, 4);
        lex_emit(lex, tokens_arena, '>');
        result = TokenType_Greater;
    }
    else if ( c0 == '&' && c1 == 'a' && c2 == 'm' && c3 == 'p' && c4 == ';')
    {
        lex_reject_cnt(lex, 5);
        lex_emit(lex, tokens_arena, '&');
        result = TokenType_BitwiseAnd;
    }
    else if ( c0 == '&' && c1 == 'a' && c2 == 'p' && c3 == 'o' && c4 == 's' && c5 == ';')
    {
        lex_reject_cnt(lex, 6);
        lex_emit(lex, tokens_arena, '\'');
        result = TokenType_Apostrophe;
    }
    else if ( c0 == '&' && c1 == 'q' && c2 == 'u' && c3 == 'o' && c4 == 't' && c5 == ';')
    {
        lex_reject_cnt(lex, 6);
        lex_emit(lex, tokens_arena, '"');
        result = TokenType_Quote;
    }
    else
    {
# if __DEBUG
        // Invalid code path
        assert_msg(0, "invalid code path, you should match with `is_start_of_xml_escape_sequence before calling this function");
        result = TokenType_Null;
# endif
    }

    return result;
}

bool
is_c11_numeric_constant(struct lexer *lex)
{
    bool result = false;

    char c0 = next(lex);

    if (is_digit_char(c0) || c0 == '.')
    {
        result = true;
    }

    undo_staging_area(lex);
    return result;
}


enum token_type
lex_c11_numeric_constant( struct lexer *lex, MArena *tokens_arena)
{
    /*
     * @ TODO :: Improve lexing of numeric constants
     */

    enum token_type type = TokenType_IntegerLiteral;

    bool is_hex = false;
    bool is_binary = false;

    (void) is_hex, (void) is_binary;

    // Match prefixes
    {
        char c0 = next(lex);
        char c1 = next(lex);
        if ( c0 == '0' && (c1 == 'x' || c1 == 'X'))
        {
            // hexadecimal integer literal
            is_hex = true;
            lex_accept_cnt(lex, tokens_arena, 2);
        }
        else if ( c0 == '0' && (c1 == 'b' || c1 == 'B'))
        {
            is_binary = true;
            lex_accept(lex, tokens_arena);
        }
        undo_staging_area(lex);
    }




    bool dot_found = false;
    bool exponent_found = false;

    { // lex_digits
        char c0 = 0;

        while( !is_end(lex))
        {
            c0 = next(lex);
            if ( is_digit_char(c0))
            {
                lex_accept(lex, tokens_arena);
            }
            else if (is_hex)
            {
                if (c0 == 'a' || c0 == 'A' || c0 == 'b' || c0 == 'B'
                    || c0 == 'c' || c0 == 'C' || c0 == 'd' || c0 == 'D'
                    || c0 == 'f' || c0 == 'F')
                {
                    lex_accept(lex, tokens_arena);
                }
                else
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        undo_staging_area(lex);
    }


    while (!is_end(lex))
    {

        char c0 = next(lex);

        if (is_digit_char(c0))
        {
            lex_accept(lex, tokens_arena);
        }
        else if (!dot_found && c0 == '.')
        {
            dot_found = true;
            lex_accept(lex, tokens_arena);
            type = TokenType_FloatLiteral;
        }
        else if ( (!exponent_found) && (c0 == 'e' || c0 == 'E'))
        {
            if (!is_hex)
            {
                exponent_found = true;
                type = TokenType_FloatLiteral;
                char c1 = next(lex);
                if ( c1 == '+' || c1 == '-')
                {
                    lex_accept_cnt(lex, tokens_arena, 2);
                }
                else
                {
                    lex_accept(lex, tokens_arena);
                }
            }
            else
            {
                break;
            }
        }
        else if ((!exponent_found) && (c0 == 'p' || c0 == 'P'))
        {
            if (is_hex)
            {
                exponent_found = true;
                type = TokenType_FloatLiteral;
                char c1 = next(lex);
                if ( c1 == '+' || c1 == '-')
                {
                    lex_accept_cnt(lex, tokens_arena, 2);
                }
                else
                {
                    lex_accept(lex, tokens_arena);
                }
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }
    }


    // Match suffixes
    {
        undo_staging_area(lex);
        char c0 = next(lex);

        if ((type == TokenType_FloatLiteral) && (c0 == 'f' || c0 == 'F'))
        {
            char c1 = next(lex);
            if ( c1 == 'f' || c1 == 'F' )
            {
                lex_accept_cnt(lex, tokens_arena, 2);
            }
            else
            {
                lex_accept(lex, tokens_arena);
            }
        }
        else if ((type == TokenType_IntegerLiteral) && (c0 == 'u' || c0 == 'U'))
        {
            lex_accept(lex, tokens_arena);
        }
        else if ((c0 == 'l' || c0 == 'L'))
        {
            char c1 = next(lex);
            if ( c1 == 'l' || c0 == 'L')
            {
                lex_accept_cnt(lex, tokens_arena, 2);
            }
            else
            {
                lex_accept(lex, tokens_arena);
            }
        }
    }

    return type;
}



void
lex_identifier_or_keyword( struct lexer *lex, MArena *tokens_arena )
{
    char c;

    while ( !is_end(lex) )
    {
        c = next(lex);
        if ( is_whitespace_char(c))
        {
            break;
        }
        else if (is_punctuator_char(c))
        {
            break;
        }
        else
        {
            lex_accept(lex, tokens_arena);
        }
    }
}


bool
is_char_const_or_string_literal ( struct lexer *lex )
{
    bool result = false;
    char c0 = next(lex);
    char c1 = next(lex);
    char c2 = next(lex);

    if ( c0 == '\'' || c0 == '\"')
    {
        result = true;
    }
    else if ( c0 == 'u' && c1 == '8' && c2 == '\"')
    {
        result = true;
    }
    else if (c0 == 'L' || c0 == 'U' || c0 == 'u')
    {
        if ( c1 == '\'' || c1 == '\"')
        {
            result = true;
        }
    }
    undo_staging_area(lex);
    return result;
}

void
lex_char_const_or_string_literal ( struct lexer *lex, struct token *token )
{
    (void) lex, (void) token;
    invalid_code_path("needs implementation");
}



bool
is_c11_punctuator(struct lexer *lex)
{
    bool result = false;
    char c0 = next(lex);
    if ( is_punctuator_char(c0))
    {
        result = true;
    }
    undo_staging_area(lex);
    return result;
}

enum token_type
lex_c11_punctuator ( struct lexer *lex, MArena *tokens_arena )
{
    enum token_type type = TokenType_Null;

    char c1 = next(lex);
    char c2 = next(lex);
    char c3 = next(lex);

    assert(is_punctuator_char(c1));
    int cnt = 0;


    if ( c1 == '(') {
        type = TokenType_OpenParen;
        cnt = 1;
    }
    else if ( c1 == ')') {
        type = TokenType_CloseParen;
        cnt = 1;
    }
    else if ( c1 == '[') {
        type = TokenType_OpenBracket;
        cnt = 1;
    }
    else if ( c1 == ']') {
        type = TokenType_CloseBracket;
        cnt = 1;
    }
    else if ( c1 == '{') {
        type = TokenType_OpenBrace;
        cnt = 1;
    }
    else if ( c1 == '}') {
        type = TokenType_CloseBrace;
        cnt = 1;
    }
    else if ( c1 == ';') {
        type = TokenType_Semicolon;
        cnt = 1;
    }
    else if ( c1 == '?') {
        type = TokenType_QuestionMark;
        cnt = 1;
    }
    else if ( c1 == ':') {
        type = TokenType_Colon;
        cnt = 1;
    }
    else if ( c1 == '.') {
        if ( c2 == '.' && c3 == '.' ) {
            type = TokenType_DotDotDot;
            cnt = 3;
        }
        else {
            type = TokenType_Dot;
            cnt = 1;
        }
    }
    else if ( c1 == ',') {
        type = TokenType_Comma;
        cnt = 1;
    }
    else if ( c1 == '\\') {
        type = TokenType_BackwardSlash;
        cnt = 1;
    }
    else if ( c1 == '`') {
        type = TokenType_BackwardApostrophe;
        cnt = 1;
    }
    else if (c1 == '"') {
        type = TokenType_Quote;
        cnt = 1;
    }
    else if (c1 == '\'') {
        type = TokenType_Apostrophe;
        cnt = 1;
    }
    else if (c1 == '@') {
        type = TokenType_AtSign;
        cnt = 1;
    }
    else if ( c1 == '+') {
        if ( c2 == '+') {
            type = TokenType_Increment;
            cnt = 2;
        }
        else if ( c2 == '=' ) {
            type = TokenType_AddEqual;
            cnt = 2;
        }
        else {
            type = TokenType_Plus;
            cnt = 1;
        }
    }
    else if ( c1 == '-') {
        if ( c2 == '-' ) {
            type = TokenType_Decrement;
            cnt = 2;
        }
        else if ( c2 == '>' ) {
            type = TokenType_Arrow;
            cnt = 2;
        }
        else if ( c2 == '=' ) {
            type = TokenType_SubtractEqual;
            cnt = 2;
        }
        else {
            type = TokenType_Minus;
            cnt = 1;
        }
    }
    else if ( c1 == '*' ) {
        if ( c2 == '=' ) {
            type = TokenType_MultiplyEqual;
            cnt = 2;
        }
        else {
            type = TokenType_Asterisk;
            cnt = 1;
        }
    }
    else if (c1 == '/' ) {
        if ( c2 == '=' ) {
            type = TokenType_DivideEqual;
            cnt = 2;
        } else if ( c2 == '>' ) {
            type = TokenType_DivideGreater;
            cnt = 2;
        } else {
            type = TokenType_Divide;
            cnt = 1;
        }
    }
    else if (c1 == '%' ) {
        if ( c2 == '=' ) {
            type = TokenType_ModuloEqual;
            cnt = 2;
        }
        else {
            type = TokenType_Modulo;
            cnt = 1;
        }

    }
    else if ( c1 == '=' ) {
        if ( c2 == '=' ) {
            type = TokenType_EqualEqual;
            cnt = 2;
        }
        else {
            type = TokenType_Equal;
            cnt = 1;
        }
    }
    else if ( c1 == '!' ) {
        if ( c2 == '=' ) {
            type = TokenType_NotEqual;
            cnt = 2;
        }
        else {
            type = TokenType_LogicalNot;
            cnt = 1;
        }
    }
    else if ( c1 == '>' ) {
        if ( c2 == '=' ) {
            type = TokenType_GreaterEqual;
            cnt = 2;
        }
        else if ( c2 == '>') {
            if ( c3 == '=' ) {
                type = TokenType_BitwiseRightShiftEqual;
                cnt = 3;
            }
            else {
                type = TokenType_BitwiseRightShift;
                cnt = 2;
            }
        }
        else {
            type = TokenType_Greater;
            cnt = 1;
        }
    }
    else if ( c1 == '<' ) {
        if ( c2 == '=' ) {
            type = TokenType_LessEqual;
            cnt = 2;
        }
        else if ( c2 == '<') {
            if ( c3 == '=' ) {
                type = TokenType_BitwiseLeftShiftEqual;
                cnt = 3;
            }
            else {
                type = TokenType_BitwiseLeftShift;
                cnt = 2;
            }
        }
        else {
            type = TokenType_Less;
            cnt = 1;
        }
    }
    else if ( c1 == '~' ) {
        type = TokenType_BitwiseNot;
        cnt = 1;
    }
    else if ( c1 == '&' ) {
        if ( c2 == '=' ) {
            type = TokenType_BitwiseAndEqual;
            cnt = 2;
        }
        else if (c2 == '&') {
            type = TokenType_LogicalAnd;
            cnt = 2;
        }
        else {
            type = TokenType_BitwiseAnd;
            cnt = 1;
        }
    }
    else if ( c1 == '|' ) {
        if ( c2 == '=' ) {
            type = TokenType_BitwiseOrEqual;
            cnt = 2;
        }
        else if (c2 == '|') {
            type = TokenType_LogicalOr;
            cnt = 2;
        }
        else {
            type = TokenType_BitwiseOr;
            cnt = 1;
        }
    }
    else if ( c1 == '^' ) {
        if ( c2 == '=' ) {
            type = TokenType_BitwiseXorEqual;
            cnt = 2;
        }
        else {
            type = TokenType_BitwiseXor;
            cnt = 1;
        }
    }

    lex_accept_cnt(lex, tokens_arena, cnt);
    assert (type != TokenType_Null);
    return type;
}
