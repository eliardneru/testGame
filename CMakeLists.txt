cmake_minimum_required(VERSION 3.10)
project(TestGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Set SDL paths
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2")
set(SDL2_IMAGE_DIR "${CMAKE_SOURCE_DIR}/lib/SDL2_image")

include_directories(
    ${SDL2_DIR}/include
    ${SDL2_IMAGE_DIR}/include
)

link_directories(
    ${SDL2_DIR}/lib
    ${SDL2_IMAGE_DIR}/lib
)

# Your source files
add_executable(TestGame
    src/main.cpp
)


add_custom_command(TARGET TestGame POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:TestGame>/assets"
)

# Link SDL libraries (adjust names as needed for your platform)
target_link_libraries(TestGame
    SDL2
    SDL2main
    SDL2_image
)
