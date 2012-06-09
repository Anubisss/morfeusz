set(proxyd_SRCS
    Proxy_Service.h
    Proxy_Service.cpp
    Proxy_Socket.h
    Proxy_Socket.cpp
    Proxy_Database.h
    Proxy_Database.cpp
    Proxyd_EC_Communicator.h
    Proxyd_EC_Communicator.cpp
    ${_CLIENT_BUILD}/Proxy_Crypto.cpp
    ${_CLIENT_BUILD}/Proxy_Handlers.cpp
)

include_directories(
    ${ACE_INCLUDE_DIR}
    ${TAO_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/configuration
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/game
    ${CMAKE_SOURCE_DIR}/common/${_CLIENT_BUILD}
    ${CMAKE_SOURCE_DIR}/proxyd/${_CLIENT_BUILD}
    ${CMAKE_SOURCE_DIR}/proxyd
)

add_library(proxyd STATIC ${proxyd_SRCS})
add_dependencies(proxyd idl)

target_link_libraries(
    proxyd
    common
    morpheusconfig
    morpheusgame
    idl
)
