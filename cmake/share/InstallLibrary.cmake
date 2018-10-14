# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)

   include( PreInstallLibrary OPTIONAL)

   install( TARGETS ${INSTALL_LIBRARY_TARGETS} DESTINATION "lib")

   include( PreInstallLibrary OPTIONAL)

endif()
