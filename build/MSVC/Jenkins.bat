del /F /S /Q devel 1>nul
rd /s /q devel
if exist devel rd /s /q devel

mkdir devel
cd devel
git clone -b engineering https://github.com/ncbi/ngs.git
git clone -b VDB-4138    https://github.com/ncbi/ncbi-vdb.git

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"

set path=C:\Program Files\Java\jdk-13.0.2\bin;%PATH%

cd ngs\ngs-sdk\win
call C:\apache-ant\bin\ant build
cd ngs\ngs-java
call C:\apache-ant\bin\ant build
cd ..\..\..\ncbi-vdb\build\MSVC
call C:\apache-ant\bin\ant runtests

