@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
python tools\scons\scons.py --build=debug --compiler=vc %1

