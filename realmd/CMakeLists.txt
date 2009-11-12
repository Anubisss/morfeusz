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
  ${CMAKE_SOURCE_DIR}/configuration
  ${CMAKE_SOURCE_DIR}/database
  ${CMAKE_SOURCE_DIR}/common
  )

add_library(realmd STATIC ${realmd_SRCS})
target_link_libraries(
  realmd
  trinityconfig
  trinitydatabase
  ${OPENSSL_LIBRARIES}
  ${TAO_LIBRARY}
  ${TAO_PortableServer_LIB}
  ${TAO_CosEvent_Skel_LIB}
  ${TAO_CosNaming_LIB}
  )
