add_library(solver_lib 
    solver.cpp
    solver.hpp
)

set_target_properties(solver_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(solver_lib PUBLIC nlohmann_json::nlohmann_json)

target_include_directories(solver_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
