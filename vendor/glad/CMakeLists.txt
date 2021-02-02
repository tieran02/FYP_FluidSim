cmake_minimum_required(VERSION 3.17)
project(Glad)

add_library(glad debug/include/glad/gl.h debug/src/gl.c)
target_include_directories(glad PUBLIC debug/include/)