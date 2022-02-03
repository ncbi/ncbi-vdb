# The NCBI SRA ( Sequence Read Archive )

### Contact:
email: sra@ncbi.nlm.nih.gov

### Change Log
Please check the CHANGES.md file for change history.

## NCBI VDB
The SRA Toolkit and SDK from NCBI is a collection of tools and libraries for
using data in the INSDC Sequence Read Archives.

## ANNOUNCEMENTS:

February 7, 2022

NCBI's SRA changed the source build system to use CMake in toolkit release 3.0.0 (February 07, 2022). This change is an important step to improve developers' productivity as it provides unified cross platform access to support multiple build systems. This change affects developers building NCBI SRA tools from source. Old makefiles and build systems are longer be supported.

This change also includes the structure of GitHub repositories, which underwent consolidation to provide an easier environment for building tools and libraries (NGS libs and dependencies are consolidated). Consolidation of NGS libraries and dependencies provides better usage scope isolation and makes building more straightforward.

### **Affected repositories**

1) ncbi/ngs (https://github.com/ncbi/ngs)

   This repository is frozen, and all the code moved to Github repository ncbi/sra-tools, under subdirectory ngs/. All future modifications
   will take place in sra-tools

2) ncbi/ncbi-vdb (https://github.com/ncbi/ncbi-vdb)

   This project's build system is based on CMake. The libraries supporting access to VDB data via NGS API have moved to Github repository
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

   This project's build system is based on CMake. The project acquired some new components:

       3a) NGS SDK (now under ngs/, formerly in Github repository ncbi/ngs)

       3b) NGS-related VDB access libraries and their dependents, formerly in Github repository ncbi/ncbi-vdb, as listed in the table above.


### Antivirus Warnings
Windows Defender and a few other anti-virus tools have recently been [falsely](https://www.virustotal.com/#/file/042af43fc1318b07a4411c1bea2ff7eed8efe07ca954685268ce42f8f818b10e/detection) warning about test-sra.exe.

We suggest verifying [MD5 checksums](https://ftp-trace.ncbi.nlm.nih.gov/sra/sdk/current/md5sum.txt) after downloading.

For additional information on using, configuring, and building the toolkit,
please visit our [wiki](https://github.com/ncbi/ncbi-vdb/wiki)
or our web site at [NCBI](http://www.ncbi.nlm.nih.gov/Traces/sra/?view=toolkit_doc)


SRA Toolkit Development Team
