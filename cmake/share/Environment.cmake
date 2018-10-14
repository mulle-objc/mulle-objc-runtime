if( NOT __ENVIRONMENT__CMAKE__)
   set( __ENVIRONMENT__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   set( MULLE_VIRTUAL_ROOT "$ENV{MULLE_VIRTUAL_ROOT}")
   set( DEPENDENCY_DIR "$ENV{DEPENDENCY_DIR}")
   set( ADDICTION_DIR "$ENV{ADDICTION_DIR}")

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
   else()
      ### If you build DEBUG buildorder, but want RELEASE interspersed, so that
      ### the debugger doesn't trace through too much fluff then set the
      ### FALLBACK_BUILD_TYPE (for lack of a better name)

      if( NOT FALLBACK_BUILD_TYPE)
         set( FALLBACK_BUILD_TYPE "$ENV{MULLE_OBJC_RUNTIME_FALLBACK_BUILD_TYPE}")
         if( NOT FALLBACK_BUILD_TYPE)
            set( FALLBACK_BUILD_TYPE "$ENV{MULLE_OBJC_RUNTIME_FALLBACK_BUILD_TYPE}")
            if( NOT FALLBACK_BUILD_TYPE)
               set( FALLBACK_BUILD_TYPE "$ENV{FALLBACK_BUILD_TYPE}")
            endif()
            if( NOT FALLBACK_BUILD_TYPE)
               set( FALLBACK_BUILD_TYPE "Debug")
            endif()
         endif()
      endif()

      if( FALLBACK_BUILD_TYPE STREQUAL "Release")
         unset( FALLBACK_BUILD_TYPE)
      endif()

      if( FALLBACK_BUILD_TYPE)
         set( CMAKE_INCLUDE_PATH
            ${CMAKE_INCLUDE_PATH}
            "${DEPENDENCY_DIR}/${FALLBACK_BUILD_TYPE}/include"
         )
         set( CMAKE_LIBRARY_PATH
            ${CMAKE_LIBRARY_PATH}
            "${DEPENDENCY_DIR}/${FALLBACK_BUILD_TYPE}/lib"
         )
         set( CMAKE_FRAMEWORK_PATH
            ${CMAKE_FRAMEWORK_PATH}
            "${DEPENDENCY_DIR}/${FALLBACK_BUILD_TYPE}/Frameworks"
         )
         message( STATUS "FALLBACK_BUILD_TYPE=\"${FALLBACK_BUILD_TYPE}\"" )
      endif()
   endif()


   include_directories( BEFORE SYSTEM
      ${DEPENDENCY_DIR}/include
      ${ADDICTION_DIR}/include
   )

   # after release include
   if( FALLBACK_BUILD_TYPE)
      include_directories( BEFORE SYSTEM
         ${DEPENDENCY_DIR}/${FALLBACK_BUILD_TYPE}/include
      )
   endif()

   # include files that get installed
   set( CMAKE_INCLUDES
      "cmake/DependenciesAndLibraries.cmake"
      "cmake/_Dependencies.cmake"
      "cmake/_Libraries.cmake"
   )

   # IDE visible cmake files
   set( CMAKE_EDITABLE_FILES
      CMakeLists.txt
      cmake/Headers.cmake
      cmake/Sources.cmake
      cmake/DependenciesAndLibraries.cmake
   )

   #
   # Parallel build support. run all "participating" projects once for
   # HEADERS_PHASE in parallel.
   # Now run all "participating" projects for COMPILE_PHASE in parallel.
   # Finally run all participating and non-participating projects in buildorder
   # serially together with the LINK_PHASE. What is tricky is that the
   # sequential projects may need to run first.
   #
   option( HEADERS_PHASE  "Install headers only phase (1)" OFF)
   option( COMPILE_PHASE  "Compile sources only phase (2)" OFF)
   option( LINK_PHASE     "Link and install only phase (3)" OFF)

   if( MULLE_MAKE_PHASE STREQUAL "HEADERS")
      set( HEADERS_PHASE ON)
   endif()
   if( MULLE_MAKE_PHASE STREQUAL "COMPILE")
      set( COMPILE_PHASE ON)
   endif()
   if( MULLE_MAKE_PHASE STREQUAL "LINK")
      set( LINK_PHASE ON)
   endif()

   if( NOT HEADERS_PHASE AND
       NOT COMPILE_PHASE AND
       NOT LINK_PHASE)
      set( HEADERS_PHASE ON)
      set( COMPILE_PHASE ON)
      set( LINK_PHASE ON)
   endif()

   include( EnvironmentAux OPTIONAL)

endif()
