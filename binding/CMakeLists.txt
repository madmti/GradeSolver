if(EMSCRIPTEN)
    add_executable(solver_wasm
        binding_api.cpp
    )
    
    target_include_directories(solver_wasm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/shared
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_S
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_D
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_P
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/json
    )
    
    target_link_libraries(solver_wasm PRIVATE 
        json_lib
        maquina_s
        maquina_d
        maquina_p
        shared_lib
    )

    set_target_properties(solver_wasm PROPERTIES
        OUTPUT_NAME "solver"
        SUFFIX ".js"
    )

    target_link_options(solver_wasm PRIVATE
        "-sWASM=1"
        "-sEXPORTED_FUNCTIONS=['_solve_process','_malloc','_free']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8']"
        "-sMODULARIZE=1"
        "-sEXPORT_NAME='createSolverModule'"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sENVIRONMENT=node,web"
        "-sMAXIMUM_MEMORY=4GB"
        "-sINITIAL_MEMORY=128MB"
    )
else()
    # Binding nativo (shared library)
    add_library(solver_bindings SHARED 
        binding_api.cpp
    )

    target_include_directories(solver_bindings PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/shared
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_S
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_D
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_P
        ${CMAKE_CURRENT_SOURCE_DIR}/../lib/json
    )

    target_link_libraries(solver_bindings PRIVATE 
        json_lib
        maquina_s
        maquina_d
        maquina_p
        shared_lib
    )

    set_target_properties(solver_bindings PROPERTIES 
        OUTPUT_NAME "gradesolver_api"
        POSITION_INDEPENDENT_CODE ON
    )
endif()