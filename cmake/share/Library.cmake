### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

### Library

if( NOT LIBRARY_NAME)
   set( LIBRARY_NAME "${PROJECT_NAME}")
endif()
if( NOT LIBRARY_IDENTIFIER)
   string( MAKE_C_IDENTIFIER "${LIBRARY_NAME}" LIBRARY_IDENTIFIER)
endif()
if( NOT LIBRARY_UPCASE_IDENTIFIER)
   string( TOUPPER "${LIBRARY_IDENTIFIER}" LIBRARY_UPCASE_IDENTIFIER)
endif()
# if( NOT LIBRARY_DOWNCASE_IDENTIFIER)
#    string( TOLOWER "${LIBRARY_IDENTIFIER}" LIBRARY_DOWNCASE_IDENTIFIER)
# endif()


if( NOT LIBRARY_SOURCES)
   set( LIBRARY_SOURCES "${SOURCES}")
   set( __LIBRARY_SOURCES_UNSET ON)
endif()

if( NOT LIBRARY_RESOURCES)
   set( LIBRARY_RESOURCES "${RESOURCES}")
   set( __LIBRARY_RESOURCES_UNSET ON)
endif()


include( PreLibrary OPTIONAL)


# support header only library
if( LIBRARY_SOURCES)
   # RPATH must be ahead of add_library, but is it really needed ?
   include( InstallRpath OPTIONAL)

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
      ${OTHER_LIBRARY_OBJECT_FILES}
      ${OTHER_${LIBRARY_UPCASE_IDENTIFIER}_OBJECT_FILES}
   )

   set_property( TARGET "_1_${LIBRARY_NAME}" PROPERTY CXX_STANDARD 11)

   #
   # Sometimes needed for elder linux ? Seen on xenial, with mulle-mmap
   #
   if( BUILD_SHARED_LIBS)
      set_property(TARGET "_1_${LIBRARY_NAME}" PROPERTY POSITION_INDEPENDENT_CODE TRUE)
   endif()


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
      if( BUILD_SHARED_LIBS)
         set_property(TARGET "_2_${LIBRARY_NAME}" PROPERTY POSITION_INDEPENDENT_CODE TRUE)
      endif()
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
         ${LIBRARY_RESOURCES}
      )

      add_dependencies( "${LIBRARY_NAME}" "_1_${LIBRARY_NAME}")
      if( STAGE2_SOURCES)
         add_dependencies( "${LIBRARY_NAME}" "_2_${LIBRARY_NAME}")
      endif()


      #
      # allow forward definitions in shared library
      #
      option( SHARED_UNRESOLVED_SYMBOLS "Shared libraries may have unresolved symbols" ON)

      include( LibraryAux OPTIONAL)

      if( BUILD_SHARED_LIBS)
         if( SHARED_UNRESOLVED_SYMBOLS)
            if( APPLE)
               target_link_libraries( "${LIBRARY_NAME}"
                  "-undefined dynamic_lookup"
               )
            endif()
         endif()

         if( NOT SHARED_LIBRARY_LIST)
            set( SHARED_LIBRARY_LIST
               ${DEPENDENCY_LIBRARIES}
               ${DEPENDENCY_FRAMEWORKS}
               ${OPTIONAL_DEPENDENCY_LIBRARIES}
               ${OPTIONAL_DEPENDENCY_FRAMEWORKS}
               ${OS_SPECIFIC_LIBRARIES}
               ${OS_SPECIFIC_FRAMEWORKS}
         )
         endif()

         include( PostSharedLibrary OPTIONAL) # additional hook

         target_link_libraries( "${LIBRARY_NAME}"
            ${SHARED_LIBRARY_LIST}
         )

         #
         # Something to set for shared libraries
         #
         # set_target_properties( "${LIBRARY_NAME}" PROPERTIES VERSION $ENV{PROJECT_VERSION})
         # set_target_properties( "${LIBRARY_NAME}" PROPERTIES SOVERSION 1)
         #
      endif()

      set( INSTALL_LIBRARY_TARGETS
         "${LIBRARY_NAME}"
         ${INSTALL_LIBRARY_TARGETS}
      )

      set( INSTALL_${LIBRARY_UPCASE_IDENTIFIER}_RESOURCES ${LIBRARY_RESOURCES})

   endif()
else()
   # header only library
   add_library("${LIBRARY_NAME}" INTERFACE
   )
endif()

include( PostLibrary OPTIONAL)

message( STATUS "PUBLIC_HEADERS=${PUBLIC_HEADERS}")
message( STATUS "PRIVATE_HEADERS=${PRIVATE_HEADERS}")
message( STATUS "PROJECT_INSTALLABLE_HEADERS=${PROJECT_INSTALLABLE_HEADERS}")


# clean LIBRARY_SOURCES for the next run, if set by this script
if( __LIBRARY_SOURCES_UNSET )
   unset( LIBRARY_SOURCES)
   unset( __LIBRARY_SOURCES_UNSET)
endif()


# clean LIBRARY_RESOURCES for the next run, if set by this script
if( __LIBRARY_RESOURCES_UNSET )
   unset( LIBRARY_RESOURCES)
   unset( __LIBRARY_RESOURCES_UNSET)
endif()
