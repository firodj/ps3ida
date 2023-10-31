cmake_minimum_required(VERSION 3.5)
project(fix_rtoc)

cmake_policy(SET CMP0042 NEW)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
set(SOURCES main.cpp)

find_package(IdaSdk REQUIRED)

add_ida_plugin(fix_rtoc ${SOURCES})

target_include_directories(fix_rtoc${_so} SYSTEM PRIVATE ${IdaSdk_INCLUDE_DIRS})
target_include_directories(fix_rtoc${_so64} SYSTEM PRIVATE ${IdaSdk_INCLUDE_DIRS})

target_compile_options(fix_rtoc${_so} PRIVATE -Wall -Wextra)
target_compile_options(fix_rtoc${_so64} PRIVATE -Wall -Wextra)

target_link_libraries(fix_rtoc${_so} ${IdaSdk_LIBRARIES})
target_link_libraries(fix_rtoc${_so64} ${IdaSdk_LIBRARIES})

set_target_properties(fix_rtoc${_so} fix_rtoc${_so64}
    PROPERTIES
    CXX_STANDARD 20
)
