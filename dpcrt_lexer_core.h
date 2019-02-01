#ifndef HGUARD_77fec7813eb24e2eb9fb44b9bc0317d0
#define HGUARD_77fec7813eb24e2eb9fb44b9bc0317d0

#include "dpcrt_utils.h"
#include "dpcrt_allocators.h"
#include "dpcrt_lexer.h"
#include "stdc/stdarg.h"

__BEGIN_DECLS



ATTRIB_PRINTF(5, 6)
static inline void
errfmt__(char *C_SRC_FILE, int C_SRC_LINE,
         struct lexer *lex, enum lexer_err errtype, char* fmt, ...)
#define errfmt(...) errfmt__(__FILE__, __LINE__, __VA_ARGS__)
{
    lex->err |= errtype;
    va_list ap;
    va_start(ap, fmt);

    lex->err_info.line_num = lex->line_num;
    lex->err_info.column   = lex->column;


    fprintf(lex->err_stream, "METADATA LOG FROM :: file: `%s`, line: %d\n", C_SRC_FILE, C_SRC_LINE);
    fprintf(lex->err_stream, "    Error at <line:column> <%d:%d> :: \n    ", lex->line_num, lex->column);
    vfprintf(lex->err_stream, fmt, ap);

    va_end(ap);
}




static inline bool
staging_area_is_empty(struct lexer *lex)
{
    if (lex->staging_area.end_it == lex->staging_area.start_it)
    {
        return true;
    }
    else
    {
        return false;
    }
}


static inline bool
staging_area_is_full(struct lexer *lex)
{
    if (((lex->staging_area.end_it + 1) % LEXER_STAGING_AREA_MAX_SIZE) == lex->staging_area.start_it)
    {
        return true;
    }
    else
    {
        return false;
    }
}



// This implementation seems to be faster than the modulo arithmetic version
#define STAGING_AREA_ITERATOR_INC_AND_WRAP__IF_VERSION(it) \
    do { if ((it++) == LEXER_STAGING_AREA_MAX_SIZE) { (it) = 0; } } while(0)

#define STAGING_AREA_ITERATOR_INC_AND_WRAP__MODULO_VERSION(it) \
    do {                                        \
        (it)++;                                 \
        (it) % = LEXER_STAGING_AREA_MAX_SIZE;   \
    } while(0)

#define STAGING_AREA_ITERATOR_INC_AND_WRAP(it)       \
    STAGING_AREA_ITERATOR_INC_AND_WRAP__IF_VERSION(it)




static inline void
staging_area_enqueue(struct lexer *lex, char c)
{
    assert(!staging_area_is_full(lex));
    lex->staging_area.buf[lex->staging_area.end_it] = c;
    STAGING_AREA_ITERATOR_INC_AND_WRAP(lex->staging_area.end_it);
}

static inline char
staging_area_deque(struct lexer *lex)
{
    assert(!staging_area_is_empty(lex));
    char result = lex->staging_area.buf[lex->staging_area.start_it];
    if (lex->staging_area.start_it == lex->staging_area.it )
    {
        STAGING_AREA_ITERATOR_INC_AND_WRAP(lex->staging_area.it);
    }
    STAGING_AREA_ITERATOR_INC_AND_WRAP(lex->staging_area.start_it);
    return result;
}

static inline char
staging_area_peek_first(struct lexer *lex)
{
    assert(!staging_area_is_empty(lex));
    return lex->staging_area.buf[lex->staging_area.start_it];
}

static inline bool
staging_area_is_undone(struct lexer *lex )
{
    return (lex->staging_area.it == lex->staging_area.start_it);
}



#ifndef EOF
# define EOF (-1)
#endif

#ifndef ETX
#  define ETX (3)
#endif

static inline bool
is_end(struct lexer *lex)
{
    // @NOTE: it's difficult to determine the end of the file in streaming based files.
    //        If the backed file is in NON Blocking mode,
    //        when querying for more characters the stream may return 0 bytes read
    //        due to buffer exhaustion but, the same buffer in the future may be refilled
    //        cause new data is available. It's difficult to tell when the stream did really end.
    //        The only solution is searching for EOF which is not guaranteed to be present,
    //        or expose from the platform layer the API to allow file in NON BLOCKING mode

    bool success = false;
    char c = 0;
    if ( staging_area_is_empty(lex))
    {
        const bool peeked = istream_peek_char(lex->istream, &c);
        success = (peeked == false)
            || (c == EOF)
            || (c == ETX)
            || (c == 0);
    }
    else
    {
        c = staging_area_peek_first(lex);
        success = (c == EOF) || (c == 0);
    }
    return success;
}

static inline void
undo_staging_area(struct lexer *lex)
{
    lex->staging_area.it = lex->staging_area.start_it;
}

static inline bool
is_invalid_char(char c)
{
    bool success = false;
    if ( (c <= 0)
         || (c >= 1 && c <= 2)
         || (c >= 4 && c <= 8)
         || (c >= 14 && c <= 31)
         || (c >= 127))
    {
        success = true;
    }
    return success;
}


// next character from the stream and or staging area.
//  Returns an arbitrarly number of virtual `0-es` if both, the stream and the staging area, are exhausted
static inline char
next(struct lexer *lex)
{
    char c = 0;
    if ( lex->staging_area.it != lex->staging_area.end_it)
    {
        c = lex->staging_area.buf[lex->staging_area.it];
    }
    else
    {
        istream_read_next_char(lex->istream, & c);
        if (c != 0 && is_invalid_char(c))
        {
            lex->err |= LexerErr_InvalidInputBytes;
        }

        staging_area_enqueue(lex, c);
    }

    STAGING_AREA_ITERATOR_INC_AND_WRAP(lex->staging_area.it);
    return c;
}

static inline char
deque(struct lexer *lex)
{
    char c = 0;
    if ( !staging_area_is_empty(lex))
    {
        // Discarding characters without calling __next() first, eg you didn't even looked at
        //    what the character was, you're discarding it unconditionally
        c = staging_area_deque(lex);
    }
    else
    {
        bool success = istream_read_next_char(lex->istream, & c);
        if (!success) {
            c = 0;
        }
    }

    (lex->column)++;

    if ( c == '\n')
    {
        lex->line_num ++;
        lex->column = 0;
    }

    return c;
}

static inline void
lex_reject(struct lexer *lex)
{
    // simply deque without storing anything
    char c = deque(lex);
    (void) c; // Lose the character, it's gone
}

static inline void
lex_reject_cnt(struct lexer *lex, const int cnt)
{
    for (int i = 0; i < cnt; i++)
    {
        lex_reject(lex);
    }
}

static inline void
lex_emit(struct lexer *lex, MArena *tokens_arena, char c)
{
    if ( !(lex->err & LexerErr_OutOfMem)
         && !(lex->err & LexerErr_InvalidInputBytes))
    {
        marena_add_char(tokens_arena, c);
        (lex->emitted_cnt)++;
    }
}


static inline void
lex_accept(struct lexer *lex, MArena *tokens_arena)
{
    char c = deque(lex);
    lex_emit(lex, tokens_arena, c);
}

static inline void
lex_accept_cnt(struct lexer *lex, MArena *tokens_arena, const int cnt)
{
    for (int i = 0; i < cnt; i++)
    {
        lex_accept(lex, tokens_arena);
    }
}



static inline char
lex_peek(struct lexer *lex)
{
    char c;
    if (staging_area_is_empty(lex))
    {
        assert_msg(!is_end(lex), "Fetching characters should stop before exhausting the whole buffer");
        bool success = istream_peek_char(lex->istream, & c); (void) success;
        assert_msg(success == true, "@WTF: we already asserted on __is_end() ???");
    }
    else
    {
        c = lex->staging_area.buf[lex->staging_area.it];
    }
    return c;
}





__END_DECLS

#endif /* HGUARD_77fec7813eb24e2eb9fb44b9bc0317d0 */
