### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# InstallCMakePackage.cmake
#    Renamed to FindPackageSupport.cmake, because what it does is make this
#    library findable with find_package(). This file only forwards, so that
#    projects whose CMakeLists.txt still says include( InstallCMakePackage)
#    keep working. Use the new name.
#
include( FindPackageSupport)
