cmake_minimum_required(VERSION 3.10)

project(houd CXX C)

set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE
    SOURCE_FILES
    src/*.cpp
    src/*.hpp
)

add_subdirectory(thirdparty/raylib)

# target_compile_options(your_target PRIVATE -save-temps=obj)
add_executable(houd ${SOURCE_FILES})

target_include_directories(houd PRIVATE
`   thirdparty/raylib/src
)

target_link_libraries(houd raylib)