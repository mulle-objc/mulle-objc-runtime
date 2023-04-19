### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

### Dependencies


if( NOT __DEPENDENCIES___CMAKE__)
   set( __DEPENDENCIES___CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( PreDependencies OPTIONAL)

   include( DependenciesAndLibraries OPTIONAL)

   #
   # So the generated dependency code stores the result of each find_library
   # in a cached variable, this means we also have to cache the
   # accmulated lists. We can't do it in DependenciesAndLibraries because
   # that file is inherited in other projects and we don't want to
   # clobber their cache. But "Dependencies.cmake" is only included by the
   # main project
   #
   if( NOT DEFINED CACHE{INHERITED_INCLUDE_DIRS})
      # remove some duplicates (can happen, if we alias to an amalgamated library)
      list( REMOVE_DUPLICATES INHERITED_INCLUDE_DIRS)
      list( REMOVE_DUPLICATES INHERITED_DEFINITIONS)

      list( REMOVE_DUPLICATES HEADER_ONLY_LIBRARIES)

      list( REMOVE_DUPLICATES DEPENDENCY_LIBRARIES)
      list( REMOVE_DUPLICATES DEPENDENCY_FRAMEWORKS)
      list( REMOVE_DUPLICATES OS_SPECIFIC_LIBRARIES)
      list( REMOVE_DUPLICATES OS_SPECIFIC_FRAMEWORKS)

      list( REMOVE_DUPLICATES ALL_LOAD_OS_SPECIFIC_LIBRARIES)
      list( REMOVE_DUPLICATES ALL_LOAD_OS_SPECIFIC_FRAMEWORKS)
      list( REMOVE_DUPLICATES ALL_LOAD_DEPENDENCY_LIBRARIES)
      list( REMOVE_DUPLICATES ALL_LOAD_DEPENDENCY_FRAMEWORKS)

      list( REMOVE_DUPLICATES ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES)
      list( REMOVE_DUPLICATES ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS)
      list( REMOVE_DUPLICATES STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES)
      list( REMOVE_DUPLICATES STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS)

      set( INHERITED_INCLUDE_DIRS ${INHERITED_INCLUDE_DIRS} CACHE INTERNAL "cache these")
      set( INHERITED_DEFINITIONS  ${INHERITED_DEFINITIONS}  CACHE INTERNAL "cache these")

      set( HEADER_ONLY_LIBRARIES   ${HEADER_ONLY_LIBRARIES}  CACHE INTERNAL "cache these")

      set( DEPENDENCY_LIBRARIES    ${DEPENDENCY_LIBRARIES}   CACHE INTERNAL "cache these")
      set( DEPENDENCY_FRAMEWORKS   ${DEPENDENCY_FRAMEWORKS}  CACHE INTERNAL "cache these")
      set( OS_SPECIFIC_LIBRARIES   ${OS_SPECIFIC_LIBRARIES}  CACHE INTERNAL "cache these")
      set( OS_SPECIFIC_FRAMEWORKS  ${OS_SPECIFIC_FRAMEWORKS} CACHE INTERNAL "cache these")

      set( ALL_LOAD_OS_SPECIFIC_LIBRARIES           ${ALL_LOAD_OS_SPECIFIC_LIBRARIES}  CACHE INTERNAL "cache these")
      set( ALL_LOAD_OS_SPECIFIC_FRAMEWORKS          ${ALL_LOAD_OS_SPECIFIC_FRAMEWORKS} CACHE INTERNAL "cache these")
      set( ALL_LOAD_DEPENDENCY_LIBRARIES            ${ALL_LOAD_DEPENDENCY_LIBRARIES}   CACHE INTERNAL "cache these")
      set( ALL_LOAD_DEPENDENCY_FRAMEWORKS           ${ALL_LOAD_DEPENDENCY_FRAMEWORKS}  CACHE INTERNAL "cache these")

      set( ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES   ${ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES}  CACHE INTERNAL "cache these")
      set( ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS  ${ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS} CACHE INTERNAL "cache these")
      set( STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES    ${STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}   CACHE INTERNAL "cache these")
      set( STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS   ${STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS}  CACHE INTERNAL "cache these")
   endif()

#
# MEMO: Do not cache anything here. It will affect projects that include
#       this file as part of the inheritance scheme
#
option( INHERIT_DEPENDENCY_INCLUDES "Make headers of dependencies available as local headers" OFF)

if( INHERIT_DEPENDENCY_INCLUDES)
   # message( STATUS "INHERITED_INCLUDE_DIRS=\"${INHERITED_INCLUDE_DIRS}\"" )

   # these generate -I arguments, that add to the user search path
   include_directories( ${INHERITED_INCLUDE_DIRS})
endif()


option( INHERIT_DEPENDENCY_DEFINITIONS "Inherit compiler flags from dependencies" ON)

if( INHERIT_DEPENDENCY_INCLUDES)
   add_compile_definitions( ${INHERITED_DEFINITIONS})
endif()

endif()
