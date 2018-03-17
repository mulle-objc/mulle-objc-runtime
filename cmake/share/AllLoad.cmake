if( NOT __ALL_LOAD__CMAKE__)
   set( __ALL_LOAD__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # Get the linker to not strip away "unused" symbols during
   # a link
   #
   # This is either done by prefixing a library or enclosing
   # link statements
   #
   if( APPLE)
      set( FORCE_LOAD_PREFIX "-force_load ")
   else()
      if( WIN32)
         set( FORCE_LOAD_PREFIX "-WHOLEARCHIVE:")
      else()
         set( BEGIN_ALL_LOAD "-Wl,--whole-archive")
         set( END_ALL_LOAD "-Wl,--no-whole-archive")
         set( FORCE_LOAD_PREFIX)
      endif()
   endif()

endif()
