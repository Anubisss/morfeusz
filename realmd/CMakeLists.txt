set(realmd_SRCS
    Realm_Service.h
    Realm_Service.cpp
    Realm_Socket.cpp
    Realm_Socket.h
    Realm_Database.h
    Realm_Database.cpp
    Realm_EC_Communicator.cpp
)

include_directories(
    ${ACE_INCLUDE_DIR}
    ${TAO_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/configuration
    ${CMAKE_SOURCE_DIR}/database
    ${CMAKE_SOURCE_DIR}/common
)

add_library(realmd STATIC ${realmd_SRCS})
add_dependencies(realmd idl)

target_link_libraries(
    realmd
    morpheusconfig
    morpheusdatabase
    idl
    ${OPENSSL_LIBRARIES}
    ${TAO_LIBRARY}
    ${TAO_PortableServer_LIB}
    ${TAO_CosEvent_Skel_LIB}
    ${TAO_CosNaming_LIB}
)
