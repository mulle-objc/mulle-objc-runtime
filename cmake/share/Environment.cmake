### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __ENVIRONMENT__CMAKE__)
   set( __ENVIRONMENT__CMAKE__ ON)

   ## USE mulle-sde -v craft -- -DMULLE_TRACE_INCLUDE=ON to trace cmake
   ##     inclusion
   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   if( NOT PROJECT_IDENTIFIER)
      string( MAKE_C_IDENTIFIER "${PROJECT_NAME}" PROJECT_IDENTIFIER)
   endif()
   if( NOT PROJECT_UPCASE_IDENTIFIER)
      string( TOUPPER "${PROJECT_IDENTIFIER}" PROJECT_UPCASE_IDENTIFIER)
   endif()
   if( NOT PROJECT_DOWNCASE_IDENTIFIER)
      string( TOLOWER "${PROJECT_IDENTIFIER}" PROJECT_DOWNCASE_IDENTIFIER)
   endif()

   #
   #
   #
   if( NOT MULLE_VIRTUAL_ROOT)
      set( MULLE_VIRTUAL_ROOT "${PROJECT_SOURCE_DIR}")
   endif()

   # get MULLE_SDK_PATH into cmake list form
   # MULLE_SDK_PATH is set my mulle-craft and usually looks like the
   # default set below. But! If you are using --sdk --platform
   # distictions the paths will be different
   #
   if( NOT MULLE_SDK_PATH)
      string( REPLACE ":" ";" MULLE_SDK_PATH "$ENV{MULLE_SDK_PATH}")

      set( TMP_DEPENDENCY_DIR "$ENV{DEPENDENCY_DIR}")
      if( NOT TMP_DEPENDENCY_DIR)
         set( TMP_DEPENDENCY_DIR "${MULLE_VIRTUAL_ROOT}/dependency")
      endif()
      set( TMP_ADDICTION_DIR "$ENV{ADDICTION_DIR}")
      if( NOT TMP_ADDICTION_DIR)
         set( TMP_ADDICTION_DIR "${MULLE_VIRTUAL_ROOT}/addiction")
      endif()

      if( NOT MULLE_SDK_PATH)
         set( MULLE_SDK_PATH
            "${TMP_DEPENDENCY_DIR}"
            "${TMP_ADDICTION_DIR}"
         )
      endif()
   else()
      # temporary fix until mulle-objc 0.18 release
      if( $ENV{MULLE_MAKE_VERSION} VERSION_LESS 0.14.0)
         string( REPLACE ":" ";" MULLE_SDK_PATH "${MULLE_SDK_PATH}")
      endif()
   endif()

   set( TMP_INCLUDE_DIRS)
   set( TMP_CMAKE_INCLUDE_PATH)
   set( TMP_CMAKE_LIBRARY_PATH)
   set( TMP_CMAKE_FRAMEWORK_PATH)

   ###
   ### If you build DEBUG craftorder, but want RELEASE interspersed, so that
   ### the debugger doesn't trace through too much fluff then set the
   ### FALLBACK_BUILD_TYPE (for lack of a better name)
   ###
   ### TODO: reenable later
   ###
   # if( NOT FALLBACK_BUILD_TYPE)
   #    set( FALLBACK_BUILD_TYPE "$ENV{ORACLE_EO_ADAPTOR_FALLBACK_BUILD_TYPE}")
   #    if( NOT FALLBACK_BUILD_TYPE)
   #       set( FALLBACK_BUILD_TYPE "$ENV{FALLBACK_BUILD_TYPE}")
   #    endif()
   #    if( NOT FALLBACK_BUILD_TYPE)
   #       set( FALLBACK_BUILD_TYPE "Debug")
   #    endif()
   # endif()
   #
   # if( FALLBACK_BUILD_TYPE STREQUAL "Release")
   #    unset( FALLBACK_BUILD_TYPE)
   # endif()
   message( STATUS "MULLE_SDK_PATH=${MULLE_SDK_PATH}")

   foreach( TMP_SDK_PATH ${MULLE_SDK_PATH})
      message( STATUS "TMP_SDK_PATH=${TMP_SDK_PATH}")
      #
      # Add build-type includes/libs first
      # Add Release as a fallback afterwards
      #
      # The CMAKE_INCLUDE_PATH path for the CMAKE_BUILD_TYPE are added
      # unconditionally just in case ppl do interesting stuff in their
      # CMakeLists
      #
      # We always prepend to "override" inherited values, so
      # the order seems reversed
      #
      if( EXISTS "${TMP_SDK_PATH}")

         set( TMP_PREFIX "${TMP_SDK_PATH}")

         #
         # add build type unconditionally if not Release
         #
         if( CMAKE_BUILD_TYPE)
            if( NOT CMAKE_BUILD_TYPE STREQUAL "Release")
               set( TMP_CMAKE_INCLUDE_PATH
                  ${TMP_CMAKE_INCLUDE_PATH}
                  "${TMP_PREFIX}/${CMAKE_BUILD_TYPE}/include"
               )
               set( TMP_INCLUDE_DIRS
                  ${TMP_INCLUDE_DIRS}
                  "${TMP_PREFIX}/${CMAKE_BUILD_TYPE}/include"
               )

               set( TMP_CMAKE_LIBRARY_PATH
                  ${TMP_CMAKE_LIBRARY_PATH}
                  "${TMP_PREFIX}/${CMAKE_BUILD_TYPE}/lib"
               )
               set( TMP_CMAKE_FRAMEWORK_PATH
                  ${TMP_CMAKE_FRAMEWORK_PATH}
                  "${TMP_PREFIX}/${CMAKE_BUILD_TYPE}/Frameworks"
               )
            endif()
         endif()

         #
         # add release as fallback always
         #
         set( TMP_SDK_RELEASE_PATH "${TMP_PREFIX}/Release")
         if( NOT EXISTS "${TMP_SDK_RELEASE_PATH}")
            set( TMP_SDK_RELEASE_PATH "${TMP_PREFIX}")
         endif()

         message( STATUS "TMP_SDK_RELEASE_PATH=${TMP_SDK_RELEASE_PATH}")
         if( EXISTS "${TMP_SDK_RELEASE_PATH}/include")
            message( STATUS "TMP_SDK_RELEASE_PATH=${TMP_SDK_RELEASE_PATH}/include exists")
         else()
            message( STATUS "TMP_SDK_RELEASE_PATH=${TMP_SDK_RELEASE_PATH}/include does not exist")
         endif()

         if( EXISTS "${TMP_SDK_RELEASE_PATH}/include")
            set( TMP_CMAKE_INCLUDE_PATH
               ${TMP_CMAKE_INCLUDE_PATH}
               "${TMP_SDK_RELEASE_PATH}/include"
            )
            set( TMP_INCLUDE_DIRS
               ${TMP_INCLUDE_DIRS}
               "${TMP_SDK_RELEASE_PATH}/include"
            )
         endif()

         if( EXISTS "${TMP_SDK_RELEASE_PATH}/lib")
            set( TMP_CMAKE_LIBRARY_PATH
               ${TMP_CMAKE_LIBRARY_PATH}
               "${TMP_SDK_RELEASE_PATH}/lib"
            )
         endif()
         if( EXISTS "${TMP_SDK_RELEASE_PATH}/Frameworks")
            set( TMP_CMAKE_FRAMEWORK_PATH
               "${TMP_SDK_RELEASE_PATH}/Frameworks"
               ${TMP_CMAKE_FRAMEWORK_PATH}
            )
         endif()

         unset( TMP_SDK_RELEASE_PATH)
      endif()
   endforeach()

   set( CMAKE_INCLUDE_PATH
      ${TMP_CMAKE_INCLUDE_PATH}
      ${CMAKE_INCLUDE_PATH}
   )
   set( CMAKE_LIBRARY_PATH
      ${TMP_CMAKE_LIBRARY_PATH}
      ${CMAKE_LIBRARY_PATH}
   )
   set( CMAKE_FRAMEWORK_PATH
      ${TMP_CMAKE_FRAMEWORK_PATH}
      ${CMAKE_FRAMEWORK_PATH}
   )

   message( STATUS "CMAKE_INCLUDE_PATH=\"${CMAKE_INCLUDE_PATH}\"" )
   message( STATUS "CMAKE_LIBRARY_PATH=\"${CMAKE_LIBRARY_PATH}\"" )
   message( STATUS "CMAKE_FRAMEWORK_PATH=\"${CMAKE_FRAMEWORK_PATH}\"" )
   message( STATUS "INCLUDE_DIRS=\"${TMP_INCLUDE_DIRS}\"" )

   # given from outside
   message( STATUS "CMAKE_PREFIX_PATH=\"${CMAKE_PREFIX_PATH}\"" )
   message( STATUS "CMAKE_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"" )

   include_directories( BEFORE SYSTEM
      ${TMP_INCLUDE_DIRS}
   )

   unset( TMP_INCLUDE_DIRS)
   unset( TMP_CMAKE_INCLUDE_PATH)
   unset( TMP_CMAKE_LIBRARY_PATH)
   unset( TMP_CMAKE_FRAMEWORK_PATH)

   include( MultiPhase OPTIONAL)

   include( EnvironmentAux OPTIONAL)

endif()
