# The NCBI SRA ( Sequence Read Archive )

### Contact:
email: sra@ncbi.nlm.nih.gov

### Change Log
Please check the CHANGES.md file for change history.

## NCBI VDB
The SRA Toolkit and SDK from NCBI is a collection of tools and libraries for
using data in the INSDC Sequence Read Archives.

## ANNOUNCEMENTS:

December 10, 2021

NCBI's SRA will change the source build system to use CMake in the next toolkit release, 3.0.0. This change is an important step to improve developers' productivity as it provides unified cross platform access to support multiple build systems. This change affects developers building NCBI SRA tools from source. Old makefiles and build systems will no longer be supported after we make this change.

This change will also include the structure of GitHub repositories, which will undergo consolidation to provide an easier environment for building tools and libraries (NGS libs and dependencies are consolidated). Consolidation of NGS libraries and dependencies provides better usage scope isolation and makes building more straightforward.

### **Affected repositories**

1) ncbi/ngs (https://github.com/ncbi/ngs)

   This repository will be frozen, and all the code moved to Github repository ncbi/sra-tools, under subdirectory ngs/. All future modifications
   will take place in sra-tools

2) ncbi/ncbi-vdb (https://github.com/ncbi/ncbi-vdb)

   This project's build system will be based on CMake. The libraries supporting access to VDB data via NGS API will be moved to Github repository
   ncbi/sra-tools.

The projects to move are:

| Old location (base URL: https://github.com/ncbi/ncbi-vdb) | New location (base URL: https://github.com/ncbi/sra-tools) |
| --------------------------------------------------------- | ---------------------------------------------------------- |
| libs/ngs | ngs/ncbi/ngs |
| libs/ngs-c++ | ngs/ncbi/ngs-c++ |
| libs/ngs-jni | ngs/ncbi/ngs-jni |
| libs/ngs-py | ngs/ncbi/ngs-py |
| libs/vdb-sqlite | libs/vdb-sqlite |
| test/ngs-java | test/ngs-java |
| test/ngs-python | test/ngs-python |


3) ncbi/sra-tools (https://github.com/ncbi/sra-tools)

   This project's build system will be based on CMake. The project will acquire some new components:

       3a) NGS SDK (now under ngs/, formerly in Github repository ncbi/ngs)

       3b) NGS-related VDB access libraries and their dependents, formerly in Github repository ncbi/ncbi-vdb, as listed in the table above.


## How To Build From Source

To build from source, you need one of the supported operating systems (Linux, Windows, MacOS) and CMake (minimum version 3.16). On Linux and MacOS you need the GNU C/C++ toolchain (On MacOS, you may also use CMake to generate an XCode project), on Windows a set of MS Build Tools for Visual Studio 2017 or 2019.

### Linux, MacOS (gmake)

1. In the root of the ncbi-vdb checkout, run:

```
./configure
```

Use ```./configure -h``` for the list of available optins

2. Once the configuration script has successfully finished, run:

```
make
```

This will invoke a Makefile that performs the following sequence:
* retrieve all the settings saved by the configuration script
* pass the settings to CMake
  * if this is the first time CMake is invoked, it will generate a project tree. The project tree will be located underneath the directory specified in the ```--build-prefix``` option of the configuration. The location can be displayed by running ```make config``` or ```make help``` inside the source tree.
  * build the CMake-generated project

Running ```make``` from any directory inside the source tree will invoke the same sequence but limit the scope of the build to the sub-tree with the current directory as the root.

The ```make``` command inside the source tree supports several additional targets; run ```make help``` for the list and short descriptions.

### MacOS (XCode)

To generate an XCode project, check out ncbi-vdb and run the standard CMake out-of-source build. For that, run CMake GUI, point it at the checkout directory and choose Xcode as the generator. Once the CMake generation succeeds, there will be an XCode project file ```ncbi-vdb.xcodeproj``` in the build's binary directory. You can open it with XCode and build from the IDE.


Alternatively, you can configure and build from the command line:

```
cmake <path-to-ncbi-vdb> -G Xcode
cmake --build . --config Debug      # or Release
```

### Windows (Visual Studio)

To generate an MS Visual Studio solution, check out ncbi-vdb and run the standard CMake out-of-source build. For that, run CMake GUI, point it at the checkout directory, choose one of the supported Visual Studio generators (see NOTE below) and a 64-bit generator, and click on "Configure" and then "Generate". Once the CMake generation succeeds, there will be an MS VS solution file ```ncbi-vdb.sln``` in the current directory. You can open it with the Visual Studio and build from the IDE.

NOTE: This release supports generators ```Visual Studio 15 2017``` and ```Visual Studio 16 2019```, only for 64 bit platforms.


Alternatively, you can configure and build from the command line (assuming the correct MSVS Build Tools are in the %PATH%), e.g.:

```
cmake <path-to-ncbi-vdb> -G "Visual Studio 16 2019" -A x64
cmake --build . --config Debug      # or Release
```

### Antivirus Warnings
Windows Defender and a few other anti-virus tools have recently been [falsely](https://www.virustotal.com/#/file/042af43fc1318b07a4411c1bea2ff7eed8efe07ca954685268ce42f8f818b10e/detection) warning about test-sra.exe.

We suggest verifying [MD5 checksums](https://ftp-trace.ncbi.nlm.nih.gov/sra/sdk/current/md5sum.txt) after downloading.

For additional information on using, configuring, and building the toolkit,
please visit our [wiki](https://github.com/ncbi/sra-tools/wiki)
or our web site at [NCBI](http://www.ncbi.nlm.nih.gov/Traces/sra/?view=toolkit_doc)


SRA Toolkit Development Team
