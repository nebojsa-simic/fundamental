@echo off
gcc ^
    --std=c17 -Os ^
    -I ..\..\..\include ^
    test.c ^
    -o test.exe
