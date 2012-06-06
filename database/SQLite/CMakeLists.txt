SET(databaseSQLite_SRCS
	SQLiteConnection.cpp
	SQLiteDriver.cpp
	SQLitePreparedStatement.cpp
	SQLiteStatement.cpp
	SQLiteResultSet.cpp
	SQLiteException.cpp
	)
	
include_directories(
    ${ACE_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/src/common
	${SQLITE3_INCLUDE_DIR}
)

add_library(morpheusdatabaseSqlite STATIC ${databaseSQLite_SRCS})

target_link_libraries(
	morpheusdatabaseSqlite
	${SQLITE3_LIBRARIES}
	)
