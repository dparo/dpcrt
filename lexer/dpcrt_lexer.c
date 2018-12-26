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
#include "dpcrt_lexer.h"
#include "dpcrt.h"

#include <stdarg.h>
#include <stddef.h>

#ifndef EOF
#  define EOF (-1)
#endif

#ifndef ETX
#  define ETX ((char) 0x03)
#endif


#include "dpcrt_lexer_core.h"

static inline void
errclear ( struct lexer *lex )
{
    lex->err               = LexerErr_None;
    lex->err_info.line_num = 0;
    lex->err_info.column   = 0;
}

static inline char *
unwrap_token_text(struct token *token)
{
    assert(token);
    assert(token->type != TokenType_Null);
    assert(*(token->payload.data + token->payload.len) == '\0');
    return token->payload.data;
}

#include "dpcrt_lexer_builtin_logic.h"

bool
lexer_next_token(struct lexer *lex,
                 MArena *tokens_arena,
                 MRef *token_ref,
                 enum token_type (*lex_logic) (struct lexer *lex, MArena *tokens_arena))
{
    *token_ref = 0;
    assert(staging_area_is_undone(lex));

    lex->emitted_cnt = 0;
    errclear(lex);

    if (is_end(lex))
    {
        return false;
    }

    if (lex->eat_whitespaces_automatically)
    {
        eat_whitespaces(lex);
        if (is_end(lex))
        {
            return false;
        }
    }

    if (lex->err == LexerErr_None && !is_end(lex))
    {
        marena_begin(tokens_arena);
        enum token_type token_type     = 0;
        I32 token_column   = lex->column;
        I32 token_line_num = lex->line_num;

        if (!marena_add(tokens_arena, sizeof(struct token), true))
        {
            marena_dismiss(tokens_arena);
            return false;
        }
        else
        {
            token_type = lex_logic(lex, tokens_arena);


            {
                // All cases should be matched in the lexing logic code above
                assert(token_type != 0);        
        
                marena_add_char(tokens_arena, '\0');   /* null-terminate the token->payload.data */
                MRef r = marena_commit(tokens_arena);
                if (!r)
                {
                    errfmt(lex, LexerErr_OutOfMem, "Failed memory allocation on the output arena");
            
                    /* Try to restore the state of the lexer to this position if possible
                       (@NOTE This only work if the stream being read is a memory buffer) */
                    lex->line_num = token_line_num;
                    lex->column = token_column;
                }
                else
                {
                    *token_ref = r;
                    struct token *t = marena_unpack_ref__unsafe(tokens_arena, r);
                    assert(t);
                    if (t)
                    {
                        t->type         = token_type;
                        t->line_num     = token_line_num;
                        t->column       = token_column;
                        t->payload.len  = lex->emitted_cnt;
                    }

#   if __DEBUG
                    char *text = t->payload.data;
                    (void) text;
                    assert(strlen(t->payload.data) == (size_t) t->payload.len);
#   endif

                }            
                undo_staging_area(lex);

            }
        }        
    }
    return true;
}



bool
lexer_init(struct lexer *lex, IStream *istream, FILE *err_stream)
{
    bool success = true;
    assert(istream);

    memclr(&(lex->staging_area), sizeof(lex->staging_area));

    lex->emitted_cnt = 0;
    lex->line_num    = 1;
    lex->column      = 0;

    lex->istream = istream;
    lex->err_stream = err_stream;
    lex->eat_whitespaces_automatically = true;

    errclear(lex);
    return success;
}


void
lexer_deinit ( struct lexer *lex )
{
    memclr(lex, sizeof(*lex));
}
