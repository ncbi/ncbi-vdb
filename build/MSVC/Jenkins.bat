mkdir devel
cd devel
git clone -b engineering https://github.com/ncbi/ngs.git
git clone -b 4138 https://github.com/ncbi/ncbi-vdb.git

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"

cd ngs\ngs-sdk\win
call C:\apache-ant-1.9.14\bin\ant build
cd ngs\ngs-java
call C:\apache-ant-1.9.14\bin\ant build
cd ..\..\..\ncbi-vdb\build\MSVC
call C:\apache-ant-1.9.14\bin\ant runtests
