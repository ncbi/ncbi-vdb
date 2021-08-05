# The NCBI SRA ( Sequence Read Archive )

### Contact:
email: sra@ncbi.nlm.nih.gov

### Change Log
Please check the CHANGES.md file for change history.

## NCBI VDB
The SRA Toolkit and SDK from NCBI is a collection of tools and libraries for
using data in the INSDC Sequence Read Archives.

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
