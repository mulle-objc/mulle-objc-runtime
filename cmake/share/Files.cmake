### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

### Files
if( NOT __FILES__CMAKE__)
   set( __FILES___CMAKE__ ON)

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

message( STATUS "PROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\"")

include( PreFiles OPTIONAL)

include( Headers OPTIONAL)
include( Sources OPTIONAL)
include( Resources OPTIONAL) 


#
# PROJECT_FILES (GUI Support)
#
if( NOT DEFINED CACHE{INSTALL_CMAKE_INCLUDES})
   if( EXISTS "${PROJECT_SOURCE_DIR}/cmake/Definitions.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/Definitions.cmake")
   endif()
   if( EXISTS "${PROJECT_SOURCE_DIR}/cmake/DependenciesAndLibraries.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/DependenciesAndLibraries.cmake")
   else()
      list( APPEND INSTALL_CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/share/DependenciesAndLibraries.cmake")
   endif()
   if( EXISTS "${PROJECT_SOURCE_DIR}/cmake/_Dependencies.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/_Dependencies.cmake")
   else()
      list( APPEND INSTALL_CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/reflect/_Dependencies.cmake")
   endif()
   if( EXISTS "${PROJECT_SOURCE_DIR}/cmake/_Libraries.cmake")
      list( APPEND INSTALL_CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/_Libraries.cmake")
   else()
      list( APPEND INSTALL_CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/reflect/_Libraries.cmake")
   endif()
   set( INSTALL_CMAKE_INCLUDES ${INSTALL_CMAKE_INCLUDES} CACHE INTERNAL "cache these")
endif()


if( NOT DEFINED CACHE{PROJCT_CMAKE_EDITABLE_FILES})

   # IDE visible cmake files, Headers etc. are no longer there by default
   if( EXISTS "${PROJECT_SOURCE_DIR}/CMakeLists.txt")
      list( APPEND PROJCT_CMAKE_EDITABLE_FILES "${PROJECT_SOURCE_DIR}/CMakeLists.txt")
   endif()

   FILE( GLOB TMP_CMAKE_FILES ${PROJECT_SOURCE_DIR}/cmake/*.cmake)
   list( APPEND PROJCT_CMAKE_EDITABLE_FILES ${TMP_CMAKE_FILES})
   set( PROJCT_CMAKE_EDITABLE_FILES ${PROJCT_CMAKE_EDITABLE_FILES} CACHE INTERNAL "cache these")
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
   ${PROJCT_CMAKE_EDITABLE_FILES}
   ${RESOURCES}
)

include_directories( ${INCLUDE_DIRS})

endif()
