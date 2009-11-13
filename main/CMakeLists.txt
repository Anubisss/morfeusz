SET(main_SRCS
  Main.cpp
  Service_Manager.cpp
  )

include_directories(
  ${CMAKE_SOURCE_DIR}/realmd
  ${CMAKE_SOURCE_DIR}/proxyd
  ${CMAKE_SOURCE_DIR}/database
  ${CMAKE_SOURCE_DIR}/common
  )

add_executable(trinity ${main_SRCS})

target_link_libraries(
  trinity
  trinitydatabase
  realmd
  proxyd
  ${ACE_LIBRARY}
  )

install(TARGETS trinity DESTINATION bin)