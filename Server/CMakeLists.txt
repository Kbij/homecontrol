cmake_minimum_required (VERSION 2.8.11)

project (HCM)
#message(STATUS "CMAKE platform: " ${HCB_PLATFORM})
 
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
 
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()
 
#set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin-${HCB_PLATFORM})
set (SYSROOT ${CMAKE_CURRENT_SOURCE_DIR}/libraries/x86)
message(STATUS "Sysroot: " ${SYSROOT})

#set (gflags_BUILD_STATIC_LIBS ON)

#add_subdirectory (./libraries/build/gflags)
#add_library(gflags ALIAS gflags_static)
#add_subdirectory (./libraries/build/glog)

add_subdirectory (./HomeControlLib)
add_subdirectory (./HomeControlServer)

