add_library(json_lib
    json_serializer.cpp
    json_serializer.hpp
)

set_target_properties(json_lib PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(json_lib PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(json_lib PUBLIC shared_lib)
target_link_libraries(json_lib PUBLIC maquina_s)
target_link_libraries(json_lib PUBLIC maquina_d)
target_link_libraries(json_lib PUBLIC maquina_p)

target_include_directories(json_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
