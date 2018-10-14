if( NOT __ALL_LOAD_C_CMAKE__)
   set( __ALL_LOAD_C_CMAKE__ ON)

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
      set( BEGIN_ALL_LOAD)
      set( END_ALL_LOAD)
   else()
      if( WIN32)
         set( FORCE_LOAD_PREFIX "-WHOLEARCHIVE:")
         set( BEGIN_ALL_LOAD)
         set( END_ALL_LOAD)
      else()
         set( BEGIN_ALL_LOAD "-Wl,--whole-archive")
         set( END_ALL_LOAD "-Wl,--no-whole-archive")
         set( FORCE_LOAD_PREFIX)
      endif()
   endif()

   function( CreateForceAllLoadList listname outputname)
      set( list ${BEGIN_ALL_LOAD})
      foreach( library ${${listname}})
         list( APPEND list "${FORCE_LOAD_PREFIX}${library}")
      endforeach()
      list( APPEND list ${END_ALL_LOAD})
      set( ${outputname} "${list}" PARENT_SCOPE)
   endfunction()

   include( AllLoadAuxC OPTIONAL)

endif()
