# Copyright (C) 2018  Davide Paro

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  add_definitions(-D_GNU_SOURCE)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  add_definitions(-D_GNU_SOURCE)
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
else()
endif()



if(MSVC)
    add_definitions(/FI"./build_config.h")
else()
    # GCC or Clang
    add_definitions(-include ./build_config.h)
endif()


add_library(
  dpcc

  STATIC

  utils/hash.c
  utils/math-utils.c

  utils/mem_layout.c
  platform/pal.c
  platform/mem.c
  platform/streams.c

  )

target_link_libraries(dpcc
  PUBLIC -lm)

if (UNIX)
target_link_libraries(dpcc
  PUBLIC -ldl)
endif()

target_include_directories(dpcc PUBLIC
  ./ platform utils)

target_include_directories(dpcc PRIVATE
  ./)

