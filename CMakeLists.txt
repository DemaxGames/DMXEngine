# CMakeList.txt: проект CMake для DMXengine; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#
cmake_minimum_required (VERSION 3.8)

project ("DMXengine")
set(CMAKE_CXX_STANDARD 20)

# Добавьте источник в исполняемый файл этого проекта.
add_executable ( DMXengine "DMXengine.cpp" "DMXengine.h" "Renderer.cpp" "Renderer.h" "Mesh.cpp" "Mesh.h" "glad.c" )

add_subdirectory(lib/glad)
target_link_libraries( DMXengine glad)

add_subdirectory(lib/glfw-3.4)
target_link_libraries( DMXengine glfw)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
