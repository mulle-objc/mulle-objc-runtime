### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

### Files
if( NOT __FILES__CMAKE__)
   set( __FILES___CMAKE__ ON)

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

include( PreFiles OPTIONAL)

include( Headers OPTIONAL)
include( Sources OPTIONAL)
include( Resources OPTIONAL) 

include_directories( ${INCLUDE_DIRS})

#
# include files that get installed
#
if( EXISTS "${PROJECT_SOURCE_DIR}/cmake/DependenciesAndLibraries.cmake")
   list( APPEND CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/DependenciesAndLibraries.cmake")
else()
   list( APPEND CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/share/DependenciesAndLibraries.cmake")
endif()
if( EXISTS "${PROJECT_SOURCE_DIR}/cmake/_Dependencies.cmake")
   list( APPEND CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/_Dependencies.cmake")
else()
   list( APPEND CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/reflect/_Dependencies.cmake")
endif()
if( EXISTS "${PROJECT_SOURCE_DIR}/cmake/_Libraries.cmake")
   list( APPEND CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/_Libraries.cmake")
else()
   list( APPEND CMAKE_INCLUDES "${PROJECT_SOURCE_DIR}/cmake/reflect/_Libraries.cmake")
endif()

# IDE visible cmake files, Headers etc. are no longer there by default
if( EXISTS "${PROJECT_SOURCE_DIR}/CMakeLists.txt")
   list( APPEND CMAKE_EDITABLE_FILES "${PROJECT_SOURCE_DIR}/CMakeLists.txt")
endif()

FILE( GLOB TMP_CMAKE_FILES ${PROJECT_SOURCE_DIR}/cmake/*.cmake)
list( APPEND CMAKE_EDITABLE_FILES ${TMP_CMAKE_FILES})

include( PostFiles OPTIONAL)

set( PROJECT_FILES
   ${PROJECT_FILES}
   ${SOURCES}
   ${PUBLIC_HEADERS}
   ${PUBLIC_GENERIC_HEADERS}
   ${PUBLIC_GENERATED_HEADERS}
   ${PRIVATE_HEADERS}
   ${PRIVATE_GENERATED_HEADERS}
   ${CMAKE_EDITABLE_FILES}
   ${RESOURCES}
)

set( PROJECT_INSTALLABLE_HEADERS
   ${PUBLIC_HEADERS}
   ${PUBLIC_GENERIC_HEADERS}
   ${PUBLIC_GENERATED_HEADERS}
   ${PRIVATE_HEADERS}
)

endif()
