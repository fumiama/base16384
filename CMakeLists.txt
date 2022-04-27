cmake_minimum_required(VERSION 3.0.0)
project(base16384 VERSION 2.0)
SET(CMAKE_BUILD_TYPE "Release")

add_executable(base16384 base16384.c)

include(TestBigEndian)
test_big_endian(isBigEndian)
if (${isBigEndian})
    add_definitions(-DWORDS_BIGENDIAN)
endif()

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_library(libbase16384 SHARED base1464.c)
ELSE()
    add_library(libbase16384 SHARED base1432.c)
ENDIF()

target_link_libraries(base16384 libbase16384)

set_target_properties(libbase16384 PROPERTIES OUTPUT_NAME base16384)

INSTALL(TARGETS base16384 RUNTIME DESTINATION bin)
INSTALL(TARGETS libbase16384 LIBRARY DESTINATION lib)
INSTALL(FILES base16384.h DESTINATION include)
