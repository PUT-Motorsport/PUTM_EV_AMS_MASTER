set(WRAPPER_LIBRARY_INC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc 
)

# set(UTILS_LIBRARY_SRC
#     ${CMAKE_CURRENT_SOURCE_DIR}/Src/utils.cpp
# )

add_library(WRAPPER_LIBRARY INTERFACE)
# target_include_directories(PUTM_EV_CAN_LIBRARY INTERFACE ${MX_Include_Dirs})
target_include_directories(WRAPPER_LIBRARY INTERFACE ${WRAPPER_LIBRARY_INC})
# target_sources(WRAPPER_LIBRARY PUBLIC ${UTILS_LIBRARY_SRC})

target_link_libraries(${CMAKE_PROJECT_NAME} WRAPPER_LIBRARY)

message("Utils included in the project")