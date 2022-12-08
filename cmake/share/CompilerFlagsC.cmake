### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __COMPILER_FLAGS_C_CMAKE__)
   set( __COMPILER_FLAGS_C_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OTHER_C_FLAGS} ${UNWANTED_C_WARNINGS}")
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OTHER_C_FLAGS} ${UNWANTED_C_WARNINGS}")

   if( CMAKE_BUILD_TYPE)
      string( TOUPPER "${CMAKE_BUILD_TYPE}" TMP_CONFIGURATION_NAME)
      add_definitions( "-D${TMP_CONFIGURATION_NAME}" )
      if( NOT (TMP_CONFIGURATION_NAME STREQUAL "DEBUG"))
         add_definitions( "-DNDEBUG" )
      else()
         if( "${MULLE_C_COMPILER_ID}" MATCHES "^(Clang|MulleClang)$")
            set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wuninitialized")
         endif()
      endif()
   endif()

   if( MULLE_TEST)
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMULLE_TEST=1")
   endif()

   # set this as the default, as we expect shared libs to be included too
   # if this is not the case you need to change this on a case per case
   # basis
   #
   if( BUILD_SHARED_LIBS)
      set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMULLE_INCLUDE_DYNAMIC=1")
   endif()
   # load in flags defined by other plugins, presumably Objective-C
   include( CompilerFlagsAuxC OPTIONAL)

endif()
