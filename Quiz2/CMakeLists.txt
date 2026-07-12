cmake_minimum_required(VERSION 3.15)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
cmake_policy(VERSION 3.15)

project(VertexArrayObject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ImportDependencies)
importDependencies()

add_definitions(-DPLATFORM_WINDOWS -DGLEW_STATIC)

set(SCENE_DIR ${CMAKE_CURRENT_LIST_DIR}/Scene)
file(GLOB SCENE_SRC "${SCENE_DIR}/*.cpp")

add_executable(VertexArrayObject ${SCENE_SRC})
target_include_directories(VertexArrayObject PRIVATE ${SCENE_DIR})
target_link_libraries(VertexArrayObject PRIVATE glfw PRIVATE libglew_static)
target_link_libraries(VertexArrayObject PRIVATE glm::glm)

if(MSVC)
    target_compile_options(VertexArrayObject PRIVATE /W4 /WX- /permissive- /std:c++17)
endif()

add_custom_command(TARGET VertexArrayObject POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_LIST_DIR}/android/app/src/main/assets"
        "$<TARGET_FILE_DIR:VertexArrayObject>/assets"
    COMMENT "Copying shader assets to build directory"
)
