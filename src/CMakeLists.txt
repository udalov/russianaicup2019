cmake_minimum_required(VERSION 3.5)
project(aicup2019)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE DEBUG)
# set(CMAKE_VERBOSE_MAKEFILE ON)

# OS and compiler checks.
if(WIN32)
    add_definitions(-DWIN32)
    set(PROJECT_LIBS Ws2_32.lib)
endif()

file(GLOB HEADERS "*.h" "*.hpp" "model/*.h" "model/*.hpp" "csimplesocket/*.h")
set_source_files_properties(${HEADERS} PROPERTIES HEADER_FILE_ONLY TRUE)
file(GLOB SRC "*.cpp" "model/*.cpp" "csimplesocket/*.cpp")
add_executable(aicup2019 ${HEADERS} ${SRC})
target_link_libraries(aicup2019 ${PROJECT_LIBS})
target_compile_options(aicup2019 PRIVATE -Wno-inconsistent-missing-override)
