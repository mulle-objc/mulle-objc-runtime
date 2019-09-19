# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)

   include( PreInstallLibrary OPTIONAL)

   install( TARGETS ${INSTALL_LIBRARY_TARGETS} DESTINATION "lib")
   foreach( TMP_NAME ${INSTALL_LIBRARY_TARGETS})
      install( FILES ${INSTALL_${TMP_NAME}_RESOURCES} DESTINATION "share/${TMP_NAME}")
      install( DIRECTORY ${INSTALL_${TMP_NAME}_RESOURCE_DIRS} DESTINATION "share/${TMP_NAME}")
   endforeach()

   include( PreInstallLibrary OPTIONAL)

endif()
