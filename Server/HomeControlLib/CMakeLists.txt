
cmake_minimum_required(VERSION 2.8)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
endif()

#INCLUDE_DIRECTORIES( ../libraries )
INCLUDE_DIRECTORIES(${SYSROOT}/include)
#INCLUDE_DIRECTORIES(${GFLAGS_PUBLIC_H})

INCLUDE_DIRECTORIES( ./src )
file(GLOB lib_SRC
    "./src/*.h"
    "./src/*.cpp"
    "./src/Comm/*.h"
    "./src/Comm/*.cpp"
    "./src/CommObjects/*.h"
    "./src/CommObjects/*.cpp"
    "./src/DAL/*.h"
    "./src/DAL/*.cpp"
    "./src/Hw/*.h"
    "./src/Hw/*.cpp"
    "./src/Logic/*.h"
    "./src/Logic/*.cpp"

)

add_library (HCServerLib ${lib_SRC})
target_link_libraries (HCServerLib libmysqlcppconn8-static.a)
target_link_libraries (HCServerLib dl)
target_link_libraries (HCServerLib resolv)

#target_link_libraries (HCServerLib glog)





