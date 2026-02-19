add_library(maquina_s
    implementacion_s.cpp
    interface_s.hpp
)

set_target_properties(maquina_s PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(maquina_s PRIVATE shared_lib)

target_include_directories(maquina_s PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
