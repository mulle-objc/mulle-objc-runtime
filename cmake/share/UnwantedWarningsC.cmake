### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __UNWANTED_WARNINGS_C_CMAKE__)
   set( __UNWANTED_WARNINGS_C_CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   if( NOT DEFINED UNWANTED_WARNINGS)
      option( UNWANTED_WARNINGS "Turn off some unwanted compiler warnings" ON)
   endif()

   if( UNWANTED_WARNINGS)
      #
      # move this to ObjC
      #
      if( "${MULLE_C_COMPILER_ID}" MATCHES "^(Clang|AppleClang|MulleClang|GNU)$")
         set( UNWANTED_C_WARNINGS "-Wno-parentheses -Wno-int-to-void-pointer-cast")
      else()
         if( "${MULLE_C_COMPILER_ID}" MATCHES "^(Intel|MSVC|MSVC-Clang|MSVC-MulleClang)$")
            # C4068: unwanted pragma
            set( UNWANTED_C_WARNINGS "/D_CRT_SECURE_NO_WARNINGS /wd4068 /wd4113")
         endif()
      endif()

      if( "${MULLE_C_COMPILER_ID}" MATCHES "^(MSVC-Clang|MSVC-MulleClang)$")
         # set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
         # 4211 is for classes..
         set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4221")
         set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4221")
         set( CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4221")
      endif()
   endif()

endif()
