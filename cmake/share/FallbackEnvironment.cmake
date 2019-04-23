#
# Some settings to set, if we are not being built with mulle-craft but
# just mulle-make or cmake/make. This doesn't work for strict dipense
# style and when compiling for anything else than sdk=Default and
# platform=Default
#
if( NOT __FALLBACK_ENVIRONMENT___CMAKE__)
   set( __FALLBACK_ENVIRONMENT___CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   set( MULLE_SDK_PATH "$ENV{MULLE_SDK_PATH}")

   if( NOT MULLE_SDK_PATH)
      set( MULLE_SDK_PATH
         "${MULLE_VIRTUAL_ROOT}/dependency"
         "${MULLE_VIRTUAL_ROOT}/addiction"
      )
   endif()

   set( TMP_INCLUDE_DIRS)

   ###
   ### If you build DEBUG buildorder, but want RELEASE interspersed, so that
   ### the debugger doesn't trace through too much fluff then set the
   ### FALLBACK_BUILD_TYPE (for lack of a better name)
   ###
   ### TODO: reenable later
   ###
   # if( NOT FALLBACK_BUILD_TYPE)
   #    set( FALLBACK_BUILD_TYPE "$ENV{MULLE_OBJC_RUNTIME_FALLBACK_BUILD_TYPE}")
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


   foreach( TMP_SDK_PATH in ${MULLE_SDK_PATH})
      #
      # Add build-type includes/libs first
      # Add Release as a fallback afterwards
      # We always prepend to "override" inherited values, so
      # the order seems reversed
      #
      set( TMP_SDK_RELEASE_PATH "${TMP_SDK_PATH}/Release")
      if( NOT EXISTS "${TMP_SDK_RELEASE_PATH}")
         set( TMP_SDK_RELEASE_PATH "${TMP_SDK_PATH}")
      endif()

      set( CMAKE_INCLUDE_PATH
         "${TMP_SDK_RELEASE_PATH}/include"
         ${CMAKE_INCLUDE_PATH}
      )
      set( CMAKE_LIBRARY_PATH
         "${TMP_SDK_RELEASE_PATH}/lib"
         ${CMAKE_LIBRARY_PATH}
      )
      set( CMAKE_FRAMEWORK_PATH
         "${TMP_SDK_RELEASE_PATH}/Frameworks"
         ${CMAKE_FRAMEWORK_PATH}
      )

      set( TMP_INCLUDE_DIRS
         "${TMP_SDK_RELEASE_PATH}/include"
         ${TMP_INCLUDE_DIRS}
      )
      unset( TMP_SDK_RELEASE_PATH)

      #
      # now add build type unconditionally
      #
      if( NOT CMAKE_BUILD_TYPE STREQUAL "Release")
         set( CMAKE_INCLUDE_PATH
            "${TMP_SDK_PATH}/${CMAKE_BUILD_TYPE}/include"
            ${CMAKE_INCLUDE_PATH}
         )
         set( CMAKE_LIBRARY_PATH
            "${TMP_SDK_PATH}/${CMAKE_BUILD_TYPE}/lib"
            ${CMAKE_LIBRARY_PATH}
         )
         set( CMAKE_FRAMEWORK_PATH
            "${TMP_SDK_PATH}/${CMAKE_BUILD_TYPE}/Frameworks"
            ${CMAKE_FRAMEWORK_PATH}
         )

         set( TMP_INCLUDE_DIRS
            "${TMP_SDK_PATH}/${CMAKE_BUILD_TYPE}/include"
            ${TMP_INCLUDE_DIRS}
         )
      endif()
   endforeach()

   message( STATUS "CMAKE_INCLUDE_PATH=\"${CMAKE_INCLUDE_PATH}\"" )
   message( STATUS "CMAKE_LIBRARY_PATH=\"${CMAKE_LIBRARY_PATH}\"" )
   message( STATUS "CMAKE_FRAMEWORK_PATH=\"${CMAKE_FRAMEWORK_PATH}\"" )
   message( STATUS "INCLUDE_DIRS=\"${TMP_INCLUDE_DIRS}\"" )


   include_directories( BEFORE SYSTEM
      ${TMP_INCLUDE_DIRS}
   )

   unset( TMP_INCLUDE_DIRS)

endif()
