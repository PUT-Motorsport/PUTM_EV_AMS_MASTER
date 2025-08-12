set(TASK_LIBRARY_INC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc 
)

set(TASK_LIBRARY_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/init.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/test_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/bq796xx_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/ads131m04_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/main/air_state_machine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/main/error_checker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/main_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/com/charger_state_machine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/com_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/Src/charger_thread.cpp
)

add_library(TASK_LIBRARY INTERFACE)
# target_include_directories(PUTM_EV_CAN_LIBRARY INTERFACE ${MX_Include_Dirs})
target_include_directories(TASK_LIBRARY INTERFACE ${TASK_LIBRARY_INC})
target_sources(TASK_LIBRARY PUBLIC ${TASK_LIBRARY_SRC})

target_link_libraries(${CMAKE_PROJECT_NAME} TASK_LIBRARY)

message("TASKS included in the project")