SET(testMysql_SRCS
	test_mysql.cpp
	)
	
SET(testSqlite_SRCS
	test_sqlite.cpp
	)
	
include_directories(
	${CMAKE_SOURCE_DIR}/src/common
	${CMAKE_SOURCE_DIR}/src/database
	
)
IF(MYSQL_FOUND)
add_executable(test_mysql ${testMysql_SRCS})


target_link_libraries(
  test_mysql
  DORDatabase
  )
ENDIF(MYSQL_FOUND)


IF(SQLITE3_FOUND)
add_executable(test_sqlite ${testSqlite_SRCS})


target_link_libraries(
  test_sqlite
  DORDatabase
  )
ENDIF(SQLITE3_FOUND)

