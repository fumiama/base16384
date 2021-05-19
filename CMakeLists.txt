cmake_minimum_required(VERSION 3.0.0)
project(base16384 VERSION 2.0)

add_executable(base16384 base16384.c)

IF(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions("-DCPUBIT64")
    add_subdirectory("./64")
    target_link_libraries(base16384 base1464)
ELSE()
    add_definitions("-DCPUBIT32")
    add_subdirectory("./32")
    target_link_libraries(base16384 base1432)
ENDIF()

INSTALL(TARGETS base14 LIBRARY DESTINATION lib)
INSTALL(TARGETS base16384 RUNTIME DESTINATION bin)