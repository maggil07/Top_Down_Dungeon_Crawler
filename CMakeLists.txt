# Min required CMAKE version
cmake_minimum_required(VERSION 3.12)

# Project name
project(undying)

# Set compilier version (C++ 11)
set(CMAKE_CXX_STANDARD 11)

# Set location for findSFML.cmake parent directory.
# If you used 'apt-get install libsfml-dev' you will need to set this
set(CMAKE_MODULE_PATH "/usr/share/SFML/cmake/Modules")

# Tell cmake to find SFML
# Declare which SFML components to use
find_package(SFML 2.4 COMPONENTS graphics audio window system REQUIRED)

# Add sources
file(GLOB SOURCES src/*.cpp)

# Copy bin into the binary folder
file(COPY bin/ DESTINATION ${CMAKE_BINARY_DIR})

add_executable(undying ${SOURCES})

# Link SFML libraries. Every component used aboved in 'find_package' must be linked here
target_link_libraries(undying sfml-graphics sfml-audio sfml-window sfml-system)
