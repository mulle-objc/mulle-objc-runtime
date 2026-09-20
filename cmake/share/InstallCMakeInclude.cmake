### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# InstallCMakeInclude.cmake
#    Renamed to AddSubdirectorySupport.cmake, because it installs nothing.
#    This file only forwards, so that projects whose CMakeLists.txt still
#    says include( InstallCMakeInclude) keep working. Use the new name.
#
include( AddSubdirectorySupport)
