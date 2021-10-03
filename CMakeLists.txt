cmake_minimum_required(VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_NAME CommandProcessor)

project(${PROJECT_NAME})

add_library(${PROJECT_NAME} INTERFACE src/CommandProcessor.hpp)

target_include_directories(${PROJECT_NAME} INTERFACE src)