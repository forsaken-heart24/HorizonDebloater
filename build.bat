@echo off
windres resource.rc -o resource.o
gcc c_program.c resource.o -o horizon.exe