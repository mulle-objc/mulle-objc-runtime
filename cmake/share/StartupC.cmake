if( NOT __STARTUP_C__CMAKE__)
   set( __STARTUP_C__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   if( STARTUP_SOURCES)
      if( NOT STARTUP_NAME)
         set( STARTUP_NAME "mulle-objc-runtime-startup")
      endif()
      set( STARTUP_DEFINITIONS ${MULLE_OBJC_RUNTIME_DEFINITIONS})

      add_library( ${STARTUP_NAME} STATIC
         ${STARTUP_SOURCES}
      )
      set_property( TARGET ${STARTUP_NAME} PROPERTY CXX_STANDARD 11)

      target_compile_definitions( ${STARTUP_NAME} PRIVATE ${STARTUP_DEFINITIONS})

      set( INSTALL_LIBRARY_TARGETS
         ${INSTALL_LIBRARY_TARGETS}
         ${STARTUP_NAME}
      )

      set( STARTUP_LIBRARY
         $<TARGET_FILE:${STARTUP_NAME}>
      )
   endif()

   include( StartupCAux OPTIONAL)
endif()
