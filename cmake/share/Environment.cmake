if( NOT __ENVIRONMENT__CMAKE__)
   set( __ENVIRONMENT__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   set( MULLE_VIRTUAL_ROOT "$ENV{MULLE_VIRTUAL_ROOT}")

   if( NOT MULLE_VIRTUAL_ROOT)
      set( MULLE_VIRTUAL_ROOT "${PROJECT_SOURCE_DIR}")
   endif()
   if( NOT DEPENDENCY_DIR)
      set( DEPENDENCY_DIR "${MULLE_VIRTUAL_ROOT}/dependency")
   endif()
   if( NOT ADDICTION_DIR)
      set( ADDICTION_DIR "${MULLE_VIRTUAL_ROOT}/addiction")
   endif()

   set( CMAKE_INCLUDE_PATH "${DEPENDENCY_DIR}/include"
      "${ADDICTION_DIR}/include"
      ${CMAKE_INCLUDE_PATH}
   )
   set( CMAKE_LIBRARY_PATH "${DEPENDENCY_DIR}/lib"
      "${ADDICTION_DIR}/lib"
      ${CMAKE_LIBRARY_PATH}
   )
   set( CMAKE_FRAMEWORK_PATH "${DEPENDENCY_DIR}/Frameworks"
      "${ADDICTION_DIR}/Frameworks"
      ${CMAKE_FRAMEWORK_PATH}
   )

   ### Additional search paths based on build style

   if( NOT CMAKE_BUILD_TYPE STREQUAL "Release")
      set( CMAKE_INCLUDE_PATH
         "${DEPENDENCY_DIR}/${CMAKE_BUILD_TYPE}/include"
         ${CMAKE_INCLUDE_PATH}
      )
      set( CMAKE_LIBRARY_PATH
         "${DEPENDENCY_DIR}/${CMAKE_BUILD_TYPE}/lib"
         ${CMAKE_LIBRARY_PATH}
      )
      set( CMAKE_FRAMEWORK_PATH
         "${DEPENDENCY_DIR}/${CMAKE_BUILD_TYPE}/Frameworks"
         ${CMAKE_FRAMEWORK_PATH}
      )

      include_directories( BEFORE SYSTEM
         ${DEPENDENCY_DIR}/${CMAKE_BUILD_TYPE}/include
      )
   endif()

   include_directories( BEFORE SYSTEM
      ${DEPENDENCY_DIR}/include
      ${ADDICTION_DIR}/include
   )

   set( CMAKE_INCLUDES
      "cmake/DependenciesAndLibraries.cmake"
      "cmake/_Dependencies.cmake"
      "cmake/_Libraries.cmake"
   )

   set( CMAKE_EDITABLE_FILES
      CMakeLists.txt
      cmake/HeadersAndSources.cmake
      cmake/DependenciesAndLibraries.cmake
   )

   include( EnvironmentAux OPTIONAL)

endif()
