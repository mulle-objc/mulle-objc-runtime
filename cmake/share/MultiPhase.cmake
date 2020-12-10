### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

### MultiPhase
if( NOT __MULTI_PHASE__CMAKE__)
   set( __MULTI_PHASE__CMAKE__ ON)

   #
   # Parallel build support. run all "participating" projects once for
   # HEADERS_PHASE in parallel.
   # Now run all "participating" projects for COMPILE_PHASE in parallel.
   # Finally run all participating and non-participating projects in craftorder
   # serially together with the LINK_PHASE. What is tricky is that the
   # sequential projects may need to run first.
   #
   option( HEADERS_PHASE  "Install headers only phase (1)" OFF)
   option( COMPILE_PHASE  "Compile sources only phase (2)" OFF)
   option( LINK_PHASE     "Link and install only phase (3)" OFF)

   if( MULLE_MAKE_PHASE STREQUAL "HEADERS")
      set( HEADERS_PHASE ON)
   endif()
   if( MULLE_MAKE_PHASE STREQUAL "COMPILE")
      set( COMPILE_PHASE ON)
   endif()
   if( MULLE_MAKE_PHASE STREQUAL "LINK")
      set( LINK_PHASE ON)
   endif()

   if( NOT HEADERS_PHASE AND
       NOT COMPILE_PHASE AND
       NOT LINK_PHASE)
      set( HEADERS_PHASE ON)
      set( COMPILE_PHASE ON)
      set( LINK_PHASE ON)
   endif()
endif()
