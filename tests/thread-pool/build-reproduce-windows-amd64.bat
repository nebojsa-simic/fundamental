@echo off
cl /std:c17 /O2 ^
    /I ..\..\include ^
    reproduce_race.c ^
    ..\..\arch\memory\windows-amd64\memory.c ^
    ..\..\arch\sync\windows-amd64\sync.c ^
    ..\..\src\thread_pool\thread_pool.c ^
    ..\..\arch\thread_pool\windows-amd64\thread_pool.c ^
    /Fe:reproduce_race.exe
