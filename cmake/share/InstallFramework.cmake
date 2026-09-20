### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# InstallFramework.cmake
#    Installs the framework targets into "Frameworks". Runs during
#    LINK_PHASE only.
#
# INPUT
#    INSTALL_FRAMEWORK_TARGETS  the targets to install, from Framework
#    LINK_PHASE                 nothing is installed without it
#    MULLE_TRACE_INCLUDE        trace inclusion of this file
#
# OUTPUT
#    none. Installs only.
#
# INCLUDES
#    PreInstallFramework (OPTIONAL), PostInstallFramework (OPTIONAL)
#
# GUARD
#    none, in theory this can be included multiple times
#

# This in theory can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


if( LINK_PHASE)

   include( PreInstallFramework OPTIONAL)

   install( TARGETS ${INSTALL_FRAMEWORK_TARGETS} DESTINATION "Frameworks")

   include( PostInstallFramework OPTIONAL)

endif()
