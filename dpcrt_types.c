/*
 * Copyright (C) 2018  Davide Paro
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

#include "dpcrt_types.h"



/* Use the `Ptr` typedef to avoid C strict aliasing rules and type-punning.
   The `Ptr` typedef should be defined to point to `1 byte` wide memory in order
   to make pointer arithmetic have no scale factor */
static_assert(sizeof(unsigned char) == 1, "unsigned char should be `1 byte` in this platform.");


static_assert(sizeof(size_t)   == sizeof(void*), "");
static_assert(sizeof(ssize_t)  == sizeof(size_t), "Those 2 should be equal in size but differ in signedness");
static_assert(sizeof(intptr_t) == sizeof(size_t), "Those 2 should be equal in size but differ in signedness");


static_assert(sizeof(byte_t) == 1, "");
static_assert(sizeof(I8)     == 1, "");
static_assert(sizeof(U8)     == 1, "");
static_assert(sizeof(I16)    == 2, "");
static_assert(sizeof(U16)    == 2, "");
static_assert(sizeof(I32)    == 4, "");
static_assert(sizeof(U32)    == 4, "");
static_assert(sizeof(I64)    == 8, "");
static_assert(sizeof(U64)    == 8, "");


// Check That types has the correct number of bytes for this architecture
static_assert(sizeof( I8_LIT(123456)) == 1, "");
static_assert(sizeof(I16_LIT(123456)) == 2, "");
static_assert(sizeof(I32_LIT(123456)) == 4, "");
static_assert(sizeof(I64_LIT(123456)) == 8, "");

static_assert(sizeof( U8_LIT(123456)) == 1, "");
static_assert(sizeof(U16_LIT(123456)) == 2, "");
static_assert(sizeof(U32_LIT(123456)) == 4, "");
static_assert(sizeof(U64_LIT(123456)) == 8, "");


static_assert(sizeof( U8_MAX) == 1, "");
static_assert(sizeof(U16_MAX) == 2, "");
static_assert(sizeof(U32_MAX) == 4, "");
static_assert(sizeof(U64_MAX) == 8, "");

static_assert(sizeof( I8_MIN) == 1, "");
static_assert(sizeof( I8_MAX) == 1, "");
static_assert(sizeof(I16_MIN) == 2, "");
static_assert(sizeof(I16_MAX) == 2, "");
static_assert(sizeof(I32_MIN) == 4, "");
static_assert(sizeof(I32_MAX) == 4, "");
static_assert(sizeof(I64_MIN) == 8, "");
static_assert(sizeof(I64_MAX) == 8, "");










bool cstr_to_int32(char *string, int32_t *i)
{
    if (*string == 0) {
        return false;
    }

    int base = 10;
    bool is_negative = (string[0] == '-');
    bool is_bitwise_negated = (string[0] == '~');
    if (is_negative || is_bitwise_negated)
        string += 1;

    if (string[0] == '0' && string[1] == 'x') {
        base = 16;
        string += 2;
    } else if (string[0] == '0' && string[1] == 'b') {
        base = 2;
        string += 2;
    }

    char *endptr = NULL;
    long int temp = strtol(string, &endptr, base);

    if ((long int)((int32_t)temp) == temp) {
        *i = (int32_t)temp;
    } else {
        // The long integer cannot be safely converted into an int32_t without loss of precision
        return false;
    }

    if (endptr == NULL || *endptr == 0) {
        if (is_negative) {
            *i = -(*i);
        } else if (is_bitwise_negated) {
            *i = ~(*i);
        }
        return true;
    }
    return false;
}

bool cstr_to_double(char *string, double *d)
{
    if (*string == 0)
        return false;

    char *endptr = NULL;
    *d = strtod(string, &endptr);

    if (endptr == NULL || *endptr == 0)
        return true;

    return false;
}

str32_list_t get_files_in_dir(char *dirpath, miface_t *allocator)
{
    str32_list_t result = { 0 };

    result.ss = ANEW(str32_t, allocator, 0);

    struct dirent *entry = NULL;
    DIR *dp = NULL;

    dp = opendir(dirpath);
    if (dp != NULL) {
        while ((entry = readdir(dp))) {
            if ((0 != strcmp(".", entry->d_name)) && (0 != strcmp("..", entry->d_name))) {
                if (APUSH(str32_t, &result.ss, allocator, &result.cnt)) {
                    str32_t *s = &result.ss[result.cnt - 1];
                    str32_t temp_string = str32_fmt(allocator, "%s/%s", dirpath, entry->d_name);
                    if (!temp_string.cstr)
                        break;
                    *s = temp_string;
                }
            }
        }
    }

    return result;
}

str32_t get_file_root(str32_t filepath, miface_t *allocator)
{
    str32_t result = { 0 };

    char *dot_location = NULL;
    char *slash_location = NULL;

    int32_t i = filepath.len - 1;
    for (; i >= 0; i--) {
        if (filepath.cstr[i] == '.') {
            dot_location = &filepath.cstr[i];
            break;
        } else if (filepath.cstr[i] == '/') {
            slash_location = &filepath.cstr[i];
            break;
        }
    }

    if (!dot_location && !slash_location) {
        return str32_fmt(allocator, "%s\0", filepath.cstr);
    }

    for (; i >= 0; i--) {
        if (filepath.cstr[i] == '/') {
            slash_location = &filepath.cstr[i];
            break;
        }
    }

    if (dot_location) {
        if (slash_location) {
            ptrdiff_t len = dot_location - slash_location - 1;
            return str32_fmt(allocator, "%.*s\0", len, slash_location + 1);
        } else {
            ptrdiff_t len = dot_location - filepath.cstr;
            return str32_fmt(allocator, "%.*s\0", len, filepath.cstr);
        }
    } else {
        return str32_fmt(allocator, "");
    }

    return result;
}

str32_t get_file_ext(str32_t filepath, miface_t *allocator)
{
    str32_t result = { 0 };

    char *dot_location = NULL;
    for (int32_t i = filepath.len - 1; i >= 0; i--) {
        if (filepath.cstr[i] == '.') {
            dot_location = &filepath.cstr[i];
            break;
        } else if (filepath.cstr[i] == '/') {
            break;
        }
    }
    if (dot_location) {
        return str32_fmt(allocator, "%s", dot_location);
    } else {
        return str32_fmt(allocator, "%s", filepath.cstr);
    }
    return result;
}

str32_t get_dirname(str32_t filepath, miface_t *allocator)
{
    str32_t result = { 0 };
    char *slash_location = NULL;
    for (int32_t i = filepath.len - 1; i >= 0; i--) {
        if (filepath.cstr[i] == '/') {
            slash_location = &filepath.cstr[i];
            break;
        }
    }

    if (slash_location) {
        ptrdiff_t len = slash_location - filepath.cstr;
        return str32_fmt(allocator, "%.*s\0", len, filepath.cstr);
    } else {
        return str32_fmt(allocator, "");
    }
}

str32_t get_basename(str32_t filepath, miface_t *allocator)
{
    str32_t result = { 0 };
    char *slash_location = NULL;
    for (int32_t i = filepath.len - 1; i >= 0; i--) {
        if (filepath.cstr[i] == '/') {
            slash_location = &filepath.cstr[i];
            break;
        }
    }

    if (slash_location) {
        uintptr_t len = (filepath.cstr + filepath.len) - slash_location;
        return str32_fmt(allocator, "%.*s\0", len, slash_location + 1);
    } else {
        return str32_fmt(allocator, "");
    }
}

path_t path_from_str32(str32_t path, miface_t *allocator)
{
    path_t result = { 0 };
    result.drive = str32_fmt(allocator, "");
    result.dirpath = get_dirname(path, allocator);
    result.fileroot = get_file_root(path, allocator);
    result.fileext = get_file_ext(path, allocator);
    return result;
}

str32_t path_to_str32(path_t path, miface_t *allocator)
{
    return str32_fmt(allocator, "%s/%s/%s%s", path.drive.cstr, path.dirpath.cstr, path.fileroot.cstr, path.fileext.cstr);
}
