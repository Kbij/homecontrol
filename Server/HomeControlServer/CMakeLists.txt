cmake_minimum_required(VERSION 2.8)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
endif()

INCLUDE_DIRECTORIES( ../HomeControlLib/src )
INCLUDE_DIRECTORIES(${SYSROOT}/include)
#INCLUDE_DIRECTORIES( ../libraries )
#INCLUDE_DIRECTORIES(${GLOG_PUBLIC_H})
#INCLUDE_DIRECTORIES(${GFLAGS_PUBLIC_H})

INCLUDE_DIRECTORIES( ./src )
file(GLOB exe_SRC
    "./src/*.h"
    "./src/*.cpp"

)

add_executable (HCServer ${exe_SRC})
target_link_libraries (HCServer HCServerLib)
target_link_libraries (HCServer libglog.a)
target_link_libraries (HCServer libgflags.a)
target_link_libraries (HCServer libboost_system.a)
target_link_libraries (HCServer pthread)
target_link_libraries (HCServer rt)

