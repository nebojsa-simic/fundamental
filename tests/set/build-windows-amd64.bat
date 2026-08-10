@echo off
set CC=gcc
set CFLAGS=-I../../include -std=c11 -Wall -Wextra -g
set SOURCES=test.c
set CORE_FILES=../../src/set/set.c ../../arch/memory/windows-amd64/memory.c ../../src/async/async.c ../../arch/async/windows-amd64/async.c
set STRING_FILES=../../src/string/stringOperations.c ../../src/string/stringConversion.c
set CONSOLE_FILES=../../src/console/console.c ../../arch/console/windows-amd64/console.c
set INCLUDES=-I../../include
set OUTPUT=test.exe
set LIBS=-lkernel32 -ladvapi32

%CC% %CFLAGS% %INCLUDES% %SOURCES% %CORE_FILES% %STRING_FILES% %CONSOLE_FILES% %LIBS% -o %OUTPUT%
