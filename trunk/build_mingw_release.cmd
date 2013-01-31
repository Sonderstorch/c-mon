@echo off
PATH=c:\mingw32\bin;%PATH%
python tools\scons\scons.py --warn no-visual-c-missing --build=release --compiler=mingw %1

