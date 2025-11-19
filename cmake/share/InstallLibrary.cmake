### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


option( INSTALL_PROJECT_ASSETS_DIR "Copy contents of project root \"assets\" directory to \"share\" on install" ON)

if( INSTALL_PROJECT_ASSETS_DIR)
   if( NOT ASSETS_DIR)
      set( ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
   endif()
   if( NOT EXISTS "${ASSETS_DIR}")
      message( STATUS "No \"assets\" directory found at \"${CMAKE_CURRENT_SOURCE_DIR}/assets\"")
      unset( ASSETS_DIR)
   endif()
else()
   unset( ASSETS_DIR)
endif()


if( LINK_PHASE)
   include( PreInstallLibrary OPTIONAL)

   include( StringCase)

   install( TARGETS ${INSTALL_LIBRARY_TARGETS} DESTINATION "lib")

   foreach( TMP_NAME ${INSTALL_LIBRARY_TARGETS})
      snakeCaseString( "${TMP_NAME}" TMP_IDENTIFIER)
      string( TOUPPER "${TMP_IDENTIFIER}" TMP_IDENTIFIER)

      # copy contents of root assets folder there
      # is is hack or covenient we will see, is this good if we have
      # multiple libraries, i neve have
      if( NOT "${ASSETS_DIR}" STREQUAL "")
         install( DIRECTORY "${ASSETS_DIR}/" DESTINATION "share/${TMP_NAME}" USE_SOURCE_PERMISSIONS)
      endif()

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

   include( PostInstallLibrary OPTIONAL)

endif()
