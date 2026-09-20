### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# Files.cmake
#    Pulls in the generated file lists (Headers, Sources, Resources) and
#    turns them into the PROJECT_FILES list an IDE displays. Also applies
#    the projects own include directories.
#
# INPUT
#    MULLE_SDE_REFLECT_DIR    reflect directory name, defaults to "reflect"
#    INCLUDE_DIRS             the projects own include directories
#    SOURCES                  from the included Sources
#    PUBLIC_HEADERS           from the included Headers
#    PUBLIC_GENERIC_HEADERS   likewise
#    PUBLIC_GENERATED_HEADERS likewise
#    PRIVATE_HEADERS          likewise
#    PRIVATE_GENERATED_HEADERS likewise
#    RESOURCES                from the included Resources
#    PROJECT_FILES            appended to, so it may be preset
#    MULLE_TRACE_INCLUDE      trace inclusion of this file
#
# OUTPUT
#    MULLE_SDE_REFLECT_DIR         set to "reflect" if it was unset
#    INSTALL_CMAKE_INCLUDES        CACHE INTERNAL. The cmake files this
#                                  project ships so that consumers can
#                                  inherit its dependency definitions
#    PROJECT_CMAKE_EDITABLE_FILES  CACHE INTERNAL. CMakeLists.txt plus
#                                  cmake/*.cmake, for IDE visibility
#    PROJECT_FILES                 everything above combined
#
#    Side effect: include_directories( ${INCLUDE_DIRS}).
#    Indirectly also whatever Headers, Sources and Resources set, notably
#    INSTALL_PUBLIC_HEADERS and INSTALL_PRIVATE_HEADERS.
#
# INCLUDES
#    PreFiles (OPTIONAL), Headers (OPTIONAL), Sources (OPTIONAL),
#    Resources (OPTIONAL), PostFiles (OPTIONAL)
#
# GUARD
#    __FILES__CMAKE__
#

### Files
if( NOT __FILES__CMAKE__)
   set( __FILES__CMAKE__ ON)

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

message( STATUS "CMAKE_CURRENT_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\"")

include( PreFiles OPTIONAL)

include( Headers OPTIONAL)
include( Sources OPTIONAL)
include( Resources OPTIONAL) 


#
# PROJECT_FILES (GUI Support)
#
if( NOT DEFINED CACHE{INSTALL_CMAKE_INCLUDES})
   if( NOT MULLE_SDE_REFLECT_DIR)
      set( MULLE_SDE_REFLECT_DIR "reflect")
   endif()
   if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Definitions.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "cmake/Definitions.cmake")
   endif()
   if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DependenciesAndLibraries.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "cmake/DependenciesAndLibraries.cmake")
   else()
      list( APPEND INSTALL_CMAKE_INCLUDES "cmake/share/DependenciesAndLibraries.cmake")
   endif()
   if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/_Dependencies.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "cmake/_Dependencies.cmake")
   elseif( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${MULLE_SDE_REFLECT_DIR}/_Dependencies.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "cmake/${MULLE_SDE_REFLECT_DIR}/_Dependencies.cmake")
   endif()
   if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/_Libraries.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "cmake/_Libraries.cmake")
   elseif( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${MULLE_SDE_REFLECT_DIR}/_Libraries.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "cmake/${MULLE_SDE_REFLECT_DIR}/_Libraries.cmake")
   endif()
   set( INSTALL_CMAKE_INCLUDES ${INSTALL_CMAKE_INCLUDES} CACHE INTERNAL "cache these")
endif()


if( NOT DEFINED CACHE{PROJECT_CMAKE_EDITABLE_FILES})

   # IDE visible cmake files, Headers etc. are no longer there by default
   if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
      list( APPEND PROJECT_CMAKE_EDITABLE_FILES "CMakeLists.txt")
   endif()

   FILE( GLOB TMP_CMAKE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake)
   list( APPEND PROJECT_CMAKE_EDITABLE_FILES ${TMP_CMAKE_FILES})
   set( PROJECT_CMAKE_EDITABLE_FILES ${PROJECT_CMAKE_EDITABLE_FILES} CACHE INTERNAL "cache these")
endif()

include( PostFiles OPTIONAL)

set( PROJECT_FILES
   ${PROJECT_FILES}
   ${SOURCES}
   ${PUBLIC_HEADERS}
   ${PUBLIC_GENERIC_HEADERS}
   ${PUBLIC_GENERATED_HEADERS}
   ${PRIVATE_HEADERS}
   ${PRIVATE_GENERATED_HEADERS}
   ${PROJECT_CMAKE_EDITABLE_FILES}
   ${RESOURCES}
)

include_directories( ${INCLUDE_DIRS})

endif()
