cmake_minimum_required( VERSION 3.15)

project( mulle-objc-runtime C)

### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/reflect")
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

include( Environment)

include( CCompiler)

include( Files)


### Library

if( HEADERS_PHASE)
   install( FILES ${INSTALL_PUBLIC_HEADERS}  DESTINATION "include/mulle-objc-runtime")
   install( FILES ${INSTALL_PRIVATE_HEADERS} DESTINATION "include/mulle-objc-runtime/private")
   install( FILES ${CMAKE_INCLUDES}          DESTINATION "include/mulle-objc-runtime/cmake")

   # short cut out
   if( NOT COMPILE_PHASE AND NOT LINK_PHASE)
      return()
   endif()
endif()

if( LINK_PHASE)
   include( Dependencies OPTIONAL)
endif()

include( Library)
include( InstallLibrary)



set( EXECUTABLE_NAME mulle-objc-uniqueid)

# only need the hash function (done like this if runtime is built as shared)
set( EXECUTABLE_SOURCES  ${UNIQUEID_SOURCES}
src/mulle-objc-uniqueid.c
)
set( EXECUTABLE_DEPENDENCY_NAMES
mulle-objc-runtime)
# only need the hash function
set( EXECUTABLE_LIBRARY_LIST "${MULLE_DATA_LIBRARY}")

include( Executable)

set( EXECUTABLE_NAME mulle-objc-signature)
# only need the signature function
set( EXECUTABLE_SOURCES  ${SIGNATURE_SOURCES}
src/mulle-objc-signature.c)
set( EXECUTABLE_LIBRARY_LIST "")
set( EXECUTABLE_DEPENDENCY_NAMES "")

include( Executable)

include( InstallExecutable)

include( FinalOutput OPTIONAL)
