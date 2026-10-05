### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# SDKEnvironment.cmake
#    The mulle-sde half of Environment.cmake. Locates the SDK, meaning the
#    dependency and addiction directories, and prepends it to the cmake
#    search paths.
#
#    Environment.cmake declares USE_DEPENDENCY_DIR and includes this file
#    when it is on. A cmake subproject pulled in with add_subdirectory()
#    always turns it off, and a plain cmake build of this project on its own
#    can pass -DUSE_DEPENDENCY_DIR=OFF. Such a build has no SDK and wants no
#    dependency directory, so none of this should run.
#
#    Nothing here is needed to compile the project itself. It only affects
#    where headers and libraries are searched for.
#
# INPUT
#    MULLE_SDK_PATH             list of SDK roots. Entry 0 becomes
#                               DEPENDENCY_DIR, entry 1 ADDICTION_DIR
#    MULLE_VIRTUAL_ROOT         mulle-sde project root. Falls back to the
#                               environment variable, then to
#                               CMAKE_CURRENT_SOURCE_DIR. Only used for the
#                               DEPENDENCY_DIR and ADDICTION_DIR fallbacks
#    DEPENDENCY_DIR             if unset and MULLE_SDK_PATH is also unset,
#                               `mulle-sde -s dependency-dir` is executed
#                               when mulle-sde is on PATH, otherwise
#                               ${MULLE_VIRTUAL_ROOT}/dependency is used
#    ADDICTION_DIR              same, via `mulle-sde -s addiction-dir`.
#                               Dropped again when the directory is absent
#    MULLE_SDK_SUBDIR           SDK subdirectory. Defaults to
#                               CMAKE_BUILD_TYPE, then "Debug". "None"
#                               means none
#    MULLE_SDK_FALLBACK_SUBDIR  second subdirectory tried after the above
#    CMAKE_BUILD_TYPE           see MULLE_SDK_SUBDIR
#    MULLE_TARGET_TRIPLE        from Environment.cmake, used for the
#                               debian style lib/<triple> search path
#    MULLE_TRACE_INCLUDE        trace inclusion of this file
#
# ENVIRONMENT
#    MULLE_VIRTUAL_ROOT         read when the variable is unset
#    MULLE_SDK_PATH             read when the variable is unset, ":"
#                               separated, except on WIN32
#    DEPENDENCY_DIR             read when the variable is unset
#    ADDICTION_DIR              read when the variable is unset
#
# OPTIONS
#    DEPENDENCY_IGNORE_SYSTEM_LIBRARIES   ON  when MULLE_SDK_PATH is set
#                                         OFF when it is not, so that a
#                                         plain cmake build still finds
#                                         system libraries
#
# OUTPUT
#    MULLE_VIRTUAL_ROOT         resolved, in cmake path syntax
#    DEPENDENCY_DIR             resolved, in cmake path syntax
#    ADDICTION_DIR              resolved, or unset when absent
#    MULLE_SDK_PATH             rebuilt as a cmake list from the two above
#    MULLE_SDK_SUBDIR           resolved subdirectory
#    MULLE_SDK_DEPENDENCY_DIR   ${DEPENDENCY_DIR}/${MULLE_SDK_SUBDIR}
#    CMAKE_INCLUDE_PATH         SDK include directories prepended
#    CMAKE_LIBRARY_PATH         SDK library directories prepended
#    CMAKE_FRAMEWORK_PATH       SDK Frameworks directories prepended
#    ADDITIONAL_BIN_PATH        SDK bin directories
#
#    Side effects: include_directories( SYSTEM) for the SDK include
#    directories, and add_definitions( -F) per framework path on APPLE.
#
# PROVIDES
#    mulle_append_list( list_name other_list)
#    mulle_append_unique_existing_path( list_name item)
#
# GUARD
#    __SDK_ENVIRONMENT__CMAKE__
#

if( NOT __SDK_ENVIRONMENT__CMAKE__)
   set( __SDK_ENVIRONMENT__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()


   macro( mulle_append_unique_existing_path list_name item)
      set( item_index -1) # supposedly makes local
      list( FIND ${list_name} "${item}" item_index)
      if( ${item_index} EQUAL -1)
         if( EXISTS "${item}")
            list( APPEND ${list_name} "${item}")
         endif()
      endif()
   endmacro()

   macro( mulle_append_list list_name other_list)
      set( item "") # supposedly makes local
      foreach( item ${${other_list}})
         list( APPEND ${list_name} "${item}")
      endforeach()
   endmacro()

   #
   # The mulle-sde project root. Only used to derive DEPENDENCY_DIR and
   # ADDICTION_DIR below when nothing else supplied them, so it lives here
   # rather than in Environment.cmake.
   #
   if( NOT MULLE_VIRTUAL_ROOT)
      set( MULLE_VIRTUAL_ROOT "$ENV{MULLE_VIRTUAL_ROOT}")
      if( NOT MULLE_VIRTUAL_ROOT)
         set( MULLE_VIRTUAL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
      endif()
   endif()
   file( TO_CMAKE_PATH ${MULLE_VIRTUAL_ROOT} MULLE_VIRTUAL_ROOT)

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
   # On WIN32 we get D:/whatevs, so we don't do the replace
   #
   if( NOT MULLE_SDK_PATH)
      if( NOT WIN32)
         string( REPLACE ":" ";" MULLE_SDK_PATH "$ENV{MULLE_SDK_PATH}")
      endif()
   endif()

   # if no MULLE_SDK_PATH is given, assume its not a mulle-sde build
   # in that case it's probably just cmake, so it would probably be bad to
   # not pick up dependencies installed into the system, by default
   if( MULLE_SDK_PATH)
      list( GET MULLE_SDK_PATH 0 DEPENDENCY_DIR)
      list( LENGTH MULLE_SDK_PATH TMP_LENGTH)
      unset( ADDICTION_DIR)
      if( TMP_LENGTH GREATER_EQUAL 2)
         list( GET MULLE_SDK_PATH 1 ADDICTION_DIR)
      endif()

      option( DEPENDENCY_IGNORE_SYSTEM_LIBRARIES "Ignore system library paths in search for dependencies" ON)
      if( NOT DEPENDENCY_IGNORE_SYSTEM_LIBRARIES)
         message( WARNING "Will also search system paths for dependencies, as MULLE_SDK_PATH is not set")
      endif()
   else()
      if( NOT MULLE_SDK_SUBDIR)
         set( MULLE_SDK_SUBDIR "${CMAKE_BUILD_TYPE}")
      endif()
      if( NOT MULLE_SDK_SUBDIR)
         set( MULLE_SDK_SUBDIR "Debug")
      endif()
      if( MULLE_SDK_SUBDIR MATCHES "None")
         set( MULLE_SDK_SUBDIR "")
      endif()
      option( DEPENDENCY_IGNORE_SYSTEM_LIBRARIES "Ignore system library paths in search for dependencies" OFF)

      if( NOT DEPENDENCY_DIR)
         set( DEPENDENCY_DIR "$ENV{DEPENDENCY_DIR}")
         if( NOT DEPENDENCY_DIR)
            find_program( MULLE_SDE mulle-sde)
            if( MULLE_SDE)
               execute_process(
                  OUTPUT_VARIABLE DEPENDENCY_DIR
                  COMMAND ${MULLE_SDE} -s dependency-dir
                  OUTPUT_STRIP_TRAILING_WHITESPACE
               )
            endif()
            if( NOT DEPENDENCY_DIR)
               set( DEPENDENCY_DIR "${MULLE_VIRTUAL_ROOT}/dependency")
            endif()
         endif()
      endif()

      if( NOT ADDICTION_DIR)
         set( ADDICTION_DIR "$ENV{ADDICTION_DIR}")
         if( NOT ADDICTION_DIR)
            find_program( MULLE_SDE mulle-sde)
            if( MULLE_SDE)
               execute_process(
                  OUTPUT_VARIABLE ADDICTION_DIR
                  COMMAND ${MULLE_SDE} -s addiction-dir
                  OUTPUT_STRIP_TRAILING_WHITESPACE
               )
            endif()
            if( NOT ADDICTION_DIR)
               set( ADDICTION_DIR "${MULLE_VIRTUAL_ROOT}/addiction")
            endif()
         endif()
      endif()
   endif()

   # if ADDICTION_DIR is not present get rid of it from search paths
   if( EXISTS "${ADDICTION_DIR}")
      set( MULLE_SDK_PATH
         "${DEPENDENCY_DIR}"
         "${ADDICTION_DIR}")

      # needed for windows
      file( TO_CMAKE_PATH ${ADDICTION_DIR} ADDICTION_DIR)
      list( APPEND ADDITIONAL_BIN_PATH "${ADDICTION_DIR}/bin")
      message( STATUS "ADDICTION_DIR=\"${ADDICTION_DIR}\"")
   else()
      message( STATUS "ADDICTION_DIR not present, therefore ignored")

      unset( ADDICTION_DIR)
      set( MULLE_SDK_PATH "${DEPENDENCY_DIR}")
   endif()

   # needed for windows
   file( TO_CMAKE_PATH ${DEPENDENCY_DIR} DEPENDENCY_DIR)
   list( APPEND ADDITIONAL_BIN_PATH "${DEPENDENCY_DIR}/bin")

   # where the output is installed by other dependencies
   set( MULLE_SDK_DEPENDENCY_DIR "${DEPENDENCY_DIR}/${MULLE_SDK_SUBDIR}")

   set( TMP_INCLUDE_DIRS)
   set( TMP_CMAKE_INCLUDE_PATH)
   set( TMP_CMAKE_LIBRARY_PATH)
   set( TMP_CMAKE_FRAMEWORK_PATH)

   message( STATUS "")
   message( STATUS "PROJECT_NAME=\"${PROJECT_NAME}\""                 )
   message( STATUS "")
   message( STATUS "MULLE_VIRTUAL_ROOT=\"${MULLE_VIRTUAL_ROOT}\""     )


   message( STATUS "MULLE_SDK_DEPENDENCY_DIR=\"${MULLE_SDK_DEPENDENCY_DIR}\"")
   message( STATUS "MULLE_SDK_FALLBACK_SUBDIR=\"${MULLE_SDK_FALLBACK_SUBDIR}\"")
   message( STATUS "MULLE_SDK_PATH=\"${MULLE_SDK_PATH}\"")
   message( STATUS "MULLE_SDK_SUBDIR=\"${MULLE_SDK_SUBDIR}\"")

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

      list( APPEND ADDITIONAL_BIN_PATH "${TMP_MULLE_SDK_PATH}/bin")
      list( APPEND ADDITIONAL_BIN_PATH "${TMP_MULLE_SDK_FALLBACK_PATH}/bin")

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
      # add build type unconditionally if not Release, these directories
      # might every well "pop up" during the cmake build, so don't go
      # for existing path (could check so)
      #
      if( "${TMP_MULLE_SDK_PATH}" MATCHES "^${DEPENDENCY_DIR}.*")
         list( APPEND TMP_CMAKE_INCLUDE_PATH "${TMP_MULLE_SDK_PATH}/include")
         list( APPEND TMP_INCLUDE_DIRS "${TMP_MULLE_SDK_PATH}/include")
         list( APPEND TMP_CMAKE_LIBRARY_PATH "${TMP_MULLE_SDK_PATH}/lib")
         list( APPEND TMP_CMAKE_FRAMEWORK_PATH "${TMP_MULLE_SDK_PATH}/Frameworks")
      else()
         mulle_append_unique_existing_path( TMP_CMAKE_INCLUDE_PATH "${TMP_MULLE_SDK_PATH}/include")
         mulle_append_unique_existing_path( TMP_INCLUDE_DIRS "${TMP_MULLE_SDK_PATH}/include")
         mulle_append_unique_existing_path( TMP_CMAKE_LIBRARY_PATH "${TMP_MULLE_SDK_PATH}/lib")
         if( UNIX)
            mulle_append_unique_existing_path( TMP_CMAKE_LIBRARY_PATH "${TMP_MULLE_SDK_PATH}/lib/${MULLE_TARGET_TRIPLE}")
         endif()
         mulle_append_unique_existing_path( TMP_CMAKE_FRAMEWORK_PATH "${TMP_MULLE_SDK_PATH}/Frameworks")
      endif()

      #
      # add release as fallback if not same as above, but only for pieces the
      # primary configuration/architecture does not already provide. Otherwise
      # the fallback pulls in a sibling install (the host or another
      # configuration) and duplicates headers or escapes the architecture.
      #
      if( NOT "${TMP_MULLE_SDK_FALLBACK_PATH}" STREQUAL "${TMP_MULLE_SDK_PATH}")
         if( NOT EXISTS "${TMP_MULLE_SDK_PATH}/include")
            mulle_append_unique_existing_path( TMP_CMAKE_INCLUDE_PATH "${TMP_MULLE_SDK_FALLBACK_PATH}/include")
            mulle_append_unique_existing_path( TMP_INCLUDE_DIRS "${TMP_MULLE_SDK_FALLBACK_PATH}/include")
         endif()
         if( NOT EXISTS "${TMP_MULLE_SDK_PATH}/lib")
            mulle_append_unique_existing_path( TMP_CMAKE_LIBRARY_PATH "${TMP_MULLE_SDK_FALLBACK_PATH}/lib")
         endif()
         if( NOT EXISTS "${TMP_MULLE_SDK_PATH}/Frameworks")
            mulle_append_unique_existing_path( TMP_CMAKE_FRAMEWORK_PATH "${TMP_MULLE_SDK_FALLBACK_PATH}/Frameworks")
         endif()
      endif()
   endforeach()


   # memo used to do list remove duplicates here
   mulle_append_list( TMP_CMAKE_INCLUDE_PATH CMAKE_INCLUDE_PATH)
   list( REMOVE_DUPLICATES TMP_CMAKE_INCLUDE_PATH)
   set( CMAKE_INCLUDE_PATH "${TMP_CMAKE_INCLUDE_PATH}")

   mulle_append_list( TMP_CMAKE_LIBRARY_PATH CMAKE_LIBRARY_PATH)
   list( REMOVE_DUPLICATES TMP_CMAKE_LIBRARY_PATH)
   set( CMAKE_LIBRARY_PATH "${TMP_CMAKE_LIBRARY_PATH}")

   mulle_append_list( TMP_CMAKE_FRAMEWORK_PATH CMAKE_FRAMEWORK_PATH)
   list( REMOVE_DUPLICATES TMP_CMAKE_FRAMEWORK_PATH)
   set( CMAKE_FRAMEWORK_PATH "${TMP_CMAKE_FRAMEWORK_PATH}")

   list( REMOVE_DUPLICATES ADDITIONAL_BIN_PATH)
   list( REMOVE_DUPLICATES TMP_INCLUDE_DIRS)


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
   message( STATUS "MULLE_TARGET_TRIPLE=\"${MULLE_TARGET_TRIPLE}\"" )
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

   unset( TMP_INCLUDE_DIRS)
   unset( TMP_CMAKE_INCLUDE_PATH)
   unset( TMP_CMAKE_LIBRARY_PATH)
   unset( TMP_CMAKE_FRAMEWORK_PATH)

endif()
