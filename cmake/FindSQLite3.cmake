# - Find SQLite3
# Find the SQLite includes and library
# This module defines
#  SQLITE3_INCLUDE_DIR, where to find mysql.h
#  SQLITE3_LIBRARIES, the libraries needed to use MySQL.
#  SQLITE3_FOUND, If false, do not try to use MySQL.
#
# Copyright (c) 2006, Jaroslaw Staniek, <js@iidea.pl>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)
   set(SQLITE3_FOUND TRUE)

else(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)

  find_path(SQLITE3_INCLUDE_DIR sqlite3.h
      /usr/include
      /usr/local/include
      $ENV{ProgramFiles}/SQLite/include
      $ENV{SystemDrive}/SQLite/include
      $ENV{ProgramFiles}/SQLite
      $ENV{SystemDrive}/SQLite
      $ENV{ProgramFiles}/SQLite3/include
      $ENV{SystemDrive}/SQLite3/include
      $ENV{ProgramFiles}/SQLite3
      $ENV{SystemDrive}/SQLite3
      )

  find_library(SQLITE3_LIBRARIES NAMES sqlite3
      PATHS
      /usr/lib
      /usr/local/lib
      $ENV{ProgramFiles}/SQLite/lib
      $ENV{SystemDrive}/SQLite/lib
      $ENV{ProgramFiles}/SQLite
      $ENV{SystemDrive}/SQLite
      $ENV{ProgramFiles}/SQLite3/lib
      $ENV{SystemDrive}/SQLite3/lib
      $ENV{ProgramFiles}/SQLite3
      $ENV{SystemDrive}/SQLite3
      )

  if(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)
    set(SQLITE3_FOUND TRUE)
    message(STATUS "Found SQLite3: ${SQLITE3_INCLUDE_DIR}, ${SQLITE3_LIBRARIES}")
  else(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)
    set(SQLITE3_FOUND FALSE)
    message(STATUS "SQLite3 not found.")
  endif(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)

  mark_as_advanced(SQLITE3_INCLUDE_DIR SQLITE3_LIBRARIES)

endif(SQLITE3_INCLUDE_DIR AND SQLITE3_LIBRARIES)
