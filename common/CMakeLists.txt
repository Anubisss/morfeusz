set(common_SRCS
    dbc/DBC_File.h
    dbc/DBC_File.cpp
    dbc/DBC_Store.h
    dbc/DBC_Store.cpp
    )

include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/configuration
    )

add_library(common STATIC ${common_SRCS})
