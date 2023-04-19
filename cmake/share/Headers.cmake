### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

option( RESOLVE_INSTALLABLE_HEADER_SYMLINKS "Resolve PROJECT_INSTALLABLE_HEADERS symlinks" OFF)
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
# PROJECT_INSTALLABLE_HEADERS
# INSTALL_PUBLIC_HEADERS
# INSTALL_PRIVATE_HEADERS
#

# keep headers to install separate to make last minute changes
set( TMP_HEADERS ${PUBLIC_HEADERS}
                 ${PUBLIC_GENERIC_HEADERS}
                 ${PUBLIC_GENERATED_HEADERS}
)
ResolveFileSymlinksIfNeeded( TMP_HEADERS INSTALL_PUBLIC_HEADERS)

set( TMP_HEADERS ${PRIVATE_HEADERS})
if( TMP_HEADERS)
   list( REMOVE_ITEM TMP_HEADERS "include-private.h")
endif()
ResolveFileSymlinksIfNeeded( TMP_HEADERS INSTALL_PRIVATE_HEADERS)

# let's not cache headers, as they are bound to fluctuate. when we change
# dependencies we expect a clean
set( PROJECT_INSTALLABLE_HEADERS
   ${INSTALL_PUBLIC_HEADERS}
   ${INSTALL_PRIVATE_HEADERS}
)

#
# You can put more source and resource file definitions here.
#
