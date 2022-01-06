### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()


include( AllLoadC)

include( ExecutableAuxC OPTIONAL)

CreateForceAllLoadList( ALL_LOAD_DEPENDENCY_LIBRARIES FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES)
CreateForceAllLoadList( ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES FORCE_ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES)
CreateForceAllLoadList( ALL_LOAD_OS_SPECIFIC_LIBRARIES FORCE_ALL_LOAD_OS_SPECIFIC_LIBRARIES)
CreateForceAllLoadList( STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES FORCE_STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES)

#
# Currently we have no distinction for static and dynamic frameworks 
# all frameworks are dynamic, so we don't really force load them.
# But we keep the code as is otherwise, because static frameworks are likely
# to arrive sooner or later.
#
if( APPLE)
   unset( FORCE_LOAD_PREFIX)
endif()

# MEMO: frameworks are always shared so we don't force them

# CreateForceAllLoadList( ALL_LOAD_DEPENDENCY_FRAMEWORKS FORCE_ALL_LOAD_DEPENDENCY_FRAMEWORKS)
# CreateForceAllLoadList( ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS FORCE_ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS)
# CreateForceAllLoadList( ALL_LOAD_OS_SPECIFIC_FRAMEWORKS FORCE_ALL_LOAD_OS_SPECIFIC_FRAMEWORKS)
# CreateForceAllLoadList( STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS FORCE_STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS)

if( NOT DEFINED EXECUTABLE_LIBRARY_LIST)
   set( EXECUTABLE_LIBRARY_LIST
         ${FORCE_ALL_LOAD_DEPENDENCY_LIBRARIES}
         ${ALL_LOAD_DEPENDENCY_FRAMEWORKS}
         ${DEPENDENCY_LIBRARIES}
         ${DEPENDENCY_FRAMEWORKS}

         ${FORCE_ALL_LOAD_OPTIONAL_DEPENDENCY_LIBRARIES}
         ${ALL_LOAD_OPTIONAL_DEPENDENCY_FRAMEWORKS}
         ${OPTIONAL_DEPENDENCY_LIBRARIES}
         ${OPTIONAL_DEPENDENCY_FRAMEWORKS}

         ${FORCE_STARTUP_ALL_LOAD_DEPENDENCY_LIBRARIES}
         ${STARTUP_ALL_LOAD_DEPENDENCY_FRAMEWORKS}
         ${STARTUP_DEPENDENCY_LIBRARIES}
         ${STARTUP_DEPENDENCY_FRAMEWORKS}

         ${FORCE_ALL_LOAD_OS_SPECIFIC_LIBRARIES}
         ${ALL_LOAD_OS_SPECIFIC_FRAMEWORKS}
         ${OS_SPECIFIC_LIBRARIES}
         ${OS_SPECIFIC_FRAMEWORKS}
   )
endif()


# extension : mulle-c/c-cmake
# directory : project/all
# template  : .../ExecutableAux.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
