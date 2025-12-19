cmake_minimum_required(VERSION 3.15)

# Project configuration
project(fast-feed-parser 
    VERSION 1.0.0
    DESCRIPTION "High-performance market data feed parser"
    LANGUAGES CXX
)

# C++ standard requirements
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(COMMON_FLAGS 
        -Wall -Wextra -Wpedantic -Werror
        -Wno-unused-parameter -Wno-unused-variable
        -fno-omit-frame-pointer
    )
    
    set(RELEASE_FLAGS
        -O3 -march=native -mtune=native
        -DNDEBUG -flto
        -ffast-math -funroll-loops
    )
    
    set(DEBUG_FLAGS
        -O0 -g3 -ggdb
        -fsanitize=address -fsanitize=undefined
        -fno-optimize-sibling-calls
    )
elseif(MSVC)
    set(COMMON_FLAGS /W4 /WX)
    set(RELEASE_FLAGS /O2 /Ob2 /DNDEBUG /GL)
    set(DEBUG_FLAGS /Od /Zi /RTC1)
endif()

# Main executable
add_executable(fast-feed-parser
    src/main.cpp
    src/feed_generator.cpp
    src/parser.cpp
)

# Apply compiler flags
target_compile_options(fast-feed-parser PRIVATE 
    ${COMMON_FLAGS}
    $<$<CONFIG:Release>:${RELEASE_FLAGS}>
    $<$<CONFIG:Debug>:${DEBUG_FLAGS}>
)

# Link-time optimizations for release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release" AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set_property(TARGET fast-feed-parser PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Include directories
target_include_directories(fast-feed-parser PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(fast-feed-parser PRIVATE winmm)
elseif(UNIX)
    target_link_libraries(fast-feed-parser PRIVATE pthread)
endif()

# Installation configuration
install(TARGETS fast-feed-parser
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "fast-feed-parser")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_PACKAGE_VENDOR "Enterprise Solutions")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

include(CPack)

# Testing support
option(BUILD_TESTING "Build tests" OFF)
if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation generation
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    option(BUILD_DOCUMENTATION "Build documentation" OFF)
    if(BUILD_DOCUMENTATION)
        add_subdirectory(docs)
    endif()
endif()

# Print build configuration
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
