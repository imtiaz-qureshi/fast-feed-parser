cmake_minimum_required(VERSION 3.15)
project(fast-feed-parser LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(-Wall -Wextra -Wpedantic -march=native -O3)

add_executable(fast-feed-parser
  src/main.cpp
  src/feed_generator.cpp
  src/parser.cpp
)
target_include_directories(fast-feed-parser PRIVATE src)
