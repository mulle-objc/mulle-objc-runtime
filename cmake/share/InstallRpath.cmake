### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# InstallRpath.cmake
#    Renamed to Rpath.cmake, because it installs nothing. It only sets
#    CMAKE_INSTALL_RPATH, and it must run before the target is created, so
#    the old name suggested the wrong position too. This file only forwards,
#    so that projects whose CMakeLists.txt still says
#    include( InstallRpath OPTIONAL) keep working. Use the new name.
#
include( Rpath)
