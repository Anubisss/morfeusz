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
    ${CMAKE_SOURCE_DIR}/proxyd/Proxy_EventsC.h
    ${CMAKE_SOURCE_DIR}/proxyd/Proxy_EventsC.cpp
    )

include_directories(
    ${ACE_INCLUDE_DIR}
    ${TAO_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/configuration
    ${CMAKE_SOURCE_DIR}/common
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/common/${_CLIENT_BUILD}
    ${CMAKE_SOURCE_DIR}/proxyd/${_CLIENT_BUILD}
    ${CMAKE_SOURCE_DIR}/proxyd
    )

add_custom_target(proxyd_idl 
    ${TAO_IDL_COMPILER} ${CMAKE_SOURCE_DIR}/idl/Proxy_Events.idl 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/proxyd)

set_source_files_properties( 
    Proxy_EventsC.cpp
    Proxy_EventsC.h
    PROPERTIES
    GENERATED TRUE
    LANGUAGE CXX)

add_library(proxyd STATIC ${proxyd_SRCS})
add_dependencies(proxyd proxyd_idl)
target_link_libraries(
    proxyd
    common
    morpheusconfig
    )
