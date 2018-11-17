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

  utils/types.c

  utils/hash.c
  utils/math-utils.c

  utils/mem_layout.c
  platform/mem.c
  platform/streams.c

  )

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
  # 32 bits
  set(__PAL_ARCHITECTURE_SIZE__ 32)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
  # 64 bits
  set(__PAL_ARCHITECTURE_SIZE__ 64)
endif()

target_compile_definitions(
  dpcc

  PUBLIC
  __PAL_ARCHITECTURE_SIZE__=${__PAL_ARCHITECTURE_SIZE__}
  )



if(WIN32)
  if(CMAKE_CL_64)
    # Compiling for a 64 Bit Windows PLATFORM
    target_compile_definitions(
      dpcc
      PUBLIC
      __PAL_WINDOWS__
      __PAL_WINDOWS_VERSION__=64
      )
  else()
    # Compiling for a 32 Bit Windows PLATFORM
    target_compile_definitions(
      dpcc
      PUBLIC
      __PAL_WINDOWS__
      __PAL_WINDOWS_VERSION__=32
      )
  endif()

elseif(UNIX AND NOT APPLE)
      target_compile_definitions(
      dpcc
      PUBLIC
      __PAL_LINUX__
      )

  else()
    target_compile_definitions(
      dpcc
      PUBLIC
      __PAL_APPLE__
      )

endif()

if(UNIX AND NOT APPLE)
  target_sources(dpcc PRIVATE platform/pal_linux.c)
elseif(WIN32)
  target_sources(dpcc PRIVATE platform/pal_win32.c)
elseif(APPLE)
  target_sources(dpcc PRIVATE platform/pal_apple.c)
else()
  message(FATAL_ERROR "Platform not supported")
endif()

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
