#!/bin/bash

PROJECT_ROOT="../.."

gcc \
    --std=c17 -Os \
    -I $PROJECT_ROOT/include \
    demo.c \
    $PROJECT_ROOT/src/network/server/server.c \
    $PROJECT_ROOT/src/network/network.c \
    $PROJECT_ROOT/arch/network/linux-amd64/network.c \
    $PROJECT_ROOT/arch/network/server/linux-amd64/server.c \
    $PROJECT_ROOT/src/async/async.c \
    $PROJECT_ROOT/arch/async/linux-amd64/async.c \
    $PROJECT_ROOT/src/console/console.c \
    $PROJECT_ROOT/arch/console/linux-amd64/console.c \
    $PROJECT_ROOT/src/config/config.c \
    $PROJECT_ROOT/src/config/iniParser.c \
    $PROJECT_ROOT/src/config/cliParser.c \
    $PROJECT_ROOT/arch/config/linux-amd64/env.c \
    $PROJECT_ROOT/src/filesystem/path.c \
    $PROJECT_ROOT/src/filesystem/file_exists.c \
    $PROJECT_ROOT/src/filesystem/directory.c \
    $PROJECT_ROOT/arch/filesystem/linux-amd64/path.c \
    $PROJECT_ROOT/arch/filesystem/linux-amd64/file_exists.c \
    $PROJECT_ROOT/arch/filesystem/linux-amd64/directory.c \
    $PROJECT_ROOT/src/hashmap/hashmap.c \
    $PROJECT_ROOT/arch/memory/linux-amd64/memory.c \
    $PROJECT_ROOT/src/string/stringConversion.c \
    $PROJECT_ROOT/src/string/stringOperations.c \
    $PROJECT_ROOT/src/string/stringValidation.c \
    -o demo

strip --strip-unneeded demo
