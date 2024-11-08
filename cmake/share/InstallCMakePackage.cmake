### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# could conceivably be used multiple times for each library

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

set( CONFIG_CMAKE_IN_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in")
if( NOT EXISTS "${CONFIG_CMAKE_IN_FILENAME}")
   set( CONFIG_CMAKE_IN_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/share/${PROJECT_NAME}-config.cmake.in")
endif()

set( HEADER_CMAKE_IN_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.h.in")
if( NOT EXISTS "${HEADER_CMAKE_IN_FILENAME}")
   set( HEADER_CMAKE_IN_FILENAME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/share/${PROJECT_NAME}-config.h.in")
endif()


# can do this multiple times
install( TARGETS ${LIBRARY_NAME}
         EXPORT ${LIBRARY_NAME}-targets)

target_include_directories( ${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# Set target properties
set_target_properties( ${LIBRARY_NAME} PROPERTIES
                       VERSION ${PROJECT_VERSION}
                       SOVERSION ${PROJECT_VERSION_MAJOR}
                       EXPORT_NAME ${LIBRARY_NAME})

# Create and install config files
include( CMakePackageConfigHelpers)

configure_package_config_file(
    "${CONFIG_CMAKE_IN_FILENAME}"
    "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}-config.cmake"
    INSTALL_DESTINATION lib/cmake/${LIBRARY_NAME})

configure_file( "${HEADER_CMAKE_IN_FILENAME}" "${LIBRARY_NAME}-config.h")

install( EXPORT ${LIBRARY_NAME}-targets
         FILE ${LIBRARY_NAME}-targets.cmake
         NAMESPACE ${LIBRARY_NAME}::
         DESTINATION lib/cmake/${LIBRARY_NAME})

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}-config-version.cmake"
  DESTINATION lib/cmake/${LIBRARY_NAME}
)
