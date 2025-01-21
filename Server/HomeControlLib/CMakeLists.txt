
cmake_minimum_required(VERSION 2.8)
if(Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
endif()

#INCLUDE_DIRECTORIES( ../libraries )
INCLUDE_DIRECTORIES(${SYSROOT}/include)
#INCLUDE_DIRECTORIES(${GFLAGS_PUBLIC_H})

INCLUDE_DIRECTORIES( ./src )
file(GLOB lib_SRC
    "./src/*.cpp"
    "./src/Comm/*.cpp"
    "./src/CommObjects/*.cpp"
    "./src/DAL/*.cpp"
    "./src/Hw/*.cpp"
    "./src/Logic/*.cpp"
    "./src/Rest/*.cpp"
)

add_library (HCServerLib ${lib_SRC})
target_link_libraries (HCServerLib libmysqlcppconn8-static.a)
target_link_libraries (HCServerLib cpprest)
target_link_libraries (HCServerLib libssl.a)
target_link_libraries (HCServerLib libcrypto.a)
target_link_libraries (HCServerLib dl)
target_link_libraries (HCServerLib resolv)

#target_link_libraries (HCServerLib glog)
