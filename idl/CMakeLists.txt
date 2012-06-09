set(idl_SRCS
    ${CMAKE_SOURCE_DIR}/idl/Proxy_EventsC.cpp
    ${CMAKE_SOURCE_DIR}/idl/Proxy_EventsC.h
    ${CMAKE_SOURCE_DIR}/idl/Proxy_EventsC.inl
    ${CMAKE_SOURCE_DIR}/idl/Proxy_EventsS.cpp
    ${CMAKE_SOURCE_DIR}/idl/Proxy_EventsS.h
)

include_directories(
    ${ACE_INCLUDE_DIR}
    ${TAO_INCLUDE_DIR}
)

add_custom_target(idl_gen ${TAO_IDL_COMPILER} ${CMAKE_SOURCE_DIR}/idl/Proxy_Events.idl WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/idl)

set_source_files_properties(
    Proxy_EventsC.cpp
    Proxy_EventsC.h
    Proxy_EventsC.inl
    Proxy_EventsS.cpp
    Proxy_EventsS.h
    PROPERTIES
    GENERATED TRUE
    LANGUAGE CXX
)

add_library(idl STATIC ${idl_SRCS})
add_dependencies(idl idl_gen)
