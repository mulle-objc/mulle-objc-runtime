### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# FindPackageSupport.cmake
#    Produces the files an unrelated cmake project needs to consume this
#    library with find_package(): the exported targets file, the config file
#    and the version file. Also configures <library>-config.h.
#
#    The generated config file sets COLLECT_DEPENDENCY_LIBRARIES_AS_NAMES and
#    re-includes _Dependencies to turn this librarys dependencies into
#    find_dependency() calls, so a consumer resolves them by name rather
#    than by the absolute paths seen at build time.
#
# INPUT
#    LIBRARY_NAME              target to export, and the file name stem
#    PROJECT_NAME              used to find the two .in templates
#    PROJECT_VERSION           target VERSION and the package version file
#    PROJECT_VERSION_MAJOR     target SOVERSION, and SameMajorVersion
#                              compatibility
#    MULLE_TRACE_INCLUDE       trace inclusion of this file
#
#    Templates, each preferred from cmake/ and taken from cmake/share/ only
#    as a fallback:
#       cmake/${PROJECT_NAME}-config.cmake.in
#       cmake/${PROJECT_NAME}-config.h.in
#
# OUTPUT
#    CONFIG_CMAKE_IN_FILENAME  resolved template path
#    HEADER_CMAKE_IN_FILENAME  resolved template path
#
#    Installs: EXPORT ${LIBRARY_NAME}-targets with namespace
#    ${LIBRARY_NAME}::, the config and config-version files, all to
#    lib/cmake/${LIBRARY_NAME}. Configures ${LIBRARY_NAME}-config.h.
#
#    Side effect: target_include_directories( ${LIBRARY_NAME} PUBLIC) with
#    the INSTALL_INTERFACE include directory, and a BUILD_INTERFACE one only
#    when the project really ships an "include" directory.
#
# INCLUDES
#    CMakePackageConfigHelpers
#
# GUARD
#    none. Can be used once per library
#

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

# Only add a source-tree BUILD_INTERFACE include if the project actually ships
# an <include> directory. Most mulle projects keep headers under src/ and rely
# on AddSubdirectorySupport to expose the generated "${CMAKE_BINARY_DIR}/include",
# so pointing at a nonexistent source include/ would leak a phantom -I to
# add_subdirectory consumers.
if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include")
   target_include_directories( ${LIBRARY_NAME} PUBLIC
       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
endif()
#
# "include" is hardcoded, like every other install destination here. It is
# not CMAKE_INSTALL_INCLUDEDIR from GNUInstallDirs, because that module was
# never included, so the variable was empty and this expanded to an empty
# $<INSTALL_INTERFACE:>. GNUInstallDirs would compute "include" anyway, it
# has no platform special cases for the include directory, only for the
# library directory, where it would give us lib64 or lib/<arch> on some
# distributions and disagree with the hardcoded lib elsewhere.
#
target_include_directories( ${LIBRARY_NAME} PUBLIC
    $<INSTALL_INTERFACE:include>)

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

install( FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}-config-version.cmake"
  DESTINATION lib/cmake/${LIBRARY_NAME}
)
