if( NOT __POST_LIBRARY__CMAKE__)
   set( __POST_LIBRARY__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   include( AllLoadC)
   include( StandaloneC)

   include( LinkManifestC)
   include( FinalOutputC)

   include( PostLibraryCAux OPTIONAL)

endif()
