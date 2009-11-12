project(trinitycore3)

include(CPack)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FindAce.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FindOpenssl.cmake)
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

FIND_LIBRARY(SSLLIB NAMES ssl DOC "SSL library")

if(OPENSSL_FOUND)
  message(STATUS "Found openssl")
else(OPENSSL_FOUND)
  message(ERROR "Couldn't find OpenSSL library!")
endif(OPENSSL_FOUND)

FIND_ACE(ACE)
if(ACE_FOUND)
  message(STATUS "Found ACE library: ${ACE_LIBRARY}")
  message(STATUS "Include dir is: ${ACE_INCLUDE_DIR}")
else(ACE_FOUND)
  message(SEND_ERROR "Couldn't find ACE library!")
endif(ACE_FOUND)

FIND_PACKAGE(MySQL)
IF(MYSQL_FOUND)
  MESSAGE(STATUS "MySQL found at: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
  ADD_DEFINITIONS(-DHAVE_MYSQL)
ELSE(MYSQL_FOUND)
  MESSAGE(STATUS "Couldn't find MySQL include files and/or library")
ENDIF(MYSQL_FOUND)

FIND_PACKAGE(SQLite3)
IF(SQLITE3_FOUND)
  MESSAGE(STATUS "SQLite3 found at: ${SQLITE3_INCLUDE_DIR}, ${SQLITE3_LIBRARIES}")
  ADD_DEFINITIONS(-DHAVE_SQLITE)
ELSE(SQLITE3_FOUND)
  MESSAGE(STATUS "Couldn't find SQLite3 include files and/or library")
ENDIF(SQLITE3_FOUND)

find_package(TAO)
if(TAO_FOUND)
message(STATUS "TAO found: ${TAO_INCLUDE_DIR}, ${TAO_LIBRARY}")
else(TAO_FOUND)
message(SEND_ERROR "Couldn't find TAO!")
endif(TAO_FOUND)

add_custom_target(doc cd ${trinitycore3_SOURCE_DIR} && doxygen ${trinitycore3_SOURCE_DIR}/Doxyfile)

if(WIN32)
include_directories(
  ${ACE_INCLUDE_DIR}
  ${OPENSSL_INCLUDE_DIR}
)
endif(WIN32)

add_subdirectory(configuration)
add_subdirectory(database)
add_subdirectory(realmd)
add_subdirectory(main)