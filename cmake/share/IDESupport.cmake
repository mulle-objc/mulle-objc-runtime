### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

### Files
if( NOT __IDE_SUPPORT_START__CMAKE__)
   set( __IDE_SUPPORT_START__CMAKE__ ON)

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

#default to ON for a while
option( IDE_SUPPORT "Enable IDE support for Clion" ON)

if( IDE_SUPPORT)
   find_program( MULLE_SDE mulle-sde)
   if( NOT MULLE_SDE)
      message( WARNING "The folder \"dependency\" is not ready.\nInstall mulle-sde and run `mulle-sde craft craftorder`")
   else()
      if( NOT DEFINED ENV{MULLE_VIRTUAL_ROOT}) # sic! NOT ${ENV}
         #
         # If there is (likely) a sourcetree, we need to build dependencies now
         # because otherwise the include( DEPENDENCIES) fails.
         #
         if( IS_DIRECTORY "${PROJECT_SOURCE_DIR}/.mulle/etc/sourcetree" OR
             IS_DIRECTORY "${PROJECT_SOURCE_DIR}/.mulle/share/sourcetree")
            #
            # If we have a dependency folder, check for the .state-complete
            #
            if( NOT EXISTS "${DEPENDENCY_DIR}/.state-complete")
               execute_process( COMMAND "${MULLE_SDE}" craft --build-type "${CMAKE_BUILD_TYPE}" craftorder
                                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                                COMMAND_ERROR_IS_FATAL ANY)
            endif()
         endif()

         #
         # In an IDE like CLion use these target to clean all. The immediate
         # rebuild is needed, because the above code won't run again.
         # mulle_sde_craft_craftorder: just rebuild. Can be useful if "clean all"
         #                             failed.
         add_custom_target( mulle_sde_craft_craftorder
            COMMENT "Craft dependency folder"
            COMMAND "${MULLE_SDE}" craft --build-type "${CMAKE_BUILD_TYPE}" craftorder
            VERBATIM
         )
         set_target_properties( mulle_sde_craft_craftorder PROPERTIES EXCLUDE_FROM_ALL ON)

         add_custom_target( mulle_sde_reflect
            COMMENT "Reflect files and sourcetree"
            COMMAND "${MULLE_SDE}" reflect
            VERBATIM
         )
         set_target_properties( mulle_sde_reflect PROPERTIES EXCLUDE_FROM_ALL ON)

         add_custom_target( mulle_sde_clean_all
            COMMENT "Clean dependency folder"
            COMMAND "${MULLE_SDE}" clean all
            VERBATIM
         )
         set_target_properties( mulle_sde_clean_all PROPERTIES EXCLUDE_FROM_ALL ON)

         add_custom_target( mulle_sde_clean_tidy
            COMMENT "Clean stash and dependency folder"
            COMMAND "${MULLE_SDE}" clean tidy
            VERBATIM
         )
         set_target_properties( mulle_sde_clean_tidy PROPERTIES EXCLUDE_FROM_ALL ON)

         add_custom_target( mulle_sde_upgrade
            COMMENT "Upgrade project to current mulle-sde version"
            COMMAND "${MULLE_SDE}" upgrade
            VERBATIM
         )
         set_target_properties( mulle_sde_upgrade PROPERTIES EXCLUDE_FROM_ALL ON)
      else()
         message( STATUS "Skipping IDE Support as we are in a mulle-sde environment")
      endif()
   endif()
endif()

endif()

# extension : mulle-sde/cmake
# directory : project/all
# template  : .../IDESupport.cmake
# Suppress this comment with `export MULLE_SDE_GENERATE_FILE_COMMENTS=NO`
