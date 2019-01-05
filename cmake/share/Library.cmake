# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

### Library

if( NOT LIBRARY_NAME)
   set( LIBRARY_NAME "${PROJECT_NAME}")
endif()

if( NOT LIBRARY_SOURCES)
   set( LIBRARY_SOURCES "${SOURCES}")
endif()


include( PreLibrary OPTIONAL)

if( NOT LIBRARY_SOURCES)
   message( FATAL_ERROR "There are no sources to compile for libray ${LIBRARY_NAME}. Did mulle-sde update run yet ?")
endif()

# Libraries are built in two stages:
#
# In the first step the PROJECT_FILES are compiled.
# In the second step STAGE2_SOURCES are added.
# This allows PostLibrary to run an analysis step over PROJECT_FILES and
# generate files to be included by STAGE2_SOURCES. If there are no
# STAGE2_SOURCES then this is just a more verbose way of doing it.
# OBJC_LOADER_INC is the generated analysis step.
#
# This also enables parallel builds, when the products for a link aren't
# available yet.
#
add_library( "_1_${LIBRARY_NAME}" OBJECT
   ${LIBRARY_SOURCES}
)

set( ALL_OBJECT_FILES
   $<TARGET_OBJECTS:_1_${LIBRARY_NAME}>
)

set_property( TARGET "_1_${LIBRARY_NAME}" PROPERTY CXX_STANDARD 11)


if( STAGE2_SOURCES)
   add_library( "_2_${LIBRARY_NAME}" OBJECT
      ${STAGE2_SOURCES}
      ${STAGE2_HEADERS}
   )
   set( ALL_OBJECT_FILES
      ${ALL_OBJECT_FILES}
      $<TARGET_OBJECTS:_2_${LIBRARY_NAME}>
   )
   set_property( TARGET "_2_${LIBRARY_NAME}" PROPERTY CXX_STANDARD 11)
else()
   if( STAGE2_HEADERS)
      message( FATAL_ERROR "No STAGE2_SOURCES found but STAGE2_HEADERS exist")
   endif()
endif()


if( LINK_PHASE)

   #
   # Three ways to do shared libraries:
   #
   # * define -DBUILD_SHARED_LIBS=ON on the cmake command line and keep the file
   #   as is.
   # * Let mulle-make do this for you with:
   #      mulle-sde definition --global set BUILD_SHARED_LIBS ON
   # * Define  LIBRARY_NAME_TYPE as SHARED
   #      set( BUILD_SHARED_LIBS ON) line.
   #
   add_library( "${LIBRARY_NAME}"
      ${ALL_OBJECT_FILES}
      ${PROJECT_INSTALLABLE_HEADERS} # else won't get installed by framework
   )

   add_dependencies( "${LIBRARY_NAME}" "_1_${LIBRARY_NAME}")
   if( STAGE2_SOURCES)
      add_dependencies( "${LIBRARY_NAME}" "_2_${LIBRARY_NAME}")
   endif()

   message( STATUS "PUBLIC_HEADERS=${PUBLIC_HEADERS}")
   message( STATUS "PRIVATE_HEADERS=${PRIVATE_HEADERS}")
   message( STATUS "PROJECT_INSTALLABLE_HEADERS=${PROJECT_INSTALLABLE_HEADERS}")

   include( LibraryAux OPTIONAL)

   if( BUILD_SHARED_LIBS)
      if( NOT SHARED_LIBRARY_LIST)
         set( SHARED_LIBRARY_LIST
            ${DEPENDENCY_LIBRARIES}
            ${OPTIONAL_DEPENDENCY_LIBRARIES}
            ${OS_SPECIFIC_LIBRARIES}
         )
      endif()

      include( PostSharedLibrary OPTIONAL) # additional hook

      target_link_libraries( "${LIBRARY_NAME}"
         ${SHARED_LIBRARY_LIST}
      )
   endif()

   #
   # Something to set for shared libraries
   #
   # set_target_properties( "${LIBRARY_NAME}" PROPERTIES VERSION $ENV{PROJECT_VERSION})
   # set_target_properties( "${LIBRARY_NAME}" PROPERTIES SOVERSION 1)

   set( INSTALL_LIBRARY_TARGETS
      "${LIBRARY_NAME}"
      ${INSTALL_LIBRARY_TARGETS}
   )

   include( PostLibrary OPTIONAL)
endif()

   ### Install
