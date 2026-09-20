### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# InstallExecutable.cmake
#    Installs the executables into "bin", plus per target resources into
#    "share/<target>". Runs during LINK_PHASE only.
#
# INPUT
#    INSTALL_EXECUTABLE_TARGETS  the targets installed into "bin"
#    INSTALL_LIBRARY_TARGETS     the list actually iterated for resources,
#                                see the MEMO below
#    INSTALL_<IDENTIFIER>_RESOURCE_DIRS  per target resource directories
#    INSTALL_<IDENTIFIER>_RESOURCES      per target resource files
#    LINK_PHASE                  nothing is installed without it
#    MULLE_TRACE_INCLUDE         trace inclusion of this file
#
# OUTPUT
#    none. Installs only.
#
#    MEMO: the resource loop iterates INSTALL_LIBRARY_TARGETS, not
#          INSTALL_EXECUTABLE_TARGETS. For an executable only project that
#          list is empty, so executable resources are not installed here.
#          Documented as it stands, not changed.
#
# INCLUDES
#    PreInstallExecutable (OPTIONAL), StringCase,
#    PostInstallExecutable (OPTIONAL)
#
# GUARD
#    none, this can be included multiple times
#

# This can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)
### Install

   include( PreInstallExecutable OPTIONAL)
   include( StringCase)

   install( TARGETS ${INSTALL_EXECUTABLE_TARGETS} DESTINATION "bin")
   foreach( TMP_NAME ${INSTALL_LIBRARY_TARGETS})
      snakeCaseString( "${TMP_NAME}" TMP_IDENTIFIER)
      string( TOUPPER "${TMP_IDENTIFIER}" TMP_IDENTIFIER)

      # avoid empty share subdir
      # CMake can't do this...
      #if( (${INSTALL_${TMP_IDENTIFIER}_RESOURCE_DIRS}) OR (${INSTALL_${TMP_IDENTIFIER}_RESOURCES}))
      if( NOT "${INSTALL_${TMP_IDENTIFIER}_RESOURCE_DIRS}" STREQUAL "")
         install( DIRECTORY ${INSTALL_${TMP_IDENTIFIER}_RESOURCE_DIRS} DESTINATION "share/${TMP_NAME}")
      endif()

      if( NOT "${INSTALL_${TMP_IDENTIFIER}_RESOURCES}" STREQUAL "")
         install( FILES ${INSTALL_${TMP_IDENTIFIER}_RESOURCES} DESTINATION "share/${TMP_NAME}")
      endif()
   endforeach()


   include( PostInstallExecutable OPTIONAL)

endif()
