cmake_minimum_required(VERSION 3.20)

project(ProjectCane LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECTCANE_IMGUI_SOURCES
    Libraries/include/imgui/imgui.cpp
    Libraries/include/imgui/imgui_draw.cpp
    Libraries/include/imgui/imgui_tables.cpp
    Libraries/include/imgui/imgui_widgets.cpp
    Libraries/include/imgui/imgui_impl_glfw.cpp
    Libraries/include/imgui/imgui_impl_opengl3.cpp
    Libraries/include/imgui/ImGuiFileDialog.cpp
)

find_package(OpenGL REQUIRED)
find_package(glfw3 QUIET)

if (NOT glfw3_FOUND)
    include(FetchContent)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        glfw
        URL https://github.com/glfw/glfw/archive/refs/tags/3.4.zip
    )
    FetchContent_MakeAvailable(glfw)
endif()

function(add_projectcane_viewer target source_dir)
    file(GLOB PROJECTCANE_TARGET_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/*.c"
    )

    add_executable(${target}
        ${PROJECTCANE_TARGET_SOURCES}
        ${PROJECTCANE_IMGUI_SOURCES}
    )

    target_include_directories(${target} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Libraries/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}"
    )

    target_compile_definitions(${target} PRIVATE
        USE_STD_FILESYSTEM
    )

    if (MSVC)
        target_compile_options(${target} PRIVATE /W3)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -fpermissive)
    endif()

    target_link_libraries(${target} PRIVATE
        OpenGL::GL
        glfw
    )

    if (UNIX AND NOT APPLE)
        target_link_libraries(${target} PRIVATE dl pthread)
    endif()

    file(GLOB PROJECTCANE_TARGET_SHADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/*.vert"
        "${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/*.frag"
        "${CMAKE_CURRENT_SOURCE_DIR}/${source_dir}/*.geom"
    )

    foreach(shader ${PROJECTCANE_TARGET_SHADERS})
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${shader}"
                "$<TARGET_FILE_DIR:${target}>"
        )
    endforeach()
endfunction()

add_projectcane_viewer(projectcane Sly1-Release)
add_projectcane_viewer(projectcane_proto Sly1-Proto)
