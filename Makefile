# Copyright (C) 2019  Davide Paro

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




#
# DPCRT Build
#


ifndef ENDIANNESS
ENDIANNESS = LITTLE
endif

ifndef ARCH
ARCH       = amd64
endif

# Size of the Architecture in Bits eg 32 or 64
ifndef ARCH_SIZE
ARCH_SIZE  = 64
endif

ifndef OS
OS         = GNU/Linux
endif

DPCRT_DEFINES = -D__DPCRT_DEFINED=1

DPCRT_DEFINES += -D__DPCRT_ENDIANNESS=${ENDIANNESS} -D__DPCRT_ARCH=${ARCH} -D__DPCRT_ARCH_SIZE=${ARCH_SIZE}

ifeq (${ENDIANNESS}, BIG)
DPCRT_DEFINES += -D__DPCRT_LITTLE_ENDIAN=0 -D__DPCRT_BIG_ENDIAN=1\
else 					  # Assume Little Endian By default
DPCRT_DEFINES += -D__DPCRT_LITTLE_ENDIAN=1 -D__DPCRT_BIG_ENDIAN=0
endif

ifeq (${ARCH}, amd64)
DPCRT_DEFINES += -D__DPCRT_ARCH_AMD64=1
endif

DPCRT_PLATFORM_SPECIFIC_SRCS = 

ifeq (${OS}, WINDOWS)
DPCRT_DEFINES += -D__DPCRT_LINUX=0 -D__DPCRT_WINDOWS=1 -D__DPCRT_APPLE=0
DPCRT_PLATFORM_SPECIFIC_SRCS += PLATFORM_SPECIFIC/dpcrt_pal_win32.c
else ifeq (${OS}, APPLE)
DPCRT_DEFINES += -D__DPCRT_LINUX=0 -D__DPCRT_WINDOWS=0 -D__DPCRT_APPLE=1
DPCRT_PLATFORM_SPECIFIC_SRCS += dpcrt_pal_apple.c
else 					  # Defaults to Linux by default
DPCRT_DEFINES += -D__DPCRT_LINUX=1 -D__DPCRT_WINDOWS=0 -D__DPCRT_APPLE=0
DPCRT_DEFINES += -D_GNU_SOURCE=1
DPCRT_PLATFORM_SPECIFIC_SRCS += PLATFORM_SPECIFIC/dpcrt_pal_linux.c
endif

