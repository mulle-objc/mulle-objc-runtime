### Files
if( NOT __FILES__CMAKE__)
   set( __FILES___CMAKE__ ON)

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

include( PreFiles OPTIONAL)

include( "cmake/Headers.cmake" OPTIONAL)
include( "cmake/Sources.cmake" OPTIONAL)

include_directories( ${INCLUDE_DIRS})

include( PostFiles OPTIONAL)

set( PROJECT_FILES
   ${PROJECT_FILES}
   ${SOURCES}
   ${PUBLIC_HEADERS}
   ${PRIVATE_HEADERS}
   ${CMAKE_EDITABLE_FILES}
)

set( PROJECT_INSTALLABLE_HEADERS
   ${PUBLIC_HEADERS}
   ${PRIVATE_HEADERS}
)

#
# remove files from RESOURCES that are inside RESOURCE_DIRS
#
if( RESOURCE_DIRS AND RESOURCES)
   set( TMP_RESOURCES ${RESOURCES})
   foreach( TMP_NAME ${RESOURCES})
      foreach( TMP_DIR_NAME ${RESOURCE_DIRS})
         string( FIND "${TMP_NAME}" "${TMP_DIR_NAME}" TMP_POSITION)
         if( TMP_POSITION EQUAL 0)
            list( REMOVE_ITEM TMP_RESOURCES "${TMP_NAME}")
         endif()
      endforeach()
   endforeach()
   set( ${RESOURCES} ${TMP_RESOURCES})
endif()

endif()
