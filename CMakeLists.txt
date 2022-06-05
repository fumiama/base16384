cmake_minimum_required(VERSION 3.0.0)
project(base16384 VERSION 2.2.0)
SET(CMAKE_BUILD_TYPE "Release")

add_executable(base16384 base16384.c)

include(TestBigEndian)
test_big_endian(isBigEndian)
if (${isBigEndian})
    add_definitions(-DWORDS_BIGENDIAN)
endif()

IF(NOT (CMAKE_SYSTEM_NAME MATCHES "Darwin"))
    add_compile_options("-lc")
ENDIF()

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_library(libbase16384 SHARED base1464.c)
    add_library(libbase16384_s STATIC base1464.c)
ELSE()
    add_library(libbase16384 SHARED base1432.c)
    add_library(libbase16384_s STATIC base1432.c)
ENDIF()

set_target_properties(libbase16384 PROPERTIES
                        OUTPUT_NAME base16384
                        VERSION 2.2.0
                    )
set_target_properties(libbase16384_s PROPERTIES
                        OUTPUT_NAME base16384
                        VERSION 2.2.0
                    )

target_link_libraries(base16384 libbase16384_s)

INSTALL(TARGETS base16384 RUNTIME DESTINATION bin)
INSTALL(TARGETS libbase16384 LIBRARY DESTINATION lib)
INSTALL(TARGETS libbase16384_s LIBRARY DESTINATION lib)
INSTALL(FILES base16384.h DESTINATION include)
INSTALL(FILES base16384.1 DESTINATION share/man/man1)
