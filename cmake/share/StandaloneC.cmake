### If you want to edit this, copy it from cmake/share to cmake. It will be
### picked up in preference over the one in cmake/share. And it will not get
### clobbered with the next upgrade.

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

if( STANDALONE)
   include( PreStandaloneAuxC OPTIONAL)

   if( NOT LIBRARY_NAME)
      set( LIBRARY_NAME "${PROJECT_NAME}")
   endif()

   include( StringCase)

   if( NOT LIBRARY_IDENTIFIER)
      snakeCaseString( "${LIBRARY_NAME}" LIBRARY_IDENTIFIER)
   endif()
   if( NOT LIBRARY_UPCASE_IDENTIFIER)
      string( TOUPPER "${LIBRARY_IDENTIFIER}" LIBRARY_UPCASE_IDENTIFIER)
   endif()
   if( NOT LIBRARY_DOWNCASE_IDENTIFIER)
      string( TOLOWER "${LIBRARY_IDENTIFIER}" LIBRARY_DOWNCASE_IDENTIFIER)
   endif()

   if( NOT STANDALONE_LIBRARY_NAME)
      set( STANDALONE_LIBRARY_NAME "${LIBRARY_NAME}-standalone")
   endif()

   if( NOT STANDALONE_DEFINITIONS)
      set( STANDALONE_DEFINITIONS ${MULLE__OBJC__RUNTIME_DEFINITIONS})
   endif()

   #
   # A standalone library has all symbols and nothing is optimized away
   # sorta like a big static library, just shared, The OS specific stuff
   # should be shared libraries, otherwise they are only normally
   # linked against (only required symbols).
   #
   if( NOT STANDALONE_ALL_LOAD_LIBRARIES)
      set( STANDALONE_ALL_LOAD_LIBRARIES
         $<TARGET_FILE:${LIBRARY_NAME}>
         ${ALL_LOAD_DEPENDENCY_LIBRARIES}
         ${ALL_LOAD_DEPENDENCY_FRAMEWORKS}
         ${DEPENDENCY_LIBRARIES}
         ${DEPENDENCY_FRAMEWORKS}
         ${OPTIONAL_DEPENDENCY_LIBRARIES}
         ${OPTIONAL_DEPENDENCY_FRAMEWORKS}
      )
   endif()

   #
   # for example take out the mulle-allocator library from the standalone
   # so that the test library can add mulle-testallocator
   #
   if( STANDALONE_EXCLUDE_LIBRARIES)
      list( REMOVE_ITEM STANDALONE_ALL_LOAD_LIBRARIES ${STANDALONE_EXCLUDE_LIBRARIES})
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
int  ___mulle__objc__runtime_unused__;
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

      set_target_properties( "${STANDALONE_LIBRARY_NAME}"
         PROPERTIES
            CXX_STANDARD 11
#            DEFINE_SYMBOL "${LIBRARY_UPCASE_IDENTIFIER}_SHARED_BUILD"
      )
      target_compile_definitions( "${STANDALONE_LIBRARY_NAME}" PRIVATE "${LIBRARY_UPCASE_IDENTIFIER}_BUILD")
      target_compile_definitions( "${STANDALONE_LIBRARY_NAME}" PRIVATE ${STANDALONE_DEFINITIONS})


      add_dependencies( "${STANDALONE_LIBRARY_NAME}" "${LIBRARY_NAME}")


      # If STANDALONE_SOURCES were to be empty, this would be needed
      # set_target_properties( ${STANDALONE_LIBRARY_NAME} PROPERTIES LINKER_LANGUAGE "C")

      # PRIVATE is a guess

      #
      # If you add DEPENDENCY_LIBRARIES to the static, adding them again to
      # MulleObjCStandardFoundationStandalone confuses cmake it seems. But they
      # are implicitly added.
      #
      CreateForceAllLoadList( STANDALONE_ALL_LOAD_LIBRARIES FORCE_STANDALONE_ALL_LOAD_LIBRARIES)

      target_link_libraries( "${STANDALONE_LIBRARY_NAME}"
         ${FORCE_STANDALONE_ALL_LOAD_LIBRARIES}
         ${OS_SPECIFIC_LIBRARIES}
         ${OS_SPECIFIC_FRAMEWORKS}
      )

      set( INSTALL_LIBRARY_TARGETS
         ${INSTALL_LIBRARY_TARGETS}
         ${STANDALONE_LIBRARY_NAME}
      )

      include( PostStandaloneAuxC OPTIONAL)

      message( STATUS "STANDALONE_LIBRARY_NAME is ${STANDALONE_LIBRARY_NAME}")
      message( STATUS "STANDALONE_ALL_LOAD_LIBRARIES is ${STANDALONE_ALL_LOAD_LIBRARIES}")
      message( STATUS "FORCE_STANDALONE_ALL_LOAD_LIBRARIES is ${FORCE_STANDALONE_ALL_LOAD_LIBRARIES}")
      message( STATUS "OS_SPECIFIC_LIBRARIES is ${OS_SPECIFIC_LIBRARIES}")
      message( STATUS "OS_SPECIFIC_FRAMEWORKS is ${OS_SPECIFIC_FRAMEWORKS}")
   endif()
endif()
