@echo off
del build_gnu\*.o
del build_gnu\*.d
del build_gnu\firmware.bin
del build_gnu\firmware.elf
del build_gnu\list.txt

del build_rvds\*.o
del build_rvds\firmware.axf
del build_rvds\firmware.bin
del build_rvds\list.txt
del build_rvds\*.htm
del build_rvds\*.dep

del installer\*manifest*
del installer\*.obj
del installer\installer.ncb
del installer\*.pdb
del installer\*.idb
del /a:h installer\installer.suo
del installer\*.htm
del installer\*.dep
del installer\*.ilk
del installer\install.exe

