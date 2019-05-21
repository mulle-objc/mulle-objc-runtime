# can be included multiple times

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

if( NOT __STANDALONE_C_CMAKE__)
   set( __STANDALONE_C_CMAKE__ ON)

   if( NOT DEFINED STANDALONE)
      option( STANDALONE "Create standalone library for debugging" OFF)
   endif()
endif()


# include before (!)

include( StandaloneAuxC OPTIONAL)

if( STANDALONE)
   if( NOT LIBRARY_NAME)
      set( LIBRARY_NAME "${PROJECT_NAME}")
   endif()

   if( NOT STANDALONE_LIBRARY_NAME)
      set( STANDALONE_LIBRARY_NAME "${LIBRARY_NAME}-standalone")
   endif()

   if( NOT STANDALONE_DEFINITIONS)
      set( STANDALONE_DEFINITIONS ${MULLE_OBJC_RUNTIME_DEFINITIONS})
   endif()

   #
   # A standalone library has all symbols and nothing is optimized away
   # sorta like a big static library, just shared, The OS specific stuff
   # should be shared libraries, otherwise they are only normally
   # linked against (only required symbols.
   #
   if( NOT STANDALONE_ALL_LOAD_LIBRARIES)
      set( STANDALONE_ALL_LOAD_LIBRARIES
         $<TARGET_FILE:${LIBRARY_NAME}>
         ${ALL_LOAD_DEPENDENCY_LIBRARIES}
         ${DEPENDENCY_LIBRARIES}
         ${OPTIONAL_DEPENDENCY_LIBRARIES}
      )
   endif()

   #
   # for example take out the mulle-allocator library from the standalone
   # so that the test library can add mulle-testallocator
   #
   if( STANDALONE_EXCLUDE_LIBRARIES)
      list( REMOVE_ITEM STANDALONE_ALL_LOAD_LIBRARIES ${STANDALONE_EXCLUDE_LIBRARIES})
   endif()

   # STARTUP_LIBRARY is supposed to be a find_library definition
   if( NOT STANDALONE_STARTUP_LIBRARY)
      set( STANDALONE_STARTUP_LIBRARY ${STARTUP_LIBRARY})
      set( STANDALONE_STARTUP_LIBRARY_NAME ${STARTUP_LIBRARY_NAME})
   endif()

   if( STANDALONE_STARTUP_LIBRARY)
      set( STANDALONE_ALL_LOAD_LIBRARIES
         ${STANDALONE_ALL_LOAD_LIBRARIES}
         ${STANDALONE_STARTUP_LIBRARY}
      )
   endif()


   #
   # If the main library is built as a shared library, we can't do it
   #
   if( BUILD_SHARED_LIBS)
      message( WARNING "Standalone can not be built with BUILD_SHARED_LIBS set to ON")
   else()
      #
      # Check for required standalone source file
      #
      if( NOT STANDALONE_SOURCES)
         message( FATAL_ERROR "You need to define STANDALONE_SOURCES. Add a file
${STANDALONE_LIBRARY_NAME}.c with contents like this to it:
int  ___mulle_objc_runtime_unused__;
and everybody will be happy")
      endif()

      #
      # symbol prefixes to export on Windows, ignored on other platforms
      #
      if( NOT STANDALONE_SYMBOL_PREFIXES)

         set( STANDALONE_SYMBOL_PREFIXES "mulle"
                 "_mulle"
         )

        if( "${MULLE_LANGUAGE}" MATCHES "ObjC")
            set( STANDALONE_SYMBOL_PREFIXES
               ${STANDALONE_SYMBOL_PREFIXES}
               "Mulle"
               "ns"
               "NS"
               "_Mulle"
               "_ns"
               "_NS"
            )
        endif()
      endif()

      #
      # Make sure static libraries aren't optimized away
      #
      foreach( prefix ${STANDALONE_SYMBOL_PREFIXES})
         list( APPEND STANDALONE_DUMPDEF_SYMBOL_PREFIXES "--prefix")
         list( APPEND STANDALONE_DUMPDEF_SYMBOL_PREFIXES "${prefix}")
      endforeach()

      #
      # On Windows we need to rexport symbols using a .def file
      #
      if( MSVC)
         set( DEF_FILE "${STANDALONE_LIBRARY_NAME}.def")
         set_source_files_properties( ${DEF_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
         set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
         set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:${DEF_FILE}")

         message( STATUS "MSVC will generate \"${DEF_FILE}\" from ${STANDALONE_ALL_LOAD_LIBRARIES}")

         add_custom_command( OUTPUT ${DEF_FILE}
                             COMMAND mulle-mingw-dumpdef.bat -o "${DEF_FILE}"
                                     --directory "${BUILD_RELATIVE_DEPENDENCY_DIR}/lib"
                                     ${STANDALONE_DUMPDEF_SYMBOL_PREFIXES}
                                     ${STANDALONE_ALL_LOAD_LIBRARIES}
                             DEPENDS ${STANDALONE_ALL_LOAD_LIBRARIES}
                             VERBATIM)
      endif()


      #
      # if STANDALONE_SOURCE is not defined, cmake on windows "forgets" to
      # produce the DEF_FILE.
      #
      # Also you get tedious linker warnings on other platforms. Creating the
      # STANDALONE_SOURCES on the fly, is just not worth it IMO.
      #
      add_library( ${STANDALONE_LIBRARY_NAME} SHARED
         ${STANDALONE_SOURCES}
         ${DEF_FILE}
      )
      set_property( TARGET ${STANDALONE_LIBRARY_NAME} PROPERTY CXX_STANDARD 11)

      add_dependencies( ${STANDALONE_LIBRARY_NAME} ${LIBRARY_NAME})
      if( STANDALONE_STARTUP_LIBRARY_NAME)
         if( TARGET ${STANDALONE_STARTUP_LIBRARY_NAME})
            add_dependencies( ${STANDALONE_LIBRARY_NAME} ${STANDALONE_STARTUP_LIBRARY_NAME})
         endif()
      endif()

      # If STANDALONE_SOURCES were to be empty, this would be needed
      # set_target_properties( ${STANDALONE_LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE "C")

      # PRIVATE is a guess
      target_compile_definitions( ${STANDALONE_LIBRARY_NAME} PRIVATE ${STANDALONE_DEFINITIONS})

      #
      # If you add DEPENDENCY_LIBRARIES to the static, adding them again to
      # MulleObjCStandardFoundationStandalone confuses cmake it seems. But they
      # are implicitly added.
      #
      # creates FORCE_STANDALONE_ALL_LOAD_LIBRARIES

      CreateForceAllLoadList( STANDALONE_ALL_LOAD_LIBRARIES FORCE_STANDALONE_ALL_LOAD_LIBRARIES)

      target_link_libraries( ${STANDALONE_LIBRARY_NAME}
         ${FORCE_STANDALONE_ALL_LOAD_LIBRARIES}
         ${OS_SPECIFIC_LIBRARIES}
         ${STANDALONE_STARTUP_LIBRARY}
      )

      set( INSTALL_LIBRARY_TARGETS
         ${INSTALL_LIBRARY_TARGETS}
         ${STANDALONE_LIBRARY_NAME}
      )

      message( STATUS "STANDALONE_LIBRARY_NAME is ${STANDALONE_LIBRARY_NAME}")
      message( STATUS "STANDALONE_ALL_LOAD_LIBRARIES is ${STANDALONE_ALL_LOAD_LIBRARIES}")
      message( STATUS "FORCE_STANDALONE_ALL_LOAD_LIBRARIES is ${FORCE_STANDALONE_ALL_LOAD_LIBRARIES}")
      message( STATUS "OS_SPECIFIC_LIBRARIES is ${OS_SPECIFIC_LIBRARIES}")
   endif()
endif()
