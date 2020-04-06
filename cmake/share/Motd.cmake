### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

if( NOT __MOTD__CMAKE__)
   set( __MOTD__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # Output message of a day to locate output.
   # But if create-build-motd doesn't exist, it's no biggy
   #
   if( MSVC)
      find_program( CREATE_MOTD_EXE mulle-create-build-motd.bat
         PATHS "${MULLE_VIRTUAL_ROOT}/.mulle/var/$ENV{MULLE_HOSTNAME}/env/bin"
      )
   else()
      # will fail on WSL if .mulle/var is elsewhere`. should get
      # location from `mulle-env vardir env`
      find_program( CREATE_MOTD_EXE mulle-create-build-motd
         PATHS "${MULLE_VIRTUAL_ROOT}/.mulle/var/$ENV{MULLE_HOSTNAME}/env/bin"
      )
   endif()

   # must run in singlephase
   if( CREATE_MOTD_EXE)
      add_custom_target( __motd__ ALL
         COMMAND "${CREATE_MOTD_EXE}" $ENV{CREATE_BUILD_MOTD_FLAGS}
                     "executable"
                        "${CMAKE_BINARY_DIR}"
                        "${EXECUTABLE_NAME}"
         COMMENT "Creating a motd file for mulle-craft"
         VERBATIM
      )

      add_dependencies( __motd__ ${EXECUTABLE_NAME})
   endif()

   include( MotdAux OPTIONAL)

endif()
