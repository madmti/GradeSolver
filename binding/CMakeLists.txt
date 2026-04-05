if(EMSCRIPTEN)
    function(configure_wasm_target target_name output_dir suffix env export_es6)
        add_executable(${target_name}
            binding_api.cpp
        )
        
        target_include_directories(${target_name} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/shared
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_S
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_D
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/MAQUINA_P
            ${CMAKE_CURRENT_SOURCE_DIR}/../lib/json
        )
        
        target_link_libraries(${target_name} PRIVATE 
            json_lib
            maquina_s
            maquina_d
            maquina_p
            shared_lib
        )

        set_target_properties(${target_name} PROPERTIES
            OUTPUT_NAME "solver"
            SUFFIX "${suffix}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/binding/${output_dir}"
        )

        target_link_options(${target_name} PRIVATE
            "-sWASM=1"
            "-sEXPORTED_FUNCTIONS=['_solve_process','_malloc','_free']"
            "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap','UTF8ToString','stringToUTF8']"
            "-sMODULARIZE=1"
            "-sEXPORT_NAME='createSolverModule'"
            "-sALLOW_MEMORY_GROWTH=1"
            "-sENVIRONMENT=${env}"
            "-sMAXIMUM_MEMORY=4GB"
            "-sINITIAL_MEMORY=128MB"
        )

        if(export_es6)
            target_link_options(${target_name} PRIVATE
                "-sEXPORT_ES6=1"
            )
        endif()
    endfunction()

    configure_wasm_target(solver_wasm_node "node" ".js" "node" OFF)
    configure_wasm_target(solver_wasm_web "web" ".mjs" "web" ON)
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