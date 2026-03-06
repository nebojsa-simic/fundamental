@echo off
set CC=gcc
set CFLAGS=-I../../include -std=c11 -Wall -Wextra -g
set INCLUDES=-I../../include
set SOURCES=test.c
set ARCH_FILES=../../arch/file/windows-amd64/fileAppend.c ../../arch/memory/windows-amd64/memory.c ../../src/async/async.c
set COMMON_IMPL=../../arch/file/windows-amd64/fileRead.c ../../arch/file/windows-amd64/fileReadMmap.c
set OUTPUT=test.exe
set LIBS=-lkernel32 -ladvapi32

%CC% %CFLAGS% %INCLUDES% %SOURCES% %ARCH_FILES% %COMMON_IMPL% %LIBS% -o %OUTPUT%