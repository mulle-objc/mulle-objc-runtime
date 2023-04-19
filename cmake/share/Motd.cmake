### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

# this can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#
# Output message of a day to locate output.
# Only run in link phase, which should be defined for singlephase builds too
#
if( EXECUTABLE_NAME AND LINK_PHASE)

   # But if create-build-motd doesn't exist, it's no biggy
   #
   if( NOT CREATE_MOTD_EXE)
      if( MSVC)
         # TODO: adapt search path
         find_program( CREATE_MOTD_EXE mulle-create-build-motd.bat
            PATHS "${MULLE_VIRTUAL_ROOT}/.mulle/var/$ENV{MULLE_HOSTNAME}/$ENV{MULLE_USERNAME}/env/bin"
         )
      else()
         # will fail on WSL if .mulle/var is elsewhere`. should get
         # location from `mulle-env vardir env`
         find_program( CREATE_MOTD_EXE mulle-create-build-motd
            PATHS "${MULLE_VIRTUAL_ROOT}/.mulle/var/$ENV{MULLE_HOSTNAME}/$ENV{MULLE_USERNAME}/env/bin"
         )
      endif()
      message( STATUS "CREATE_MOTD_EXE is ${CREATE_MOTD_EXE}")
   endif()

   #
   # there is no real order, in which these motds are generated
   # as they are hooked into the cmake dependency system
   #
   if( CREATE_MOTD_EXE)
      if( NOT TARGET "__cleanmotd__")
         add_custom_target( "__cleanmotd__" ALL
            COMMAND "test" "!" "-f" "${CMAKE_BINARY_DIR}/.motd" "||" "rm" "${CMAKE_BINARY_DIR}/.motd"
            COMMENT "Remove old motd file for mulle-craft"
            VERBATIM
         )
      endif()
      add_dependencies( "${EXECUTABLE_NAME}" "__cleanmotd__")

      add_custom_target( "_${EXECUTABLE_NAME}__motd__" ALL
         COMMAND ${CMAKE_COMMAND} -E env "MULLE_VIRTUAL_ROOT=${MULLE_VIRTUAL_ROOT}"
                        "${CREATE_MOTD_EXE}"
                        $ENV{CREATE_BUILD_MOTD_FLAGS}
                        "--append"
                     "executable"
                        "${CMAKE_BINARY_DIR}"
                        "${EXECUTABLE_NAME}"
         COMMENT "Append to motd file for mulle-craft"
         VERBATIM
      )

      add_dependencies( "_${EXECUTABLE_NAME}__motd__" ${EXECUTABLE_NAME})
   else()
      message( WARNING "Tool \"mulle-create-build-motd\" not found")
   endif()

   include( MotdAux OPTIONAL)
endif()
