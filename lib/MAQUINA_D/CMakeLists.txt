add_library(maquina_d
    implementacion_d.cpp
    interface_d.hpp
    estrategias/minimum.cpp
    estrategias/balanced.cpp
    estrategias/max_weight_first.cpp
    estrategias/min_weight_first.cpp
)

set_target_properties(maquina_d PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(maquina_d PRIVATE shared_lib)
target_link_libraries(maquina_d PRIVATE maquina_s)

target_include_directories(maquina_d PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
