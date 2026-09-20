### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# MultiPhase.cmake
#    Turns the single MULLE_MAKE_PHASE value handed over by mulle-craft into
#    the three phase flags the other modules test. A build with no phase
#    given runs all three phases, which is what a plain cmake build wants.
#
# INPUT
#    MULLE_MAKE_PHASE     "HEADERS"/"HEADER", "COMPILE" or "LINK". Usually
#                         passed with -D by mulle-craft. Any other value,
#                         including unset, enables all three phases
#    MULLE_TRACE_INCLUDE  trace inclusion of this file
#
# OPTIONS
#    HEADER_PHASE         OFF   install headers only (phase 1)
#    COMPILE_PHASE        OFF   compile sources only (phase 2)
#    LINK_PHASE           OFF   link and install only (phase 3)
#
#    The defaults are only the starting point; the rules below can switch
#    them on. Because cmake caches them, a three phase build progresses from
#    ON OFF OFF to ON ON OFF to ON ON ON.
#
# OUTPUT
#    HEADER_PHASE         also implied by COMPILE_PHASE
#    COMPILE_PHASE        also implied by LINK_PHASE
#    LINK_PHASE
#    HEADERS_PHASE        legacy alias, set when HEADER_PHASE is on
#
#    So the flags are cumulative: LINK_PHASE alone results in all three
#    being on, and no phase at all also results in all three being on.
#
# GUARD
#    __MULTI_PHASE__CMAKE__
#

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

### MultiPhase
if( NOT __MULTI_PHASE__CMAKE__)
   set( __MULTI_PHASE__CMAKE__ ON)

   #
   # Parallel build support. run all "participating" projects once for
   # HEADER_PHASE in parallel.
   # Now run all "participating" projects for COMPILE_PHASE in parallel.
   # Finally run all participating and non-participating projects in craftorder
   # serially together with the LINK_PHASE. What is tricky is that the
   # sequential projects may need to run first.
   #
   #
   # MEMO: as cmake caches the ON flags, this will slowly progress
   #       from ON OFF OFF to ON ON OFF to ON ON ON during a three
   #       phase build
   #
   option( HEADER_PHASE  "Install headers only phase (1)" OFF)
   option( COMPILE_PHASE "Compile sources only phase (2)" OFF)
   option( LINK_PHASE    "Link and install only phase (3)" OFF)

   if( MULLE_MAKE_PHASE STREQUAL "HEADERS" OR MULLE_MAKE_PHASE STREQUAL "HEADER")
      set( HEADER_PHASE ON)
   endif()
   if( MULLE_MAKE_PHASE STREQUAL "COMPILE")
      set( COMPILE_PHASE ON)
   endif()
   if( MULLE_MAKE_PHASE STREQUAL "LINK")
      set( LINK_PHASE ON)
   endif()

   if( NOT HEADER_PHASE AND
       NOT COMPILE_PHASE AND
       NOT LINK_PHASE)
      set( HEADER_PHASE ON)
      set( COMPILE_PHASE ON)
      set( LINK_PHASE ON)
   endif()

   if( COMPILE_PHASE AND NOT HEADER_PHASE)
      set( HEADER_PHASE ON)
   endif()
   if( LINK_PHASE AND NOT COMPILE_PHASE)
      set( COMPILE_PHASE ON)
   endif()

   # backwards compatibility
   if( HEADER_PHASE)
      set( HEADERS_PHASE ON)
   endif()

   message( STATUS "HEADER_PHASE=${HEADER_PHASE}")
   message( STATUS "COMPILE_PHASE=${COMPILE_PHASE}")
   message( STATUS "LINK_PHASE=${LINK_PHASE}")
endif()
