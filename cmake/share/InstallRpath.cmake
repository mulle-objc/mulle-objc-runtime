### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# This can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# https://stackoverflow.com/questions/32469953/why-is-cmake-designed-so-that-it-removes-runtime-path-when-installing/32470070#32470070
#
# MULLE_NO_CMAKE_INSTALL_RPATH can be used to kill this codepath
# This is not used during a regular craft, but only when "install" is run.
# One could enable CMAKE_INSTALL_RPATH for build by setting
# set( CMAKE_BUILD_WITH_INSTALL_RPATH ON)
# but this is usually counterproductive, since lib will be really in
# ../../dependency/lib
#
# CMAKE_INSTALL_RPATH must be defined before add_executable or add_library
#
if( NOT MULLE_NO_CMAKE_INSTALL_RPATH)
   if( APPLE)
      set( CMAKE_INSTALL_RPATH
         "@loader_path/../lib/"
         "@loader_path/../Frameworks/"
      )
   else()
      set( CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
   endif()
endif()


# extension : mulle-sde/cmake
# directory : project/all
# template  : .../InstallRpath.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
