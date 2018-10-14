# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)



### Install

   include( PreInstallExecutable OPTIONAL)

   install( TARGETS ${INSTALL_EXECUTABLE_TARGETS} DESTINATION "bin")

   include( PostInstallExecutable OPTIONAL)

endif()
