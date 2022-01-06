### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)

   include( PreInstallFramework OPTIONAL)

   install( TARGETS ${INSTALL_FRAMEWORK_TARGETS} DESTINATION "Frameworks")

   include( PostInstallFramework OPTIONAL)

endif()


# extension : mulle-sde/cmake
# directory : project/all
# template  : .../InstallFramework.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
