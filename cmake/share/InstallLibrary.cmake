### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)
   include( PreInstallLibrary OPTIONAL)

   install( TARGETS ${INSTALL_LIBRARY_TARGETS} DESTINATION "lib")
   foreach( TMP_NAME ${INSTALL_LIBRARY_TARGETS})
      string( MAKE_C_IDENTIFIER "${TMP_NAME}" TMP_IDENTIFIER)
      string( TOUPPER "${TMP_IDENTIFIER}" TMP_IDENTIFIER)
      # avoid empty share subdir
      if( (${INSTALL_${TMP_IDENTIFIER}_RESOURCE_DIRS}) OR (${INSTALL_${TMP_IDENTIFIER}_RESOURCES}))
         install( DIRECTORY ${INSTALL_${TMP_IDENTIFIER}_RESOURCE_DIRS} DESTINATION "share/${TMP_NAME}")
         install( FILES ${INSTALL_${TMP_IDENTIFIER}_RESOURCES} DESTINATION "share/${TMP_NAME}")
      endif()
   endforeach()

   include( PostInstallLibrary OPTIONAL)

endif()


# extension : mulle-sde/cmake
# directory : project/all
# template  : .../InstallLibrary.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
