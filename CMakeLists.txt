# ===========================================================================
#
#                            PUBLIC DOMAIN NOTICE
#               National Center for Biotechnology Information
#
#  This software/database is a "United States Government Work" under the
#  terms of the United States Copyright Act.  It was written as part of
#  the author's official duties as a United States Government employee and
#  thus cannot be copyrighted.  This software/database is freely available
#  to the public for use. The National Library of Medicine and the U.S.
#  Government have not placed any restriction on its use or reproduction.
#
#  Although all reasonable efforts have been taken to ensure the accuracy
#  and reliability of the software and data, the NLM and the U.S.
#  Government do not and cannot warrant the performance or results that
#  may be obtained by using this software or data. The NLM and the U.S.
#  Government disclaim all warranties, express or implied, including
#  warranties of performance, merchantability or fitness for any particular
#  purpose.
#
#  Please cite the author in any work or product based on this material.
#
# ===========================================================================

# ===========================================================================
#
# This is the CMake build for the ncbi-vdb project.
#
# Usage
#   cmake [options] <path-to-source>
#
# Parameter:
#   path-to-source    : path to this file's directory
#
# Custom CMake variables and defaults:
#
#   -DNGS_INCDIR=<path>   : a path to the include/ directory of NGS. By default,
#                           <path-to-source>/../ngs/ngs-sdk
#
#   -DNGS_LIBDIR=<path>   : a path to the NGS libraries. If omitted, tests in
#                           test/ngs-java will be skipped.
#   -DNGS_JAVA_JAR=<path> : a path to the jar created by a build of ngs/ngs-java.
#                           If omitted, tests in test/ngs-java will be skipped.
#	-DPYTHON_PATH=<path>  : a path to the Python executable to be used to run
#                           tests in test/ngs-py. If omitted, the tests will be skipped.
#
#   In addition, you can use all the standard CMake options, e.g.:
#   -DCMAKE_BUILD_TYPE=<Debug|Release>
#
## ===========================================================================

cmake_minimum_required(VERSION 3.16)
if (WIN32)
	set (CMAKE_SYSTEM_VERSION 10.0 CACHE TYPE INTERNAL FORCE)
endif()

project(ncbi-vdb)

# calculate global settiungs, create output directories
include( build/env.cmake NO_POLICY_SCOPE )

add_subdirectory(libs)
add_subdirectory(test)
#add_subdirectory(vdb3)
