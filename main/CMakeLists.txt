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

add_executable(morpheus ${main_SRCS})

target_link_libraries(
    morpheus
    morpheusdatabase
    common
    realmd
    proxyd
    ${ACE_LIBRARY}
    )

install(TARGETS morpheus DESTINATION ${INSTALL_DIR}/bin)
install(FILES morpheus.conf.dist DESTINATION ${INSTALL_DIR}/bin)
