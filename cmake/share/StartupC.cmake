if( NOT __STARTUP_C_CMAKE__)
   set( __STARTUP_C_CMAKE__ ON)

# can be included multiple times

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()


   if( STARTUP_SOURCES)
      if( NOT STARTUP_LIBRARY_NAME)
         set( STARTUP_LIBRARY_NAME "${PROJECT_NAME}-startup")
      endif()
      if( NOT STARTUP_DEFINITIONS)
         set( STARTUP_DEFINITIONS ${MULLE_OBJC_RUNTIME_DEFINITIONS})
      endif()

      add_library( ${STARTUP_LIBRARY_NAME} STATIC
         ${STARTUP_SOURCES}
      )
      set_property( TARGET ${STARTUP_LIBRARY_NAME} PROPERTY CXX_STANDARD 11)

      target_compile_definitions( ${STARTUP_LIBRARY_NAME} PRIVATE ${STARTUP_DEFINITIONS})

      set( INSTALL_LIBRARY_TARGETS
         ${INSTALL_LIBRARY_TARGETS}
         ${STARTUP_LIBRARY_NAME}
      )

      set( STARTUP_LIBRARY
         $<TARGET_FILE:${STARTUP_LIBRARY_NAME}>
      )
   else()
      #
      # For mulle-objc, the startup library contains
      # ___get_or_create_mulle_objc_universe and
      # the startup code to create the universe.
      # For C it's rarely if ever needed
      #
      if( STARTUP_LIBRARY_NAME)
         if( NOT STARTUP_LIBRARY)
            find_library( STARTUP_LIBRARY NAMES ${CMAKE_STATIC_LIBRARY_PREFIX}${STARTUP_LIBRARY_NAME}${CMAKE_STATIC_LIBRARY_SUFFIX}
                                             ${STARTUP_LIBRARY_NAME})
         endif()
         if( NOT STARTUP_LIBRARY)
            message( FATAL_ERROR "Startup library \"${STARTUP_LIBRARY_NAME}\" not found")
         endif()
      endif()

      # MEMO: do not add to DEPENDENCY_LIBRARIES (mulle-objc-compat)
   endif()

   message( STATUS "STARTUP_LIBRARY_NAME is ${STARTUP_LIBRARY_NAME}")
   message( STATUS "STARTUP_LIBRARY is ${STARTUP_LIBRARY}")

   include( StartupAuxC OPTIONAL)

endif()
