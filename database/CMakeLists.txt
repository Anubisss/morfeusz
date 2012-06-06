SET(database_SRCS
    Database.cpp
    DriverManager.cpp
    )

include_directories(
    ${ACE_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/common
    )

add_library(morpheusdatabase STATIC ${database_SRCS})

IF(SQLITE3_FOUND)
add_subdirectory(SQLite)
target_link_libraries(
	morpheusdatabase
	morpheusdatabaseSqlite
	${SQLITE3_LIBRARIES}
	)
ENDIF(SQLITE3_FOUND)

IF(MYSQL_FOUND)
add_subdirectory(MySQL)
target_link_libraries(
	morpheusdatabase
	morpheusdatabaseMysql
	${MYSQL_LIBRARIES}
	)
include_directories(
  ${MYSL_INCLUDE_DIR}
  )
ENDIF(MYSQL_FOUND)

target_link_libraries(
	morpheusdatabase
	${ACE_LIBRARY}
	)

#add_subdirectory(test)
