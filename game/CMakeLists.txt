SET(game_SRCS
    Player.cpp
    ObjectMgr.cpp
    )

include_directories(
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/proxyd
    )

add_library(morpheusgame STATIC ${game_SRCS})
