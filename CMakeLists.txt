cmake_minimum_required( VERSION 3.1)

project( mulle-objc-runtime C)


### mulle-sde environment

# add cmake module paths to search path
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake/share")
list( INSERT CMAKE_MODULE_PATH 0 "${PROJECT_SOURCE_DIR}/cmake")

include( Environment)

#
# these definitions are just used when compiling the
# runtime itself, with a regular C compiler
#
add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (default: on)
add_definitions( -D__MULLE_OBJC_NO_TRT__)  ## thread local runtime (default: off)
add_definitions( -D__MULLE_OBJC_FMC__)     ## fast method calls (default: on)
add_definitions( -DMULLE_C11_NO_NOOB_WARNINGS=1)


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

include( PostFiles OPTIONAL)

set( PROJECT_FILES
   ${PROJECT_FILES}
   ${DEBUG_SOURCES}
   ${SOURCES}
   ${DEBUG_HEADERS}
   ${PUBLIC_HEADERS}
   ${PRIVATE_HEADERS}
   ${CMAKE_EDITABLE_FILES}
)

### Library

include( PreLibrary OPTIONAL)

# ##################################
# Mainly of interest for Objective-C
# ##################################
# Libraries are built in two stages:
#
# In the first step the PROJECT_FILES are compiled.
# In the second step STAGE2_SOURCES are added.
# This allows PostLibrary to run an analysis step over PROJECT_FILES and
# generate files to be included by STAGE2_SOURCES. If there are no
# STAGE2_SOURCES then this is just a more verbose way of doing it.
# OBJC_LOADER_INC is the generated analysis step.
#
#
add_library( "_1_mulle-objc-runtime" OBJECT
   ${PROJECT_FILES}
)

set( ALL_OBJECT_FILES
   $<TARGET_OBJECTS:_1_mulle-objc-runtime>
)

set_property( TARGET "_1_mulle-objc-runtime" PROPERTY CXX_STANDARD 11)

if( STAGE2_SOURCES)
   add_library( "_2_mulle-objc-runtime" OBJECT
      ${STAGE2_SOURCES}
      ${STAGE2_HEADERS}
   )
   set( ALL_OBJECT_FILES
      ${ALL_OBJECT_FILES}
      $<TARGET_OBJECTS:_2_mulle-objc-runtime>
   )
   set_property( TARGET "_2_mulle-objc-runtime" PROPERTY CXX_STANDARD 11)
else()
   if( STAGE2_HEADERS)
      message( FATAL_ERROR "No STAGE2_SOURCES found but STAGE2_HEADERS exist")
   endif()
endif()


add_library( "mulle-objc-runtime"
   ${ALL_OBJECT_FILES}
)


set( INSTALL_LIBRARY_TARGETS "mulle-objc-runtime")

include( PostLibrary OPTIONAL)


#
# defined by cmake -DBUILD_SHARED_LIBS=ON
#
if( BUILD_SHARED_LIBS)
   set( SHARED_LIBRARY_LIST
      ${DEPENDENCY_LIBRARIES}
      ${OPTIONAL_DEPENDENCY_LIBRARIES}
      ${OS_SPECIFIC_LIBRARIES}
   )

   include( PostSharedLibrary OPTIONAL) # additional hook

   target_link_libraries( mulle-objc-runtime
      ${SHARED_LIBRARY_LIST}
   )
endif()


### Executable


add_executable( mulle-objc-uniqueid
   ${UNIQUEID_SOURCES}
)
set_property( TARGET "mulle-objc-uniqueid" PROPERTY CXX_STANDARD 11)


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
install( FILES ${PUBLIC_HEADERS} DESTINATION "include/mulle-objc-runtime")
install( FILES ${DEBUG_HEADERS} DESTINATION "include/mulle-objc-runtime")
install( FILES ${PRIVATE_HEADERS} DESTINATION "include/mulle-objc-runtime/private")

install( FILES ${CMAKE_INCLUDES}  DESTINATION "include/mulle-objc-runtime/cmake")

include( PostInstall OPTIONAL)
