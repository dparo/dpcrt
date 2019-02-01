/*
 * Copyright (C) 2018  Davide Paro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "dpcrt.h"
#include "dpcrt_streams.h"
#include "dpcrt_allocators.h"


/* =======================================
   LEXER CONFIGS 
   =======================================*/

/* Should the lexer generate new tokens for whitespaces,
   or simply "eat" them without generating tokens.
   @NOTE: Enabling this flag may require a more difficult
   parser implementation since it must be aware that 
   whitespaces between words and identifier maybe present or not,
   depending on the input*/
#ifndef LEX_WHITESPACES_ENABLED
#  define LEX_WHITESPACES_ENABLED (0)
#endif




__BEGIN_DECLS

enum token_type {
    TokenType_Null              = 0,
    TokenType_InlineComment     = 1,
    TokenType_EnclosedComment   = 2,
    TokenType_Whitespace        = 3,

    // @NOTE: Keywords are just identifiers, only the parser gives more meaning to them
    //        but the lexer doesn't care or differentiate between the 2
    TokenType_Identifier        = 4,  /* a  b  hello  __id__  */
    TokenType_Keyword           = 4,

    TokenType_StringLiteral     = 5,  /* "hello \"world\"" */
    TokenType_CharacterConstant = 6,  /* 'a' '\n' */
    TokenType_IntegerLiteral    = 7,
    TokenType_FloatLiteral      = 8,
    TokenType_Directive         = 9,  /* #define, #include */


    TokenType_Less          = 10,  TokenType_XmlOpeningTag = 10,    /* < */
    TokenType_DivideGreater = 11,  TokenType_XmlClosingTag = 11,    /* /> */



    TokenType_EqualEqual,               /* == */
    TokenType_NotEqual,                 /* != */
    TokenType_Greater,                  /* > */
    TokenType_GreaterEqual,             /* >= */
    TokenType_LessEqual,                /* <= */

    TokenType_Equal,                    /* = */
    TokenType_Plus,                     /* + */
    TokenType_Minus,                    /* - */
    TokenType_Asterisk,                 /* * */
    TokenType_Divide,                   /* / */
    TokenType_Modulo,                   /* % */
    TokenType_Increment,                /* ++ */
    TokenType_Decrement,                /* -- */

    TokenType_AddEqual,                 /* += */
    TokenType_SubtractEqual,            /* -= */
    TokenType_MultiplyEqual,            /* *= */
    TokenType_DivideEqual,              /* /= */
    TokenType_ModuloEqual,              /* %= */
    TokenType_BitwiseAndEqual,          /* &= */
    TokenType_BitwiseOrEqual,           /* |= */
    TokenType_BitwiseXorEqual,          /* ^= */
    TokenType_BitwiseLeftShiftEqual,    /* <<= */
    TokenType_BitwiseRightShiftEqual,   /* >>= */


    TokenType_LogicalNot,               /* ! */
    TokenType_LogicalAnd,               /* && */
    TokenType_LogicalOr,                /* || */

    TokenType_BitwiseNot,               /* ~ */
    TokenType_BitwiseAnd,               /* & */
    TokenType_BitwiseOr,                /* | */
    TokenType_BitwiseXor,               /* ^ */
    TokenType_BitwiseLeftShift,         /* <<= */
    TokenType_BitwiseRightShift,        /* >>= */


    TokenType_OpenParen,                /* ( */
    TokenType_CloseParen,               /* ) */
    TokenType_OpenBracket,              /* [ */
    TokenType_CloseBracket,             /* ] */
    TokenType_OpenBrace,                /* { */
    TokenType_CloseBrace,               /* } */

    TokenType_Dot,                      /* . */
    TokenType_Comma,                    /* , */
    TokenType_Semicolon,                /* ; */
    TokenType_Colon,                    /* : */
    TokenType_QuestionMark,             /* ? */
    TokenType_Arrow,                    /* -> */
    TokenType_DotDotDot,                /* ... */

    TokenType_BackwardSlash,            /* \ */
    TokenType_Quote,                    /* " */
    TokenType_Apostrophe,               /* ' */
    TokenType_BackwardApostrophe,       /* ` */
    TokenType_AtSign,                   /* @ */




    TokenType_LastMarker, // Marker value to indicate the max value allowed for this enum
};


typedef struct token {
    enum token_type   type;      /* Usefull for quick parsing, punctuators match with a simple integer
                                    instead of using the heavier string matching solution */
    I32  line_num;
    I32  column;

    PStr32 payload;
} token_t;



// Keep this value to a power of 2 ( `8` or `16` should be fine)
//     to make modulo `%` operator fast for the queue rounding ups
#define LEXER_STAGING_AREA_MAX_SIZE (16) // 1 byte will be lost to implement the circular queue

typedef struct lexer__staging_area {
    I8 start_it;
    I8 end_it;
    I8 it;
    char buf[LEXER_STAGING_AREA_MAX_SIZE];
} lexer__staging_area_t;


enum lexer_err {
    LexerErr_None                   = 0,
    LexerErr_OutOfMem               = (1 << 0),
    LexerErr_InvalidInputBytes      = (1 << 1),
    LexerErr_InvalidConstantLiteral = (1 << 2),
    LexerErr_InvalidString          = (1 << 3),
    LexerErr_PrematureEndOfString   = (1 << 4),
};



typedef struct lexer_errinfo {
    I32 line_num;
    I32 column;
} lexer_err_info_t;

typedef struct lexer {

    IStream *istream; // streaming input buffer, may be a realtime generated streaming buffer, or just a wrapper around a whole allocated memory block
    FILE    *err_stream;
    struct lexer__staging_area staging_area;

    I32 text_len_accumulator;
    I32 emitted_cnt;
    I32 line_num;
    I32 column;
    

    enum lexer_err err;
    struct lexer_errinfo err_info;
    bool8 eat_whitespaces_automatically;   
} lexer_t;


bool
lexer_init(struct lexer *lex, IStream *istream, FILE *err_stream);

void
lexer_deinit ( struct lexer *lex );

/*
   [#] Memory arena layout of the `lexer_next_token()` call output
   ===============================================================

   Tokens inside the arena are loosely packed together, the memory layout goes as follows:

   token0 :: [type, line_num, column, text_len, <...text...>, '\0']  // C-Style string termination + len for the token text
   token1 :: [type, line_num, column, text_len, <...text...>, '\0']  // C-Style string termination + len for the token text
   token2 :: [type, line_num, column, text_len, <...text...>, '\0']  // C-Style string termination + len for the token text
   token3 :: [type, line_num, column, text_len, <...text...>, '\0']  // C-Style string termination + len for the token text
   ...


   Tokens follows each other in memory rounded up on aligment constraints.
   Eg after the termination of the previous token and the text of the token,
   the next one (if any) follows immediately after rounded up on a sizeof(token_t) boundary.
   So between tokens there may be at most sizeof(token_t) empty (or cleared to 0) bytes.

   To find the i-th token you're forced to iterate since you do not know the text lengths
   of the previous tokens. This is usually not a problem because tokens are processed sequentially.

   The functions returns you a `mem_ref_t`, which is guaranteed to survive across
   `lexer_next_token()` calls and even survives mem_arena buffer grows.
   The `mem_ref_t` will remain valid until the arena is cleared.

   You should check for errors in a loop every time this function returns with the `lex->err` field.
   If any error (especially memory errors occured) the token_ref given back from this function
   may not be valid, because there's no token. Process the reference only in code paths
   where it is guaranteed that no lexer error has occured.
 */

bool
lexer_next_token(struct lexer *lex,
                 MArena *tokens_arena,
                 MRef *token_ref,
                 enum token_type (*lex_logic) (struct lexer *lex, MArena *tokens_arena));


bool
lexer_is_end(struct lexer *lex);




const char *
token_type_tostring(enum token_type type);


ATTRIB_FUNCTIONAL static inline struct token *
tokens_arena_begin(MArena *tokens_arena)
{
    return (struct token*) (tokens_arena->buffer + MARENA_MINIMUM_ALLOWED_STACK_POINTER_VALUE);
}

ATTRIB_FUNCTIONAL static inline struct token *
tokens_arena_next(struct token *prev_token)
{
    U8* ptr = (U8*) prev_token + offsetof(struct token, payload.data) + prev_token->payload.len + 1;
#if TOKENS_ARENA_PACK_WITH_ALIGNMENT
    ptr = (ALIGN_PTR(ptr, sizeof(struct token)));
#endif
    return (struct token*) ptr;
}


ATTRIB_FUNCTIONAL static inline struct token *
tokens_arena_end(MArena *tokens_arena)
{
    struct token *result = (struct token*) (tokens_arena->buffer + tokens_arena->data_size);
    return result;
}


ATTRIB_FUNCTIONAL static inline const char *
token_text(struct token *t)
{
    static const char null_text[] = "";
    if (t) {
        return t->payload.data;
    } else {
        return null_text;
    }
}


ATTRIB_FUNCTIONAL static inline bool
token_type_matches(struct token *t, enum token_type type)
{
    if (t) {
        return t->type == type;
    } else {
        return false;
    }
}

ATTRIB_FUNCTIONAL static inline bool
token_text_matches(struct token *t, const char *string)
{
    assert(string);
    if (t) {
        return 0 == strcmp(token_text(t), string);
    } else {
        return false;
    }
}




__END_DECLS


#endif
