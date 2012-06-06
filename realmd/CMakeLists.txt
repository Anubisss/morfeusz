set(realmd_SRCS
    Realm_Service.h
    Realm_Service.cpp
    Realm_Socket.cpp
    Realm_Socket.h
    Realm_Database.h
    Realm_Database.cpp
    Realm_EC_Communicator.cpp
    #we need ${CMAKE_SOURCE_DIR} for generated files.
    ${CMAKE_SOURCE_DIR}/realmd/Proxy_EventsC.h
    ${CMAKE_SOURCE_DIR}/realmd/Proxy_EventsC.cpp
    )

include_directories(
    ${ACE_INCLUDE_DIR}
    ${TAO_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/configuration
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/common
    )

add_custom_target(realmd_idl 
    ${TAO_IDL_COMPILER} ${CMAKE_SOURCE_DIR}/idl/Proxy_Events.idl 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/realmd)

set_source_files_properties( 
    Proxy_EventsC.cpp
    Proxy_EventsC.h
    PROPERTIES
    GENERATED TRUE
    LANGUAGE CXX)

add_library(realmd STATIC ${realmd_SRCS})
add_dependencies(realmd realmd_idl)
target_link_libraries(
    realmd
    morpheusconfig
    morpheusdatabase
    ${OPENSSL_LIBRARIES}
    ${TAO_LIBRARY}
    ${TAO_PortableServer_LIB}
    ${TAO_CosEvent_Skel_LIB}
    ${TAO_CosNaming_LIB}
    )
