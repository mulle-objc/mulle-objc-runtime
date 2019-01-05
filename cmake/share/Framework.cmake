# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# FRAMEWORKS do not support three-phase build
#
if( NOT FRAMEWORK_NAME)
   set( FRAMEWORK_NAME "${PROJECT_NAME}")
endif()

if( NOT FRAMEWORK_FILES)
   set( FRAMEWORK_FILES "${PROJECT_FILES}")
endif()

include( PreFramework OPTIONAL)

if( NOT SOURCES)
   message( FATAL_ERROR "There are no sources to compile for framework ${FRAMEWORK_NAME}. Did mulle-sde update run yet ?")
endif()

add_library( "${FRAMEWORK_NAME}" SHARED
   ${FRAMEWORK_FILES}
)

include( FrameworkAux OPTIONAL)

if( NOT FRAMEWORK_LIBRARY_LIST)
   ${DEPENDENCY_LIBRARIES}
   ${OPTIONAL_DEPENDENCY_LIBRARIES}
   ${OS_SPECIFIC_LIBRARIES}
)

set( SHARED_LIBRARY_LIST ${FRAMEWORK_LIBRARY_LIST})

include( PostSharedLibrary OPTIONAL) # additional hook

target_link_libraries( "${FRAMEWORK_NAME}"
   ${SHARED_LIBRARY_LIST} # use SHARED_LIBRARY_LIST because of PostSharedLibrary
)

set( INSTALL_RESOURCES
  ${RESOURCES}
)

set_target_properties( "${FRAMEWORK_NAME}" PROPERTIES
  FRAMEWORK TRUE
  # FRAMEWORK_VERSION A
  # MACOSX_FRAMEWORK_IDENTIFIER <|PUBLISHER_REVERSE_DOMAIN|>.${LIBRARY_NAME}
  MACOSX_FRAMEWORK_INFO_PLIST ${PROJECT_SOURCE_DIR}/cmake/share/MacOSXFrameworkInfo.plist.in
  # headers must be part of LIBRARY_NAME target else it don't work
  PUBLIC_HEADER "${INSTALL_PUBLIC_HEADERS}"
  PRIVATE_HEADER "${INSTALL_PRIVATE_HEADERS}"
  RESOURCE "${INSTALL_RESOURCES}"
  # XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
)

message( STATUS "INSTALL_PUBLIC_HEADERS=${INSTALL_PUBLIC_HEADERS}")
message( STATUS "INSTALL_PRIVATE_HEADERS=${INSTALL_PRIVATE_HEADERS}")
message( STATUS "INSTALL_RESOURCES=${INSTALL_RESOURCES}")

set( INSTALL_FRAMEWORK_TARGETS
   "${FRAMEWORK_NAME}"
   ${INSTALL_FRAMEWORK_TARGETS}
)

include( PostFramework OPTIONAL)
