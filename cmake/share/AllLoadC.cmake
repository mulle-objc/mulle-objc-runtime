### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

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
   # link statements. Sometimes it seemed as if Apple needed
   # -lx -force_load x though. ?
   #
   # First, try to detect if we're using `lld`
   if( APPLE)
      set( FORCE_LOAD_PREFIX "-force_load ")
      set( BEGIN_ALL_LOAD)
      set( END_ALL_LOAD)
   else()
      if( MSVC)  # not WIN32, currently we cross compile with the default clang
         set( FORCE_LOAD_PREFIX "-WHOLEARCHIVE:")
         set( BEGIN_ALL_LOAD)
         set( END_ALL_LOAD)
      else()
         if( CMAKE_LINKER_ID STREQUAL "GNU" OR CMAKE_LINKER_ID STREQUAL "Gold" OR CMAKE_LINKER_ID STREQUAL "Solaris")
            message(STATUS "Using a linker that supports --no-as-needed, adding the flag.")
            set( BEGIN_ALL_LOAD "-Wl,--whole-archive -Wl,--no-as-needed")
            set( END_ALL_LOAD "-Wl,--as-needed -Wl,--no-whole-archive")
         else()
            set( BEGIN_ALL_LOAD "-Wl,--whole-archive")
            set( END_ALL_LOAD "-Wl,--no-whole-archive")
         endif()
         set( FORCE_LOAD_PREFIX)
      endif()
   endif()


   #
   # For APPLE we mention the library twice. That's because if it happens to
   # be a shared library, for some reason, that cmake still picks it up
   # to generate an RPATH. 
   #
   # ALL_LOAD_PREFIX can be set to "-Xlinker -reexport_library " and then 
   # symbols will be reexported. 
   #
   function( CreateForceAllLoadList listname outputname)
      set( list "")
      if( ${listname})
         set( list ${BEGIN_ALL_LOAD})
         foreach( library ${${listname}})
            if( APPLE)
               list( APPEND list "${ALL_LOAD_PREFIX}${library}")
               # if FORCE_LOAD_PREFIX is empty, we can skip the output
               # which is handy sometimes when we have dynamic frameworks
               # (hacque). But only on APPLE, where we emitted something
               # already...
               if( FORCE_LOAD_PREFIX)
                  list( APPEND list "${FORCE_LOAD_PREFIX}${library}")
               endif()
            else()
               list( APPEND list "${FORCE_LOAD_PREFIX}${library}")
            endif()
         endforeach()
         list( APPEND list ${END_ALL_LOAD})
      endif()
      set( ${outputname} "${list}" PARENT_SCOPE)
   endfunction()

   include( AllLoadAuxC OPTIONAL)

endif()
