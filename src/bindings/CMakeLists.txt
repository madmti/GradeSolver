if(EMSCRIPTEN)
    add_executable(solver_wasm
        binding_api.cpp
        ../lib/solver.cpp
    )
    target_include_directories(solver_wasm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib
    )
    target_link_libraries(solver_wasm PRIVATE nlohmann_json::nlohmann_json)

    set_target_properties(solver_wasm PROPERTIES
        OUTPUT_NAME "solver"
        SUFFIX ".js"
    )

    target_link_options(solver_wasm PRIVATE
        "-sWASM=1"
        "-sEXPORTED_FUNCTIONS=['_solve_process']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        "-sMODULARIZE=1"
        "-sEXPORT_ES6=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sENVIRONMENT=web"
    )
else()
    add_library(solver_bindings SHARED binding_api.cpp)

    target_link_libraries(solver_bindings PRIVATE solver_lib)

    set_target_properties(solver_bindings PROPERTIES OUTPUT_NAME "gradesolver_api")
endif()
