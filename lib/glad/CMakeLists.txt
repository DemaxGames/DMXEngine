cmake_minimum_required(VERSION 3.8)

project(Glad)

add_library(glad STATIC include/glad/glad.h E:/DMXengineGIT/DMXengine/glad.c)
target_include_directories(glad PUBLIC include/)