### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __CMAKE_TWEAKS_C_CMAKE__)
   set( __CMAKE_TWEAKS_C_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   cmake_policy( SET CMP0054 NEW)

   # makes nicer Xcode projects, I see no detriment
   set_property( GLOBAL PROPERTY USE_FOLDERS ON)

   if( APPLE)
      cmake_minimum_required (VERSION 3.0)

      # CMAKE_OSX_SYSROOT must be set for CMAKE_OSX_DEPLOYMENT_TARGET (cmake bug)
      if( NOT CMAKE_OSX_SYSROOT)
         set( CMAKE_OSX_SYSROOT "/" CACHE STRING "SDK for OSX" FORCE)   # means current OS X
      endif()

      # baseline set to OSX_VERSION for rpath (is this still needed?)
      if( NOT CMAKE_OSX_DEPLOYMENT_TARGET AND NOT $ENV{MACOSX_DEPLOYMENT_TARGET} )
         execute_process( COMMAND sw_vers -productVersion
                          OUTPUT_VARIABLE OSX_VERSION_FULL
                          OUTPUT_STRIP_TRAILING_WHITESPACE)
         string( REGEX REPLACE "\\.[^.]*$" "" OSX_VERSION ${OSX_VERSION_FULL} )

         set(CMAKE_OSX_DEPLOYMENT_TARGET "${OSX_VERSION}" CACHE STRING "Deployment target for OSX" FORCE)
      endif()

      set( CMAKE_POSITION_INDEPENDENT_CODE FALSE)

   else()
      if( WIN32)
         # may not be enough though...

         cmake_minimum_required (VERSION 3.4)

         # set only for libraries ?
         set( CMAKE_POSITION_INDEPENDENT_CODE TRUE)
      else()
         # UNIXy gcc based
         cmake_minimum_required (VERSION 3.0)

         # set only for libraries ?
         set( CMAKE_POSITION_INDEPENDENT_CODE TRUE)
      endif()
   endif()

   include( CMakeTweaksAuxC OPTIONAL)

endif()


# extension : mulle-c/c-cmake
# directory : project/all
# template  : .../CMakeTweaksC.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
