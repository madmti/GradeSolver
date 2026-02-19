add_executable(solver_cli main.cpp)

target_link_libraries(solver_cli PRIVATE json_lib)
target_link_libraries(solver_cli PRIVATE maquina_s)
target_link_libraries(solver_cli PRIVATE maquina_p)
target_link_libraries(solver_cli PRIVATE maquina_d)
target_link_libraries(solver_cli PRIVATE shared_lib)