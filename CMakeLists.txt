cmake_minimum_required(VERSION 3.10)
project(TestGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
# set Compiler as llvm, you can change this to use another, but i personally think that msvc sucks
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# Set SDL paths
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2")
set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_image")
set(SDL3_DIR "${CMAKE_SOURCE_DIR}/lib/SDL3")

include_directories(
    ${SDL2_DIR}/include
    ${SDL2_IMAGE_DIR}/include
)

link_directories(
    ${SDL2_DIR}/lib
    ${SDL2_IMAGE_DIR}/lib
    ${SDL3_DIR}/lib
)

add_executable(TestGame
    src/main.cpp
)

# Copy assets
add_custom_command(TARGET TestGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:TestGame>/assets"
)

# Copy DLLs
add_custom_command(TARGET TestGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL3_DIR}/lib/SDL3.dll"
        "$<TARGET_FILE_DIR:TestGame>"
)

add_custom_command(TARGET TestGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL2_DIR}/lib/SDL2.dll"
        "$<TARGET_FILE_DIR:TestGame>"
)

add_custom_command(TARGET TestGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${SDL2_IMAGE_DIR}/lib/SDL2_image.dll"
        "$<TARGET_FILE_DIR:TestGame>"
)

target_link_libraries(TestGame
    SDL2
    SDL2main
    SDL2_image
)