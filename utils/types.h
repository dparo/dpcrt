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
#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "compiler.h"

__BEGIN_DECLS

typedef uint8_t* ptr_t;

typedef float float32;
typedef double float64;

typedef int8_t   int8;
typedef uint8_t  uint8;
typedef uint8    byte_t;

typedef int16_t  int16;
typedef uint16_t uint16;

typedef int32_t  int32;
typedef uint32_t uint32;

typedef int64_t  int64;
typedef uint64_t uint64;

__END_DECLS

#endif
