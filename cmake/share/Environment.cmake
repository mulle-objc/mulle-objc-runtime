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

   include( StringCase)

   if( NOT PROJECT_IDENTIFIER)
      snakeCaseString( "${PROJECT_NAME}" PROJECT_IDENTIFIER)
   endif()
   if( NOT PROJECT_UPCASE_IDENTIFIER)
      string( TOUPPER "${PROJECT_IDENTIFIER}" PROJECT_UPCASE_IDENTIFIER)
   endif()
   if( NOT PROJECT_DOWNCASE_IDENTIFIER)
      string( TOLOWER "${PROJECT_IDENTIFIER}" PROJECT_DOWNCASE_IDENTIFIER)
   endif()

   if( NOT MULLE_VIRTUAL_ROOT)
      set( MULLE_VIRTUAL_ROOT "$ENV{MULLE_VIRTUAL_ROOT}")
      if( NOT MULLE_VIRTUAL_ROOT)
         set( MULLE_VIRTUAL_ROOT "${PROJECT_SOURCE_DIR}")
      endif()
   endif()

   #
   # MULLE_SDK is dependency/addiction. Not sysroot!
   #
   # Get MULLE_SDK_PATH into cmake list form
   # MULLE_SDK_PATH is set by mulle-craft and usually looks like the
   # default set below. But! If you are using --sdk --platform
   # distinctions, the paths will be different
   #
   # Slight change. MULLE_SDK_PATH contains now addiction_dir
   # and depedency_dir, but nothing else. The configuration/sdk/platform
   # is passed in MULLE_SDK_SUBDIR
   #
   if( NOT MULLE_SDK_PATH)
      string( REPLACE ":" ";" MULLE_SDK_PATH "$ENV{MULLE_SDK_PATH}")
   endif()

   # if no MULLE_SDK_PATH is given, assume its not a mulle-sde build
   # in that case it's probably just cmake, so it would probably be bad to
   # not pick up dependencies installed into the system, by default
   if( MULLE_SDK_PATH)
      list( GET MULLE_SDK_PATH 0 DEPENDENCY_DIR)
      list( GET MULLE_SDK_PATH 1 ADDICTION_DIR)

      option( DEPENDENCY_IGNORE_SYSTEM_LIBARIES "Ignore system library paths in search for dependencies" ON)
      if( NOT DEPENDENCY_IGNORE_SYSTEM_LIBARIES)
         message( WARNING "Will also search system paths for dependencies, as MULLE_SDK_PATH is not set")
      endif()
   else()
      if( NOT MULLE_SDK_SUBDIR)
         set( MULLE_SDK_SUBDIR "${CMAKE_BUILD_TYPE}")
      endif()
      if( NOT MULLE_SDK_SUBDIR)
         set( MULLE_SDK_SUBDIR "Debug")
      endif()
      option( DEPENDENCY_IGNORE_SYSTEM_LIBARIES "Ignore system library paths in search for dependencies" OFF)
   endif()

   if( NOT DEPENDENCY_DIR)
      set( DEPENDENCY_DIR "$ENV{DEPENDENCY_DIR}")
      if( NOT DEPENDENCY_DIR)
         find_program( MULLE_SDE mulle-sde)
         if( MULLE_SDE)
            execute_process(
               OUTPUT_VARIABLE DEPENDENCY_DIR
               COMMAND ${MULLE_SDE} dependency-dir
               OUTPUT_STRIP_TRAILING_WHITESPACE
            )
         endif()
         if( NOT DEPENDENCY_DIR)
            set( DEPENDENCY_DIR "${MULLE_VIRTUAL_ROOT}/dependency")
         endif()
      endif()
   endif()


   message( STATUS "DEPENDENCY_DIR=\"${DEPENDENCY_DIR}\"")
   list( APPEND ADDITIONAL_BIN_PATH "${DEPENDENCY_DIR}/bin")

   if( NOT ADDICTION_DIR)
      set( ADDICTION_DIR "$ENV{ADDICTION_DIR}")
      if( NOT ADDICTION_DIR)
         find_program( MULLE_SDE mulle-sde)
         if( MULLE_SDE)
            execute_process(
               OUTPUT_VARIABLE ADDICTION_DIR
               COMMAND ${MULLE_SDE} addiction-dir
               OUTPUT_STRIP_TRAILING_WHITESPACE
            )
         endif()
         if( NOT ADDICTION_DIR)
            set( ADDICTION_DIR "${MULLE_VIRTUAL_ROOT}/addiction")
         endif()
      endif()
   endif()

   # if ADDICTION_DIR is not present get rid of it from search paths
   if( EXISTS "${ADDICTION_DIR}")
      set( MULLE_SDK_PATH
         "${DEPENDENCY_DIR}"
         "${ADDICTION_DIR}")
      list( APPEND ADDITIONAL_BIN_PATH "${ADDICTION_DIR}/bin")
      message( STATUS "ADDICTION_DIR=\"${ADDICTION_DIR}\"")
   else()
      message( STATUS "ADDICTION_DIR not present, therefore ignored")

      unset( ADDICTION_DIR)
      set( MULLE_SDK_PATH "${DEPENDENCY_DIR}")
   endif()

   # where the output is installed by other dependencies
   set( MULLE_SDK_DEPENDENCY_DIR "${DEPENDENCY_DIR}/${MULLE_SDK_SUBDIR}")

   set( TMP_INCLUDE_DIRS)
   set( TMP_CMAKE_INCLUDE_PATH)
   set( TMP_CMAKE_LIBRARY_PATH)
   set( TMP_CMAKE_FRAMEWORK_PATH)

   message( STATUS "MULLE_SDK_PATH=\"${MULLE_SDK_PATH}\"")
   message( STATUS "MULLE_SDK_FALLBACK_SUBDIR=\"${MULLE_SDK_FALLBACK_SUBDIR}\"")
   message( STATUS "MULLE_SDK_SUBDIR=\"${MULLE_SDK_SUBDIR}\"")
   message( STATUS "MULLE_SDK_DEPENDENCY_DIR=\"${MULLE_SDK_DEPENDENCY_DIR}\"")

   foreach( TMP_MULLE_SDK_PATH ${MULLE_SDK_PATH})
      set( TMP_MULLE_SDK_FALLBACK_PATH "${TMP_MULLE_SDK_PATH}")
      # keep pretty
      if( MULLE_SDK_FALLBACK_SUBDIR)
         set( TMP_MULLE_SDK_FALLBACK_PATH "${TMP_MULLE_SDK_FALLBACK_PATH}/${MULLE_SDK_FALLBACK_SUBDIR}")
      endif()
      if( MULLE_SDK_SUBDIR)
         set( TMP_MULLE_SDK_PATH "${TMP_MULLE_SDK_PATH}/${MULLE_SDK_SUBDIR}")
      endif()

      message( STATUS "TMP_MULLE_SDK_PATH=\"${TMP_MULLE_SDK_PATH}\"")
      message( STATUS "TMP_MULLE_SDK_FALLBACK_PATH=\"${TMP_MULLE_SDK_FALLBACK_PATH}\"")

      list( APPEND ADDITIONAL_BIN_PATH "${TMP_MULLE_SDK_PATH}/bin"
                                       "${TMP_MULLE_SDK_FALLBACK_PATH}/bin")

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

      #
      # add build type unconditionally if not Release
      #
      set( TMP_CMAKE_INCLUDE_PATH
         ${TMP_CMAKE_INCLUDE_PATH}
         "${TMP_MULLE_SDK_PATH}/include"
      )
      set( TMP_INCLUDE_DIRS
         ${TMP_INCLUDE_DIRS}
         "${TMP_MULLE_SDK_PATH}/include"
      )

      set( TMP_CMAKE_LIBRARY_PATH
         ${TMP_CMAKE_LIBRARY_PATH}
         "${TMP_MULLE_SDK_PATH}/lib"
      )
      set( TMP_CMAKE_FRAMEWORK_PATH
         ${TMP_CMAKE_FRAMEWORK_PATH}
         "${TMP_MULLE_SDK_PATH}/Frameworks"
      )

      #
      # add release as fallback if not same as above
      #
      if( NOT "${TMP_MULLE_SDK_FALLBACK_PATH}" STREQUAL "${TMP_MULLE_SDK_PATH}")
         if( EXISTS "${TMP_MULLE_SDK_FALLBACK_PATH}/include")
            set( TMP_CMAKE_INCLUDE_PATH
               ${TMP_CMAKE_INCLUDE_PATH}
               "${TMP_MULLE_SDK_FALLBACK_PATH}/include"
            )
            set( TMP_INCLUDE_DIRS
               ${TMP_INCLUDE_DIRS}
               "${TMP_MULLE_SDK_FALLBACK_PATH}/include"
            )
         endif()

         if( EXISTS "${TMP_MULLE_SDK_FALLBACK_PATH}/lib")
            set( TMP_CMAKE_LIBRARY_PATH
               ${TMP_CMAKE_LIBRARY_PATH}
               "${TMP_MULLE_SDK_FALLBACK_PATH}/lib"
            )
         endif()
         if( EXISTS "${TMP_MULLE_SDK_FALLBACK_PATH}/Frameworks")
            set( TMP_CMAKE_FRAMEWORK_PATH
               "${TMP_MULLE_SDK_FALLBACK_PATH}/Frameworks"
               ${TMP_CMAKE_FRAMEWORK_PATH}
            )
         endif()
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

   list( REMOVE_DUPLICATES ADDITIONAL_BIN_PATH)
   list( REMOVE_DUPLICATES TMP_INCLUDE_DIRS)  # superflous ?

   # these generate -isystem arguments, that add to the system search path
   # if we use BEFORE we would need to reverse the order in TMP_INCLUDE_DIRS
   include_directories( SYSTEM
      ${TMP_INCLUDE_DIRS}
   )

   message( STATUS "CMAKE_INCLUDE_PATH=\"${CMAKE_INCLUDE_PATH}\"" )
   message( STATUS "CMAKE_LIBRARY_PATH=\"${CMAKE_LIBRARY_PATH}\"" )
   message( STATUS "INCLUDE_DIRS=\"${TMP_INCLUDE_DIRS}\"" )
   message( STATUS "ADDITIONAL_BIN_PATH=\"${ADDITIONAL_BIN_PATH}\"")

   # given from outside
   message( STATUS "CMAKE_PREFIX_PATH=\"${CMAKE_PREFIX_PATH}\"" )
   message( STATUS "CMAKE_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\"" )
   # message( STATUS "TMP_INCLUDE_DIRS=\"${TMP_INCLUDE_DIRS}\"" )

   # not sure why cmake doesn't do this itself, we only add the custom
   # paths though
   if( APPLE)
      message( STATUS "CMAKE_FRAMEWORK_PATH=\"${CMAKE_FRAMEWORK_PATH}\"" )
      foreach( TMP_FRAMEWORK_PATH ${TMP_CMAKE_FRAMEWORK_PATH})
         add_definitions( -F "${TMP_FRAMEWORK_PATH}")
      endforeach()
      unset( TMP_FRAMEWORK_PATH)
   endif()


   #
   # add "d" to library names on windows MSVC for debugging libraries
   #
   if( MSVC AND NOT CMAKE_DEBUG_POSTFIX AND "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
      set( CMAKE_DEBUG_POSTFIX "d")
   endif()

   unset( TMP_INCLUDE_DIRS)
   unset( TMP_CMAKE_INCLUDE_PATH)
   unset( TMP_CMAKE_LIBRARY_PATH)
   unset( TMP_CMAKE_FRAMEWORK_PATH)

   include( MultiPhase OPTIONAL)

   include( EnvironmentAux OPTIONAL)

endif()
