cmake_minimum_required(VERSION 3.0.0)
project(base16384 VERSION 2.2.0)
SET(CMAKE_BUILD_TYPE "Release")

add_executable(base16384_b base16384.c)

include(TestBigEndian)
test_big_endian(isBigEndian)
if (${isBigEndian})
    add_definitions(-DWORDS_BIGENDIAN)
endif()

IF(NOT (CMAKE_SYSTEM_NAME MATCHES "Darwin"))
    add_compile_options("-lc")
ENDIF()

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_library(base16384 SHARED base1464.c)
    add_library(base16384_s STATIC base1464.c)
ELSE()
    add_library(base16384 SHARED base1432.c)
    add_library(base16384_s STATIC base1432.c)
ENDIF()

set_target_properties(base16384_b PROPERTIES OUTPUT_NAME base16384)
set_target_properties(base16384_s PROPERTIES OUTPUT_NAME base16384)
set_target_properties(base16384   PROPERTIES VERSION ${CMAKE_PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

target_link_libraries(base16384_b base16384_s)

INSTALL(TARGETS base16384_b RUNTIME DESTINATION bin)
INSTALL(TARGETS base16384   LIBRARY DESTINATION lib)
INSTALL(TARGETS base16384_s ARCHIVE DESTINATION lib)
INSTALL(FILES base16384.h DESTINATION include)
INSTALL(FILES base16384.1 DESTINATION share/man/man1)
