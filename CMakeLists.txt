cmake_minimum_required( VERSION 3.0)

project( mulle-objc-runtime C)


set( MULLE_OBJC_RUNTIME_LIBRARY ON)

add_definitions( -D__MULLE_OBJC_TPS__)     ## tagged pointers runtime (default: on)
add_definitions( -D__MULLE_OBJC_NO_TRT__)  ## thread local runtime (default: off)
add_definitions( -D__MULLE_OBJC_FMC__)     ## fast method calls (default: on)
add_definitions( -DMULLE_C11_NO_NOOB_WARNINGS=1)

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/mulle-configuration/cmake)
message(STATUS "CMAKE_MODULE_PATH is ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_BUILD_TYPE is ${CMAKE_BUILD_TYPE}")

if( "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  include( Debug)
else()
  include( Release)
endif()

include( CMakeSourcesAndHeaders.txt)
include( CMakeDependencies.txt)


add_library( mulle_objc_runtime STATIC
${SOURCES}
${DEBUG_SOURCES}
${C_DEPENDENCY_LIBRARIES}
)


set( TARGET_ALL_LOAD_LIBRARIES
$<TARGET_FILE:mulle_objc_runtime>
${C_DEPENDENCY_LIBRARIES}
)

foreach( library ${TARGET_ALL_LOAD_LIBRARIES})
   list( APPEND TARGET_FORCE_ALL_LOAD_LIBRARIES "${FORCE_LOAD_PREFIX}${library}")
endforeach( library)


if( MSVC)
   set( DEF_FILE mulle_objc_runtime_standalone.def)
   set_source_files_properties( ${DEF_FILE} PROPERTIES HEADER_FILE_ONLY TRUE)
   set( CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)
   set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DEF:${DEF_FILE}")
   # /NODEFAULTLIB:MSVCRT")

   add_definitions( -DMULLE_ALLOCATOR_EXTERN_GLOBAL=extern)
   add_definitions( -DMULLE_OBJC_RUNTIME_EXTERN_GLOBAL=extern)

   add_custom_command( OUTPUT ${DEF_FILE}
                       COMMAND mulle-mingw-dumpdef.bat -o "${DEF_FILE}"
                               --name "mulle_objc_runtime_standalone"
                               --prefix "mulle"
                               ${TARGET_ALL_LOAD_LIBRARIES}
                       DEPENDS "${TARGET_ALL_LOAD_LIBRARIES}"
                       VERBATIM)
endif()


add_library( mulle_objc_runtime_standalone SHARED
src/mulle_objc_runtime_standalone.c
${DEF_FILE}
)

add_dependencies( mulle_objc_runtime_standalone mulle_objc_runtime)


target_link_libraries( mulle_objc_runtime_standalone
${BEGIN_ALL_LOAD}
${TARGET_FORCE_ALL_LOAD_LIBRARIES}
${END_ALL_LOAD}
${OS_SPECIFIC_LIBRARIES}
)

add_executable( mulle-objc-uniqueid
src/mulle-objc-uniqueid/main.c
)

target_link_libraries( mulle-objc-uniqueid
mulle_objc_runtime
${C_DEPENDENCY_LIBRARIES}
${OS_SPECIFIC_LIBRARIES}
)

add_dependencies( mulle-objc-uniqueid mulle_objc_runtime)
#set_target_properties( mulle-objc-uniqueid PROPERTIES LINKER_LANGUAGE C)


install( TARGETS mulle_objc_runtime mulle_objc_runtime_standalone DESTINATION "lib")
install( TARGETS mulle-objc-uniqueid DESTINATION "bin")
install( FILES ${PUBLIC_HEADERS} DESTINATION "include/mulle_objc_runtime")
install( FILES CMakeDependencies.txt DESTINATION "include/mulle_objc_runtime")
