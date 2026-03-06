@echo off
set CC=gcc
set CFLAGS=-I../../include -std=c11 -Wall -Wextra -g
set INCLUDES=-I../../include
set SOURCES=test.c
set ARCH_FILES=../../arch/file/windows-amd64/fileLock.c 
set DEPENDENCIES=../../arch/memory/windows-amd64/memory.c ../../src/async/async.c
set OTHER_DEPS=../../arch/file/windows-amd64/fileRead.c ../../arch/file/windows-amd64/fileReadMmap.c
set OUTPUT=test.exe
set LIBS=-lkernel32 -ladvapi32

%CC% %CFLAGS% %INCLUDES% %SOURCES% %ARCH_FILES% %DEPENDENCIES% %OTHER_DEPS% %LIBS% -o %OUTPUT%