add_subdirectory(PUTM_EV_CAN_LIBRARY_2024)
target_link_libraries(${CMAKE_PROJECT_NAME} PUTM_EV_CAN_LIBRARY)
message("PUTM_EV_CAN_LIBRARY version: " ${PUTM_EV_CAN_LIBRARY_VER} " included in the project")

add_subdirectory(BQ796XX)
target_link_libraries(${CMAKE_PROJECT_NAME} BQ796XX_LIBRARY)
message("BQ796XX included in the project")

add_subdirectory(ADS131M04)
target_link_libraries(${CMAKE_PROJECT_NAME} ADS131M04_LIBRARY)
message("ADS131M04 included in the project")

add_subdirectory(Utils)
target_link_libraries(${CMAKE_PROJECT_NAME} UTILS_LIBRARY)
message("Utils included in the project")

add_subdirectory(Wrappers)
target_link_libraries(${CMAKE_PROJECT_NAME} WRAPPER_LIBRARY)
message("Utils included in the project")

add_subdirectory(Data)
target_link_libraries(${CMAKE_PROJECT_NAME} DATA_LIBRARY)
message("Data included in the project")

add_subdirectory(Config)
target_link_libraries(${CMAKE_PROJECT_NAME} CONFIG_LIBRARY)
message("Config included in the project")

add_subdirectory(StateMachine)
target_link_libraries(${CMAKE_PROJECT_NAME} STATE_MACHINE_LIBRARY)
message("State Machine included in the project")

# ArduinoJson
add_subdirectory(ArduinoJson)
target_link_libraries(${CMAKE_PROJECT_NAME} ArduinoJson)
message("Utils included in the project")