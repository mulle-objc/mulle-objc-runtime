### Dependencies


if( NOT __DEPENDENCIES___CMAKE__)
   set( __DEPENDENCIES___CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( PreDependencies OPTIONAL)

   include( "cmake/DependenciesAndLibraries.cmake" OPTIONAL)

endif()
