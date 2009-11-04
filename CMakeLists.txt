project(trinitycore3)

include(CPack)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

cmake_minimum_required(VERSION 2.6 FATAL_ERROR)

FIND_PACKAGE(MySQL)
IF(MYSQL_FOUND)
         MESSAGE("MySQL found at: ${MYSQL_INCLUDE_DIR}, ${MYSQL_LIBRARIES}")
         ADD_DEFINITIONS(-DHAVE_MYSQL)
ELSE(MYSQL_FOUND)
         MESSAGE("Couldn't find MySQL include files and/or library")
ENDIF(MYSQL_FOUND)

FIND_PACKAGE(SQLite3)
IF(SQLITE3_FOUND)
        MESSAGE("SQLite3 found at: ${SQLITE3_INCLUDE_DIR}, ${SQLITE3_LIBRARIES}")
        ADD_DEFINITIONS(-DHAVE_SQLITE)
ELSE(SQLITE3_FOUND)
         MESSAGE("Couldn't find SQLite3 include files and/or library")
ENDIF(SQLITE3_FOUND)

add_custom_target(doc cd ${trinitycore3_SOURCE_DIR} && doxygen ${trinitycore3_SOURCE_DIR}/Doxyfile)

add_subdirectory(configuration)
add_subdirectory(database)
add_subdirectory(realmd)
add_subdirectory(main)