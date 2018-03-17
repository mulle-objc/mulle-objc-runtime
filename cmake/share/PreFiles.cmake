if( NOT __PRE_FILES__CMAKE__)
   set( __PRE_FILES__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # Set Search Paths
   #
   include( CMakeTweaks)


   ### Additional search paths based on build style

   if( CMAKE_BUILD_STYLE STREQUAL "Debug")
      set( CMAKE_INCLUDE_PATH
         "${DEPENDENCY_DIR}/Debug/include"
         "${ADDICTION_DIR}/Debug/include"
         ${CMAKE_INCLUDE_PATH}
      )
      set( CMAKE_LIBRARY_PATH
         "${DEPENDENCY_DIR}/Debug/lib"
         "${ADDICTION_DIR}/Debug/lib"
         ${CMAKE_LIBRARY_PATH}
      )
      set( CMAKE_FRAMEWORK_PATH
         "${DEPENDENCY_DIR}/Debug/Frameworks"
         "${ADDICTION_DIR}/Debug/Frameworks"
         ${CMAKE_FRAMEWORK_PATH}
      )
   endif()


   # a place to add stuff for ObjC or C++
   include( PreFilesCAux OPTIONAL)

endif()
