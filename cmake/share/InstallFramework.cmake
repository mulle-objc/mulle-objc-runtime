# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)

   include( PreInstallFramework OPTIONAL)

   install( TARGETS ${INSTALL_FRAMEWORK_TARGETS} DESTINATION "Frameworks")

   include( PreInstallFramework OPTIONAL)

endif()
