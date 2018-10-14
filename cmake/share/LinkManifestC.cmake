# can probably not be included multiple times but not sure

if( MULLE_TRACE_INCLUDE)
   message( STATUS "# Include \"${CMAKE_CURRENT_LIST_FILE}\"" )
endif()

if( NOT __LINK_MANIFEST_C_CMAKE__)
   set( __LINK_MANIFEST_C_CMAKE__ ON)

   option( LINK_MANIFEST "Enable link info output" ON)
endif()


#
# Since tests do not know how to link stuff, and we don't really want to
# write a CMakeLists.txt for each test
# Let's emit some specific information for tests
#
# We could also just grep CMakeCache.txt here, but how stable is its format ?
#
if( LINK_MANIFEST)

   if( NOT LIBRARY_NAME)
      set( LIBRARY_NAME "${PROJECT_NAME}")
   endif()

   set( LINK_MANIFEST_FILES
         ${PROJECT_BINARY_DIR}/os-specific-libraries.txt
         ${PROJECT_BINARY_DIR}/dependency-libraries.txt
         ${PROJECT_BINARY_DIR}/optional-dependency-libraries.txt
         ${PROJECT_BINARY_DIR}/all-load-dependency-libraries.txt
   )

   add_custom_target( __link-manifest__ ALL
                  DEPENDS ${LINK_MANIFEST_FILES}
   )

   add_dependencies( __link-manifest__ "${LIBRARY_NAME}")

  # add_dependencies( MulleObjC os_specific_libs)

   add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/os-specific-libraries.txt

                      COMMAND echo "${OS_SPECIFIC_LIBRARIES}" | tr ";" "\\012" > ${PROJECT_BINARY_DIR}/os-specific-libraries.txt

                      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      COMMENT "Create: os-specific-libraries.txt"
                      VERBATIM)


   add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/dependency-libraries.txt

                      COMMAND echo "${DEPENDENCY_LIBRARIES}" | tr ";" "\\012" > ${PROJECT_BINARY_DIR}/dependency-libraries.txt

                      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      COMMENT "Create: dependency-libraries.txt"
                      VERBATIM)

   add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/optional-dependency-libraries.txt

                      COMMAND echo "${OPTIONAL_DEPENDENCY_LIBRARIES}" | tr ";" "\\012" > ${PROJECT_BINARY_DIR}/optional-dependency-libraries.txt

                      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      COMMENT "Create: optional-dependency-libraries.txt"
                      VERBATIM)


   add_custom_command( OUTPUT ${PROJECT_BINARY_DIR}/all-load-dependency-libraries.txt

                      COMMAND echo "${ALL_LOAD_DEPENDENCY_LIBRARIES}" | tr ";" "\\012" > ${PROJECT_BINARY_DIR}/all-load-dependency-libraries.txt

                      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                      COMMENT "Create: all-load-dependency-libraries.txt"
                      VERBATIM)

   # definitions from cmake/share/LinkManifest.cmake
   install( FILES ${LINK_MANIFEST_FILES}  DESTINATION "include/${LIBRARY_NAME}/link")

endif()

include( LinkManifestAuxC OPTIONAL)
