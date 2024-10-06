@echo off
windres resource.rc -o resource.o
gcc c_program_logger.c resource.o -o horizon_logger_x86-64.exe
del resource.o