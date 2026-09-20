### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

#
# DependenciesAndLibraries.cmake
#    Includes the two generated resolution files, libraries first so that
#    local library definitions win over inherited ones.
#
#    This file is also inherited by other projects, through
#    INSTALL_CMAKE_INCLUDES. It therefore must not cache anything and has
#    no include guard. Caching is done by Dependencies.cmake, which only the
#    main project includes.
#
# INPUT
#    COLLECT_DEPENDENCY_LIBRARIES_AS_NAMES
#                                    honoured by the generated
#                                    _Dependencies. When ON it collects
#                                    plain dependency names instead of
#                                    resolved library file paths, which is
#                                    also how a parent project that resolves
#                                    dependencies itself suppresses all
#                                    searching. Used by the installed
#                                    <project>-config.cmake
#    MULLE_TRACE_INCLUDE             trace inclusion of this file
#
#    The generated _Dependencies additionally honours DEPENDENCY_DIR,
#    CMAKE_PREFIX_PATH and DEPENDENCY_IGNORE_SYSTEM_LIBRARIES. None of them
#    are inspected here, see the comment at the include below.
#
# OUTPUT
#    Appends to the dependency and library lists documented in
#    Dependencies.cmake. Nothing is cached here.
#
# INCLUDES
#    _Libraries (OPTIONAL), _Dependencies (OPTIONAL)
#
# GUARD
#    none, deliberately
#

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# Get Libraries first. That way local library definitions override those
# we might inherit from dependencies. The link order should not be affected by
# this.
#
include( _Libraries OPTIONAL)

#
# Always include this. _Dependencies resolves each dependency in three steps,
# cmake target first, then a find_library restricted to CMAKE_PREFIX_PATH and
# friends, then optionally the system paths. Which of those can succeed is not
# knowable from here, so there is nothing to gate on: a DEPENDENCY_DIR test
# would answer "did mulle-craft build into a directory", while the question is
# "can these dependencies be resolved at all". Including this file cannot
# invent a dependency, it can only report one it fails to find.
#
# When a dependency cannot be resolved the generated file says so. Callers
# that legitimately have nothing to resolve, because a parent project links
# the dependencies itself, set COLLECT_DEPENDENCY_LIBRARIES_AS_NAMES to
# collect plain names and skip all searching.
#
include( _Dependencies OPTIONAL)
