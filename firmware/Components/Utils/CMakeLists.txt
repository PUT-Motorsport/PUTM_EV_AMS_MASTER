set(UTILS_LIBRARY_INC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc 
)

set(UTILS_LIBRARY_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/utils.cpp
)

add_library(UTILS_LIBRARY INTERFACE)
# target_include_directories(PUTM_EV_CAN_LIBRARY INTERFACE ${MX_Include_Dirs})
target_include_directories(UTILS_LIBRARY INTERFACE ${UTILS_LIBRARY_INC})
target_sources(UTILS_LIBRARY PUBLIC ${UTILS_LIBRARY_SRC})

target_link_libraries(${CMAKE_PROJECT_NAME} UTILS_LIBRARY)

message("Utils included in the project")