# Pumkin Tower
Open Source Pumkin Tower Project

NOTE: If you run into issues with newer Sharp Ocarina versions, please use Sharp Ocarina 1.43 for this project.

Windows building instructions-

1. Clone the Pumkin Tower Open Source repo: ``git clone https://github.com/Pumkin-Tower/PumkinTower.git``

2.Download z64rom: https://github.com/z64tools/z64rom/releases/download/1.5.10/app_win32.zip and extract it. Rename app_win32 to z64rom for organization and move that folder to the Pumkin Tower Repo. Run z64rom.exe once to install its dependencies.

3. Decompress a copy of ``mm-u.z64`` and also move that to the Pumkin Tower repo folder. Name it ``mm-u-dec.z64``

4. Move ``msys-2.0.dll`` from ``PumkinTower\z64rom\tools\mips64-binutils\bin`` into your system32 folder or you'll get errors when trying to compile actors.

5. Click and drag mm_build.rtl onto zzrtl.exe

Building a Pumkin Tower deubug rom:
``zzrtl.exe mm_build.rtl --clean``
``zzrtl.exe mm_build.rtl --debug``

Building a Pumkin Tower release rom:
``zzrtl.exe mm_build.rtl --clean``
``zzrtl.exe mm_build.rtl --release``
