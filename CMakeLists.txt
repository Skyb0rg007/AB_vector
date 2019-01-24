cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(AB_vector
    VERSION 0.1
    HOMEPAGE_URL "https://github.com/Skyb0rg007/AB_vector"
    DESCRIPTION "A generic vector implementation for C, modeled after kvec.h"
    LANGUAGES C)

find_package(Doxygen MODULE QUIET OPTIONAL_COMPONENTS dot)

add_library(AB_vector INTERFACE)
target_include_directories(AB_vector
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(AB_vector
    INTERFACE c_std_90)

if(DOXYGEN_FOUND)
    set(DOXYGEN_PREDEFINED "__DOXYGEN__")
    doxygen_add_docs(AB_vector-docs
        AB_vector.h
        COMMENT "Generating AB_vector documentation")
endif()

if(NOT DEFINED BUILD_TESTING)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
endif()
include(CTest)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
