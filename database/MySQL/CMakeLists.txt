SET(databaseMysql_SRCS
	MySQLConnection.cpp
	MySQLDriver.cpp
	MySQLPreparedResultSet.cpp
	MySQLPreparedStatement.cpp
	MySQLStatement.cpp
	MySQLResultSet.cpp
	MySQLException.cpp
	)
	
include_directories(
    ${ACE_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/src/common
	${MYSQL_INCLUDE_DIR}
)

add_library(morpheusdatabaseMysql STATIC ${databaseMysql_SRCS})

target_link_libraries(
	morpheusdatabaseMysql
	${MYSQL_LIBRARIES}
	)
