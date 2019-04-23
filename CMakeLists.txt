cmake_minimum_required( VERSION 3.1)

project( mulle-objc-runtime C)

option( MULLE_OBJC_DEBUG_SUPPORT "Add html/dot debug support to mulle-objc" ON)


### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

include( Environment)

include( CCompiler)

include( Files)

if( MULLE_OBJC_DEBUG_SUPPORT)
   set( SOURCES
${SOURCES}
${DEBUG_SOURCES}
)

   set( PUBLIC_HEADERS
${PUBLIC_HEADERS}
${DEBUG_HEADERS}
)

   set( INSTALL_PUBLIC_HEADERS
${INSTALL_PUBLIC_HEADERS}
${DEBUG_HEADERS}
)
   add_definitions( -DMULLE_OBJC_DEBUG_SUPPORT)
endif()


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

if( "${C_COMPILER_NAME}" MATCHES "mulle-cl*")
   target_compile_options( "${LIBRARY_NAME}" PRIVATE -x objective-c)
   target_compile_options( ${STARTUP_NAME} PRIVATE -x objective-c)
endif()

set( EXECUTABLE_NAME mulle-objc-uniqueid)
set( EXECUTABLE_SOURCES  ${UNIQUEID_SOURCES})
set( EXECUTABLE_DEPENDENCY_NAMES mulle-objc-runtime)
# only need the hash function
set( EXECUTABLE_LIBRARY_LIST
   $<TARGET_FILE:mulle-objc-runtime>
)

include( Executable)

set( EXECUTABLE_NAME mulle-objc-signature)
set( EXECUTABLE_SOURCES  ${SIGNATURE_SOURCES})
set( EXECUTABLE_LIBRARY_LIST
   $<TARGET_FILE:mulle-objc-runtime>
)

include( Executable)

include( InstallExecutable)

include( FinalOutput OPTIONAL)
