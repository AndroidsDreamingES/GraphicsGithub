cmake_minimum_required(VERSION 3.22.1)
project("vertexarrayobject")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT ANDROID)
    message(FATAL_ERROR "Android-only CMakeLists.")
endif()

add_definitions(-DANDROID -DPLATFORM_ANDROID)

set(SCENE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../Scene)

add_library(vertexarrayobject SHARED
    NativeTemplate.cpp
    ${SCENE_DIR}/Renderer.cpp
    ${SCENE_DIR}/Cube.cpp
    ${SCENE_DIR}/Grid.cpp
)

target_include_directories(vertexarrayobject PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${SCENE_DIR}
)

find_library(log-lib     log)
find_library(android-lib android)
find_library(glesv3-lib  GLESv3)
find_library(egl-lib     EGL)

target_link_libraries(vertexarrayobject
    ${log-lib} ${android-lib} ${glesv3-lib} ${egl-lib})
