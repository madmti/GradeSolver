add_library(maquina_p
    implementacion_p.cpp
    interface_p.hpp
)

set_target_properties(maquina_p PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(maquina_p PRIVATE shared_lib)
target_link_libraries(maquina_p PRIVATE maquina_s)
target_link_libraries(maquina_p PRIVATE maquina_d)

target_include_directories(maquina_p PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
