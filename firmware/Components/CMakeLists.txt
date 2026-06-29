add_subdirectory(PUTM_EV_CAN_LIBRARY)
target_link_libraries(${CMAKE_PROJECT_NAME} putm_ev_can)
message("PUTM_EV_CAN_LIBRARY included in the project")

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
message("Wrappers included in the project")

add_subdirectory(Data)
target_link_libraries(${CMAKE_PROJECT_NAME} DATA_LIBRARY)
message("Data included in the project")

add_subdirectory(Config)
target_link_libraries(${CMAKE_PROJECT_NAME} CONFIG_LIBRARY)
message("Config included in the project")

add_subdirectory(StateMachine)
target_link_libraries(${CMAKE_PROJECT_NAME} STATE_MACHINE_LIBRARY)
message("State Machine included in the project")

add_subdirectory(ErrorChecker)
target_link_libraries(${CMAKE_PROJECT_NAME} ERROR_CHECKER_LIBRARY)
message("Error Checker included in the project")

add_subdirectory(Charger)
target_link_libraries(${CMAKE_PROJECT_NAME} CHARGER_LIBRARY)
message("Charger included in the project")

add_subdirectory(SoC)
target_link_libraries(${CMAKE_PROJECT_NAME} SOC_LIBRARY)
message("SoC included in the project")

add_subdirectory(DSP)
target_link_libraries(${CMAKE_PROJECT_NAME} DSP_LIBRARY)
message("DSP included in the project")

add_subdirectory(Logger)
target_link_libraries(${CMAKE_PROJECT_NAME} LOGGER_LIBRARY)
message("Logger included in the project")

# ArduinoJson
add_subdirectory(ArduinoJson)
target_link_libraries(${CMAKE_PROJECT_NAME} ArduinoJson)
message("ArduinoJson included in the project")