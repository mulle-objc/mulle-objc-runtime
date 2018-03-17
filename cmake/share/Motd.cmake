
#
# Output message of a day to locate output.
# But if create-build-motd doesn't exist, it's no biggy
#
find_program( CREATE_MOTD_EXE create-build-motd
   PATHS "${MULLE_VIRTUAL_ROOT}/.mulle-sde/bin"
         "${MULLE_VIRTUAL_ROOT}/.mulle-sde/share/bin"
)

if( CREATE_MOTD_EXE)
   add_custom_target( __motd__ ALL
      COMMAND "${CREATE_MOTD_EXE}" $ENV{CREATE_BUILD_MOTD_FLAGS}
                  "executable"
                     "${CMAKE_BINARY_DIR}"
                     "${PROJECT_NAME}"
      COMMENT "Creating a motd file for mulle-craft"
   )

   add_dependencies( __motd__ ${PROJECT_NAME})
endif()
