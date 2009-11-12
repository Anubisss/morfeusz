
set(TAO_FOUND FALSE)
set(TAO_FIND_LIBS "PortableServer"  "CosNaming" "CosEvent" "CosEvent_Skel" "AnyTypeCode")

find_path(TAO_INCLUDE_DIR tao/corba.h PATHS ${ACE_INCLUDE_DIR} /usr/include /usr/local/include)
find_library(TAO_LIBRARY NAMES TAO TAOd ${ACE_INCLUDE_DIR}/lib $ENV{TAO_ROOT}/lib /usr/lib /usr/local/lib)
find_program(TAO_IDL_COMPILER "tao_idl" ${ACE_INCLUDE_DIR}/bin /usr/bin /usr/local/bin ${TAO_INCLUDE_DIR}/bin)
message(STATUS "tao_idl at: ${TAO_IDL_COMPILER}")

if(TAO_INCLUDE_DIR AND TAO_LIBRARY AND TAO_IDL_COMPILER)
  SET(TAO_FOUND TRUE)
endif(TAO_INCLUDE_DIR AND TAO_LIBRARY AND TAO_IDL_COMPILER)
#now let's search for parts of tao we need.

if(TAO_FOUND)

  foreach(LIBRARY ${TAO_FIND_LIBS})
    find_library(TAO_${LIBRARY}_LIB NAMES "TAO_${LIBRARY}" "TAO_${LIBRARY}d" ${TAO_LIBRARY} ${ACE_ROOT}/lib /usr/lib /usr/local/lib)
    if(TAO_${LIBRARY}_LIB)
      message(STATUS "${LIBRARY} found at: ${TAO_${LIBRARY}_LIB}")
      #list(APPEND ${TAO_LIBRARY} TAO_${LIBRARY}_LIB)
    else(TAO_${LIBRARY}_LIB)
      SET(TAO_FOUND FALSE)
    endif(TAO_${LIBRARY}_LIB)
  endforeach()

endif(TAO_FOUND)


