### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __COMPILER_DETECTION_C_CMAKE__)
   set( __COMPILER_DETECTION_C_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # for windows want a MSVC prefix
   #
   # CMAKE_C_COMPILER_ID, doesn't detect mulle-clang necessarily, so fallback
   # on -DCMAKE_C_COMPILER 
   #
   if( NOT MULLE_C_COMPILER_ID)
      string( TOUPPER "${CMAKE_C_COMPILER_ID}" TMP_NAME)
      string( REGEX REPLACE "[^A-Za-z0-9]" "" TMP_NAME "${TMP_NAME}")
      if( ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows") AND ( "${TMP_NAME}" MATCHES "CLANG") )
         set( MULLE_C_COMPILER_ID "MSVC-${TMP_NAME}")
      else()
         get_filename_component( TMP_NAME2 "${CMAKE_C_COMPILER}" NAME_WE)
         string( TOUPPER "${TMP_NAME2}" TMP_NAME2)
         string( REGEX REPLACE "[^A-Za-z0-9]" "" TMP_NAME2 "${TMP_NAME2}")
         if( "${TMP_NAME2}" MATCHES "MULLECLANG")
            set( MULLE_C_COMPILER_ID "${TMP_NAME2}")
         else()
            set( MULLE_C_COMPILER_ID "${TMP_NAME}")
         endif()
      endif()
   endif()

   if( NOT MULLE_CXX_COMPILER_ID)
      string( TOUPPER "${CMAKE_CXX_COMPILER_ID}" TMP_NAME)
      string( REGEX REPLACE "[^A-Za-z0-9]" "" TMP_NAME "${TMP_NAME}")
      if( ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows") AND ( "${TMP_NAME}" MATCHES "CLANG") )
         set( MULLE_CXX_COMPILER_ID "MSVC-${TMP_NAME}")
      else()
         get_filename_component( TMP_NAME2 "${CMAKE_CXX_COMPILER}" NAME_WE)
         string( TOUPPER "${TMP_NAME2}" TMP_NAME2)
         string( REGEX REPLACE "[^A-Za-z0-9]" "" TMP_NAME2 "${TMP_NAME2}")
         if( "${TMP_NAME2}" MATCHES "MULLECLANG")
            set( MULLE_CXX_COMPILER_ID "${TMP_NAME2}")
         else()
            set( MULLE_CXX_COMPILER_ID "${TMP_NAME}")
         endif()
      endif()
   endif()

   include( CompilerDetectionAuxC OPTIONAL)

endif()
