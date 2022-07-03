cmake_minimum_required(VERSION 3.20)
project(WM VERSION 1.0)
set(CMAKE_CXX_STANDARD 20 )
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB_RECURSE SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
find_package(X11 REQUIRED)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME wm)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_link_libraries(${PROJECT_NAME} PUBLIC ${X11_LIBRARIES})

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin"
)

#Debug
target_compile_options(${PROJECT_NAME} PUBLIC   -Werror -ggdb -O0 -Wall -Wextra  -Wextra -Weffc++  -Wsign-conversion -pedantic-errors)

#Release
# target_compile_options(${PROJECT_NAME} PUBLIC   -Werror  -O3 -Wall -Wextra  -Wextra -Weffc++  -Wsign-conversion -pedantic-errors)
#-Werror add this option if you want to treat warnings as errors
