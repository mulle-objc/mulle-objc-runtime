### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# AllLoadSupport.cmake
#    Exports the force-link requirement of an initialization library to
#    consumers that link it, both as an add_subdirectory() subproject and as
#    an installed find_package() dependency.
#
#    This library is force-linkable: it contains atinit/atexit constructors
#    that must end up in the consumer binary even when no symbol of the
#    library is referenced directly. A plain
#
#       target_link_libraries( app PRIVATE ${LIBRARY_NAME})
#
#    would drop those objects from the static archive.
#
#    The requirement is carried by a separate INTERFACE wrapper target,
#    because a target can not whole-archive itself: CMake strips a
#    self-reference from INTERFACE_LINK_LIBRARIES, and
#
#       $<LINK_LIBRARY:...>
#
#    is only valid in link library positions, not in INTERFACE_LINK_OPTIONS.
#    The wrapper removes both problems, needs no per-platform linker flags and
#    survives export.
#
#    BUILD_INTERFACE/INSTALL_INTERFACE carry the build-tree and the namespaced
#    install-tree spelling: CMake does not namespace the target name inside
#    $<LINK_LIBRARY:...> when exporting, so it has to be written out.
#
#    This file is generic and is meant for the shared layer (the mulle-sde
#    template). Anything specific to the sources a project compiles does not
#    belong here.
#
# INPUT
#    LIBRARY_LINK_TARGET        the library target to wrap
#    LIBRARY_NAME               export namespace, defaults to the target name
#    BUILD_SHARED_LIBS          force-linking is skipped for shared libraries
#    MULLE_LIBRARY_FORCE_LINK   ON    force-link this library into consumers.
#                                     Declared with its explanation in the
#                                     project's CMakeLists.txt. Defaults to ON
#                                     here as well, so including the module is
#                                     enough.
#    MULLE_TRANSITIVE_LINKER_FLAGS
#                               ON    create the wrapper
#    MULLE_TRANSITIVE_LINK_LIBRARIES
#                               fallback default for the above, so consumers
#                               that disabled the former also opt out
#    MULLE_TRACE_INCLUDE        trace inclusion of this file
#
# OUTPUT
#    Creates ${LIBRARY_LINK_TARGET}-force INTERFACE target, links it into
#    ${LIBRARY_LINK_TARGET} and adds it to the ${LIBRARY_NAME}-targets export.
#    Sets MULLE_ALL_LOAD_ACTIVE to ON when the wrapper was created, so modules
#    that must react to whole-archiving (for example StacktraceSupport) can
#    key off it.
#
# REQUIRES
#    CMake 3.24, for $<LINK_LIBRARY:WHOLE_ARCHIVE>. Below that the module is a
#    silent no-op, so it can live in a template that older projects include.
#
# GUARD
#    __ALL_LOAD_SUPPORT__CMAKE__
#

if( NOT __ALL_LOAD_SUPPORT__CMAKE__)
   set( __ALL_LOAD_SUPPORT__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # Force-linking is on by default, because the mulle projects that use this
   # module so far are Objective-C libraries with atinit/atexit constructors.
   # A plain C library that has no such constructors declares this option with
   # the default OFF in its CMakeLists.txt (or passes
   # -DMULLE_LIBRARY_FORCE_LINK=OFF), otherwise its consumers whole-archive it
   # needlessly.
   #
   option( MULLE_LIBRARY_FORCE_LINK
           "Force-link this library into consumers"
           ON)

   #
   # Dedicated switch. When it was not set explicitly, inherit the old
   # transitive link flag so an existing consumer that disabled that one keeps
   # getting the old (no force-link) behaviour.
   #
   if( NOT DEFINED MULLE_TRANSITIVE_LINKER_FLAGS)
      if( DEFINED MULLE_TRANSITIVE_LINK_LIBRARIES)
         set( _MULLE_ALL_LOAD_DEFAULT "${MULLE_TRANSITIVE_LINK_LIBRARIES}")
      else()
         set( _MULLE_ALL_LOAD_DEFAULT ON)
      endif()
      option( MULLE_TRANSITIVE_LINKER_FLAGS
              "Propagate force-link options to consumers"
              "${_MULLE_ALL_LOAD_DEFAULT}")
      unset( _MULLE_ALL_LOAD_DEFAULT)
   endif()

   set( MULLE_ALL_LOAD_ACTIVE OFF)

   if( MULLE_LIBRARY_FORCE_LINK AND MULLE_TRANSITIVE_LINKER_FLAGS AND NOT BUILD_SHARED_LIBS)
      if( CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
         if( NOT LIBRARY_NAME)
            set( LIBRARY_NAME "${LIBRARY_LINK_TARGET}")
         endif()

         set( _MULLE_ALL_LOAD_WRAPPER "${LIBRARY_LINK_TARGET}-force")

         add_library( "${_MULLE_ALL_LOAD_WRAPPER}" INTERFACE)
         target_link_libraries( "${_MULLE_ALL_LOAD_WRAPPER}" INTERFACE
            "$<BUILD_INTERFACE:$<LINK_LIBRARY:WHOLE_ARCHIVE,${LIBRARY_LINK_TARGET}>>"
            "$<INSTALL_INTERFACE:$<LINK_LIBRARY:WHOLE_ARCHIVE,${LIBRARY_NAME}::${LIBRARY_NAME}>>"
         )
         target_link_libraries( "${LIBRARY_LINK_TARGET}" INTERFACE
            "${_MULLE_ALL_LOAD_WRAPPER}")

         #
         # FindPackageSupport exports ${LIBRARY_NAME} as ${LIBRARY_NAME}-targets
         # with the ${LIBRARY_NAME}:: namespace. Add the wrapper to the same
         # export set. Install rule order does not matter: the export set is
         # collected at generate time from every install( TARGETS ... EXPORT).
         #
         install( TARGETS "${_MULLE_ALL_LOAD_WRAPPER}"
                  EXPORT "${LIBRARY_NAME}-targets")

         unset( _MULLE_ALL_LOAD_WRAPPER)
         set( MULLE_ALL_LOAD_ACTIVE ON)
      else()
         message( STATUS "AllLoadSupport: CMake ${CMAKE_VERSION} < 3.24, "
                         "force-link is not propagated")
      endif()
   endif()
endif()
