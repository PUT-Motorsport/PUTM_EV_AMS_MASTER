set(LOGGER_INC 
    ${CMAKE_CURRENT_SOURCE_DIR}/Inc 
)

# set(LOGGER_SRC
#     ${CMAKE_CURRENT_SOURCE_DIR}/Src/logger.cpp
# )

add_library(LOGGER_LIBRARY INTERFACE)
target_include_directories(LOGGER_LIBRARY INTERFACE ${LOGGER_INC})