@echo off
set CC=gcc
set CFLAGS=-I../../include -std=c11 -Wall -Wextra -g
set INCLUDES=-I../../include
set SOURCES=testFileStreamWrite.c
set MAIN_FILES=../../src/stream/streamFile.c ../../src/stream/streamLifecycle.c ../../src/stream/streamFlow.c
set ARCH_FILES=../../arch/stream/windows-amd64/streamOpen.c ../../arch/stream/windows-amd64/streamRead.c ../../arch/stream/windows-amd64/streamWrite.c 
set DEPENDENCIES=../../arch/memory/windows-amd64/memory.c ../../src/async/async.c
set OUTPUT=test.exe
set LIBS=-lkernel32 -ladvapi32

%CC% %CFLAGS% %INCLUDES% %SOURCES% %MAIN_FILES% %ARCH_FILES% %DEPENDENCIES% %LIBS% -o %OUTPUT%