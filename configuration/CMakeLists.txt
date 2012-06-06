SET(configuration_SRCS
    Configuration.cpp
    )

include_directories(
    ${ACE_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/common
    )

add_library(morpheusconfig STATIC ${configuration_SRCS})
