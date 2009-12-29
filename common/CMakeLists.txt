set(common_SRCS
  dbc/DBC_File.h
  dbc/DBC_File.cpp
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/common
  )

add_library(common STATIC ${common_SRCS})