#
# A standalone library is a combination of multiple static libraries into
# one shared library. Due to deficiencies in cmake on windows it is necessary
# to have at lease one extra .c or .m file in the standalone library
# defined in STANDALONE_SOURCES.
#

#
# Input:
#
# STANDALONE_BASE_NAME
# STANDALONE_ALL_LOAD_LIBRARIES
# STANDALONE_SOURCES
#
# Optional:
#
# STANDALONE_NAME
# STANDALONE_DEFINITIONS
# STANDALONE_SYMBOL_PREFIXES
# STANDALONE_NORMAL_LOAD_LIBRARIES
# MULLE_LANGUAGE
#

if( NOT __STANDALONE__CMAKE__)
   set( __STANDALONE__CMAKE__ ON)

   if( MULLE_TRACE_INCLUDE)
      message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
   endif()

   #
   # option must be set in CMakeLists.txt
   # option( STANDALONE "Create standalone library for debugging" OFF)

   if( STANDALONE)
      set( STANDALONE_VERSION 3)

      #
      # If the main library is built as a shared library, we can't do it
      #
      if( NOT BUILD_SHARED_LIBS)

         include( AllLoad)

         if( NOT STANDALONE_NAME)
            set( STANDALONE_NAME "${STANDALONE_BASE_NAME}Standalone")
         endif()

         #
         # Check for required standalone source file
         #
         if( NOT STANDALONE_SOURCES)
            message( FATAL_ERROR "You need to define STANDALONE_SOURCES. Add a file
mulle-objc-runtime-standalone.c with contents like this:
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
         foreach( library ${STANDALONE_ALL_LOAD_LIBRARIES})
            list( APPEND STANDALONE_FORCE_ALL_LOAD_LIBRARIES "${FORCE_LOAD_PREFIX}${library}")
         endforeach()

         foreach( prefix ${STANDALONE_SYMBOL_PREFIXES})
            list( APPEND STANDALONE_DUMPDEF_SYMBOL_PREFIXES "--prefix")
            list( APPEND STANDALONE_DUMPDEF_SYMBOL_PREFIXES "${prefix}")
         endforeach()

         #
         # On Windows we need to rexport symbols using a .def file
         #
         if( MSVC)
            set( DEF_FILE "${STANDALONE_NAME}.def")
            set_source_files_properties( ${DEF_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
            set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
            set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:${DEF_FILE}")

            message( STATUS "MSVC will generate \"${DEF_FILE}\" from ${TARGET_ALL_LOAD_LIBRARIES}")

            add_custom_command( OUTPUT ${DEF_FILE}
                                COMMAND mulle-mingw-dumpdef.bat -o "${DEF_FILE}"
                                        --directory "${BUILD_RELATIVE_DEPENDENCY_DIR}/lib"
                                        ${STANDALONE_DUMPDEF_SYMBOL_PREFIXES}
                                        ${STANDALONE_ALL_LOAD_LIBRARIES}
                                DEPENDS ${STANDALONE_ALL_LOAD_LIBRARIES}
                                VERBATIM)
         endif()


         #
         # if STANDALONE_SOURCE is not defined, cmake on windows "forgets" to produce
         # the DEF_FILE.
         #
         # Also you get tedious linker warnings on other platforms. Creating the
         # STANDALONE_SOURCES on the fly, is just not worth it IMO.
         #
         add_library( ${STANDALONE_NAME} SHARED
            ${STANDALONE_SOURCES}
            ${DEF_FILE}
         )

         if( NOT STANDALONE_DEPENDENCIES)
            set( STANDALONE_DEPENDENCIES
               ${STANDALONE_BASE_NAME}
               ${STANDALONE_STARTUP}
            )
         endif()

         add_dependencies( ${STANDALONE_NAME} ${STANDALONE_DEPENDENCIES})

         # If STANDALONE_SOURCES were to be empty, this would be needed
         # set_target_properties( ${STANDALONE_NAME} PROPERTIES LINKER_LANGUAGE "C")

         # PRIVATE is a guess
         target_compile_definitions( ${STANDALONE_NAME} PRIVATE ${STANDALONE_DEFINITIONS})

         #
         # If you add DEPENDENCY_LIBRARIES to the static, adding them again to
         # MulleObjCStandardFoundationStandalone confuses cmake it seems. But they are
         # implicitly added.
         #
         target_link_libraries( ${STANDALONE_NAME}
            ${BEGIN_ALL_LOAD}
            ${STANDALONE_FORCE_ALL_LOAD_LIBRARIES}
            ${END_ALL_LOAD}
            ${STANDALONE_NORMAL_LOAD_LIBRARIES}
         )
      else()
         message( WARNING "Standalone can not be built with BUILD_SHARED_LIBS set to ON")
      endif()

   endif()
endif()
