SET(database_SRCS
  Database.cpp
  DriverManager.cpp
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/common
  )

add_library(trinitydatabase STATIC ${database_SRCS})

IF(SQLITE3_FOUND)
add_subdirectory(SQLite)
target_link_libraries(
	trinitydatabase
	trinitydatabaseSqlite
	${SQLITE3_LIBRARIES}
	)
ENDIF(SQLITE3_FOUND)

IF(MYSQL_FOUND)
add_subdirectory(MySQL)
target_link_libraries(
	trinitydatabase
	trinitydatabaseMysql
	${MYSQL_LIBRARIES}
	)
ENDIF(MYSQL_FOUND)

target_link_libraries(
	trinitydatabase
	ACE
	)

#add_subdirectory(test)