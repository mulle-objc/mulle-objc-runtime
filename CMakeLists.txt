cmake_minimum_required( VERSION 3.0)

project( mulle-objc-runtime C)


### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

include( Environment)


### Files

include( PreFiles OPTIONAL)

set( MULLE_OBJC_RUNTIME_LIBRARY ON)

#
# Include all non underscored .cmake files in ./cmake
#
file( GLOB LOCAL_CMAKE_INCLUDES
   "cmake/[A-Za-z0-9]*.cmake"
)

foreach( FILE ${LOCAL_CMAKE_INCLUDES} )
   include( "${FILE}")
endforeach()


include_directories( ${INCLUDE_DIRS})

set( PROJECT_FILES
   ${SOURCES}
   ${DEBUG_SOURCES}
   ${PUBLIC_HEADERS}
   ${PRIVATE_HEADERS}
   ${CMAKE_EDITABLE_FILES}
)


include( PostFiles OPTIONAL)

### Library


#
# these definitions are just used when compiling the
# runtime itself, with a regular C compiler
#
add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (default: on)
add_definitions( -D__MULLE_OBJC_NO_TRT__)  ## thread local runtime (default: off)
add_definitions( -D__MULLE_OBJC_FMC__)     ## fast method calls (default: on)
add_definitions( -DMULLE_C11_NO_NOOB_WARNINGS=1)


include( PreLibrary OPTIONAL)

add_library( "mulle-objc-runtime"
${PROJECT_FILES}
)

if( BUILD_SHARED_LIBS)
   target_link_libraries( mulle-objc-runtime
      ${DEPENDENCY_LIBRARIES}
      ${OS_SPECIFIC_LIBRARIES}
   )
endif()

target_compile_definitions( mulle-objc-runtime
PRIVATE ${MULLE_OBJC_RUNTIME_DEFINITIONS}
)

add_library( mulle-objc-runtime-startup STATIC
${STARTUP_SOURCES}
)

set( STARTUP_LIBRARY
mulle-objc-runtime-startup
)

set( INSTALL_LIBRARY_TARGETS
"mulle-objc-runtime"
"mulle-objc-runtime-startup"
)

include( PostLibrary OPTIONAL)


add_executable( mulle-objc-uniqueid
${UNIQUEID_SOURCES}
)


target_compile_definitions( mulle-objc-uniqueid
PRIVATE ${MULLE_OBJC_RUNTIME_DEFINITIONS}
)


add_dependencies( mulle-objc-uniqueid
mulle-objc-runtime
)

target_link_libraries( mulle-objc-uniqueid
$<TARGET_FILE:mulle-objc-runtime>
${C_DEPENDENCY_LIBRARIES}
${OS_SPECIFIC_LIBRARIES}
)


set( INSTALL_EXECUTABLE_TARGETS
"mulle-objc-uniqueid"
)


### Install

include( PreInstall OPTIONAL)

install( TARGETS ${INSTALL_EXECUTABLE_TARGETS} DESTINATION "bin")
install( TARGETS ${INSTALL_LIBRARY_TARGETS} DESTINATION "lib")
install( FILES ${PUBLIC_HEADERS}  DESTINATION "include/mulle-objc-runtime")
install( FILES ${DEBUG_HEADERS}  DESTINATION "include/mulle-objc-runtime")
install( FILES ${PRIVATE_HEADERS} DESTINATION "include/mulle-objc-runtime/private")

# definitions from cmake/share/Environment.cmake
install( FILES ${CMAKE_INCLUDES}  DESTINATION "include/mulle-objc-runtime/cmake")

# definitions from cmake/share/LinkManifest.cmake
install( FILES ${LINK_MANIFEST_FILES}  DESTINATION "include/mulle-objc-runtime/link")

include( PostInstall OPTIONAL)
