### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# AddSubdirectorySupport.cmake
#    Decides what an add_subdirectory() consumer inherits from this library.
#    Two things, each with its own option:
#
#    Headers, two separate things:
#
#    1) staging: an embedded library copies its public headers into
#       "${CMAKE_BINARY_DIR}/include" during the build, so its consumer sees
#       the layout an installed dependency has. This is not a matter of
#       transitivity and does not depend on the option below; it is what an
#       embedded library offers.
#
#       Only a library that can be flattened stages anything. The copy keeps
#       the basename only, so an amalgamation - src/${LIBRARY_NAME} is a
#       directory of constituent directories and several constituents ship an
#       include.h - would map several headers onto one output file, and cmake
#       refuses to configure on the duplicate custom rule. Such a library hands
#       out its source directory instead.
#
#    2) the include directory the consumer is handed automatically, as
#       -isystem. That is what MULLE_TRANSITIVE_SOURCE_INCLUDE decides, and all
#       it decides: the source directory directly when src/${LIBRARY_NAME}
#       exists, otherwise the single staged "${CMAKE_BINARY_DIR}/include".
#       With the option off the consumer names the include roots it wants
#       itself; the staged headers are there to be named.
#
#    A top level build has no consumer, so it neither stages nor exports.
#
#    Libraries, when MULLE_TRANSITIVE_LINK_LIBRARIES is on: re-export those
#    dependencies that resolved to cmake targets, so that a plain
#    target_link_libraries( app PRIVATE ${LIBRARY_NAME}) suffices for the
#    consumer.
#
#    Both only ever do something when this project was pulled into another
#    one. In a mulle-craft build the dependencies are files rather than
#    targets and the headers come from DEPENDENCY_DIR, so nothing here
#    applies, which is why the projects CMakeLists.txt includes this module
#    only when USE_DEPENDENCY_DIR is off.
#
# INPUT
#    LIBRARY_NAME              the target to attach the include directory and
#                              the interface link libraries to
#    DEPENDENCY_LIBRARIES      and the FRAMEWORKS/OPTIONAL_/STARTUP_/
#                              OS_SPECIFIC_/ALL_LOAD_ variants, from
#                              Dependencies. Only the entries that are cmake
#                              targets are re-exported
#    CMAKE_SOURCE_DIR          compared against
#    CMAKE_CURRENT_SOURCE_DIR  ... to detect being embedded; only an embedded
#                              project stages headers or exports a directory
#    INSTALL_PUBLIC_HEADERS    copied into "${CMAKE_BINARY_DIR}/include"
#    INSTALL_PRIVATE_HEADERS   likewise
#    RESOLVE_INSTALLABLE_HEADER_SYMLINKS
#                              copy symlinked headers as files, for embedded
#                              projects that distribute their headers
#    MULLE_TRACE_INCLUDE       trace inclusion of this file
#
# OPTIONS
#    MULLE_TRANSITIVE_SOURCE_INCLUDE
#                        ON    hand the include directory to add_subdirectory
#                              consumers automatically, as -isystem: their src
#                              directory when src/${LIBRARY_NAME} exists, the
#                              staged ${CMAKE_BINARY_DIR}/include otherwise
#                        OFF   export no include directory; the consumer names
#                              the roots itself. Staging is unaffected
#    MULLE_TRANSITIVE_LINK_LIBRARIES
#                        ON    re-export dependencies that resolved to cmake
#                              targets through this librarys INTERFACE, so an
#                              add_subdirectory consumer inherits them.
#                              Dependencies that resolved to files are never
#                              re-exported, their absolute paths would leak
#                              into the consumers link line. All load
#                              dependencies keep their whole archive
#                              semantics, which needs cmake 3.24 or better
#
# OUTPUT
#    _MULLE_USE_TRANSITIVE_SOURCE_INCLUDE   the source directory is the one to
#                              hand out, i.e. src/${LIBRARY_NAME} exists
#    _MULLE_INTERFACE_INCLUDE   the include directory a consumer would be handed
#                              automatically, empty when there is none
#
#    Side effect: the custom commands that copy headers and the staging target
#    they belong to, target_include_directories( ${LIBRARY_NAME} INTERFACE ...)
#    when MULLE_TRANSITIVE_SOURCE_INCLUDE is on, plus
#    target_link_libraries( ${LIBRARY_NAME} INTERFACE ...) for the
#    dependencies that resolved to targets.
#
# GUARD
#    __ADD_SUBDIRECTORY_SUPPORT__CMAKE__
#

if( NOT __ADD_SUBDIRECTORY_SUPPORT__CMAKE__)
   set( __ADD_SUBDIRECTORY_SUPPORT__CMAKE__ ON)


   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   option( MULLE_TRANSITIVE_SOURCE_INCLUDE
           "Propagate source include directories to add_subdirectory consumers"
           ON)
   set( _MULLE_USE_TRANSITIVE_SOURCE_INCLUDE OFF)

   if( MULLE_TRANSITIVE_SOURCE_INCLUDE
       AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${LIBRARY_NAME}")
      set( _MULLE_USE_TRANSITIVE_SOURCE_INCLUDE ON)
   endif()

   #
   # Where our headers are, for a consumer. Two things, deliberately separate:
   #
   #  * the staging below happens for every embedded library that can be
   #    flattened, so that the consumer sees the layout an installed dependency
   #    has. Not a matter of transitivity, and not the business of the option.
   #  * the include directory handed to the consumer automatically, as -isystem,
   #    is what MULLE_TRANSITIVE_SOURCE_INCLUDE decides, and all it decides.
   #
   set( _MULLE_INTERFACE_INCLUDE)

   if( _MULLE_USE_TRANSITIVE_SOURCE_INCLUDE)
      set( _MULLE_INTERFACE_INCLUDE
           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>)
   elseif( NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR
           AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/${LIBRARY_NAME}")

      list( APPEND INSTALL_TMP_HEADERS ${INSTALL_PUBLIC_HEADERS} ${INSTALL_PRIVATE_HEADERS})

      #
      # we copy our headers into a build directory, so we don't have to gum
      # up our project with a root /include, that duplicates the headers (as
      # we did in the code below)
      #
      if( INSTALL_TMP_HEADERS)
         unset( HEADER_OUTPUTS)
         
         foreach( HEADER ${INSTALL_TMP_HEADERS})
             get_filename_component( HEADER_FILENAME ${HEADER} NAME)
             set( OUTPUT_FILE "${CMAKE_BINARY_DIR}/include/${LIBRARY_NAME}/${HEADER_FILENAME}")
             list( APPEND HEADER_OUTPUTS ${OUTPUT_FILE})

             add_custom_command(
                 OUTPUT ${OUTPUT_FILE}
                 COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/include/${LIBRARY_NAME}"
                 COMMAND ${CMAKE_COMMAND} -E copy_if_different ${HEADER} "${OUTPUT_FILE}"
                 DEPENDS ${HEADER}
                 WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                 COMMENT "Copying ${HEADER} to ${OUTPUT_FILE}"
             )
         endforeach()

         add_custom_target( ${LIBRARY_NAME}-headers ALL DEPENDS ${HEADER_OUTPUTS})
         add_dependencies( ${LIBRARY_NAME} ${LIBRARY_NAME}-headers)
         set_property( GLOBAL APPEND PROPERTY
                       MULLE_HEADER_STAGING_TARGETS
                       "${LIBRARY_NAME}")

         set( _MULLE_INTERFACE_INCLUDE
              $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
              $<INSTALL_INTERFACE:include>)
      endif()

   endif()

   #
   # Hand the include directory to the consumer, or don't. With the option off
   # the consumer names the roots it wants itself - the staged headers above are
   # there to be named.
   # MEMO: not doing this anymore to no ill effect so far
   # if( MULLE_TRANSITIVE_SOURCE_INCLUDE AND _MULLE_INTERFACE_INCLUDE)
   #    target_include_directories( "${LIBRARY_NAME}" INTERFACE
   #       ${_MULLE_INTERFACE_INCLUDE})
   # endif()

   #
   # We deliberately do NOT export one INTERFACE include directory per
   # constituent here. The single amalgamated directory above mirrors the
   # layout an installed dependency has. Exporting the source directories
   # too would add dozens of -I options per consumer and would leak our
   # source layout.
   #

   #
   # INTERFACE propagation for add_subdirectory consumers.
   #
   # In that world our dependencies resolve to cmake targets, and they have
   # to be exported transitively so that a simple
   # target_link_libraries( app PRIVATE <library>) suffices.
   #
   # In a mulle-craft build the dependencies are files, not targets, and
   # are already linked in via PRIVATE above. They must not be re-exported:
   # absolute paths would leak into consumer link lines.
   #
   # Ordinary dependencies can be propagated as target names. All-load
   # dependencies are handled separately below because reducing them to
   # ordinary target names loses their whole-archive semantics.
   option( MULLE_TRANSITIVE_LINK_LIBRARIES
           "Propagate dependency libraries through target interfaces"
           ON)
   if( MULLE_TRANSITIVE_LINK_LIBRARIES)
      set( _INTERFACE_LIBS )
      foreach( _item
         ${DEPENDENCY_LIBRARIES}
         ${DEPENDENCY_FRAMEWORKS}
         ${OPTIONAL_DEPENDENCY_LIBRARIES}
         ${OPTIONAL_DEPENDENCY_FRAMEWORKS}
         ${STARTUP_DEPENDENCY_LIBRARIES}
         ${STARTUP_DEPENDENCY_FRAMEWORKS}
         ${OS_SPECIFIC_LIBRARIES}
         ${OS_SPECIFIC_FRAMEWORKS}
      )
         if( TARGET "${_item}")
            list( APPEND _INTERFACE_LIBS "${_item}")
         endif()
      endforeach()
      unset( _item)

      if( _INTERFACE_LIBS)
         target_link_libraries( "${LIBRARY_NAME}" INTERFACE ${_INTERFACE_LIBS})
      endif()
      unset( _INTERFACE_LIBS)

      # Keep all-load semantics for add_subdirectory consumers. The raw
      # ALL_LOAD_* variables contain library names/targets; the FORCE_* lists
      # are platform-specific linker fragments produced by AllLoadC and must
      # not be propagated as ordinary CMake targets.
      set( _INTERFACE_ALL_LOAD_LIBS )
      if( CMAKE_VERSION VERSION_GREATER_EQUAL "3.24")
         foreach( _item
            ${ALL_LOAD_DEPENDENCY_LIBRARIES}
            ${ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES}
            ${STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
         )
            if( TARGET "${_item}")
               list( APPEND _INTERFACE_ALL_LOAD_LIBS
                  "$<LINK_LIBRARY:WHOLE_ARCHIVE,${_item}>"
               )
            endif()
         endforeach()
         unset( _item)

         if( _INTERFACE_ALL_LOAD_LIBS)
            target_link_libraries( "${LIBRARY_NAME}" INTERFACE
               ${_INTERFACE_ALL_LOAD_LIBS}
            )
         endif()
      endif()
      unset( _INTERFACE_ALL_LOAD_LIBS)
   endif()
endif()


#   function( unprotect_cmake_include_directory dir)
#       if( NOT EXISTS "${dir}")
#          file( MAKE_DIRECTORY "${dir}")
#       else()
#          if( WIN32)
#              execute_process(COMMAND attrib -R "${dir}\\*.*" /S)
#          else()
#              execute_process(COMMAND chmod -R ugo+w "${dir}")
#          endif()
#       endif()
#   endfunction()
#
#   function( protect_cmake_include_directory dir)
#       if( WIN32)
#           execute_process(COMMAND attrib +R "${dir}\\*.*" /S)
#       else()
#           execute_process(COMMAND chmod -R ugo-w "${dir}")
#       endif()
#   endfunction()
#
#   #
#   # This can run at anytime really, targets must not rely on these headers
#   #
#   if( INSTALL_CMAKE_INCLUDE_ENABLED)
#      set( SOURCE_ROOT_INCLUDE_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/include")
#      set( SOURCE_PUBLIC_INCLUDE_DIR  "${SOURCE_ROOT_INCLUDE_DIR}/${PROJECT_NAME}")
#      set( SOURCE_PRIVATE_INCLUDE_DIR "${SOURCE_ROOT_INCLUDE_DIR}/${PROJECT_NAME}")
##
## noone's gonna look there anyway
##      set( SOURCE_CMAKE_INCLUDE_DIR   "${SOURCE_ROOT_INCLUDE_DIR}/${PROJECT_NAME}/cmake")
#
#      unprotect_cmake_include_directory( "${SOURCE_ROOT_INCLUDE_DIR}")
#
#      file( MAKE_DIRECTORY "${SOURCE_PUBLIC_INCLUDE_DIR}")
#      file( MAKE_DIRECTORY "${SOURCE_PRIVATE_INCLUDE_DIR}")
##      file( MAKE_DIRECTORY "${SOURCE_CMAKE_INCLUDE_DIR}")
#
#      # Copy public headers
#      foreach( HEADER ${INSTALL_PUBLIC_HEADERS})
#          file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}"
#               DESTINATION "${SOURCE_PUBLIC_INCLUDE_DIR}")
#      endforeach()
#
#      # Copy private headers
#      foreach( HEADER ${INSTALL_PRIVATE_HEADERS})
#          file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}"
#               DESTINATION "${SOURCE_PRIVATE_INCLUDE_DIR}")
#      endforeach()
##
##      # Copy cmake headers
##      foreach( HEADER ${INSTALL_CMAKE_INCLUDES})
##          file( COPY "${CMAKE_CURRENT_SOURCE_DIR}/${HEADER}"
##                DESTINATION "${SOURCE_CMAKE_INCLUDE_DIR}")
##      endforeach()
#
#      protect_cmake_include_directory( "${SOURCE_ROOT_INCLUDE_DIR}")
#   endif()
