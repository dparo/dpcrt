# Copyright (C) 2018  Davide Paro

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

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
  dpcrt

  STATIC

  types.c

  hash.c
  math/statistics.c

  mem_layout.c

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
  dpcrt

  PUBLIC
  __PAL_ARCHITECTURE_SIZE__=${__PAL_ARCHITECTURE_SIZE__}
  )

option(IS_BIG_ENDIAN_MACHINE "Set to true if compiling for a BIG ENDIAN platform" 0)

if (IS_BIG_ENDIAN_MACHINE)
  target_compile_definitions(
    dpcrt
    
    PUBLIC
    __PAL_LITTLE_ENDIAN__=0
    __PAL_BIG_ENDIAN__=1
    )

else()
    target_compile_definitions(
    dpcrt
    
    PUBLIC
    __PAL_LITTLE_ENDIAN__=1
    __PAL_BIG_ENDIAN__=0
    )

endif()

if(WIN32)
  if(CMAKE_CL_64)
    # Compiling for a 64 Bit Windows PLATFORM
    target_compile_definitions(
      dpcrt
      PUBLIC
      __PAL_WINDOWS__
      __PAL_WINDOWS_VERSION__=64
      )
  else()
    # Compiling for a 32 Bit Windows PLATFORM
    target_compile_definitions(
      dpcrt
      PUBLIC
      __PAL_WINDOWS__
      __PAL_WINDOWS_VERSION__=32
      )
  endif()

elseif(UNIX AND NOT APPLE)
  target_compile_definitions(
    dpcrt
    PUBLIC
    __PAL_LINUX__

    PRIVATE
    _GNU_SOURCE=1
    )

else()
  target_compile_definitions(
    dpcrt
    PUBLIC
    __PAL_APPLE__
    )

endif()

if(UNIX AND NOT APPLE)
  target_sources(dpcrt PRIVATE platform/pal_linux.c)
elseif(WIN32)
  target_sources(dpcrt PRIVATE platform/pal_win32.c)
elseif(APPLE)
  target_sources(dpcrt PRIVATE platform/pal_apple.c)
else()
  message(FATAL_ERROR "Platform not supported")
endif()

target_link_libraries(dpcrt
  PUBLIC -lm)

if (UNIX AND NOT APPLE)
target_link_libraries(dpcrt
  PRIVATE -ldl)
endif()

target_include_directories(dpcrt PUBLIC
  ./ platform ./math ./memory)

target_include_directories(dpcrt PRIVATE
  ./)


add_library (
  dpcrt_stdc

  STATIC


  stdc/time.c
  )

target_link_libraries(dpcrt_stdc PUBLIC dpcrt)
