add_library(solver_bindings SHARED binding_api.cpp)

target_link_libraries(solver_bindings PRIVATE solver_lib)

set_target_properties(solver_bindings PROPERTIES OUTPUT_NAME "gradesolver_api")
