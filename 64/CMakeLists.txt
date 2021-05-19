cmake_minimum_required(VERSION 3.0.0)
project(base1464 VERSION 2.0)
SET(CMAKE_BUILD_TYPE "Release")

add_library(base1464 STATIC base14.c)
add_library(base14 SHARED base14.c)

INSTALL(FILES base14.h DESTINATION include)