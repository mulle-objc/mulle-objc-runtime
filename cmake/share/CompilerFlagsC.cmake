if( NOT __COMPILER_FLAGS_C_CMAKE__)
   set( __COMPILER_FLAGS_C_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   if( CMAKE_BUILD_STYLE STREQUAL "Debug")
      if( MULLE_C_COMPILER_ID MATCHES "^(Intel|MSVC|MSVC-Clang|MSVC-MulleClang)$")
         set( OTHER_C_FLAGS "${OTHER_C_FLAGS} /DDEBUG=1")
      else()
         set( OTHER_C_FLAGS "${OTHER_C_FLAGS} -DDEBUG")
      endif()
   else()
      if( MULLE_C_COMPILER_ID MATCHES "^(Intel|MSVC|MSVC-Clang|MSVC-MulleClang)$")
         set( OTHER_C_FLAGS "${OTHER_C_FLAGS} /DNDEBUG=1")
      else()
         set( OTHER_C_FLAGS "${OTHER_C_FLAGS} -DNDEBUG=1")
      endif()
   endif()

   set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OTHER_C_FLAGS} ${UNWANTED_C_WARNINGS}")
   set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OTHER_C_FLAGS} ${UNWANTED_C_WARNINGS}")

   include( CompilerFlagsCAux OPTIONAL)

endif()
