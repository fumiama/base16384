cmake_minimum_required(VERSION 3.0.0)
project(base1432 VERSION 2.0)

add_library(base1432 STATIC base14.c)
add_library(base14 SHARED base14.c)

INSTALL(FILES base14.h DESTINATION include)