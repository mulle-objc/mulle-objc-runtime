### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

option( RESOLVE_INSTALLABLE_HEADER_SYMLINKS "Resolve installable header symlinks" OFF)
message( STATUS "RESOLVE_INSTALLABLE_HEADER_SYMLINKS is ${RESOLVE_INSTALLABLE_HEADER_SYMLINKS}")


#
# The following includes include definitions generated
# during `mulle-sde reflect`. Don't edit those files. They are
# overwritten frequently.
#
# === MULLE-SDE START ===

include( _Headers OPTIONAL)

# === MULLE-SDE END ===
#

# When multi-reflect is enabled, keep only the selected reflect header tree
# in build-time include/header lists and ignore all other reflect variants.
set( _MULLE_ACTIVE_REFLECT_DIR "src/reflect")
if( MULLE_SOURCETREE_CONFIG_NAME)
   set( _MULLE_ACTIVE_REFLECT_DIR "src/reflect.${MULLE_SOURCETREE_CONFIG_NAME}")
endif()

function( _mulle_filter_reflect_entries listname keepdir)
   unset( _list)
   foreach( _item ${${listname}})
      # match src/reflect. at any depth (handles ../src/reflect. too)
      string( REGEX MATCH "(^|/)src/reflect[.]" _is_reflect "${_item}")
      if( _is_reflect)
         # check if it's the active reflect dir or a child of it
         string( FIND "${_item}" "${keepdir}" _is_keep)
         if( _is_keep GREATER_EQUAL 0)
            list( APPEND _list "${_item}")
         endif()
      else()
         list( APPEND _list "${_item}")
      endif()
   endforeach()
   set( ${listname} ${_list} PARENT_SCOPE)
endfunction()

_mulle_filter_reflect_entries( INCLUDE_DIRS "${_MULLE_ACTIVE_REFLECT_DIR}")
_mulle_filter_reflect_entries( PRIVATE_HEADERS "${_MULLE_ACTIVE_REFLECT_DIR}")
_mulle_filter_reflect_entries( PRIVATE_GENERATED_HEADERS "${_MULLE_ACTIVE_REFLECT_DIR}")
_mulle_filter_reflect_entries( PUBLIC_HEADERS "${_MULLE_ACTIVE_REFLECT_DIR}")
_mulle_filter_reflect_entries( PUBLIC_GENERATED_HEADERS "${_MULLE_ACTIVE_REFLECT_DIR}")

list( REMOVE_DUPLICATES INCLUDE_DIRS)

#
# If you don't like the "automatic" way of generating _Headers
#
# MULLE_MATCH_TO_CMAKE_HEADERS_FILE="DISABLE" # or NONE
#


function( ResolveFileSymlinksIfNeeded listname outputname)
   unset( list)
   if( RESOLVE_INSTALLABLE_HEADER_SYMLINKS)
      foreach( TMP_HEADER ${${listname}})
         file( REAL_PATH "${TMP_HEADER}" TMP_RESOLVED_HEADER)
         list( APPEND list "${TMP_RESOLVED_HEADER}")
      endforeach()
      message( STATUS "Resolved symlinks of ${outputname}=${list}")
   else()
      set( list ${${listname}})
   endif()
   set( ${outputname} ${list} PARENT_SCOPE)
endfunction()


#
# INSTALL_PUBLIC_HEADERS
# INSTALL_PRIVATE_HEADERS
#

# keep headers to install separate to make last minute changes
set( TMP_HEADERS ${PUBLIC_HEADERS}
                 ${PUBLIC_GENERIC_HEADERS}
                 ${PUBLIC_GENERATED_HEADERS}
)
ResolveFileSymlinksIfNeeded( TMP_HEADERS INSTALL_PUBLIC_HEADERS)

#
# Do not install generated private headers and include-private.h
# which aren't valid outside of the project scope.
#
set( TMP_HEADERS ${PRIVATE_HEADERS})
if( TMP_HEADERS)
   list( REMOVE_ITEM TMP_HEADERS "include-private.h")
endif()
ResolveFileSymlinksIfNeeded( TMP_HEADERS INSTALL_PRIVATE_HEADERS)

#
# You can put more source and resource file definitions here.
#
