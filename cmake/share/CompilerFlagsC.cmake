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
      if( NOT TMP_CONFIGURATION_NAME STREQUAL "DEBUG")
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

endif()
