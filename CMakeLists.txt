cmake_minimum_required(VERSION 3.5)

project(vpp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(SUBHOOK_STATIC ON)

add_subdirectory(subhook)

include_directories(include)
add_compile_definitions(DEBUG=1)
set(SOURCES_VPP
    "src/vpp.cpp"
    "src/entry.cpp"
    "src/logger.cpp"
    "src/modules.cpp"
    "src/scanner.cpp"
)

add_library(vpp SHARED ${SOURCES_VPP})
target_link_libraries(vpp subhook)
