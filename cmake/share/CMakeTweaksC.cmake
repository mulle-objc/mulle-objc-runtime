### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __CMAKE_TWEAKS_C_CMAKE__)
   set( __CMAKE_TWEAKS_C_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   # https://cmake.org/cmake/help/v3.1/policy/CMP0054.html
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

         set( CMAKE_OSX_DEPLOYMENT_TARGET "${OSX_VERSION}" CACHE STRING "Deployment target for OSX" FORCE)
      endif()

      set( CMAKE_POSITION_INDEPENDENT_CODE OFF)

   else()
      if( WIN32)
         # may not be enough though...
         cmake_minimum_required( VERSION 3.4)
      else()
         # UNIXy gcc based
         cmake_minimum_required( VERSION 3.0)
      endif()
      #
      # so we build static libs, but they might be linked into code
      # that needs -fPIC, but on some platforms (MUSL, COSMOPOLITAN)
      # we don't want PIC/PIE executables (why?)
      #
      # https://github.com/jart/cosmopolitan/issues/703
      #
      if( COSMOPOLITAN OR MUSL_STATIC_ONLY)
         set( CMAKE_SKIP_RPATH ON)
         # (230213) turn this on for mulle-objc-optimize
         set( CMAKE_POSITION_INDEPENDENT_CODE ON)
         # but don't link as such
         set( CMAKE_EXE_LINKER_FLAGS "-no-pie ${CMAKE_EXE_LINKER_FLAGS}")
      else()
         set( CMAKE_POSITION_INDEPENDENT_CODE ON)
      endif()
   endif()

   include( CMakeTweaksAuxC OPTIONAL)

endif()
