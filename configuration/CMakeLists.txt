SET(configuration_SRCS
  Configuration.cpp
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/common
  )

add_library(trinityconfig STATIC ${configuration_SRCS})
