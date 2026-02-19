cmake_minimum_required(VERSION 3.15)
project(GradeSolver VERSION 2.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

add_subdirectory(lib/shared)
add_subdirectory(lib/MAQUINA_S)
add_subdirectory(lib/MAQUINA_P)
add_subdirectory(lib/MAQUINA_D)
add_subdirectory(lib/json)
add_subdirectory(cli)
add_subdirectory(binding)
