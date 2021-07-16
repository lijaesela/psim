:: this script's defaults assume you have the itch.io bundle installation of Raylib.

set exe_name=psim
set srcs=main.c ui.c
set raylib_path=C:\raylib\raylib
set cc=C:\raylib\mingw\bin\gcc
set cflags=%raylib_path%\src\raylib.rc.data -s -static -Os -std=c99 -Wall -I%raylib_path%\src -Iexternal
set ldflags=-lraylib -lopengl32 -lgdi32 -lwinmm

%cc% -o %exe_name%.exe %srcs% %cflags% %ldflags%
