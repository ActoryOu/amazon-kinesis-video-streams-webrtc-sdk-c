# This file is to add source files and include directories
# into variables so that it can be reused from different repositories
# in their Cmake based build system by including this file.
#
# Files specific to the repository such as test runner, platform tests
# are not added to the variables.

# include coreJSON files.
include( ${CMAKE_CURRENT_LIST_DIR}/source/dependency/coreJSON/jsonFilePaths.cmake )

# Signal library source files.
set( SIGNAL_SOURCES
     "${CMAKE_CURRENT_LIST_DIR}/source/signal_api.c"
     "${JSON_SOURCES}" )

# Signal library Public Include directories.
set( SIGNAL_INCLUDE_PUBLIC_DIRS
     "${CMAKE_CURRENT_LIST_DIR}/source/include"
     "${JSON_INCLUDE_PUBLIC_DIRS}" )