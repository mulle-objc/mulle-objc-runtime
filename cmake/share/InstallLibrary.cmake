### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


option( INSTALL_PROJECT_ASSET_DIR "Copy contents of project root \"asset\" directory to \"share\" on install" ON)

if( INSTALL_PROJECT_ASSET_DIR)
   if( NOT ASSET_DIR)
      set( ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/asset")
   endif()
   if( NOT EXISTS "${ASSET_DIR}")
      message( STATUS "No \"asset\" directory found at \"${CMAKE_CURRENT_SOURCE_DIR}/asset\"")
      unset( ASSET_DIR)
   endif()
else()
   unset( ASSET_DIR)
endif()


if( LINK_PHASE)
   include( PreInstallLibrary OPTIONAL)

   include( StringCase)

   #
   # Use explicit per-artifact destinations so CMake handles all platforms
   # correctly:
   #   RUNTIME = DLLs on Windows  -> bin
   #   LIBRARY = shared libs on Unix -> lib
   #   ARCHIVE = static libs + import libs -> lib
   #
   install( TARGETS ${INSTALL_LIBRARY_TARGETS}
            RUNTIME DESTINATION "bin"
            LIBRARY DESTINATION "lib"
            ARCHIVE DESTINATION "lib")

   foreach( TMP_NAME ${INSTALL_LIBRARY_TARGETS})
      snakeCaseString( "${TMP_NAME}" TMP_IDENTIFIER)
      string( TOUPPER "${TMP_IDENTIFIER}" TMP_IDENTIFIER)

      # copy contents of root asset folder there
      # is is hack or covenient we will see, is this good if we have
      # multiple libraries, i neve have
      if( NOT "${ASSET_DIR}" STREQUAL "")
         install( DIRECTORY "${ASSET_DIR}/" DESTINATION "share/${TMP_NAME}" USE_SOURCE_PERMISSIONS)
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
