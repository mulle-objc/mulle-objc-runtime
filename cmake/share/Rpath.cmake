### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# Rpath.cmake
#    Sets the RPATH used when "install" is run, so an installed binary finds
#    its libraries in ../lib. Not used during a regular craft.
#
#    Must be included before add_library or add_executable, because
#    CMAKE_INSTALL_RPATH is read at target creation time.
#
# INPUT
#    MULLE_NO_CMAKE_INSTALL_RPATH   skip all of this and set
#                                   CMAKE_SKIP_BUILD_RPATH instead. Useful
#                                   for cosmopolitan and musl static builds,
#                                   where an RPATH is a hindrance
#    MULLE_TRACE_INCLUDE            trace inclusion of this file
#
# OUTPUT
#    CMAKE_INSTALL_RPATH     "$ORIGIN/../lib" elsewhere, and the two
#                            "@loader_path" entries on APPLE with cmake
#                            older than 3.20. Newer cmake handles APPLE
#                            library paths by itself
#    CMAKE_SKIP_BUILD_RPATH  only with MULLE_NO_CMAKE_INSTALL_RPATH
#
# GUARD
#    none, this can be included multiple times
#

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
# https://cmake.org/cmake/help/latest/prop_tgt/INSTALL_RPATH.html#prop_tgt:INSTALL_RPATH
#
if( MULLE_NO_CMAKE_INSTALL_RPATH)
   # for cosmopolitan and musl static builds RPATH can be a hindrance
   set( CMAKE_SKIP_BUILD_RPATH ON)
else()
   if( APPLE)
      # Modern CMake handles lib path automatically (at least for libraries
      # it seems, for executable apparently not and there it is handled
      # with a separate target property install)
      if( CMAKE_VERSION VERSION_LESS 3.20)
         set( CMAKE_INSTALL_RPATH
             "@loader_path/../lib/"
             "@loader_path/../Frameworks/"
         )
      endif()
   else()
      set( CMAKE_INSTALL_RPATH "\$ORIGIN/../lib")
   endif()
endif()
