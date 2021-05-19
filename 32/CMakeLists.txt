cmake_minimum_required(VERSION 3.0.0)
project(base1432 VERSION 2.0)
SET(CMAKE_BUILD_TYPE "Release")

add_library(base1432 STATIC base14.c)
add_library(base14 SHARED base14.c)

INSTALL(TARGETS base14 LIBRARY DESTINATION lib)
INSTALL(FILES base14.h DESTINATION include)