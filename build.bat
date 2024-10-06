@echo off
windres resource.rc -o resource.o
gcc c_program.c resource.o -o horizon_x86-64.exe
del resource.o